

QMAKE_CXXFLAGS += -fexceptions

HEADERS += $$PWD/jkqtpbaseplotter.h \
           $$PWD/jkqtpdatastorage.h \
           $$PWD/jkqtpelements.h \
           $$PWD/jkqtmathtext.h \
           $$PWD/jkqtpbaseelements.h \
           $$PWD/jkqtplotter.h \
           $$PWD/jkqtptools.h \
           $$PWD/jkqttools.h \
           $$PWD/jkqtpimageelements.h \
           $$PWD/jkqtpimagetools.h \
           $$PWD/jkqtpparsedfunctionelements.h \
           $$PWD/jkqtpoverlayelements.h \
           $$PWD/jkqtpgeoelements.h \
           $$PWD/../tools.h \
           $$PWD/../jkmathparser.h


SOURCES += $$PWD/jkqtpbaseplotter.cpp \
           $$PWD/jkqtpdatastorage.cpp \
           $$PWD/jkqtpelements.cpp \
           $$PWD/jkqtmathtext.cpp \
           $$PWD/jkqtpbaseelements.cpp \
           $$PWD/jkqtplotter.cpp \
           $$PWD/jkqtptools.cpp \
           $$PWD/jkqttools.cpp  \
           $$PWD/jkqtpimageelements.cpp  \
           $$PWD/jkqtpimagetools.cpp  \
           $$PWD/jkqtpparsedfunctionelements.cpp  \
           $$PWD/jkqtpoverlayelements.cpp  \
           $$PWD/jkqtpgeoelements.cpp  \
           $$PWD/../tools.cpp \
           $$PWD/../jkmathparser.cpp


RESOURCES += $$PWD/jkqtpbaseplotter.qrc

INCLUDEPATH += $$PWD \
               $$PWD/../

QT += gui

QT += core gui svg xml
win32:LIBS += -lgdi32
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport


