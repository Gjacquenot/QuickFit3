/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center

    last modification: $LastChangedDate: 2015-05-02 18:18:13 +0200 (Sa, 02 Mai 2015) $  (revision $Rev: 3946 $)

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

#include "qffitfunctionstirfcsdiffflowe2.h"
#include <cmath>
#include "qftools.h"
#include "imfcstools.h"
#define sqr(x) qfSqr(x)
#define cube(x) qfCube(x)
#define pow4(x) qfPow4(x)
#define NAVOGADRO QF_NAVOGADRO

QFFitFunctionsTIRFCSDiffFlowE2::QFFitFunctionsTIRFCSDiffFlowE2() {
    //           type,         id,                        name,                                                    label,                      unit,          unitlabel,               fit,       userEditable, userRangeEditable, displayError,                initialValue, minValue, maxValue, inc, absMin, absMax
    addParameter(IntCombo,     "n_components",            "number of diffusing components",                        "components",               "",            "",                      false,     true,         false,             QFFitFunction::NoError,      false, 1,            1,        3,        1,   1,      2);
    #define FCSSDiff_n_components 0
    addParameter(FloatNumber,  "n_particle",              "particle number N",                                     "N",                        "",            "",                      true,      true,         true,              QFFitFunction::DisplayError, false, 1,            1e-10,    1e5,      1,   0);
    #define FCCSDiff_n_particle 1
    addParameter(FloatNumber,  "1n_particle",             "1/particle number N",                                   "1/N",                      "",            "",                      false,     false,        false,             QFFitFunction::DisplayError, false, 0.01,         1e-10,    1e5,      0.1, 0);
    #define FCCSDiff_1n_particle 2
    addParameter(FloatNumber,  "diff_rho1",               "fraction of first component",                           "&rho;<sub>1</sub>",        "",            "",                      false,     false,        false,             QFFitFunction::DisplayError, false, 0.5,          0,        0.99999,  0.1, 0,      1);
    #define FCSSDiff_diff_rho1 3
    addParameter(FloatNumber,  "diff_coeff1",             "diffusion coefficient of species 1",                    "D<sub>1</sub>",            "micron^2/s", "&mu;m<sup>2</sup>/s",    true,      true,         true,              QFFitFunction::DisplayError, false, 10,           1e-10,    1e50,     1    );
    #define FCSSDiff_diff_coeff1 4
    addParameter(FloatNumber,  "diff_rho2",               "fraction of second component",                          "&rho;<sub>2</sub>",        "",            "",                      true,      true,         true,              QFFitFunction::DisplayError, false, 0.5,          0,        0.99999,  0.1, 0,      1  );
    #define FCSSDiff_diff_rho2 5
    addParameter(FloatNumber,  "diff_coeff2",             "diffusion coefficient of species 2",                    "D<sub>2</sub>",            "micron^2/s", "&mu;m<sup>2</sup>/s",    true,      true,         true,              QFFitFunction::DisplayError, false, 10,           1e-10,    1e50,     1    );
    #define FCSSDiff_diff_coeff2 6
    addParameter(FloatNumber,  "diff_rho3",               "fraction of third component",                           "&rho;<sub>3</sub>",        "",            "",                      true,      true,         true,              QFFitFunction::DisplayError, false, 0.5,          0,        0.99999,  0.1, 0,      1  );
    #define FCSSDiff_diff_rho3 7
    addParameter(FloatNumber,  "diff_coeff3",             "diffusion coefficient of species 3",                    "D<sub>3</sub>",            "micron^2/s", "&mu;m<sup>2</sup>/s",    true,      true,         true,              QFFitFunction::DisplayError, false, 10,           1e-10,    1e50,     1    );
    #define FCSSDiff_diff_coeff3 8

    addParameter(FloatNumber,  "vflowx",                  "flow speed in x-direction",                  "v<sub>flow, x</sub>",      "micron/s", "&mu;m/s",                  true,    true,        true,                 QFFitFunction::EditError, false, 100,          -1e50,        1e50,     1    );
    #define FCCSDiff_vflowx 9
    addParameter(FloatNumber,  "vflowy",                  "flow speed in y-direction",                  "v<sub>flow, y</sub>",      "micron/s", "&mu;m/s",                  true,    true,        true,                 QFFitFunction::EditError, false, 0,          -1e50,        1e50,     1    );
    #define FCCSDiff_vflowy 10
    addParameter(FloatNumber,  "vflow",                   "flow speed",                               "v<sub>flow</sub>",         "micron/s", "&mu;m/s",                  false,    false,        false,              QFFitFunction::DisplayError, false, 500,          0,        1e50,     1    );
    #define FCCSDiff_vflow 11
    addParameter(FloatNumber,  "vflow_angle",             "angle of the flow speed",                               "&alpha;<sub>flow</sub>",         QLatin1String("\xB0"), QLatin1String("\xB0"),                  false,    false,        false,              QFFitFunction::DisplayError, false, 500,          0,        1e50,     1    );
    #define FCCSDiff_alphaflow 12
    addParameter(FloatNumber,  "vflow_angle_centered",             "centered angle of the flow speed",                               "&alpha;<sub>flow</sub>'",         QLatin1String("\xB0"), QLatin1String("\xB0"),                  false,    false,        false,              QFFitFunction::DisplayError, false, 0,          0,        1e50,     1    );
    #define FCCSDiff_alphaflow_centered 13
    addParameter(FloatNumber,  "offset",                  "correlation offset",                                    "G<sub>&infin;</sub>",      "",           "",                       true,      true,         true,              QFFitFunction::DisplayError, true, 0,            -10,      10,       0.1  );
    #define FCCSDiff_offset 14

    addParameter(FloatNumber,  "offset",                  "correlation offset",                                    "G<sub>&infin;</sub>",      "",           "",                       true,      true,         true,              QFFitFunction::DisplayError, true, 0,            -10,      10,       0.1  );
    #define FCSSDiff_offset 15
    addParameter(FloatNumber,  "focus_width",             "PSF: lateral radius (1/e^2 radius)",                     "w<sub>x,y</sub>",    "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 600,          0,        1e4,      10    );
    #define FCSSDiff_focus_width 16
    addParameter(FloatNumber,  "pixel_width",             "pixel width",                                           "a",                        "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 400,          0,        1e4,      10    );
    #define FCSSDiff_pixel_width 17
    addParameter(FloatNumber,  "effective_area",            "focus: effective area",                               "A<sub>eff</sub>",          "micron^2",         "&mu;m<sup>2</sup>",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0.5,          0,        1e50,     1    );
    #define FCSSDiff_focus_area 18
    addParameter(FloatNumber,  "concentration",           "particle concentration in focus",                       "C<sub>all</sub>",          "particles/micron^2",         "particles/&mu;m<sup>2</sup>",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0.5,          0,        1e50,     1    );
    #define FCSSDiff_concentration 19
    addParameter(FloatNumber,  "count_rate",              "count rate during measurement",                         "count rate",               "Hz",         "Hz",                     false,    true,         false,              QFFitFunction::EditError,    false, 0,            0,        1e50,     1    );
    #define FCSSDiff_count_rate 20
    addParameter(FloatNumber,  "background",              "background count rate during measurement",              "background",               "Hz",         "Hz",                     false,    true,         false,              QFFitFunction::EditError  ,  false, 0,            0,        1e50,     1    );
    #define FCSSDiff_background 21
    addParameter(FloatNumber,  "cpm",                     "photon counts per molecule",                            "cnt/molec",                "Hz",         "Hz",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0,            0,        1e50,     1    );
    #define FCSSDiff_cpm 22
}

double QFFitFunctionsTIRFCSDiffFlowE2::evaluate(double t, const double* data) const {
    const int comp=data[FCSSDiff_n_components];
    const double N=data[FCCSDiff_n_particle];
    const double D1=data[FCSSDiff_diff_coeff1];
    const double D2=data[FCSSDiff_diff_coeff2];
    const double D3=data[FCSSDiff_diff_coeff3];
    const double r2=data[FCSSDiff_diff_rho2];
    const double r3=data[FCSSDiff_diff_rho3];

    double r1=1;
    if (comp==2) {
        r1=1.0-r2;
    }
    if (comp==3) {
        r1=1.0-r2-r3;
    }
    const double dx=0;//data[FCCSDiff_focus_distancex]/1000.0;
    const double dy=0;//data[FCCSDiff_focus_distancey]/1000.0;
    const double vx=data[FCCSDiff_vflowx];
    const double vy=data[FCCSDiff_vflowy];


    const double wxy=data[FCSSDiff_focus_width]/1.0e3;
    const double a=data[FCSSDiff_pixel_width]/1.0e3;
    const double offset=data[FCSSDiff_offset];
    const double background=data[FCSSDiff_background];
    const double cr=data[FCSSDiff_count_rate];
    double backfactor=sqr(cr-background)/sqr(cr);
    if (fabs(cr)<1e-15 || fabs(background)<1e-10) backfactor=1;



    double cfac=r1*QFFitFunctionsTIRFCCSFWDiff2ColorCCF_corrfactor_2Dxy(a, dx-vx*t, dy-vy*t, D1, t, wxy, wxy);
    if (comp>1)cfac=cfac+r2*QFFitFunctionsTIRFCCSFWDiff2ColorCCF_corrfactor_2Dxy(a, dx-vx*t, dy-vy*t, D2, t, wxy, wxy);
    if (comp>2)cfac=cfac+r2*QFFitFunctionsTIRFCCSFWDiff2ColorCCF_corrfactor_2Dxy(a, dx-vx*t, dy-vy*t, D3, t, wxy, wxy);

    const double Veff=TIRFCS_newAeff(a, wxy);
    const double pre=1.0;
    return offset+pre/(N/Veff)*cfac*backfactor;
}



void QFFitFunctionsTIRFCSDiffFlowE2::calcParameter(double* data, double* error) const {
    double N=data[FCSSDiff_n_components];
    double eN=0;
    //double D1=data[FCSSDiff_diff_coeff1];
    double eD1=0;
    double wxy=data[FCSSDiff_focus_width]/1.0e3;
    double ewxy=0;
    double a=data[FCSSDiff_pixel_width]/1.0e3;
    double ea=0;
    //double offset=data[FCSSDiff_offset];
    double eoffset=0;

    int comp=data[FCSSDiff_n_components];
    double rho2=data[FCSSDiff_diff_rho2];
    double erho2=0;
    double rho3=data[FCSSDiff_diff_rho3];
    double erho3=0;


//    double dx=data[FCCSDiff_focus_distancex];
//    double edx=0;
//    double dy=data[FCCSDiff_focus_distancey];
//    double edy=0;
    double vx=data[FCCSDiff_vflowx];
    double evx=0;
    double vy=data[FCCSDiff_vflowy];
    double evy=0;

    double cps=data[FCSSDiff_count_rate];
    double ecps=0;
    double ecpm=0;
    double background=data[FCSSDiff_background];
    double ebackground=0;

    if (error) {
        erho2=error[FCSSDiff_diff_rho2];
        erho3=error[FCSSDiff_diff_rho3];

        eN=error[FCSSDiff_n_components];
        eD1=error[FCSSDiff_diff_coeff1];
        ewxy=error[FCSSDiff_focus_width]/1.0e3;
        ea=error[FCSSDiff_pixel_width]/1.0e3;
        eoffset=error[FCSSDiff_offset];

        ecps=error[FCSSDiff_count_rate];
        ecpm=error[FCSSDiff_cpm];
        ebackground=error[FCSSDiff_background];
//        edx=error[FCCSDiff_focus_distancex];
//        edy=error[FCCSDiff_focus_distancey];
        evx=error[FCCSDiff_vflowx];
        evy=error[FCCSDiff_vflowy];
    }

    double rho1=1;
    double erho1=0;
    if (comp==2) {
        if (rho2>1.0) rho2=1.0;
        if (rho2<0.0) rho2=0.0;
        if (rho2>1.0) {
            rho1=0;
            rho2=0.5;
        }
        rho1=1.0-rho2;
        erho1=erho2;
    }
    if (comp==3) {
        if (rho2>1.0) rho2=1.0;
        if (rho2<0.0) rho2=0.0;
        if (rho3>1.0) rho3=1.0;
        if (rho3<0.0) rho3=0.0;
        if (rho2+rho3>1.0) {
            rho1=0;
            rho2=rho3=0.5;
        }
        rho1=1.0-rho2-rho3;
        erho1=sqrt(erho2*erho2+erho3*erho3);
    }

    data[FCSSDiff_diff_rho1]=rho1;
    data[FCSSDiff_diff_rho2]=rho2;
    data[FCSSDiff_diff_rho3]=rho3;
    if (error) {
        error[FCSSDiff_diff_rho1]=erho1;
        error[FCSSDiff_diff_rho2]=erho2;
        error[FCSSDiff_diff_rho3]=erho3;
    }

    // calculate dxy=sqrt(dx^2+dy^2)
//    data[FCCSDiff_focus_distance]=sqrt(dx*dx+dy*dy);
//    if (error) {
//        error[FCCSDiff_focus_distance]=sqrt(sqr(edx*2.0*dx)/(dx*dx+dy*dy)+sqr(edy*2.0*dy)/(dx*dx+dy*dy));
//        if (!QFFloatIsOK(error[FCCSDiff_focus_distance])) error[FCCSDiff_focus_distance]=0;
//    }

    // calculate vxy=sqrt(vx^2+vy^2)
    data[FCCSDiff_vflow]=sqrt(vx*vx+vy*vy);
    if (error) {
        error[FCCSDiff_vflow]=sqrt(sqr(evx*2.0*vx)/(vx*vx+vy*vy)+sqr(evy*2.0*vy)/(vx*vx+vy*vy));
        if (!QFFloatIsOK(error[FCCSDiff_vflow])) error[FCCSDiff_vflow]=0;
    }


    // calculate valpha=(atan(vy/vx)+pi)/pi*180
    double an;
    data[FCCSDiff_alphaflow]=an=(atan2(vy,vx)+M_PI)/M_PI*180.0;
    if (error) {
        error[FCCSDiff_alphaflow]=0;
        if (!QFFloatIsOK(error[FCCSDiff_alphaflow])) error[FCCSDiff_alphaflow]=0;
    }

    // calculate centered valpha=(atan(vy/vx)+pi)/pi*180
    data[FCCSDiff_alphaflow_centered]=an;
    if (an>180.0) data[FCCSDiff_alphaflow_centered]=an-360.0;
    if (error) {
        error[FCCSDiff_alphaflow_centered]=error[FCCSDiff_alphaflow];
    }

    // calculate 1/N
    data[FCCSDiff_1n_particle]=1.0/N;
    if (error) error[FCCSDiff_1n_particle]=fabs(eN/N/N);

    // calculate Veff
    data[FCSSDiff_focus_area]=TIRFCS_newAeff(a, wxy);;
    if (error) error[FCSSDiff_focus_area]=TIRFCS_newAeffError(a, ea, wxy, ewxy);

    // calculate C = N / Veff
    if (data[FCSSDiff_focus_area]!=0) {
        data[FCSSDiff_concentration]=N/data[FCSSDiff_focus_area];
    } else {
        data[FCSSDiff_concentration]=0;
    }
    if (data[FCSSDiff_focus_area]!=0 && error) {
        error[FCSSDiff_concentration]=sqrt(qfSqr(eN/data[FCSSDiff_focus_area])+qfSqr(error[FCSSDiff_focus_area]*N/qfSqr(data[FCSSDiff_focus_area])));//N/data[FCSSDiff_focus_volume]
    }


    // calculate CPM = (CPS-background)/N
    data[FCSSDiff_cpm]=(cps-background)/N;
    error[FCSSDiff_cpm]=sqrt(sqr(ecps/N)+sqr(ebackground/N)+sqr(eN*(cps-background)/sqr(N)));

}

bool QFFitFunctionsTIRFCSDiffFlowE2::isParameterVisible(int parameter, const double* data) const {
    int comp=data[FCSSDiff_n_components];
    switch(parameter) {
        case FCSSDiff_diff_rho1:  return comp>1;
        case FCSSDiff_diff_coeff1: return comp>0;
        case FCSSDiff_diff_rho2: case FCSSDiff_diff_coeff2: return comp>1;
        case FCSSDiff_diff_rho3: case FCSSDiff_diff_coeff3: return comp>2;
    }
    return true;
}

unsigned int QFFitFunctionsTIRFCSDiffFlowE2::getAdditionalPlotCount(const double* params) const {

    Q_UNUSED(params);
    return 0;
}

QString QFFitFunctionsTIRFCSDiffFlowE2::transformParametersForAdditionalPlot(int plot, double* params) const {


    Q_UNUSED(plot);
    Q_UNUSED(params);
    return "";
}
