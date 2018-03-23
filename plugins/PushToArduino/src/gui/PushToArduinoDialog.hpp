//Copied from AngleMeasure plugin

#ifndef _PUSHTOARDUINODIALOG_HPP_
#define _PUSHTOARDUINODIALOG_HPP_

#include "StelDialog.hpp"
#include "StelGui.hpp"
#include "StelTranslator.hpp"

#include <QString>

class Ui_pushToArduinoDialog;
class PushToArduino;

class PushToArduinoDialog : public StelDialog
{
	Q_OBJECT

public:
	PushToArduinoDialog();
	~PushToArduinoDialog();

public slots:
	void retranslate();
    void setButtonsToConnectionState(bool connection);
    
    void show();
    
    void refreshSerialPortsList();
    
    void connectButtonClicked();
    void disconnectButtonClicked();
    
    void portDisconnected();
    
    void reconnectCheckBoxClicked(bool checked);

protected:
	void createDialogContent();

private:
	Ui_pushToArduinoDialog* ui;
	PushToArduino* plugin;
};

#endif /* _PUSHTOARDUINODIALOG_HPP_ */
