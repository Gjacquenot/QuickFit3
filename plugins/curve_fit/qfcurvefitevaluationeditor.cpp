/*
Copyright (c) 2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

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

#include "qfcurvefitevaluationeditor.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "qfrawdatarecord.h"
#include "qfevaluationitem.h"
#include <iostream>
#include <cfloat>
#include "qfcurvefitevaluation.h"
#include "cpptools.h"
#include "statistics_tools.h"
#include <QThread>
#include "qmoretextobject.h"
#include "qmodernprogresswidget.h"
#include "qffitfunctionmanager.h"
#include "qffitalgorithmmanager.h"
#include "qffitfunctionplottools.h"






QFCurveFitEvaluationEditor::QFCurveFitEvaluationEditor(QFPluginServices* services, QFEvaluationPropertyEditor* propEditor, QWidget* parent):
    QFFitResultsByIndexEvaluationEditorWithWidgets("curvefitevaleditor/", propEditor, services, parent, true, true, tr("curve"), true, true)
{

    createWidgets();
    btnFirstRun->setText(tr("first curve"));
    if (spinRun) spinRun->setSpecialValueText(QString());
    setGuessingEnabled(true);
    actXLogScale->setVisible(true);
    actYLogScale->setVisible(true);
    chkXLogScale->setText(tr("x-log  "));
    chkYLogScale->setText(tr("y-log  "));

}

QFCurveFitEvaluationEditor::~QFCurveFitEvaluationEditor()
{
    //dtor
}


void QFCurveFitEvaluationEditor::createWidgets() {
    cmbWeights=new QFCurveWeightingCombobox(this);
//    cmbWeights->setEditable(false);
//    cmbWeights->addItem(tr("none"));
//    cmbWeights->addItem(tr("by error"));
    cmbWeights->setMaximumWidth(150);
    cmbWeights->setMinimumWidth(150);
    QLabel* l=new QLabel(tr("&Weight Model: "), this);
    l->setBuddy(cmbWeights);
    layAlgorithm->addSpacing(32);
    layAlgorithm->addWidget(l);
    layAlgorithm->addWidget(cmbWeights);
    layAlgorithm->addStretch();

    widFitErrorEstimate=new QFFitAlgorithmErrorEstimateModeWidget(this);
    l=new QLabel(tr("&Error Estimation: "), this);
    l->setBuddy(widFitErrorEstimate);
    layAfterAlgorithm->addSpacing(32);
    layAfterAlgorithm->addWidget(l);
    layAfterAlgorithm->addWidget(widFitErrorEstimate);
    layAfterAlgorithm->addStretch();



    spinRepeats=new QSpinBox(this);
    spinRepeats->setRange(1, 100);
    spinRepeats->setValue(1);
    spinRepeats->setToolTip(tr("repeats the fit the given number of times for every click of fit. The output of the first curve is used as initial parameters for the second, ..."));
    int row=layButtons->rowCount();
    layButtons->addWidget(new QLabel(tr("repeats:"), this), row, 0);
    layButtons->addWidget(spinRepeats, row, 1);
    connect(spinRepeats, SIGNAL(valueChanged(int)), this, SLOT(repeatsChanged(int)));


    menuTools=propertyEditor->addOrFindMenu("&Tools", 0);
    //menuTools->addAction(actOverlayPlot);

    splitPlot->setStretchFactor(0,9);
    splitPlot->setStretchFactor(1,3);
    splitPlot->setStretchFactor(2,4);
    splitModel->setStretchFactor(0,1);
    splitModel->setStretchFactor(1,0);

}


void QFCurveFitEvaluationEditor::connectWidgets(QFEvaluationItem* current, QFEvaluationItem* old) {
    QFFitResultsByIndexEvaluationEditorWithWidgets::connectDefaultWidgets(current, old, false);

    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);

    if (old!=NULL) {
        disconnect(cmbWeights, SIGNAL(currentIndexChanged(int)), this, SLOT(weightsChanged(int)));
        disconnect(widFitErrorEstimate, SIGNAL(parametersChanged()), this, SLOT(errorEstimateModeChanged()));
    }



    if (eval) {

        dataEventsEnabled=false;
        cmbWeights->setCurrentIndex(current->getProperty("weights", 0).toInt());
        widFitErrorEstimate->readSettings(eval);
        spinRepeats->setValue(current->getProperty("FIT_REPEATS", 1).toInt());
        dataEventsEnabled=true;
        if (current->propertyExists("PRESET_FIT_MODEL")) {
            cmbModel->setCurrentFitFunction(current->getProperty("PRESET_FIT_MODEL", eval->getFitFunctionID()).toString());
            eval->setFitFunction(current->getProperty("PRESET_FIT_MODEL", eval->getFitFunctionID()).toString());
            current->deleteProperty("PRESET_FIT_MODEL");
        }
    }

    connect(cmbWeights, SIGNAL(currentIndexChanged(int)), this, SLOT(weightsChanged(int)));
    connect(widFitErrorEstimate, SIGNAL(parametersChanged()), this, SLOT(errorEstimateModeChanged()));

    replotData();
    displayModel(true);
}


void QFCurveFitEvaluationEditor::readSettings() {
    QFFitResultsByIndexEvaluationEditorWithWidgets::readSettings();
}

void QFCurveFitEvaluationEditor::writeSettings() {
    QFFitResultsByIndexEvaluationEditorWithWidgets::writeSettings();
    settings->getQSettings()->setValue(QString("fitevaleditor_%1%2/weights").arg(current->getType()).arg(current->getID()), cmbWeights->currentIndex());

}


void QFCurveFitEvaluationEditor::fillRunCombo(QFFitResultsByIndexEvaluation *cur, QFRawDataRecord *rdr)
{
    if (!cmbRun) return;
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(cur);
    if (eval) {
        int imin=eval->getIndexMin(rdr);
        int imax=eval->getIndexMax(rdr);
        cmbRun->clear();
        for (int i=imin; i<=imax; i++) {
            cmbRun->addItem(QString("#%1: %2").arg(i).arg(eval->getIndexName(rdr, i)), i);
        }
    }
}

QString QFCurveFitEvaluationEditor::getPlotXLabel() const
{
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (eval) {
        QFRawDataRecord* data=current->getHighlightedRecord();
        if (data) {
            return data->curvesGetXLabel(eval->getCurrentIndex());
        }
    }
    return "X";
}

QString QFCurveFitEvaluationEditor::getPlotYLabel() const
{
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (eval) {
        QFRawDataRecord* data=current->getHighlightedRecord();
        if (data) {
            return data->curvesGetYLabel(eval->getCurrentIndex());
        }
    }
    return "Y";
}

QString QFCurveFitEvaluationEditor::getFitName() const
{
    return tr("Least-Squares Fit");
}

bool QFCurveFitEvaluationEditor::getPlotXLog() const
{
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (eval) {
        QFRawDataRecord* data=current->getHighlightedRecord();
        if (data) {
            return data->curvesGetLogX(eval->getCurrentIndex());
        }
    }
    return false;
}

bool QFCurveFitEvaluationEditor::getPlotYLog() const
{
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (eval) {
        QFRawDataRecord* data=current->getHighlightedRecord();
        if (data) {
            return data->curvesGetLogY(eval->getCurrentIndex());
        }
    }
    return false;
}

void QFCurveFitEvaluationEditor::highlightingChanged(QFRawDataRecord* formerRecord, QFRawDataRecord* currentRecord) {
    //qDebug()<<"### highlightingChanged("<<formerRecord<<currentRecord<<")";
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (eval) {
        dataEventsEnabled=false;
        //qDebug()<<"highlightingChanged "<<eval->getFitDataWeighting();
        cmbWeights->setCurrentWeight(eval->getFitDataWeighting());
        widFitErrorEstimate->readSettings(current);
        dataEventsEnabled=true;
    }
    QFFitResultsByIndexEvaluationEditorWithWidgets::highlightingChanged(formerRecord, currentRecord);
}







void QFCurveFitEvaluationEditor::updateFitFunctionsPlot() {
    if (!current) return;
    if (!cmbModel) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    QFRDRCurvesInterface* data=qobject_cast<QFRDRCurvesInterface*>(record);
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    int run=eval->getCurrentIndex();
    JKQTPdatastore* ds=pltData->getDatastore();
    JKQTPdatastore* dsres=pltResiduals->getDatastore();
    JKQTPdatastore* dsresh=pltResidualHistogram->getDatastore();
    JKQTPdatastore* dsresc=pltResidualCorrelation->getDatastore();
    QFFitFunction* ffunc=eval->getFitFunction();
    //qDebug()<<" **** updateFitFunctions "<<eval<<data;
    //qDebug()<<"ff="<<eval->getFitFunctionID()<<"  "<<ffunc;

    if (!ffunc) return;

    if ((!eval)||(!data)) {
        return;
    }


    QTime t, t1;
    t.start();
    t1.start();

    int residualStyle=cmbResidualStyle->currentIndex();
    int residualHistogramBins=spinResidualHistogramBins->value();
    int datacut_min=datacut->get_userMin();
    int datacut_max=datacut->get_userMax();


    //bool updEn=updatesEnabled();
    //setUpdatesEnabled(false);
    try {
        QVector<double> x,y,weights,weightsNorm;
        x=data->curvesGetX(run);
        y=data->curvesGetY(run);
        //weightsNorm=weights=data->curvesGetYError(run);
        long N=qMin(x.size(), y.size());
        //qDebug()<<"x.size="<<x.size()<<"  y.size="<<y.size()<<"  N="<<N;
        if (N>0) {
            eval->set_doEmitPropertiesChanged(false);
            eval->set_doEmitResultsChanged(false);
            record->disableEmitResultsChanged();
            //qDebug()<<"    a "<<t.elapsed()<<" ms";
            t.start();


            /////////////////////////////////////////////////////////////////////////////////
            // retrieve data and tau-values from rawdata record
            /////////////////////////////////////////////////////////////////////////////////
            int runAvgWidth=11;

            weightsNorm=weights=eval->allocVecWeights(NULL, record, eval->getCurrentIndex());
            //qDebug()<<"weights="<<weights.size()<<"  "<<weights;


            /////////////////////////////////////////////////////////////////////////////////
            // retrieve fit parameters and errors. run calcParameters to fill in calculated parameters and make sure
            // we are working with a complete set of parameters
            /////////////////////////////////////////////////////////////////////////////////
            double* fullParams=eval->allocFillParameters();
            double* errors=eval->allocFillParameterErrors();
            bool* paramsFix=eval->allocFillFix(record, eval->getCurrentIndex());
            ffunc->calcParameter(fullParams, errors);

            /////////////////////////////////////////////////////////////////////////////////
            // calculate fit statistics
            /////////////////////////////////////////////////////////////////////////////////
            record->disableEmitResultsChanged();
            QStringList pnames;
            QFFitStatistics fitStats=eval->calcFitStatisticsV(eval->hasFit(record, run), ffunc, N, x, y, weights, datacut_min, datacut_max, fullParams, errors, paramsFix, runAvgWidth, residualHistogramBins, record, run, QString("fitstat_"), QString("fit statistics"), QVector<double>(), 200, true, &pnames);
            record->enableEmitResultsChanged();



            size_t c_fit = ds->addCopiedColumn(fitStats.fitfunc, "fit_model");
            //qDebug()<<"    f "<<t.elapsed()<<" ms";
            t.start();

            /////////////////////////////////////////////////////////////////////////////////
            // plot fit model and additional function graphs
            /////////////////////////////////////////////////////////////////////////////////
//                JKQTPxyLineGraph* g_fit=new JKQTPxyLineGraph(pltData->get_plotter());
//                g_fit->set_drawLine(true);
//                g_fit->set_title("fit function");
//                g_fit->set_xColumn(c_x);
//                g_fit->set_yColumn(c_fit);
//                g_fit->set_datarange_start(datacut->get_userMin());
//                g_fit->set_datarange_end(datacut->get_userMax());
            JKQTPxQFFitFunctionLineGraph* g_fit=new JKQTPxQFFitFunctionLineGraph(pltData);
            g_fit->set_drawLine(true);
            g_fit->set_title("fit function");
            g_fit->set_fitFunction(ffunc->id());
            g_fit->set_copiedParams(fullParams, ffunc->paramCount());
            g_fit->set_lineWidth(2);
            for (int i=0; i<(int)ffunc->getAdditionalPlotCount(fullParams); i++) {
                double* params=eval->allocFillParameters();
                QString name=ffunc->transformParametersForAdditionalPlot(i, params);
                //qDebug()<<arrayToString(params, ffunc->paramCount());
                double* afitfunc=(double*)qfMalloc(N*sizeof(double));
                for (int j=0; j<N; j++) {
                    afitfunc[j]=ffunc->evaluate(x[j], params);
                }
                size_t c_afit=ds->addCopiedColumn(afitfunc, N, QString("add_fit_model_%1").arg(i));
                /*JKQTPxyLineGraph* g_afit=new JKQTPxyLineGraph(pltData->get_plotter());
                g_afit->set_drawLine(true);
                g_afit->set_title(name);
                g_afit->set_xColumn(c_x);
                g_afit->set_yColumn(c_afit);
                g_afit->set_datarange_start(datacut->get_userMin());
                g_afit->set_datarange_end(datacut->get_userMax());*/
                JKQTPxQFFitFunctionLineGraph* g_afit=new JKQTPxQFFitFunctionLineGraph(pltData);
                g_afit->set_drawLine(true);
                g_afit->set_title(name);
                g_afit->set_fitFunction(ffunc->id());
                g_afit->set_copiedParams(params, ffunc->paramCount());
                g_afit->set_lineWidth(1);
                pltData->addGraph(g_afit);
                qfFree(params);
                qfFree(afitfunc);
            }
            pltData->addGraph(g_fit);
            //qDebug()<<"    g "<<t.elapsed()<<" ms";
            t.start();

            int c_x=ds->getColumnNum(data->curvesGetXLabel(eval->getCurrentIndex()));
            if (c_x>=0) { // we only add a graph, if we have a column with tau values

                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_taures=c_x;//dsres->addCopiedColumn(data->getCorrelationT(), N, "tau");
                size_t c_residuals=0;
                JKQTPxyLineGraph* g_residuals=new JKQTPxyLineGraph(pltResiduals->get_plotter());
                if (chkWeightedResiduals->isChecked()) {
                    c_residuals=dsres->addCopiedColumn(fitStats.residuals_weighted,  "residuals_weighted");
                    g_residuals->set_title("weighted residuals");
                } else {
                    c_residuals=dsres->addCopiedColumn(fitStats.residuals,  "residuals");
                    g_residuals->set_title("residuals");
                }
                g_residuals->set_xColumn(c_taures);
                g_residuals->set_yColumn(c_residuals);
                g_residuals->set_symbolSize(8);
                g_residuals->set_symbolWidth(1);
                g_residuals->set_datarange_start(datacut->get_userMin());
                g_residuals->set_datarange_end(datacut->get_userMax());
                g_residuals->set_drawLine(true);
                if (residualStyle==0) { // draw points
                    g_residuals->set_drawLine(false);
                    g_residuals->set_symbol(JKQTPcross);
                } else if (residualStyle==2) {
                    g_residuals->set_symbol(JKQTPcross);
                }
                pltResiduals->addGraph(g_residuals);
                //qDebug()<<"    h "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals running average
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_tauresra=dsres->addCopiedColumn(fitStats.tau_runavg,  "tau_resid_runavg");
                size_t c_residualsra=0;
                JKQTPxyLineGraph* g_residualsra=new JKQTPxyLineGraph(pltResiduals->get_plotter());


                if (chkWeightedResiduals->isChecked()) {
                    c_residualsra=dsres->addCopiedColumn(fitStats.residuals_runavg_weighted,  "residuals_runavg_weighted");
                    g_residualsra->set_title("weighted residuals, movAvg");
                } else {
                    c_residualsra=dsres->addCopiedColumn(fitStats.residuals_runavg,  "residuals_runavg");
                    g_residualsra->set_title("residuals, movAvg");
                }
                g_residualsra->set_xColumn(c_tauresra);
                g_residualsra->set_yColumn(c_residualsra);
                g_residualsra->set_symbol(JKQTPnoSymbol);
                g_residualsra->set_symbolSize(6);
                g_residualsra->set_symbolWidth(1);
                g_residualsra->set_color(g_residuals->get_color().darker());
                //g_residuals->set_datarange_start(datacut->get_userMin());
                //g_residuals->set_datarange_end(datacut->get_userMax());
                g_residualsra->set_drawLine(true);
                /*if (residualStyle==0) { // draw points
                    // always draw as lines
                    g_residualsra->set_symbol(JKQTPplus);
                } else if (residualStyle==2) {
                    g_residualsra->set_symbol(JKQTPplus);
                }*/
                pltResiduals->addGraph(g_residualsra);
                //qDebug()<<"    i "<<t.elapsed()<<" ms";
                t.start();

                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals histogram
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_residualHistogramX=0;
                size_t c_residualHistogramY=0;
                if (chkWeightedResiduals->isChecked()) {
                    c_residualHistogramX=dsresh->addLinearColumn(residualHistogramBins, fitStats.rminw+fitStats.residHistWBinWidth/2.0, fitStats.rmaxw-fitStats.residHistWBinWidth/2.0, "residualhist_weighted_x");
                    c_residualHistogramY=dsresh->addCopiedColumn(fitStats.resWHistogram,  "residualhist_weighted_y");
                } else {
                    c_residualHistogramX=dsresh->addLinearColumn(residualHistogramBins, fitStats.rmin+fitStats.residHistBinWidth/2.0, fitStats.rmax-fitStats.residHistBinWidth/2.0, "residualhist_x");
                    c_residualHistogramY=dsresh->addCopiedColumn(fitStats.resHistogram,  "residualhist_y");
                }
                JKQTPbarHorizontalGraph* g_residualsHistogram=new JKQTPbarHorizontalGraph(pltResidualHistogram->get_plotter());
                g_residualsHistogram->set_xColumn(c_residualHistogramX);
                g_residualsHistogram->set_yColumn(c_residualHistogramY);
                g_residualsHistogram->set_shift(0);
                g_residualsHistogram->set_width(1.0);
                pltResidualHistogram->addGraph(g_residualsHistogram);
                //qDebug()<<"    j "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals correlations
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_residualCorrelationX=dsresc->addLinearColumn(fitStats.resN-1, 1, fitStats.resN-1, "residualcorr_x");
                size_t c_residualCorrelationY=0;
                if (chkWeightedResiduals->isChecked()) {
                    double* data=fitStats.resWCorrelation.data();
                    c_residualCorrelationY=dsresc->addCopiedColumn(&(data[1]), fitStats.resN-1, "residualcorr_weighted_y");
                } else {
                    double* data=fitStats.resCorrelation.data();
                    c_residualCorrelationY=dsresh->addCopiedColumn(&(data[1]), fitStats.resN-1, "residualcorr_y");
                }
                JKQTPxyLineGraph* g_residualsCorrelation=new JKQTPxyLineGraph(pltResidualCorrelation->get_plotter());
                g_residualsCorrelation->set_xColumn(c_residualCorrelationX);
                g_residualsCorrelation->set_yColumn(c_residualCorrelationY);
                pltResidualCorrelation->addGraph(g_residualsCorrelation);
                //qDebug()<<"    k "<<t.elapsed()<<" ms";
                t.start();

                /////////////////////////////////////////////////////////////////////////////////
                // update display of fit results
                /////////////////////////////////////////////////////////////////////////////////
                QString txtFit="<font face=\"Arial\">";
                QString fitResult=record->resultsGetAsString(eval->getEvaluationResultID(record), "fitalg_messageHTML");

                if (!fitResult.isEmpty()) {
                    txtFit+=txtFit+tr("<div style=\"border-style:solid\"><b>Fit Result Message:</b><center>%1</center></div><br>").arg(fitResult);
                } else {
                    txtFit+=txtFit+tr("<div style=\"border-style:solid\"><b>Fit Result Message:</b><center>not fit yet</center></div><br>");
                }
                txtFit+=QString("<b>%1</b><center>").arg(tr("Fit Statistics:"));


                txtFit+=fitStats.getAsHTMLTable(false);
                txtFit+=tr("<br><br>Parameters in matrix:<br><blockquote><tt>%1</tt></blockquote>").arg(pnames.join(", "));
                txtFit+=QString("<br><br>");
                txtFit+=fitStats.getHTMLExplanation();


               // txtFit+=fitStats.getAsHTMLTable();
                t.start();

                txtFit+=QString("</center></font>");
                fitStatisticsReport=txtFit;
                txtFitStatistics->setHtml(txtFit);

                //qDebug()<<"    m_presignals "<<t.elapsed()<<" ms";
                eval->set_doEmitPropertiesChanged(true);
                eval->set_doEmitResultsChanged(true);
                //record->enableEmitResultsChanged();

                //qDebug()<<"    m "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // clean memory
                /////////////////////////////////////////////////////////////////////////////////
                qfFree(fullParams);
                qfFree(errors);
                qfFree(paramsFix);
                fitStats.free();

                //qDebug()<<"    n "<<t.elapsed()<<" ms";
                t.start();
            }
        }
    } catch(std::exception& E) {
        services->log_error(tr("error during plotting, error message: %1\n").arg(E.what()));
    }
    //setUpdatesEnabled(updEn);
    //qDebug()<<"    updateFitFunctions end   runtime = "<<t1.elapsed()<<"ms";

}



























