#ifndef JOYTHREAD_H
#define JOYTHREAD_H

#include <QThread>

// for joystick
#include <windows.h>
#include <mmsystem.h>

// for socket
#include <QTcpSocket>
#include <QHostAddress>

// for Roboteq driver
#include "RoboteqDevice.h"
#include "Constants.h"
#include "ErrorCodes.h"

#define SERVER_MODE 1
#define DEVICE_MODE 2

// inheritance of QThread to get joystick position
class JoyThread : public QThread
{
    Q_OBJECT

public:
    quint8 joyId;                       // joystick id
    QString comPort;                    // COM port
    double joyMidValue = 32767;         // middle joystick value of each axis
    double motorMaxValue = 1000;        // maximum value of RoboteQ motor command
    double motorMinValue = -1000;       // minimum value of RoboteQ motor command
    double throttleMaxRotValue = 100;   // maximum joystick value for robot's rotation
    int numMotors;                      // number of motors

    JoyThread();                    // constructor
    ~JoyThread();                   // destructor
    void stop();                    // to stop thread
    void startJoyThread(int JoyId, QStringList leftMotors, QStringList rightMotors, QStringList indiMotors1, QStringList indiMotors2, int numMotors);
    void startServerJoyThread(int JoyId, QTcpSocket* mySocket, QStringList leftMotors, QStringList rightMotors, QStringList indiMotors1, QStringList indiMotors2, int numMotors);
    void startDeviceJoyThread(int JoyId, RoboteqDevice* myDevice, QStringList leftMotors, QStringList rightMotors, QStringList indiMotors1, QStringList indiMotors2, int numMotors);
    void changeMotorIDs(QStringList leftMotorList, QStringList rightMotorList, QStringList indi1MotorList, QStringList indi2MotorList);

    int connectMode;                // switch connection server or driver

signals:
    // signal for sending joystick value
    void printJoyValue(const QString &x, const QString &y, const QString &z, const QString &r, const QString &bt);

    // signal for sending encoder value
    void printEncoderValue(const QStringList enc);

    // signal for sending motor power value
    void printPowerValue(const QStringList pow);

    // signal for sending motor alive motor list
    void setAliveMotors(const QStringList aliveMotorList);

protected:
    void run(); // running process

private:
    volatile bool runFlag;              // flag for thread running (volatile: surpress optimization of compile)
    JOYINFOEX joyInfo;                  // joystick interface
    QTcpSocket* mySocket;               // TCP socket
    RoboteqDevice* myDevice;            // RoboteQ device
    QStringList leftMotorList;          // left motor ids
    QStringList rightMotorList;         // right motor ids
    QStringList indi1MotorList;         // individual motor ids, group 1
    QStringList indi2MotorList;         // individual motor ids, group 2
    QStringList aliveMotorList;         // alive motor ID string

    // limit value in then range between min and max
    double JoyThread::limitValue(double value,double min,double max);

    // send/receive command to/from the server or driver
    QByteArray SetCommandSocket(QString cmd, quint8 id, int val);
    QByteArray GetValueSocket(QString cmd, quint8 id);
    QByteArray SetCommandDevice(QString cmd, quint8 id, int val);
    QByteArray GetValueDevice(QString cmd, quint8 id);
    QByteArray SetCommand(QString cmd, quint8 id, int val, int mode);
    QByteArray GetValue(QString cmd, quint8 id, int mode);

};
#endif // JOYTHREAD_H
