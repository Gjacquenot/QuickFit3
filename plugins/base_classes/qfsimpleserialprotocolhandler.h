/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg
	
	

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QFSIMPLESERIALPROTOCOLHANDLER_H
#define QFSIMPLESERIALPROTOCOLHANDLER_H
#include "qfserialconnection.h"
#include <QString>
#include "qfpluginservices.h"

class QFSimpleSerialProtocolHandler {
    public:
        /** \brief class constructor
         *
         *  \param com com port to use
         *  \param name name of the device (for error messages)
         */
        QFSimpleSerialProtocolHandler(QFSerialConnection* com, QString name, const QString& sendTerminationString=QString("\n"), const QString& receiveReadUntilString=QString("\n"));

        void setLogging(QFPluginLogService* log, QString LOG_PREFIX);

        bool checkComConnected();

        /** \brief send a command to the Mercury controller (this automatically adds a command terminating character (line feed \c 0x0A) */
        void sendCommand(QString command);

        /** \brief send a command to the Mercury controller (this automatically adds a command terminating character (line feed \c 0x0A)
         *         and returns the result (the standard finishing sequence CR LF ETX will be cut from the string) */
        QString queryCommand(QString command);

        void checkComError();

        bool hasErrorOccured();
        QString getLastError();
        void setSendTerminationString(const QString& str);
        void setReceiveUntilString(const QString& str);
    protected:
        QFSerialConnection* com;
        QFPluginLogService* log;
        QString LOG_PREFIX;
        QString name;
        QString sendTerminationString;
        QString receiveReadUntilString;

};

#endif // QFSIMPLESERIALPROTOCOLHANDLER_H

