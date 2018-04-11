//
//  PushToArduino.cpp
//  Stellarium PushTo plugin
//
//  Created by Rafał Białek on 03/02/2017.
//  Copyright © 2017 Rafał Białek. All rights reserved.
//

#include "StelProjector.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelLocaleMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelUtils.hpp"
#include "StelPainter.hpp"
#include "StelMovementMgr.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelTextureMgr.hpp"
#include "StelFileMgr.hpp"
#include "PushToArduino.hpp"
#include "PushToSerialComm.hpp"
#include "PushToArduinoDialog.hpp"

#include "PTAHelpers.hpp"

#include <QFontMetrics>
#include <QTimer>
#include <QDebug>

using namespace PTAHelpers;

#define VEC_DIST(x, y) (sqrt(pow(x[0]-y[0],2.0)+pow(x[1]-y[1],2.0)))
#define VEC_SAME(x, y) (VEC_DIST(x, y) < 0.00001)
#define MAX(x, y) ((x>y)?x:y)

/*************************************************************************
 This method is the one called automatically by the StelModuleMgr just
 after loading the dynamic library
 *************************************************************************/
StelModule* PushToArduinoStelPluginInterface::getStelModule() const
{
    return new PushToArduino();
}

StelPluginInfo PushToArduinoStelPluginInterface::getPluginInfo() const
{
    // Allow to load the resources when used as a static plugin
    Q_INIT_RESOURCE(PushToArduino);
    
    StelPluginInfo info;
    info.id = "PushToArduino";
    info.displayedName = "PushTo Arduino";
    info.authors = "Rafal Bialek";
    info.contact = "bialek.rafal@gmail.com";
    info.description = "Connects to a DSC equipped dobsonian telescope interfaced by an Arduino running special firmware. The telescope can then be used as PushTo device.";
    return info;
}

/*************************************************************************
 Constructor
 *************************************************************************/
PushToArduino::PushToArduino()
:manager(NULL)
,setTargetButton(NULL)
,trackTelescopeButton(NULL)
,configureDialog(NULL)
,telescopeAim(0.,0.)
,isMovingToTelescope(false)
,hasAim(false)
,isTracking(false)
{
    setObjectName("PushToArduino");
    serialComm = new PushToSerialComm();
    configureDialog = new PushToArduinoDialog();
    settings = StelApp::getInstance().getSettings();
}

/*************************************************************************
 Destructor
 *************************************************************************/
PushToArduino::~PushToArduino()
{
    delete serialComm;
    delete configureDialog;
}

/*************************************************************************
 Reimplementation of the getCallOrder method
 *************************************************************************/
double PushToArduino::getCallOrder(StelModuleActionName actionName) const
{
    if (actionName==StelModule::ActionDraw)
        return StelApp::getInstance().getModuleMgr().getModule("ConstellationMgr")->getCallOrder(actionName)+1.;
    return 0;
}

bool PushToArduino::configureGui(bool show) {
    if (show)
        configureDialog->show();
    return true;
}

/*************************************************************************
 Init our module
 *************************************************************************/
