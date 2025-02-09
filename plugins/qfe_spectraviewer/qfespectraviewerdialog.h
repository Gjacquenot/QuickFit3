/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg

    

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

#ifndef QFESPECTRAVIEWERDIALOG_H
#define QFESPECTRAVIEWERDIALOG_H

#include "qfdialog.h"
#include <QList>
#include <QSettings>
#include <QListWidget>
#include "qftablemodel.h"
class QFESpectraViewer;
class SpectrumManager;

namespace Ui {
    class QFESpectraViewerDialog;
}

enum QFESpectraViewerPlotItemType {
    qfesFluorohpore=0,
    qfesLightSourceSpectrum=1,
    qfesLightSourceSingleLine=2,
    qfesFilterSpectrum=3,
    qfesFilterBandpass=4,
    qfesFilterLongpass=5,
    qfesFilterShortpass=6,
    qfesFilterNotch=7,
    qfesDetector=8
};

struct QFESpectraViewerPlotItem {
    QFESpectraViewerPlotItem(QFESpectraViewerPlotItemType type=qfesFluorohpore);
    QFESpectraViewerPlotItemType type;
    QString name;
    bool showExcitation;
    bool showEmission;
    bool filterIsExcitation;
    double centralWavelength;
    double spectralWidth;
    QString displayName;
};

class QFESpectraViewerDialog : public QFDialog
{
        Q_OBJECT
        
    public:
        explicit QFESpectraViewerDialog(QFESpectraViewer* plugin, QWidget *parent = 0);
        ~QFESpectraViewerDialog();

        void writeSettings();
        void readSettings();

        void loadSpectraConfig(QSettings& settings, const QString& prefix=QString(""));
        void saveSpectraConfig(QSettings& settings, const QString& prefix=QString(""));
        int findLaserLineIndex(double line, double width=-1);
    protected slots:
        void updatePlots();
        void on_btnAddFluorophore_clicked();
        void on_btnAddFilter_clicked();
        void on_btnAddLightsource_clicked();
        void on_btnAddDetector_clicked();
        void on_btnDelete_clicked();
        void spectrumSelected(bool force=false);
        void saveFromWidgets();
        void loadToWidgets();
        void updateItemPropertiesModel();
        void createLightSourceSpectrum();
        void createDetectorSpectrum();
        void on_btnEditLightsource_clicked();
        void createFilterSpectrum();
        void on_btnEditFilter_clicked();
        void on_btnEditDetector_clicked();
        void createFluorophoreSpectrum();
        void on_btnEditFluorophore_clicked();
        void reloadComboboxes();
        void calcSpilloverTable();
        void FRETCalculator();
        void on_btnMailFluorophore_clicked();
        void on_btnMailLightsource_clicked();
        void on_btnMailFilter_clicked();
        void on_btnMailDetector_clicked();
        void on_cmbFilterType_currentIndexChanged(int i);
        void on_cmbLaserLine_currentIndexChanged(int i);
        void on_spinLaserCentral_valueChanged(double value);
        void on_spinLaserLinewidth_valueChanged(double value);
        void showHelp();
    protected:
        Ui::QFESpectraViewerDialog *ui;
        QFESpectraViewer* plugin;
        SpectrumManager* manager;

        QFTableModel modItemProperties;

        int loadMoredata(const QMap<QString, QVariant>& data, int row0);

        int currentIndex;

        QList<QFESpectraViewerPlotItem> plotItems;
};

#endif // QFESPECTRAVIEWERDIALOG_H
