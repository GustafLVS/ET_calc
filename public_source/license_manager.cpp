#include "license_manager.h"
#include <QSettings>
#include <QDate>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

// ─────────────────────────────────────────────────────────────────────────────
//  Embedded secrets
// ─────────────────────────────────────────────────────────────────────────────
static const quint32 _P0 = 0xF2A40000u;
static const quint32 _P1 = 0x0000C81Eu;
static const quint32 _P2 = 0x9E3779B9u;
static const quint32 _P3 = 0x7B000000u;
static const quint32 _P4 = 0x003D5960u;
static const quint32 _P5 = 0xA1B20000u;
static const quint32 _P6 = 0x0000C3D4u;

static inline quint32 SA() { return _P0 | _P1; }
static inline quint32 SB() { return _P2;        }
static inline quint32 SC() { return _P3 | _P4;  }
static inline quint32 RX() { return _P5 | _P6;  }

// ─────────────────────────────────────────────────────────────────────────────
//  Whitelist — one-way hashes of valid serials (cannot be reversed)
// ─────────────────────────────────────────────────────────────────────────────
static const quint32 SERIAL_HASHES[] = {
    0x707C1B03u, 0x25AEC385u, 0xDE400A53u, 0x4F97F6F7u, 0xA7D05D46u,
    0xAE681708u, 0x4841C1FEu, 0x0CB9B0FFu, 0xF6328E97u, 0x12A459D7u,
    0xD5CD9B93u, 0xCA11C063u, 0x3EC1372Cu, 0xDF6BDDE7u, 0x524A232Au,
    0x2DD954B9u, 0xBF8746E7u, 0x7911C646u, 0x5530C450u, 0xBF336E53u,
    0xEB293C93u, 0x2A8EF11Fu, 0x40741E5Fu, 0x7F04950Cu, 0xB20E7F92u,
    0xBA9D13FDu, 0xF98C574Bu, 0xCBD55AB5u, 0x3245663Eu, 0xC0136540u,
    0x7A92F01Cu, 0xB3FCE28Bu, 0x0D40CF64u, 0x534D7A91u, 0x982CD0BEu,
    0x9DD00355u, 0x7E41664Fu, 0x9FAD421Fu, 0xE3C901D4u, 0x6C345B98u,
    0xB30B326Bu, 0x22F407D1u, 0xDA42D5FAu, 0x976C2F51u, 0xAD0F5E82u,
    0x6AB96B5Au, 0xADE0048Bu, 0x76234CA3u, 0xBA5CC33Cu, 0xC836BE24u
};
static const int N_SERIALS = static_cast<int>(sizeof(SERIAL_HASHES)/sizeof(SERIAL_HASHES[0]));

static const QDate EPOCH(2024, 1, 1);
static const int   TRIAL_DAYS = 5;
static const char  REG_PATH[] = "HKEY_CURRENT_USER\\Software\\ETCalc\\Config";

// ─────────────────────────────────────────────────────────────────────────────
//  Key cryptography
// ─────────────────────────────────────────────────────────────────────────────
quint32 LicenseManager::encodeKey(quint16 day, quint16 serial)
{
    quint32 packed = static_cast<quint32>(day) | (static_cast<quint32>(serial) << 16);
    return packed ^ SA();
}

quint32 LicenseManager::keyChecksum(quint32 enc)
{
    quint32 h = enc;
    h ^= (h >> 16);
    h *= 0x45d9f3bu;
    h ^= (h >> 16);
    h *= SB();
    h ^= (h >> 16);
    h ^= SC();
    return h;
}

static quint32 hashSerial(quint16 serial)
{
    quint32 h = serial;
    h ^= (_P0 | _P1);
    h  = (h * 0x45d9f3bu) & 0xFFFFFFFFu;
    h ^= (h >> 16);
    h  = (h * _P2) & 0xFFFFFFFFu;
    h ^= (_P3 | _P4);
    h ^= (h >> 16);
    return h;
}

bool LicenseManager::isWhitelistedSerial(quint16 serial)
{
    const quint32 h = hashSerial(serial);
    for (int i = 0; i < N_SERIALS; ++i)
        if (SERIAL_HASHES[i] == h) return true;
    return false;
}