void PushToArduino::init()
{
    manager = GETSTELMODULE(StelMovementMgr);
    
    try
    {
        StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
        if (gui!=NULL)
        {
            setTargetButton = new StelButton(NULL,
                                             QPixmap(":/pushToArduino/bt_pushto_set_on.png"),
                                             QPixmap(":/pushToArduino/bt_pushto_set_off.png"),
                                             QPixmap(":/graphicGui/glow32x32.png"));
            trackTelescopeButton = new StelButton(NULL,
                                                 QPixmap(":/pushToArduino/bt_pushto_track_on.png"),
                                                 QPixmap(":/pushToArduino/bt_pushto_track_off.png"),
                                                 QPixmap(":/graphicGui/glow32x32.png"));
            
            gui->getButtonBar()->addButton(setTargetButton, "066-pushtoarduinoGroup");
            gui->getButtonBar()->addButton(trackTelescopeButton, "066-pushtoarduinoGroup");
            
            connect(setTargetButton, SIGNAL(toggled(bool)), this, SLOT(setTargetButtonToggled(bool)));
            connect(trackTelescopeButton, SIGNAL(toggled(bool)), this, SLOT(trackTelescopeButtonToggled(bool)));
        }
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: unable create toolbar button for PushTo Arduino plugin: " << e.what();
    }
    
    telescopeMarkerTexture = StelApp::getInstance().getTextureManager().createTexture(":/pushToArduino/pushto_marker.png");
    
    if ( !settings->childGroups().contains("PushToArduino") ) {
        settings->beginGroup("PushToArduino");
    }
    else {
        QString reconnectPort = settings->value("PushToArduino/reconnectPort", "").toString();
        if ( reconnectPort.length() != 0 )
            serialComm->openPort(reconnectPort);
    }
    
    connect(serialComm, SIGNAL(connected()), this, SLOT(portConnected()));
    connect(serialComm, SIGNAL(disconnected()), this, SLOT(portDisconnected()));
    connect(serialComm, SIGNAL(recievedAim(float, float)), this, SLOT(portRecievedAim(float, float)));
}

void PushToArduino::update(double time) {
    Vec2d currentAim = getUserAimAzAlt();
    
    if ( isMovingToTelescope ) {
        movementDuration += time;
        if ( movementDuration > 1.5 ) {
            //timeout in case somebody hijacks the movement
            isMovingToTelescope = false;
            stopTracking();
        }
        if ( VEC_SAME(currentAim, telescopeAim) ) {
            isMovingToTelescope = false;
        }
    }
    else if ( !VEC_SAME(currentAim, telescopeAim) ) {
        //We are not moving to the telescope aim and the position has changed, this means that the user is not longer tracking
        stopTracking();
    }
    
    if ( serialComm->isConnected() && hasAim ) {
        Vec2d raDecTelescopeAim = azAltToJ2000(SDSSToHorizontal(degToRad(telescopeAim)));
        static Vec2d lastRaDecTelescopeAim = Vec2d(0.,0.);
        
        if ( VEC_DIST(lastRaDecTelescopeAim, raDecTelescopeAim) > 0.00003 ) {
            serialComm->sendJ2000(raDecTelescopeAim[0], raDecTelescopeAim[1]);
            lastRaDecTelescopeAim = raDecTelescopeAim;
        }
        
    }
}

/*************************************************************************
 Draw our module. This should draw line in the main window
 *************************************************************************/
void PushToArduino::draw(StelCore* core)
{
    if ( serialComm->isConnected() && hasAim ) {
        //TODO check the flag
        const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz, StelCore::RefractionOff);
        StelPainter painter(prj);
        
        Vec3d radSDSSTelescopeAim;
        radSDSSTelescopeAim = horizontalToSDSS(degToRad(telescopeAim));
        
        Vec3d aim, screen;
        StelUtils::spheToRect(radSDSSTelescopeAim[0], radSDSSTelescopeAim[1], aim);
        prj->project(aim, screen);
        
        float x=screen[0], y=screen[1];
        
        drawTelescopeMarker(&painter, x, y);
        
        StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
        if( omgr->getSelectedObject().count() == 0 )
            drawAimLabels(&painter);
    }
}

void PushToArduino::drawTelescopeMarker(StelPainter *painter, float x, float y) {
    painter->setColor(1.f, 1.f, 1.f, .7f);
    
    painter->setBlending(true);
    telescopeMarkerTexture->bind();
    
    painter->drawSprite2dMode(x, y, markerRadius);
}