void QFCurveFitEvaluationEditor::weightsChanged(int /*model*/) {
    if (!dataEventsEnabled) return;
    if (!current) return;
    if (!current->getHighlightedRecord()) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    current->setQFProperty("weights", cmbWeights->currentIndex(), false, false);
    QFCurveFitEvaluation* data=qobject_cast<QFCurveFitEvaluation*>(current);
    if (data) {
        data->setFitDataWeighting(cmbWeights->currentWeight());
    }
    displayModel(true);
    replotData();
    QApplication::restoreOverrideCursor();
}

void QFCurveFitEvaluationEditor::repeatsChanged(int r)
{
    if (!current) return;
    current->setQFProperty("FIT_REPEATS", r, false, false);

}

void QFCurveFitEvaluationEditor::errorEstimateModeChanged()
{
    if (!current) return;
    widFitErrorEstimate->saveSettings(current);
}



























int QFCurveFitEvaluationEditor::getUserMin(QFRawDataRecord* rec, int index) {
    return getUserMin(rec, index, datacut->get_userMin());
}

int QFCurveFitEvaluationEditor::getUserMax(QFRawDataRecord* rec, int index) {
    return getUserMax(rec, index, datacut->get_userMax());
}

int QFCurveFitEvaluationEditor::getUserMin() {
    return getUserMin(datacut->get_userMin());
}

