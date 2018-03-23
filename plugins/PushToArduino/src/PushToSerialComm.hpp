//
//  PushToSerialComm.hpp
//  Stellarium PushTo plugin
//
//  Created by Rafał Białek on 05/02/2017.
//  Copyright © 2017 Rafał Białek. All rights reserved.
//

#ifndef PushToSerialComm_hpp
#define PushToSerialComm_hpp

#include <QObject>
#include <QString>
#include <QByteArray>

class QSerialPort;
class PushToArduino;

class PushToSerialComm : public QObject {
    Q_OBJECT
    
    QSerialPort *serialPort;
    QByteArray serialBuffer;
    bool deviceError;
    
public:
    PushToSerialComm(QObject *parent = 0);
    
    bool openPort(QString portName, QString *err = NULL);
    void closePort();
    QString portName();
    bool isConnected();
    
    bool deviceReportsError();
    
signals:
    void recievedAim(float azi, float alt);
    void connected();
    void connecteionFailed(QString errorMessage);
    void disconnected();
    
public slots:
    void sendAim(float azi, float alt);
    void sendJ2000(float ra, float dec);
    
private slots:
    void serialReadyToRead();
    
private:
    void resetDeviceError();
    void processRecieved(QList<QByteArray> &parts);
};


#endif /* PushToSerialComm_hpp */