void PushToArduino::drawAimLabels(StelPainter *painter) {
    double scale = painter->getProjector()->getDevicePixelsPerPixel();
    int viewportHeight = painter->getProjector()->getViewportHeight();
    
    Vec2d radTelescopeAim = degToRad(telescopeAim);
    Vec2d j2000Aim = azAltToJ2000(SDSSToHorizontal(radTelescopeAim));
    
    QFont font;
    font.setPixelSize(13);
    painter->setFont(font);
    QFontMetrics metrics = painter->getFontMetrics();
    
    QString altAzString = QString("Az/Alt: ") + StelUtils::radToDmsStr(radTelescopeAim[0]) + "/" + StelUtils::radToDmsStr(radTelescopeAim[1]);
    QString j2000String = QString("Ra/Dec: ") + StelUtils::radToHmsStr(j2000Aim[0]) + "/" + StelUtils::radToDmsStr(j2000Aim[1]);
    QString errorString = QString(tr("Measurement error, please realign."));
    
    int textHeight = metrics.tightBoundingRect(altAzString).size().height();

    painter->drawText(0 + (13.*scale), viewportHeight - (13.+textHeight) * scale , altAzString);
    painter->drawText(0 + (13.*scale), viewportHeight - (15.+2.*textHeight) * scale , j2000String);
    if ( serialComm->deviceReportsError() ) {
        painter->drawText(0 + (13.*scale), viewportHeight - (17.+3.*textHeight) * scale , errorString);
    }
}

void PushToArduino::portConnected() {
    hasAim = false;
}

void PushToArduino::portDisconnected() {
    hasAim = false;
    isTracking = false;
    stopTracking();
}

void PushToArduino::portRecievedAim(float azi, float alt) {
    telescopeAim = radToDeg(Vec2d(azi, alt));
    if ( isTracking && !isMovingToTelescope ) {
        setUserAimAzAlt(telescopeAim, 0.0);
    }
}

void PushToArduino::unCheckSetTargetButton() {
    setTargetButton->setChecked(false);
}

void PushToArduino::unCheckTrackTelescopeButton() {
    trackTelescopeButton->setChecked(false);
}

void PushToArduino::setTargetButtonToggled(bool state) {
    if (state) {
        if ( serialComm->isConnected() ) {
            QTimer::singleShot(100, this, SLOT(unCheckSetTargetButton()));
            setCurrentTelescopeAim();
        }
        else {
            unCheckSetTargetButton();
        }
    }
}

void PushToArduino::trackTelescopeButtonToggled(bool state) {
    if (state) {
        if ( serialComm->isConnected() && hasAim )
            startTracking();
        else
            unCheckTrackTelescopeButton();
    }
    else {
        stopTracking();
    }
}

void PushToArduino::moveToTelescope() {
    if ( hasAim ) {
        isMovingToTelescope = true;
        movementDuration = 0.0;
    
        manager->setFlagLockEquPos(false);
        manager->setFlagTracking(false);
        setUserAimAzAlt(telescopeAim);
    }
}

Vec2d PushToArduino::getUserAimRaDec() {
    Vec3d currentAim = manager->getViewDirectionJ2000();
    
    double ra, dec;
    StelUtils::rectToSphe(&ra, &dec, currentAim);
    Vec2d userAim(ra, dec);
    
    return userAim;
}

Vec2d PushToArduino::getUserAimAzAlt() {
    Vec3d currentAim = StelApp::getInstance().getCore()->j2000ToAltAz(manager->getViewDirectionJ2000(), StelCore::RefractionOff);
    Vec2d azAlt;
    StelUtils::rectToSphe(&azAlt[0], &azAlt[1], currentAim);
    azAlt = radToDeg(SDSSToHorizontal(azAlt));
    return azAlt;
}

void PushToArduino::setUserAimAzAlt(Vec2d aim, double movementDuration) {
    manager->setEquatorialMount(false);
    aim = horizontalToSDSS(degToRad(aim));
    Vec3d rect; StelUtils::spheToRect(aim[0], aim[1], rect);
    manager->moveToAltAzi(rect, Vec3d(0.,0.,1.), movementDuration);
}

void PushToArduino::setCurrentTelescopeAim() {
    if ( serialComm->isConnected() ) {
        telescopeAim = getUserAimAzAlt();
        hasAim = true;
    
        serialComm->sendAim(degToRad(telescopeAim[0]), degToRad(telescopeAim[1]));
    }
}

void PushToArduino::startTracking() {
    trackTelescopeButton->setChecked(true);
    moveToTelescope();
    isTracking = true;
}

void PushToArduino::stopTracking() {
    unCheckTrackTelescopeButton();
    isTracking = false;
}
