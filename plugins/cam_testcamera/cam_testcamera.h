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

#ifndef QFECamTestCamera_H
#define QFECamTestCamera_H

#include <time.h>
#include <QObject>
#include "qfextension.h"
#include "qfextensioncamera.h"
#include "highrestimer.h"
// include libTIFF header
#include <tiffio.h>
#include "qfdialog.h"


/*!
    \defgroup qf3ext_testcamera QFExtensionCamera implementation which outputs configurable test images
    \ingroup qf3extensionplugins
*/

/*! \brief QFExtensionCamera implementation which outputs configurable test images
    \ingroup qf3ext_testcamera
 */
class QFECamTestCamera : public QObject, public QFExtensionBase, public QFExtensionCamera {
        Q_OBJECT
        Q_INTERFACES(QFExtension QFExtensionCamera)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        Q_PLUGIN_METADATA(IID "www.dkfz.de.QuickFit3.Plugins.QFECamTestCamera")
#endif

    public:
        /** Default constructor */
        QFECamTestCamera(QObject* parent=NULL);
        /** Default destructor */
        virtual ~QFECamTestCamera();


    /////////////////////////////////////////////////////////////////////////////
    // QFExtension routines
    /////////////////////////////////////////////////////////////////////////////

        /** \copydoc QFExtension::getID() */
        virtual QString getID() const  { return QString("cam_testcamera"); };

        /** \copydoc QFExtension::getName() */
        virtual QString getName() const  { return tr("Test Image Camera Device"); };

        /** \copydoc QFExtension::getDescription() */
        virtual QString getDescription() const  { return tr("outputs configurable test images and pretends to be a camera extension"); };

        /** \copydoc QFExtension::getAuthor() */
        virtual QString getAuthor() const  { return tr("Jan Krieger"); };

        /** \copydoc QFExtension::getCopyright() */
        virtual QString getCopyright() const  { return tr("(c) 2011 by Jan Krieger"); };

        /** \copydoc QFExtension::getWeblink() */
        virtual QString getWeblink() const  { return tr(""); };

        /** \copydoc QFExtension::getIconFilename() */
        virtual QString getIconFilename() const  { return QString(":/cam_testcamera_logo.png"); };
        /** \brief plugin version  */
        virtual void getVersion(int& major, int& minor) const {
            major=1;
            minor=1;
        };

        /** \copydoc QFExtension::deinit() */
        virtual void deinit();

    protected:

        /** \copydoc QFExtensionBase::projectChanged() */
        virtual void projectChanged(QFProject* oldProject, QFProject* project);

        /** \copydoc QFExtensionBase::initExtension() */
        virtual void initExtension();

        /** \copydoc QFExtensionBase::loadSettings() */
        virtual void loadSettings(ProgramOptions* settings);

        /** \copydoc QFExtensionBase::storeSettings() */
        virtual void storeSettings(ProgramOptions* settings);


    /////////////////////////////////////////////////////////////////////////////
    // QFExtensionCamera routines
    /////////////////////////////////////////////////////////////////////////////

    public:


        /** \copydoc QFExtensionCamera::getCameraCount() */
        virtual unsigned int getCameraCount() const;

        /** \copydoc QFExtensionCamera::showCameraSettingsWidget() */
        virtual void showCameraSettingsDialog(unsigned int camera, QSettings& settings, QWidget* parent=NULL);
        /** \copydoc QFExtensionCamera::useCameraSettings() */
        virtual void useCameraSettings(unsigned int camera, const QSettings& settings);
        /** \copydoc QFExtensionCamera::getImageWidth() */
        virtual int getCameraImageWidth(unsigned int camera);
        /** \copydoc QFExtensionCamera::getImageHeight() */
        virtual int getCameraImageHeight(unsigned int camera);
        /** \copydoc QFExtensionCamera::isConnected() */
        virtual bool isCameraConnected(unsigned int camera);
        /** \copydoc QFExtensionCamera::acquire() */
        virtual bool acquireOnCamera(unsigned int camera, uint32_t* data, uint64_t* timestamp=NULL, QMap<QString, QVariant>* parameters=NULL);
        /** \copydoc QFExtensionCamera::connectDevice() */
        virtual bool connectCameraDevice(unsigned int camera);
        /** \copydoc QFExtensionCamera::disconnectDevice() */
        virtual void disconnectCameraDevice(unsigned int camera);
        /** \copydoc QFExtensionCamera::getExposureTime() */
        virtual double getCameraExposureTime(unsigned int camera);
        /** \copydoc QFExtensionCamera::setLogging() */
        virtual void setCameraLogging(QFPluginLogService* logService) { this->logService=logService; };
        /** \copydoc QFExtensionCamera::getPixelWidth() */
        virtual double getCameraPixelWidth(unsigned int camera);
        /** \copydoc QFExtensionCamera::getPixelHeight() */
        virtual double getCameraPixelHeight(unsigned int camera);
        /** \copydoc QFExtensionCamera::getCameraName() */
        virtual QString getCameraName(unsigned int camera);
        /** \copydoc QFExtensionCamera::getCameraSensorName() */
        virtual QString getCameraSensorName(unsigned int camera);

