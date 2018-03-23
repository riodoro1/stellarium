//Copied from AngleMeasure plugin

#include "PushToArduino.hpp"
#include "PushToSerialComm.hpp"
#include "PushToArduinoDialog.hpp"
#include "ui_pushToArduinoDialog.h"

#include "StelApp.hpp"
#include "StelLocaleMgr.hpp"
#include "StelModule.hpp"
#include "StelModuleMgr.hpp"

#include <QSerialPortInfo>
#include <QDebug>
#include <QSettings>

PushToArduinoDialog::PushToArduinoDialog()
	: plugin(NULL)
{
	dialogName = "PushToArduino";
	ui = new Ui_pushToArduinoDialog();
}

PushToArduinoDialog::~PushToArduinoDialog()
{
	delete ui;
}

void PushToArduinoDialog::retranslate()
{
	if (dialog)
	{
		ui->retranslateUi(dialog);
	}
}

void PushToArduinoDialog::createDialogContent()
{
    plugin = GETSTELMODULE(PushToArduino);
    PushToSerialComm *serialComm = plugin->getSerialComm();
    ui->setupUi(dialog);
    
    setButtonsToConnectionState(false);
    refreshSerialPortsList();
    
    connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));
    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));
    
    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(refreshSerialPortsList()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectButtonClicked()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectButtonClicked()));
    connect(ui->reconnectCheckBox, SIGNAL(clicked(bool)), this, SLOT(reconnectCheckBoxClicked(bool)));
    
    connect(serialComm, SIGNAL(disconnected()), this, SLOT(portDisconnected()));
}

void PushToArduinoDialog::show() {
    setVisible(true);
    
    plugin = GETSTELMODULE(PushToArduino);
    PushToSerialComm *serialComm = plugin->getSerialComm();
    
    if ( serialComm->isConnected() ) {
        qDebug()<<serialComm->portName();
        setButtonsToConnectionState(true);
        int index = ui->serialPortComboBox->findText(serialComm->portName());
        if ( index == -1 ) {
            ui->serialPortComboBox->addItem(serialComm->portName());
            index = ui->serialPortComboBox->count() - 1;
        }
        
        ui->serialPortComboBox->setCurrentIndex(index);
    }
    
    QSettings *settings = plugin->getSettings();
    QString reconnectPort = settings->value("PushToArduino/reconnectPort", "").toString();
    
    if ( reconnectPort.length() > 0 ) {
        ui->reconnectCheckBox->setChecked(true);
    }
}

void PushToArduinoDialog::portDisconnected() {
    setButtonsToConnectionState(false);
}

void PushToArduinoDialog::refreshSerialPortsList() {
    ui->serialPortComboBox->clear();

    foreach ( QSerialPortInfo serialPort , QSerialPortInfo::availablePorts()) {
        QString portName = serialPort.portName();
        if ( !portName.startsWith("cu") )
            ui->serialPortComboBox->addItem(serialPort.portName());
    }
}

void PushToArduinoDialog::setButtonsToConnectionState(bool connection) {
    ui->connectButton->setEnabled(!connection);
    ui->disconnectButton->setEnabled(connection);
}

void PushToArduinoDialog::connectButtonClicked() {
    PushToSerialComm *serialComm = plugin->getSerialComm();
    
    int serialPortIndex = ui->serialPortComboBox->currentIndex();
    if ( serialPortIndex == -1 )
        return;
    
    QString serialPortName = ui->serialPortComboBox->itemText(serialPortIndex);
    QString error;
    
    if ( serialComm->openPort(serialPortName, &error) ) {
        setButtonsToConnectionState(true);
        
        if ( ui->reconnectCheckBox->isChecked() ) {
            plugin->getSettings()->setValue("PushToArduino/reconnectPort", serialPortName);
        }
    }
    else {
        setButtonsToConnectionState(false);
        qDebug()<<"Error opening port "<<serialPortName<<": "<<error;
    }
}

void PushToArduinoDialog::disconnectButtonClicked() {
    PushToSerialComm *serialComm = plugin->getSerialComm();
    if(serialComm->isConnected()) {
        serialComm->closePort();
    }
}

void PushToArduinoDialog::reconnectCheckBoxClicked(bool checked) {
    PushToSerialComm *serialComm = plugin->getSerialComm();
    QSettings *settings = plugin->getSettings();
    
    if ( checked ) {
        if ( serialComm->isConnected() ) {
            settings->setValue("PushToArduino/reconnectPort", serialComm->portName());
        }
    }
    else {
        settings->setValue("PushToArduino/reconnectPort", "");
    }
}