int QFCurveFitEvaluationEditor::getUserMax() {
    return getUserMax(datacut->get_userMax());
}

int QFCurveFitEvaluationEditor::getUserRangeMax(QFRawDataRecord *rec, int index) {
    QFRDRCurvesInterface* data=qobject_cast<QFRDRCurvesInterface*>(rec);
    if (data) {
        return qMin(data->curvesGetX(index).size(), data->curvesGetY(index).size());
    }
    return 0;
}

int QFCurveFitEvaluationEditor::getUserRangeMin(QFRawDataRecord */*rec*/, int /*index*/) {
    return 0;
}

void QFCurveFitEvaluationEditor::getPlotData(QFRawDataRecord *rec, int index, QList<QFGetPlotdataInterface::GetPlotDataItem> &plots, int option, const QString &/*optionName*/)
{
    QFRDRCurvesInterface* data=qobject_cast<QFRDRCurvesInterface*>(rec);
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (data&&eval) {
        double norm=1;
        int run=index;
        QVector<double> x, y;
        x=data->curvesGetX(run);
        y=data->curvesGetY(run);
        long N=qMin(x.size(), y.size());

        QFFitFunction* ffunc=eval->getFitFunction(rec);
        int datacut_min=datacut->get_min();
        int datacut_max=datacut->get_max();

        try {
            if (N>0) {

                int runAvgWidth=11;
                //double* weights=eval->allocWeights(NULL, rec, eval->getCurrentIndex(), datacut_min, datacut_max, true);


                /////////////////////////////////////////////////////////////////////////////////
                // retrieve fit parameters and errors. run calcParameters to fill in calculated parameters and make sure
                // we are working with a complete set of parameters
                /////////////////////////////////////////////////////////////////////////////////
                double* fullParams=eval->allocFillParameters(rec, index, ffunc);
                double* errors=eval->allocFillParameterErrors(rec, index, ffunc);
                ffunc->calcParameter(fullParams, errors);

                for (int i=0; i<x.size(); i++) {
                    y<<ffunc->evaluate(x[i], fullParams);
                }


                /////////////////////////////////////////////////////////////////////////////////
                // clean memory
                /////////////////////////////////////////////////////////////////////////////////
                qfFree(fullParams);
                qfFree(errors);
                //qfFree(weights);
            }
        } catch(std::exception& E) {
            services->log_error(tr("error during plotting, error message: %1\n").arg(E.what()));
        }




        QFGetPlotdataInterface::GetPlotDataItem item;
        item.x=x;
        item.y=y;
        bool ok=true;
        QVector<double> w=eval->allocVecWeights(&ok, rec, index, true);
        if (ok && w.size()>0) {
            item.yerrors=w;
        }
        item.name=rec->getName()+": "+data->curvesGetName(index);
        item.averageGroupIndex=0;
        if (option==2 || option==3) {
            item.name+=tr(", normalized");

            for (int i=0; i<item.y.size(); i++) {
                item.y[i]=item.y[i]*norm;
            }
            for (int i=0; i<item.yerrors.size(); i++) {
                item.yerrors[i]=item.yerrors[i]*norm;
            }

        }
        item.name=QString("\\verb{%1}").arg(item.name);
        plots.append(item);


    }
}

