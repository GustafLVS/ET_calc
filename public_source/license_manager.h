#pragma once
#include <QString>
#include <QDate>
#include <QObject>

class LicenseManager : public QObject {
    Q_OBJECT
public:
    enum Status { Valid, Expired, Invalid, NotActivated };

    static LicenseManager& instance();

    Status checkStatus();
    Status activate(const QString& key);
    int    daysRemaining();
    QDate  issueDate() const { return m_issueDate; }

private:
    explicit LicenseManager(QObject* parent = nullptr);

    bool  loadKey(QString& key) const;
    void  saveKey(const QString& key);
    void  setBurned(const QString& key);
    bool  isBurned(const QString& key) const;

    static bool    parseKey(const QString& raw, QDate& outDate, quint16& outSerial);
    static bool    isWhitelistedSerial(quint16 serial);
    static quint32 encodeKey(quint16 day, quint16 serial);
    static quint32 keyChecksum(quint32 encodedPacked);

    QDate m_issueDate;
};
