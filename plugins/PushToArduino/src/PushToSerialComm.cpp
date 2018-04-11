//
//  PushToSerialComm.cpp
//  Stellarium PushTo plugin
//
//  Created by Rafał Białek on 05/02/2017.
//  Copyright © 2017 Rafał Białek. All rights reserved.
//

#include "PushToArduino.hpp"
#include "PushToSerialComm.hpp"
#include <QSerialPort>
#include <QDebug>

#define MAGIC_NUMBER 0x79   //at the end of every message

PushToSerialComm::PushToSerialComm(QObject *parent)
:QObject(parent)
{
    serialPort = new QSerialPort(this);
}

bool PushToSerialComm::openPort(QString portName, QString *err) {
    serialPort->setPortName(portName);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    resetDeviceError();
    
    if ( serialPort->open(QIODevice::ReadWrite) ) {
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(serialReadyToRead()));
        emit(connected());
        return true;
    }
    else {
        if ( err != NULL ) {
            (*err) = serialPort->errorString();
        }
        emit(connecteionFailed(serialPort->errorString()));
    }
    return false;
}

void PushToSerialComm::closePort() {
    if ( serialPort->isOpen() ) {
        serialPort->close();
        emit(disconnected());
    }
}

QString PushToSerialComm::portName() {
    return serialPort->portName();
}

bool PushToSerialComm::isConnected() {
    return serialPort->isOpen();
}

void PushToSerialComm::sendAim(float azi, float alt) {
    if( serialPort->isOpen() ) {
        QByteArray data;
        data.append('U');
        data.append(reinterpret_cast<const char*>(&azi), sizeof(float));
        data.append(reinterpret_cast<const char*>(&alt), sizeof(float));
        data.append(MAGIC_NUMBER);
        Q_ASSERT(data.size() == 10);
        
        PTA_DEBUG_PRINT("Sending update: ");
        PTA_DEBUG_PRINT(data.toHex());
        PTA_DEBUG_PRINT(*reinterpret_cast<const float*>(data.mid(1,4).constData()));
        PTA_DEBUG_PRINT(*reinterpret_cast<const float*>(data.right(4).constData()));

        
        serialPort->write(data);
        
        resetDeviceError();
    }
}

void PushToSerialComm::sendJ2000(float ra, float dec) {
    if( serialPort->isOpen() ) {
        QByteArray data;
        data.append('J');
        data.append(reinterpret_cast<const char*>(&ra), sizeof(float));
        data.append(reinterpret_cast<const char*>(&dec), sizeof(float));
        data.append(MAGIC_NUMBER);
        Q_ASSERT(data.size() == 10);
        
        //PTA_DEBUG_PRINT("Sending J2000: ");
        //PTA_DEBUG_PRINT(data.toHex());
        //PTA_DEBUG_PRINT(*reinterpret_cast<const float*>(data.mid(1,4).constData()));
        //PTA_DEBUG_PRINT(*reinterpret_cast<const float*>(data.right(4).constData()));
        
        serialPort->write(data);
    }
}

void PushToSerialComm::processRecieved(QList<QByteArray> &responses) {
    QByteArray lastResponse = responses.last();
    char code = lastResponse.at(0);
    QByteArray aziData = lastResponse.mid(1,4);
    QByteArray altData = lastResponse.mid(5,8);
    char magic = lastResponse.at(9);
    
    float azi = *reinterpret_cast<const float*>(aziData.constData());
    float alt = *reinterpret_cast<const float*>(altData.constData());
    
    if ( magic != MAGIC_NUMBER || ( azi == azi && alt == alt ) ) {
        deviceError = (code == 'E') ? true : false;
        emit(recievedAim(azi, alt));
    }
}

void PushToSerialComm::serialReadyToRead() {
    serialBuffer.append(serialPort->readAll());
    QList<QByteArray> responses; //10-byte chunks
    
    while ( serialBuffer.size() >= 10 ) {
        responses.append(serialBuffer.left(10));
        serialBuffer = serialBuffer.right(serialBuffer.size() - 10);
    }
    
    if ( responses.length() > 0 ) {
        processRecieved(responses);
    }
}

bool PushToSerialComm::deviceReportsError() {
    return deviceError;
}

void PushToSerialComm::resetDeviceError() {
    deviceError = false;
}
