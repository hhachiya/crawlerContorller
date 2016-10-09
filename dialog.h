#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

// joystick thread
#include "joythread.h"

// serial connection
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

// headers for web view
#include <QWebEngineView>

// for config file loading
#include <QSettings>

// headers for capturing camera image
#include <QCamera>



namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    // slots for printing joystick values
    void printJoyValue(const QString &x,const QString &y,const QString &z,const QString &r);


    /*
    // slots for printing encoder values
    void printEncoderValue(const QStringList &enc);
    */

    // slots for printing motor power values
    void printPowerValue(const QStringList &pow);

    // slots for changing and reseting motor ids
    void on_motorChangePB_clicked();
    void on_motorResetPB_clicked();

    void on_disconnectPB_clicked();
    void on_connectServerPB_clicked();
    void on_connectDevicePB_clicked();

private:
    Ui::Dialog *ui;
    JoyThread myJoyThread;              // joystick thread with id 0

    // socket
    QString ipAddress;                  // IP address used for TCP socket
    quint16 controlPort;                // TCP port for control
    quint16 videoPort;                  // TCP port for video
    QTcpSocket *tcpSocket;              // TCP socket


    // device
    RoboteqDevice *roboteqDevice;        // RoboteQ device
    QString roboteqDevicePort;          // COM port for RoboteQ device

    // motor list
    QStringList leftMotorList;          // left motor ID string
    QStringList rightMotorList;         // right motor ID string
    QStringList indi1MotorList;         // individual motor ID string, group 1
    QStringList indi2MotorList;         // individual motor ID string, group 2
    int numMotors;                      // number of motors

    // config
    QString configPath = ".\\test.ini"; // config file path

    // visiable flag
    int serverEnabled;
    int roboteqDeviceEnabled;

    // display motor ids
    void displayMotorIDs();

    // camera interface
    QCamera *camera;
};

#endif // DIALOG_H