        /** \copydoc QFExtensionCamera::prepareAcquisition() */
        virtual bool prepareCameraAcquisition(unsigned int camera, const QSettings& settings, QString filenamePrefix=QString(""));
        /** \copydoc QFExtensionCamera::startAcquisition() */
        virtual bool startCameraAcquisition(unsigned int camera);
        /** \copydoc QFExtensionCamera::cancelAcquisition() */
        virtual void cancelCameraAcquisition(unsigned int camera);
        /** \copydoc QFExtensionCamera::isAcquisitionRunning() */
        virtual bool isCameraAcquisitionRunning(unsigned int camera);
        /** \copydoc QFExtensionCamera::getAcquisitionDescription() */
        virtual void getCameraAcquisitionDescription(unsigned int camera, QList<QFExtensionCamera::CameraAcquititonFileDescription>* files, QMap<QString, QVariant>* parameters);
        /** \copydoc QFExtensionCamera::getAcquisitionPreview() */
        virtual bool getCameraAcquisitionPreview(unsigned int camera, uint32_t* data);
        /** \copydoc QFExtensionCamera::getAcquisitionProgress() */
        virtual int getCameraAcquisitionProgress(unsigned int camera);


        /** \copydoc QFExtensionCamera::isCameraSettingChangable() */
        virtual bool isCameraSettingChangable(QFExtensionCamera::CameraSetting which) ;
        /** \copydoc QFExtensionCamera::changeCameraSetting() */
        virtual void changeCameraSetting(QSettings& settings, QFExtensionCamera::CameraSetting which, QVariant value);
        /** \copydoc QFExtensionCamera::getCameraSetting() */
        virtual QVariant getCameraSetting(QSettings& settings, QFExtensionCamera::CameraSetting which) ;
        /** \copydoc QFExtensionCamera::getCurrentCameraSetting() */
        virtual QVariant getCameraCurrentSetting(int camera, CameraSetting which) ;
        /** \brief returns \c true if the given CameraSetting is changable by changeCameraSetting() */
        inline virtual bool isCameraSettingChangable(const QString& which)  { return isCameraSettingChangable(QStringToSetting(which)); }
        /*! \brief change the given CameraSetting in the given QSettings object

            \note <b>this will change the contents of your QSettings object, so possibly this changed version is also written back to the harddisk!!!</b>
         */
        inline virtual void changeCameraSetting(QSettings& settings, const QString& which, QVariant value)  {  changeCameraSetting(settings, QStringToSetting(which), value); }
        /** \brief extract the given CameraSetting from the given QSettings object */
        inline virtual QVariant getCameraSetting(QSettings& settings, const QString& which)  { return getCameraSetting(settings, QStringToSetting(which)); }
        /** \brief extract the given CameraSetting from the given camera */
        inline virtual QVariant getCameraCurrentSetting(int camera, const QString& which)  { return getCameraCurrentSetting(camera, QStringToSetting(which)); }

        /** \brief log project text message
         *  \param message the message to log
         */
        virtual void log_text(QString message);
        /** \brief log project warning message
         *  \param message the warning message to log
         */
        virtual void log_warning(QString message);
        /** \brief log project error message
         *  \param message the error message to log
         */
        virtual void log_error(QString message);

    protected:
        QFPluginLogService* logService;
        /** \brief are we connected? */
        bool conn[2];
        /** \brief image counter */
        uint64_t counter[2];
        /** \brief time when device was connected */
        time_t startTime[2];
        /** \brief timer */
        HighResTimer timer[2];
        /** \brief width of the resulting image */
        int width[2];
        /** \brief height of the resulting image */
        int height[2];
        /** \brief type of test pattern */
        int testpattern[2];
        /** \brief noise level 1 = 100% */
        double noise[2];
        /** \brief number of particles in "particle" mode */
        int particleN[2];
        /** \brief number of particles in "particle" mode */
        double particleBackground[2];
        /** \brief x-coordinates of particles */
        int* particleX[2];
        /** \brief y-coordinates of particles */
        int* particleY[2];
        /** \brief average number of photons per step */
        double particleBrightnes[2];
        /** \brief size of the particle PSF (gaussian) */
        double particlePSF[2];
        /** \brief indicate whether there should be some hot pixels */
        int hotpixels[2];
        /** \brief dualview channel brightnes for rhs half, compared to lhs */
        double dualviewBrightness[2];


        void initParticles(int camera, int n);
        void stepParticles(int camera);

        /** \brief how many images to acquire after startAcquisition() */
        int seriesAcquisitions;
        int seriesDelay;
        QString seriesFilenamePrefix[2];

        int seriesCount[2];
        bool seriesRunning[2];
        TIFF* tif[2];

    protected slots:
        void seriesStep1();
        void seriesStep2();

};

#endif // QFECamTestCamera_H