bool QFCurveFitEvaluationEditor::getPlotDataSpecs(QStringList *optionNames, QList<QFGetPlotdataInterface::GetPlotPlotOptions> *listPlotOptions)
{
    if (optionNames) {
        *optionNames<<tr("data");
        *optionNames<<tr("data + fits");
    }
    if (listPlotOptions) {
        *listPlotOptions<<QFGetPlotdataInterface::GetPlotPlotOptions(tr("X"), tr("curve Y=f(X)"), true, false);
        *listPlotOptions<<QFGetPlotdataInterface::GetPlotPlotOptions(tr("X"), tr("curve Y=f(X)"), true, false);
    }
    return true;
}


void QFCurveFitEvaluationEditor::replotData() {


    if (!current) return;
    if (!cmbModel) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    QFRDRCurvesInterface* data=qobject_cast<QFRDRCurvesInterface*>(record);
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    QFCurveWeightingTools* wdata=dynamic_cast<QFCurveWeightingTools*>(current.data());
    JKQTPdatastore* ds=pltData->getDatastore();
    JKQTPdatastore* dsres=pltResiduals->getDatastore();
    JKQTPdatastore* dsresh=pltResidualHistogram->getDatastore();
    JKQTPdatastore* dsresc=pltResidualCorrelation->getDatastore();

    if ((!eval)||(!data)) {
        pltData->clearGraphs();
        ds->clear();
        return;
    }

    //qDebug()<<" **** replotData()";
    QTime t, t1;
    t.start();
    t1.start();

    //bool updEn=updatesEnabled();
    //setUpdatesEnabled(false);

    pltResiduals->set_doDrawing(false);
    pltResiduals->set_emitSignals(false);
    pltResiduals->clearGraphs();
    pltData->set_doDrawing(false);
    pltData->set_emitSignals(false);
    pltData->clearGraphs();
    pltResidualHistogram->set_doDrawing(false);
    pltResidualHistogram->set_emitSignals(false);
    pltResidualHistogram->clearGraphs();
    pltResidualCorrelation->set_doDrawing(false);
    pltResidualCorrelation->set_emitSignals(false);
    pltResidualCorrelation->clearGraphs();

    pltData->get_plotter()->set_showKey(chkKey->isChecked());
    pltResiduals->get_plotter()->set_showKey(chkKey->isChecked());
    pltResidualHistogram->get_plotter()->set_showKey(chkKey->isChecked());
    pltResidualCorrelation->get_plotter()->set_showKey(chkKey->isChecked());
    dsres->clear();
    ds->clear();
    dsresh->clear();
    dsresc->clear();

    //qDebug()<<"   a "<<t.elapsed()<<" ms";
    t.start();

    pltResiduals->getXAxis()->set_logAxis(chkXLogScale->isChecked());
    pltData->getXAxis()->set_logAxis(chkXLogScale->isChecked());
    pltData->getYAxis()->set_logAxis(chkYLogScale->isChecked());
    if (chkXLogScale->isChecked()) {
        pltData->getXAxis()->set_minorTicks(9);
        pltResiduals->getXAxis()->set_minorTicks(9);
    } else {
        pltData->getXAxis()->set_minorTicks(1);
        pltResiduals->getXAxis()->set_minorTicks(1);
    }
    if (chkYLogScale->isChecked()) {
        pltData->getYAxis()->set_minorTicks(9);
        pltResiduals->getYAxis()->set_minorTicks(9);
    } else {
        pltData->getYAxis()->set_minorTicks(1);
        pltResiduals->getYAxis()->set_minorTicks(1);
    }
    pltResiduals->getXAxis()->set_drawGrid(chkGrid->isChecked());
    pltResiduals->getYAxis()->set_drawGrid(chkGrid->isChecked());
    pltData->getXAxis()->set_drawGrid(chkGrid->isChecked());
    pltData->getYAxis()->set_drawGrid(chkGrid->isChecked());
    pltData->getYAxis()->set_minTicks(5);
    pltResiduals->getYAxis()->set_minTicks(5);
    //pltData->getXAxis()->set_logAxis(data->curvesGetLogX(eval->getCurrentIndex()));
    //pltResiduals->getXAxis()->set_logAxis(data->curvesGetLogX(eval->getCurrentIndex()));
    //pltData->getYAxis()->set_logAxis(data->curvesGetLogY(eval->getCurrentIndex()));
    pltResiduals->getXAxis()->set_axisLabel(data->curvesGetXLabel(eval->getCurrentIndex()));
    pltData->getYAxis()->set_axisLabel(data->curvesGetYLabel(eval->getCurrentIndex()));

    //qDebug()<<"   b "<<t.elapsed()<<" ms";
    t.start();

    //int errorStyle=cmbErrorStyle->currentIndex();
    int plotStyle=cmbPlotStyle->currentIndex();
    //int residualStyle=cmbResidualStyle->currentIndex();

    QVector<double> x, y, err;
    x=data->curvesGetX(eval->getCurrentIndex());
    y=data->curvesGetY(eval->getCurrentIndex());
    //err=data->curvesGetYError(eval->getCurrentIndex());
    err=eval->allocVecWeights(NULL, record, eval->getCurrentIndex(), true);
    long N=qMin(x.size(), y.size());

    if (N>0) {
        size_t c_tau=ds->addCopiedColumn(x, data->curvesGetXLabel(eval->getCurrentIndex()));


        //////////////////////////////////////////////////////////////////////////////////
        // Plot average + error markers
        //////////////////////////////////////////////////////////////////////////////////
        size_t c_mean=0;
        QString graphName="";
        int c_std=-1;
        QString errorName="";

        c_mean=ds->addCopiedColumn(y, QString("curve"+QString::number(eval->getCurrentIndex())));
        graphName=tr("\\verb{%1} %2").arg(record->getName()).arg(data->curvesGetName(eval->getCurrentIndex()));
        if (!eval->isEqualWeights() && err.size()>0) {
            errorName=wdata->dataWeightToName(eval->getFitDataWeighting(), m_runName);
            c_std=ds->addCopiedColumn(err, QString("cerr_")+wdata->dataWeightToString(eval->getFitDataWeighting()));
        }

        JKQTPerrorPlotstyle styl=cmbErrorStyle->getErrorStyle();

        //qDebug()<<"   c "<<t.elapsed()<<" ms";
        t.start();

        JKQTPxyLineErrorGraph* g=new JKQTPxyLineErrorGraph(pltData->get_plotter());
        QColor gcolor=pltData->get_plotter()->get_graphColor();
        g->set_color(gcolor);
        g->set_errorColor(gcolor.lighter());
        QColor errfc=g->get_errorColor();
        errfc.setAlphaF(0.5);
        g->set_errorFillColor(errfc);
        g->set_lineWidth(2);
        g->set_symbolSize(8);
        g->set_symbolWidth(1);
        g->set_xColumn(c_tau);
        g->set_yColumn(c_mean);
        g->set_yErrorColumn(c_std);
        g->set_title(graphName);
        g->set_yErrorStyle(styl);
        g->set_xErrorStyle(JKQTPnoError);
        g->set_datarange_start(datacut->get_userMin());
        g->set_datarange_end(datacut->get_userMax());
        // draw lines is default
        if (plotStyle==0) { // draw points
            g->set_drawLine(false);
            g->set_symbol(JKQTPcross);
        } else if (plotStyle==2) {
            g->set_symbol(JKQTPcross);
        }
        pltData->addGraph(g);
        //qDebug()<<"   d "<<t.elapsed()<<" ms";
        t.start();

        updateFitFunctionsPlot();
        //qDebug()<<"   e "<<t.elapsed()<<" ms";
        t.start();

        pltData->zoomToFit(true, true);
        pltResiduals->zoomToFit(false, true);

        pltResiduals->setX(pltData->getXMin(), pltData->getXMax());

        pltResidualHistogram->zoomToFit(true, true);
        pltResidualCorrelation->zoomToFit(true, true);
        //qDebug()<<"   f "<<t.elapsed()<<" ms";
        t.start();
    }


    pltResiduals->set_doDrawing(true);
    pltResiduals->set_emitSignals(true);
    pltData->set_doDrawing(true);
    pltData->set_emitSignals(true);
    pltResidualHistogram->set_doDrawing(true);
    pltResidualHistogram->set_emitSignals(true);
    pltResidualCorrelation->set_doDrawing(true);
    pltResidualCorrelation->set_emitSignals(true);
    //qDebug()<<"   g "<<t.elapsed()<<" ms";
    t.start();

    pltResiduals->update_plot();
    //qDebug()<<"   h "<<t.elapsed()<<" ms";
    t.start();
    pltData->update_plot();
    //qDebug()<<"   i "<<t.elapsed()<<" ms";
    t.start();
    pltResidualHistogram->update_plot();
    //qDebug()<<"   j "<<t.elapsed()<<" ms";
    t.start();
    pltResidualCorrelation->update_plot();
    //qDebug()<<"   k "<<t.elapsed()<<" ms";
    t.start();
    //qDebug()<<"  replotData end  runtime = "<<t1.elapsed()<<" ms";

    //setUpdatesEnabled(updEn);
}



