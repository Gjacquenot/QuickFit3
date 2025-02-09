#
#    Copyright (c) 2015
#	
#	 last modification: $LastChangedDate$  (revision $Rev$)
#
#    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).
#
#    This software is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = lib
CONFIG += plugin


TARGET = curves
DEFINES += TARGETNAME=$$TARGET
DEPENDPATH += ./

include(../../qf3plugins.pri)
include(../../libquickfitwidgets.pri)
DESTDIR = $$QFOUTPUT/plugins

# Input
HEADERS += qfrdrcurve.h \
           qfrdrcurve_data.h \
                   qfrdrcurve_dataeditor.h \
    qfrdrcurveparserfunctions.h

SOURCES += qfrdrcurve.cpp \
           qfrdrcurve_data.cpp \
                   qfrdrcurve_dataeditor.cpp \
    qfrdrcurveparserfunctions.cpp

FORMS = \ 
    qfrdrcurve_dataeditor.ui

RESOURCES += qfrdrcurve.qrc

TRANSLATIONS= ./translations/de.curves.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/ \
               ../../../../../LIB/trunk/ \
               ../../../../../LIB/trunk/qt/

include(../../quickfit3_configqtwidgets.pri)



