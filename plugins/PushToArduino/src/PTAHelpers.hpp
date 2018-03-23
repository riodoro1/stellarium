//
//  PTAHelpers.h
//  Stellarium PushTo plugin
//
//  Created by Rafał Białek on 08/02/2017.
//  Copyright © 2017 Rafał Białek. All rights reserved.
//

#ifndef PTAHelpers_h
#define PTAHelpers_h

#include "VecMath.hpp"
#include "StelUtils.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"

namespace PTAHelpers {
    
    Vec2d SDSSToHorizontal(Vec2d SDSS);
    Vec2d horizontalToSDSS(Vec2d horizontal);
    
    double radToDeg(double rad);
    Vec2d radToDeg(Vec2d radv);
    
    double degToRad(double dec);
    Vec2d degToRad(Vec2d decv);
    
    Vec2d j2000ToAzAlt(Vec2d J2000);
    Vec2d azAltToJ2000(Vec2d azAlt);
    
}

Vec2d PTAHelpers::SDSSToHorizontal(Vec2d SDSS) {
    //This method works on radians.
    SDSS[0] = M_PI - SDSS[0];
    return SDSS;
}

Vec2d PTAHelpers::horizontalToSDSS(Vec2d horizontal) {
    return SDSSToHorizontal(horizontal);  //The function is reversible
}

double PTAHelpers::radToDeg(double rad) {
    return rad*180./M_PI;
}

Vec2d PTAHelpers::radToDeg(Vec2d radv) {
    return Vec2d(radToDeg(radv[0]), radToDeg(radv[1]));
}

double PTAHelpers::degToRad(double dec) {
    return dec*M_PI/180.;
}

Vec2d PTAHelpers::degToRad(Vec2d decv) {
    return Vec2d(degToRad(decv[0]), degToRad(decv[1]));
}

//Takes argument in radians and gives radians back
Vec2d PTAHelpers::j2000ToAzAlt(Vec2d j2000) {
    Vec3d rectJ2000;
    StelUtils::spheToRect(j2000[0], j2000[1], rectJ2000);
    Vec3d rectAzAlt = StelApp::getInstance().getCore()->j2000ToAltAz(rectJ2000, StelCore::RefractionOff);
    Vec2d spheAzAlt;
    StelUtils::rectToSphe(&spheAzAlt[0], &spheAzAlt[1], rectAzAlt);
    return spheAzAlt;
}

//Takes argument in radians (SDSS) and gives radians back
Vec2d PTAHelpers::azAltToJ2000(Vec2d azAlt) {
    Vec3d rectAzAlt;
    StelUtils::spheToRect(azAlt[0], azAlt[1], rectAzAlt);
    Vec3d rectJ2000 = StelApp::getInstance().getCore()->altAzToJ2000(rectAzAlt, StelCore::RefractionOff);
    Vec2d spheJ2000;
    StelUtils::rectToSphe(&spheJ2000[0], &spheJ2000[1], rectJ2000);
    return spheJ2000;
}

#endif /* PTAHelpers_h */