//void QFCurveFitEvaluationEditor::displayModel(bool newWidget) {
//    QFFitResultsByIndexEvaluationEditorWithWidgets::displayModel(newWidget);
//    if (!current) return;
//    if (!cmbModel) return;
//    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
//    QFRawDataRecord* rdr=eval->getHighlightedRecord();
//    QFFitFunction* ffunc=eval->getFitFunction(rdr);


//    if (!ffunc) {
//        /////////////////////////////////////////////////////////////////////////////////////////////
//        // delete all fit parameter actions
//        /////////////////////////////////////////////////////////////////////////////////////////////
//        clearEstimateActions();
//        return;
//    }


//    if (newWidget) {
//        /////////////////////////////////////////////////////////////////////////////////////////////
//        // first delete all fit parameter actions
//        /////////////////////////////////////////////////////////////////////////////////////////////
//        clearEstimateActions();

//        /////////////////////////////////////////////////////////////////////////////////////////////
//        // create new parameter actions
//        /////////////////////////////////////////////////////////////////////////////////////////////

//        for (int i=0; i<ffunc->paramCount(); i++) {
//            QString id=ffunc->getParameterID(i);

//            QFFitFunction::ParameterDescription d=ffunc->getDescription(i);
//            if (ffunc->isParameterXYEstimateable(i)
//                    && (d.widgetType==QFFitFunction::LogFloatNumber || d.widgetType==QFFitFunction::FloatNumber)
//                    && d.fit) {
//                QAction* act=new QFActionWithNoMenuRole(tr("estimate '%1'").arg(d.name), this);
//                connect(act, SIGNAL(triggered()), this, SLOT(estimateActionClicked()));
//                actsEstimate.insert(act, id);
//                menuEstimate->addAction(act);
//            }