/*static*/ bool LicenseManager::parseKey(const QString& raw, QDate& outDate, quint16& outSerial)
{
    QString s = raw.trimmed().toUpper().remove(' ');
    if (s.length() == 16) s.insert(8, '-');

    const QStringList parts = s.split('-');
    if (parts.size() != 2 || parts[0].length() != 8 || parts[1].length() != 8)
        return false;

    bool ok1 = false, ok2 = false;
    const quint32 enc = parts[0].toUInt(&ok1, 16);
    const quint32 chk = parts[1].toUInt(&ok2, 16);
    if (!ok1 || !ok2)             return false;
    if (keyChecksum(enc) != chk)  return false;

    const quint32 packed = enc ^ SA();
    const quint16 day    = static_cast<quint16>(packed & 0xFFFF);
    const quint16 serial = static_cast<quint16>((packed >> 16) & 0xFFFF);

    if (!isWhitelistedSerial(serial)) return false;

    const QDate d = EPOCH.addDays(day);
    if (!d.isValid() || d < EPOCH || d > QDate::currentDate().addDays(1))
        return false;

    outDate   = d;
    outSerial = serial;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Registry helpers
// ─────────────────────────────────────────────────────────────────────────────
static QByteArray xorBuf(const QByteArray& in)
{
    const quint32 rx = (_P5 | _P6);
    QByteArray out = in;
    for (int i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(
            static_cast<quint8>(out[i]) ^ static_cast<quint8>((rx >> ((i%4)*8)) & 0xFF));
    return out;
}

bool LicenseManager::loadKey(QString& key) const
{
    const QSettings s(REG_PATH, QSettings::NativeFormat);
    if (!s.contains("K")) return false;
    key = QString::fromLatin1(xorBuf(s.value("K").toByteArray()));
    return !key.isEmpty();
}

void LicenseManager::saveKey(const QString& key)
{
    QSettings s(REG_PATH, QSettings::NativeFormat);
    s.setValue("K", xorBuf(key.toLatin1()));
    s.remove("B");
    s.remove("BK");
}

void LicenseManager::setBurned(const QString& key)
{
    QSettings s(REG_PATH, QSettings::NativeFormat);
    s.setValue("B",  RX() ^ 0xDEADBEEFu);
    s.setValue("BK", xorBuf(key.trimmed().toUpper().toLatin1()));

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QFile mf(dir + "/.cfg_state");
    if (mf.open(QIODevice::WriteOnly)) { mf.write("EXPIRED"); mf.close(); }
}

bool LicenseManager::isBurned(const QString& key) const
{
    const QSettings s(REG_PATH, QSettings::NativeFormat);
    if (s.value("B", 0u).toUInt() != (RX() ^ 0xDEADBEEFu)) return false;
    return QString::fromLatin1(xorBuf(s.value("BK").toByteArray()))
           == key.trimmed().toUpper();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Singleton & public API
// ─────────────────────────────────────────────────────────────────────────────
LicenseManager& LicenseManager::instance()
{
    static LicenseManager inst;
    return inst;
}

LicenseManager::LicenseManager(QObject* parent) : QObject(parent) {}

LicenseManager::Status LicenseManager::checkStatus()
{
    QString key;
    if (!loadKey(key)) return NotActivated;

    QDate   issue;
    quint16 serial = 0;
    if (!parseKey(key, issue, serial)) return Invalid;
    if (isBurned(key))                 return Expired;

    const int elapsed = static_cast<int>(issue.daysTo(QDate::currentDate()));
    if (elapsed < 0)          return Invalid;
    if (elapsed > TRIAL_DAYS) { setBurned(key); return Expired; }

    m_issueDate = issue;
    return Valid;
}

LicenseManager::Status LicenseManager::activate(const QString& key)
{
    QString norm = key.trimmed().toUpper().remove(' ');
    if (norm.length() == 16) norm.insert(8, '-');

    QDate   issue;
    quint16 serial = 0;
    if (!parseKey(norm, issue, serial)) return Invalid;
    if (isBurned(norm))                 return Expired;

    const int elapsed = static_cast<int>(issue.daysTo(QDate::currentDate()));
    if (elapsed < 0)          return Invalid;
    if (elapsed > TRIAL_DAYS) { setBurned(norm); return Expired; }

    saveKey(norm);
    m_issueDate = issue;
    return Valid;
}

int LicenseManager::daysRemaining()
{
    QString key;
    if (!loadKey(key)) return 0;
    QDate   issue;
    quint16 serial = 0;
    if (!parseKey(key, issue, serial)) return 0;
    return TRIAL_DAYS - static_cast<int>(issue.daysTo(QDate::currentDate()));
}
