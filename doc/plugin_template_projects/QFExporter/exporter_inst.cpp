/*
Copyright (c) 2014
	
	last modification: $LastChangedDate: 2014-06-24 16:05:58 +0200 (Di, 24 Jun 2014) $  (revision $Rev: 3289 $)

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


#include "exporter_inst.h"

QStringList QFFitAlgorithmInst::getIDs() const {
    QStringList res;
    res<<"ff_id";
    return res;
}

QFExporter* QFFitAlgorithmInst::createExporter(QString id) const  {
    if (id=="ff_id") {
        return new QFFitAlgorithmInst_F1();
    } 
    return NULL;
}

Q_EXPORT_PLUGIN2(target_id, QFFitAlgorithmInst)