//        }

//    }

//}

//void QFCurveFitEvaluationEditor::updateParameterValues(QFRawDataRecord* rec)
//{
//    QFFitResultsByIndexEvaluationEditorWithWidgets::updateParameterValues(rec);
//    if (!current) return;
//    if (!cmbModel) return;


//    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
//    QFFitFunction* ffunc=eval->getFitFunction(rec);

//    if (!ffunc) return;

//    QVector<double> fullParams=eval->allocVecFillParameters(ffunc);
//    QVector<double> errors=eval->allocVecFillParameterErrors(ffunc);
//    ffunc->calcParameter(fullParams, errors);


//    QMapIterator<QAction*,QString> it(actsEstimate);
//    while (it.hasNext()) {
//        it.next();
//        it.key()->setVisible(ffunc->isParameterVisible(it.value(), fullParams));
//    }

//}

//void QFCurveFitEvaluationEditor::dataplotContextMenuOpened(double x, double y, QMenu */*contextMenu*/)
//{
//    if (!current) return;
//    if (!cmbModel) return;


//    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
//    if (!eval) return;
//    QFFitFunction* ffunc=eval->getFitFunction(current->getHighlightedRecord());

//    if (!ffunc) return;

//    QVector<double> fullParams=eval->allocVecFillParameters(ffunc);
//    QVector<double> errors=eval->allocVecFillParameterErrors(ffunc);
//    ffunc->calcParameter(fullParams, errors);


//    QMapIterator<QAction*,QString> it(actsEstimate);
//    while (it.hasNext()) {
//        it.next();
//        if (it.key()->isVisible()) {
//            QString id=it.value();
//            double np=eval->getFitValue(eval->getHighlightedRecord(), eval->getCurrentIndex(), id);
//            if (ffunc->estimateParameterFromXY(np, id, x, y, fullParams)) {
//                QFFitFunction::ParameterDescription d=ffunc->getDescription(id);
//                it.key()->setText(tr("estimate '%1' = %2 %3").arg(d.name).arg(np).arg(d.unit));
//            }
//        }

//    }
//}

//void QFCurveFitEvaluationEditor::estimateActionClicked()
//{
//    if (!current) return;
//    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
//    if (!eval) return;
//    QFFitFunction* ffunc=eval->getFitFunction(current->getHighlightedRecord());
//    if (!ffunc || !eval) return;
//    if (!eval->getHighlightedRecord()) return;

