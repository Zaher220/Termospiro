#-------------------------------------------------
#
# Project created by QtCreator 2015-08-14T11:59:48
#
#-------------------------------------------------

QT       += core \
            widgets\
            printsupport\
            sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = Termospiro
TEMPLATE = app

SOURCES += main.cpp\
    bdgod.cpp \
    tsanalitics.cpp \
    tscontroller.cpp \
    tscurvebuffer.cpp \
    tsmodel.cpp \
    tsrealcalc.cpp \
    tsrealcalcwrapper.cpp \
    tstempanalitic.cpp \
    volumesolver.cpp \
    inputs/tscombobox.cpp \
    inputs/tslineedit.cpp \
    inputs/tstableview.cpp \
    models/TSExaminations.cpp \
    models/TSPatients.cpp \
    tools/tsvalidationtools.cpp \
    ADCDataReader.cpp \
    rawdataparser.cpp \
    examscontroller.cpp \
    reportprinter.cpp \
    plotterwidjet.cpp \
    qcustomplot.cpp \
    calibrationpkotterwidjet.cpp \
    analyze/signalanalyzer.cpp \
    analyze/volumevaluescalc.cpp

HEADERS  += \
    bdgod.h \
    extremum.h \
    Rtusbapi.h \
    ts_types.h \
    tsanalitics.h \
    tscontroller.h \
    tscurvebuffer.h \
    tsmodel.h \
    tsrealcalc.h \
    tsrealcalcwrapper.h \
    tstempanalitic.h \
    volumesolver.h \
    inputs/tscombobox.h \
    inputs/tslineedit.h \
    inputs/tstableview.h \
    models/TSExaminations.h \
    models/TSPatients.h \
    tools/tsvalidationtools.h \
    ADCDataReader.h \
    rawdataparser.h \
    src/datatypes.h \
    examscontrollerl.h \
    reportprinter.h \
    plotterwidjet.h \
    qcustomplot.h \
    calibrationpkotterwidjet.h \
    analyze/signalanalyzer.h \
    analyze/volumevaluescalc.h \
    analyze/datatypes.h

FORMS    += \
    colibrateDialog.ui \
    patientprofile.ui \
    tsprintview.ui \
    tsprogressdialog.ui \
    tsresultsview.ui \
    tsview.ui \
    tsvolsignalwidget.ui \
    plotterwidjet.ui \
    calibrationpkotterwidjet.ui

RESOURCES += \
    iconResourse.qrc

DISTFILES += \
    db_structure.sql

win32: LIBS += -L$$PWD/lib/ -lRtusbapi

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/
