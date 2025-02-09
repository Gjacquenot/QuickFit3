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

#include "qfpfitfunctionsfcsdistribution.h"
#include "qffitfunctionfcsdistributiongaussianD.h"
#include "qffitfunctionfcsdistributionloggaussian.h"
#include "qffitfunctionfcsdistributiongaussian.h"
#include "qffitfunctionfcsdistributiongaussianint.h"
#include "qffitfunctionfcsdistributionloggaussianint.h"
#include "qffitfunctionfcsdistributiongaussianDint.h"

QStringList QFPFitFunctionsFCSDistribution::getIDs() const {
    QStringList res;
    res<<"fcs_dist_norm"<<"fcs_dist_norm_d"<<"fcs_dist_lognorm"<<"fcs_dist_int_lognorm"<<"fcs_dist_int_norm"<<"fcs_dist_int_norm_d";
    return res;
}

QFFitFunction* QFPFitFunctionsFCSDistribution::get(const QString &id) const  {
    if (id=="fcs_dist_lognorm") {
        return new QFFitFunctionFCSDistributionLogGaussian();
    } else if (id=="fcs_dist_norm") {
            return new QFFitFunctionFCSDistributionGaussian();
    } else if (id=="fcs_dist_int_norm") {
            return new QFFitFunctionFCSDistributionIntGaussian();
    } else if (id=="fcs_dist_int_lognorm") {
            return new QFFitFunctionFCSDistributionIntLogGaussian();
    } else if (id=="fcs_dist_norm_d") {
        return new QFFitFunctionFCSDistributionGaussianD();
    } else if (id=="fcs_dist_int_norm_d") {
        return new QFFitFunctionFCSDistributionDIntGaussian();
    }

    return NULL;
}

Q_EXPORT_PLUGIN2(fitfunctions_fcsdistribution, QFPFitFunctionsFCSDistribution)