//    QAction* act=qobject_cast<QAction*>(sender());
//    if (act && actsEstimate.contains(act)) {
//        QString id=actsEstimate[act];
//        double np=eval->getFitValue(eval->getHighlightedRecord(), eval->getCurrentIndex(), id);
//        if (ffunc->estimateParameterFromXY(np, id, pltData->get_mouseContextX(), pltData->get_mouseContextY(), eval->allocVecFillParameters(ffunc))) {
//            eval->setFitValue(eval->getHighlightedRecord(), eval->getCurrentIndex(), id, np);
//        }
//    }
//}

//void QFCurveFitEvaluationEditor::clearEstimateActions()
//{
//    for(int i=0; i<menuEstimate->actions().size(); i++) {
//        disconnect(menuEstimate->actions().at(i), SIGNAL(triggered()), this, SLOT(estimateActionClicked()));
//    }
//    menuEstimate->clear();
//    actsEstimate.clear();

//}
















/////////////////////////////////////////////////////////////////////
// REPORT GENERATION
/////////////////////////////////////////////////////////////////////

void QFCurveFitEvaluationEditor::createReportDoc(QTextDocument* document) {
    // make sure all widgets ahave the right size
    {   int trci=tabResidulas->currentIndex();
        for (int i=0;i<tabResidulas->count(); i++) {
            tabResidulas->setCurrentIndex(i);
        }
        tabResidulas->setCurrentIndex(trci);
    }

    int PicTextFormat=QTextFormat::UserObject + 1;
    QObject *picInterface = new QPictureTextObject;
    document->documentLayout()->registerHandler(PicTextFormat, picInterface);

    if (!current) return;
    if (!cmbModel) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    QFRDRCurvesInterface* data=qobject_cast<QFRDRCurvesInterface*>(record);
    QFCurveFitEvaluation* eval=qobject_cast<QFCurveFitEvaluation*>(current);
    if (!eval) return;
    if (!data) return;

    QFFitFunction* ffunc=eval->getFitFunction();
    QFFitAlgorithm* algorithm=eval->getFitAlgorithm();
    //int run=eval->getCurrentIndex();
    double* params=eval->allocFillParameters();

    QTextCursor cursor(document);
    QTextCharFormat fText=cursor.charFormat();
    fText.setFontPointSize(8);
    QTextCharFormat fTextSmall=fText;
    fTextSmall.setFontPointSize(0.85*fText.fontPointSize());
    QTextCharFormat fTextBold=fText;
    fTextBold.setFontWeight(QFont::Bold);
    QTextCharFormat fTextBoldSmall=fTextBold;
    fTextBoldSmall.setFontPointSize(0.85*fText.fontPointSize());
    QTextCharFormat fHeading1=fText;
    QTextBlockFormat bfLeft;
    bfLeft.setAlignment(Qt::AlignLeft);
    QTextBlockFormat bfLeftNB=bfLeft;
    bfLeftNB.setNonBreakableLines(true);
    QTextBlockFormat bfRight;
    bfRight.setAlignment(Qt::AlignRight);
    QTextBlockFormat bfRightNB=bfRight;
    bfRightNB.setNonBreakableLines(true);
    QTextBlockFormat bfCenter;
    bfCenter.setAlignment(Qt::AlignHCenter);
    QTextBlockFormat bfCenterNB=bfCenter;
    bfCenterNB.setNonBreakableLines(true);

    fHeading1.setFontPointSize(2*fText.fontPointSize());
    fHeading1.setFontWeight(QFont::Bold);
    cursor.insertText(tr("Curve Fit Report:\n\n"), fHeading1);
    cursor.movePosition(QTextCursor::End);

    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    QTextTable* table = cursor.insertTable(4, 4, tableFormat);
    table->cellAt(0, 0).firstCursorPosition().insertText(tr("file:"), fTextBold);
    table->cellAt(0, 1).firstCursorPosition().insertText(record->getName(), fText);
    table->cellAt(0, 2).firstCursorPosition().insertText(tr("curve:"), fTextBold);
    table->cellAt(0, 3).firstCursorPosition().insertText(QString("%1 [%2]").arg((eval->getCurrentIndex()<0)?tr("average"):QString::number(eval->getCurrentIndex())).arg(data->curvesGetName(eval->getCurrentIndex())), fText);
    table->cellAt(1, 0).firstCursorPosition().insertText(tr("fit algorithm:"), fTextBold);
    table->cellAt(1, 1).firstCursorPosition().insertText(algorithm->name(), fText);
    table->cellAt(1, 2).firstCursorPosition().insertText(tr("data range:"), fTextBold);
    table->cellAt(1, 3).firstCursorPosition().insertText(tr("%1 ... %2 / %3 ... %4").arg(datacut->get_userMin()).arg(datacut->get_userMax()).arg(datacut->get_min()).arg(datacut->get_max()), fText);
    table->cellAt(2, 0).firstCursorPosition().insertText(tr("fit function:"), fTextBold);
    table->cellAt(2, 1).firstCursorPosition().insertText(ffunc->name(), fText);
    table->cellAt(2, 2).firstCursorPosition().insertText(tr("data weighting:"), fTextBold);
    table->cellAt(2, 3).firstCursorPosition().insertText(cmbWeights->currentText(), fText);
    table->cellAt(2, 2).firstCursorPosition().insertText(tr("error estimates:"), fTextBold);
    table->cellAt(2, 3).firstCursorPosition().insertText(widFitErrorEstimate->toString(), fText);
    cursor.movePosition(QTextCursor::End);

    cursor.insertBlock(); cursor.insertBlock();
    cursor.insertText(tr("Plots:\n"), fTextBold);
    QTextTableFormat tableFormat1;
    tableFormat1.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat1.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 66);
    constraints << QTextLength(QTextLength::PercentageLength, 34);
    tableFormat1.setColumnWidthConstraints(constraints);
    table = cursor.insertTable(2, 2, tableFormat1);
    table->mergeCells(0,0,2,1);
    {
        QTextCursor tabCursor=table->cellAt(0, 0).firstCursorPosition();
        QPicture pic;
        JKQTPEnhancedPainter* painter=new JKQTPEnhancedPainter(&pic);
        pltData->get_plotter()->draw(*painter, QRect(0,0,pltData->width(),pltData->width()));
        delete painter;
        double scale=document->textWidth()*0.62/pic.boundingRect().width();
        if (scale<=0) scale=1;
        tabCursor.insertText(tr("correlation data, model and residuals:\n"), fTextBoldSmall);
        insertQPicture(tabCursor, PicTextFormat, pic, QSizeF(pic.boundingRect().width(), pic.boundingRect().height())*scale, 0.62);
    }
    QApplication::processEvents();

    {
        QTextCursor tabCursor=table->cellAt(0, 1).firstCursorPosition();
        QPicture pic;
        JKQTPEnhancedPainter* painter=new JKQTPEnhancedPainter(&pic);
        pltResidualHistogram->get_plotter()->draw(*painter, QRect(0,0,pltResidualHistogram->width(),pltResidualHistogram->width()));
        delete painter;
        double scale=document->textWidth()*0.3/pic.boundingRect().width();
        if (scale<=0) scale=1;
        tabCursor.insertText(tr("residual histogram:\n"), fTextBoldSmall);
        insertQPicture(tabCursor, PicTextFormat, pic, QSizeF(pic.boundingRect().width(), pic.boundingRect().height())*scale, 0.3);
    }
    QApplication::processEvents();
    {
        QTextCursor tabCursor=table->cellAt(1, 1).firstCursorPosition();
        QPicture pic;
        JKQTPEnhancedPainter* painter=new JKQTPEnhancedPainter(&pic);
        pltResidualCorrelation->get_plotter()->draw(*painter, QRect(0,0,pltResidualCorrelation->width(),pltResidualCorrelation->width()));
        delete painter;
        double scale=document->textWidth()*0.3/pic.boundingRect().width();
        if (scale<=0) scale=1;
        tabCursor.insertText(tr("residual correlations:\n"), fTextBoldSmall);
        insertQPicture(tabCursor, PicTextFormat, pic, QSizeF(pic.boundingRect().width(), pic.boundingRect().height())*scale, 0.3);
    }
    cursor.movePosition(QTextCursor::End);
    QApplication::processEvents();

    int fitParamCount=0;
    for (int i=0; i<ffunc->paramCount(); i++) {
        if (ffunc->isParameterVisible(i, params)) fitParamCount++;
    }
    QApplication::processEvents();

    cursor.insertBlock(); cursor.insertBlock();
    if (eval->hasFit()) cursor.insertText(tr("Model Parameters (fit results):"), fTextBold);
    else cursor.insertText(tr("Model Parameters (initial values):"), fTextBold);
    QTextTableFormat tableFormat2;
    tableFormat2.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat2.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    table = cursor.insertTable(ceil((double)fitParamCount/2.0)+1, 12, tableFormat2);
    QTextCursor tableCursor;
    QApplication::processEvents();

    tableCursor=table->cellAt(0, 0).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Parameter"), fTextBoldSmall);
    tableCursor=table->cellAt(0, 2).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Value"), fTextBoldSmall);
    if (algorithm->get_supportsBoxConstraints()) table->cellAt(0, 5).firstCursorPosition().insertText(tr("Range"), fTextBoldSmall);
    QApplication::processEvents();

    tableCursor=table->cellAt(0, 6).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Parameter"), fTextBoldSmall);
    tableCursor=table->cellAt(0, 8).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Value"), fTextBoldSmall);
    if (algorithm->get_supportsBoxConstraints()) table->cellAt(0, 11).firstCursorPosition().insertText(tr("Range"), fTextBoldSmall);
    QApplication::processEvents();


    int rowStart=1;
    int colStart=0;
    for (int i=0; i<ffunc->paramCount(); i++) {
        QApplication::processEvents();
        QString id=ffunc->getParameterID(i);
        double error=roundError(eval->getFitError(id),2);
        double value=roundWithError(eval->getFitValue(id), error, 2);
        QString value_string=floattohtmlstr(value, 5, true).c_str();
        bool fix=eval->getFitFix(id);
        QFFitFunction::ParameterDescription d=ffunc->getDescription(id);
        QString range=QString("%1...%2").arg(QString(floattohtmlstr(d.minValue, 5, true).c_str())).arg(QString(floattohtmlstr(d.maxValue, 5, true).c_str()));
        if ((d.widgetType==QFFitFunction::IntCombo)&&((int)value>=0)&&((int)value<d.comboItems.size())) {
            value_string="<i>"+d.comboItems[(int)value]+"</i>";
        }
        if (ffunc->isParameterVisible(i, params)) {
            QString err="";
            if (d.displayError!=QFFitFunction::NoError) {
                err=QString("&plusmn;&nbsp;%1").arg(QString(floattohtmlstr(error, 5, true).c_str()));
            }

            tableCursor=table->cellAt(rowStart, colStart).firstCursorPosition();
            tableCursor.setBlockFormat(bfRightNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(d.label));
            tableCursor.insertText(" = ", fTextSmall);

            tableCursor=table->cellAt(rowStart, colStart+1).firstCursorPosition();
            tableCursor.setBlockFormat(bfLeftNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            if (d.fit) {
                tableCursor.insertText(tr("F"), fTextSmall);
            }
            if (!d.userEditable) {
                tableCursor.insertText(tr("C"), fTextSmall);
            }
            if (fix) {
                tableCursor.insertText(tr("X"), fTextSmall);
            }


            tableCursor=table->cellAt(rowStart, colStart+2).firstCursorPosition();
            tableCursor.setBlockFormat(bfRightNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(value_string)));

            tableCursor=table->cellAt(rowStart, colStart+3).firstCursorPosition();
            tableCursor.setBlockFormat(bfLeftNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(err)));

            tableCursor=table->cellAt(rowStart, colStart+4).firstCursorPosition();
            tableCursor.setBlockFormat(bfLeftNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(d.unitLabel)));

            if (algorithm->get_supportsBoxConstraints()) {
                tableCursor=table->cellAt(rowStart, colStart+5).firstCursorPosition();
                tableCursor.setBlockFormat(bfLeftNB);
                tableCursor.setBlockCharFormat(fTextSmall);
                tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(range)));
            }
            rowStart++;
        };
        if (rowStart>=table->rows()) {
            rowStart=1;
            colStart+=6;
        }
    }
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.setBlockFormat(bfCenterNB);
    cursor.setBlockCharFormat(fTextSmall);
    cursor.insertFragment(QTextDocumentFragment::fromHtml(tr("<i><u>legend:</u> <b>F</b>: fit parameter, <b>X</b>: fixed parameter, <b>C</b>: calculated parameter</i>")));
    QApplication::processEvents();

    cursor.setBlockFormat(bfLeft);
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(); cursor.insertBlock();
    QString htmlBot=fitStatisticsReport;
    htmlBot.replace("width=\"95%\"", "");
    cursor.insertFragment(QTextDocumentFragment::fromHtml(htmlBot));

}


