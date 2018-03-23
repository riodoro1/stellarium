//
//  PushToArduino.hpp
//  Stellarium PushTo plugin
//
//  Created by Rafał Białek on 03/02/2017.
//  Copyright © 2017 Rafał Białek. All rights reserved.
//

#ifndef PushToArduino_hpp
#define PushToArduino_hpp

#include <QFont>
#include <QObject>
#include <QSettings>
#include "VecMath.hpp"
#include "StelModule.hpp"
#include "StelPluginInterface.hpp"
#include "StelTextureTypes.hpp"

#define PTA_DEBUG_EN
#ifdef PTA_DEBUG_EN
#   define PTA_DEBUG(x) x
#   define PTA_DEBUG_PRINT(x) qDebug()<<x;
#else
#   define PTA_DEBUG(x)
#   define PTA_DEBUG_PRINT(x)
#endif

class PushToSerialComm;
class PushToArduinoDialog;
class StelButton;
class StelMovementMgr;
class StelPainter;

class PushToArduino : public StelModule
{
    Q_OBJECT
    
public:
    PushToArduino();
    virtual ~PushToArduino();
    
    ///////////////////////////////////////////////////////////////////////////
    // Methods defined in the StelModule class
    virtual void init();
    virtual void update(double);
    virtual void draw(StelCore* core);
    virtual double getCallOrder(StelModuleActionName actionName) const;
    virtual bool configureGui(bool show=true);
    
    PushToSerialComm *getSerialComm() {return serialComm;}
    QSettings *getSettings() {return settings;}
    
private slots:
    void setTargetButtonToggled(bool state);
    void trackTelescopeButtonToggled(bool state);
    void unCheckSetTargetButton();
    void unCheckTrackTelescopeButton();
    
    void portConnected();
    void portDisconnected();
    void portRecievedAim(float azi, float alt);
    
private:
    double markerRadius = 13.;
    
    PushToSerialComm *serialComm;
    StelMovementMgr *manager;
    StelTextureSP telescopeMarkerTexture;
    StelButton *setTargetButton;
    StelButton *trackTelescopeButton;
    PushToArduinoDialog *configureDialog;
    QSettings *settings;
    
    Vec2d telescopeAim;                                                // current telescope aim, {Az, Alt} in degrees
    bool hasAim;                                                        // flag saying if the aim has been set
    bool isTracking = false;                                            // flag saying if we are tracking
    
    bool isMovingToTelescope;                                           // set whenever the stellarium is currently moving to the telescope aim
    double movementDuration;                                            // how long we have been moving
    
    Vec2d getUserAimAzAlt();                                            // gets current aim in altazimuth coordinate
    void setUserAimAzAlt(Vec2d aim, double movementDuration=1.0);       // sets the current aim of the user in altazimuth coordinates
    Vec2d getUserAimRaDec();
    
    void drawTelescopeMarker(StelPainter *painter, float x, float y);   // draws the marker of the telescopes aim in screen coordinates
    void drawAimLabels(StelPainter *painter);                           // draws the labels indicating curent telescopes alt/az aim
    
    void moveToTelescope();                                             // sets the user aim to where the telescope is looking
    void setCurrentTelescopeAim();                                      // sets the telescope aim to where the user is looking
    
    void startTracking();                                               // moves to the telescope and tracks its movements
    void stopTracking();                                                // stops tracking telescopes movements
};

//! This class is used by Qt to manage a plug-in interface
class PushToArduinoStelPluginInterface : public QObject, public StelPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID StelPluginInterface_iid)
    Q_INTERFACES(StelPluginInterface)
public:
    virtual StelModule* getStelModule() const;
    virtual StelPluginInfo getPluginInfo() const;
    virtual QObjectList getExtensionList() const { return QObjectList(); }
};

#endif /* PushToArduino_hpp */
