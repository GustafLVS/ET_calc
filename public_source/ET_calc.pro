QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = ET_calc
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    formula_engine.cpp \
    formula_library.cpp \
    circuit_widget.cpp \
    custom_formula_dialog.cpp \
    formula_input_widget.cpp \
    temp_coeff_dialog.cpp \
    license_manager.cpp \
    license_dialog.cpp

HEADERS += \
    mainwindow.h \
    formula_engine.h \
    formula_library.h \
    circuit_widget.h \
    custom_formula_dialog.h \
    formula_input_widget.h \
    temp_coeff_dialog.h \
    license_manager.h \
    license_dialog.h

RESOURCES += resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
