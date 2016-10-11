#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{    
    ui->setupUi(this);

    //----------------------
    // load parameter config
    QString tmp;
    QSettings settings(configPath,QSettings::IniFormat);

    // motor section
    settings.beginGroup("motor");
    numMotors = settings.value("numMotors","6").toInt();
    tmp = settings.value("leftMotors","1").toString();
    leftMotorList = tmp.split(",");
    tmp = settings.value("rightMotors","2").toString();
    rightMotorList = tmp.split(",");
    tmp = settings.value("indi1Motors","2").toString();
    indi1MotorList = tmp.split(",");
    tmp = settings.value("indi2Motors","2").toString();
    indi2MotorList = tmp.split(",");
    settings.endGroup();

    // server section
    settings.beginGroup("server");
    ipAddress = settings.value("ipAddress","127.0.0.1").toString();
    controlPort = quint16(settings.value("controlPort","7777").toInt());
    videoPort = quint16(settings.value("videoPort","8081").toInt());
    settings.endGroup();

    // device section
    settings.beginGroup("roboteqDevice");
    roboteqDeviceEnabled = settings.value("isEnabled","0").toInt();
    qDebug() << roboteqDeviceEnabled;
    roboteqDevicePort = settings.value("devicePort","com4").toString();
    settings.endGroup();
    //----------------------

    //======================
    // display at GUI

    // motor ids
    displayMotorIDs();

    //----------------------
    // server
    ui->ipAddrText->setText(ipAddress);         // ip address
    ui->controlPortText->setText(QString::number(controlPort)); // control port number
    ui->videoPortText->setText(QString::number(videoPort)); // video port number
    ui->webView->load(QUrl("http://"+ipAddress+":"+QString::number(videoPort)));    // set web browser
    //----------------------
    //======================

    //----------------------
    // JOYSTICK
    // check joystick availability and add corresponding item to joyComboBox
    JOYCAPS joyCaps;

    for(unsigned int i=0;i<joyGetNumDevs();i++){
        if(JOYERR_NOERROR == joyGetDevCaps(i, &joyCaps,sizeof(joyCaps))){
            ui->joyComboBox->addItem("Joypad"+QString::number(i));
        }
    }

    // connect JoyThread::printJoyValue to Dialog::printJoyValue
    connect(&myJoyThread, &JoyThread::printJoyValue, this, &Dialog::printJoyValue);

    /*
    // connect JoyThread::printEncoderValue to Dialog::printEncoderValue
    connect(&myJoyThread, &JoyThread::printEncoderValue, this, &Dialog::printEncoderValue);
    */

    // connect JoyThread::printPowerValue to Dialog::printPowerValue
    connect(&myJoyThread, &JoyThread::printPowerValue, this, &Dialog::printPowerValue);
    //----------------------

    //----------------------
    // roboteqDevice
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ui->serialComboBox->addItem(info.portName());
    }

    //----------------------
    // set the viewfinder of camera and start to display image
    camera = new QCamera;
    camera->setViewfinder(ui->viewfinder);
    camera->start();
    //----------------------

    //----------------------
    // disable disconnect button
    ui->disconnectDevicePB->setVisible(false);
    ui->disconnectServerPB->setVisible(false);
    ui->connectIndi->setStyleSheet("background-color:gray;");
    //----------------------

}

void Dialog::printJoyValue(const QString &x, const QString &y, const QString &z, const QString &r)
{
    ui->joyXText->setText(x);
    ui->joyYText->setText(y);
    ui->joyZText->setText(z);
    ui->joyRText->setText(r);
}

/*
void Dialog::printEncoderValue(const QStringList enc)
{
    for(QString encVal: enc)
        ui->Enc1Text->setText(encVal);
}
*/

void Dialog::printPowerValue(const QStringList &pow)
{
    //-------------------
    // active motor ids
    QList<QWidget *> activeMotorWidgets = ui->activeMotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));

    // widget color change
    int index=0;
    for(QString powVal: pow){
        qDebug() << "powVal=" << powVal;
        if(abs(powVal.toInt()) > 0)
            activeMotorWidgets[index]->setStyleSheet("background-color:red;");
        else
            activeMotorWidgets[index]->setStyleSheet("background-color:light grey;");
        index++;
    }
}

Dialog::~Dialog()
{
    delete ui;
}



void Dialog::on_motorChangePB_clicked()
{
    QString motorID;

    //-------------------
    // left motor ids
    leftMotorList.clear();
    QList<QWidget *> leftMotorWidgets = ui->leftMotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> leftMotorCBs = ui->leftMotorGB->findChildren<QCheckBox *>();
    QString leftMotorIDs = "";

    // widget reset
    for(int i=0;i<leftMotorWidgets.size();i++)
        leftMotorWidgets[i]->setStyleSheet("background-color:light gray;");

    // set list, check box, widget
    for(int i=0;i<leftMotorCBs.size();i++){
        if(leftMotorCBs.at(i)->isChecked()){
            motorID = QString::number(i+1);

            // motor list
            leftMotorList.append(motorID);

            // config parameter string
            if(leftMotorIDs.length()==0)
                leftMotorIDs = motorID;
            else
                leftMotorIDs = leftMotorIDs + "," + motorID;

            // widget
            leftMotorWidgets[i]->setStyleSheet("background-color:blue;");
        }
    }

    //-------------------

    //-------------------
    // right motor ids
    rightMotorList.clear();
    QList<QWidget *> rightMotorWidgets = ui->rightMotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> rightMotorCBs = ui->rightMotorGB->findChildren<QCheckBox *>();
    QString rightMotorIDs = "";

    // widget reset
    for(int i=0;i<rightMotorWidgets.size();i++)
        rightMotorWidgets[i]->setStyleSheet("background-color:light gray;");

    // set list, check box, widget
    for(int i=0;i<rightMotorCBs.size();i++){
        if(rightMotorCBs.at(i)->isChecked()){
            qDebug() << i;
            motorID = QString::number(i+1);

            // motor list
            rightMotorList.append(motorID);

            // config parameter string
            if(rightMotorIDs.length()==0)
                rightMotorIDs = motorID;
            else
                rightMotorIDs = rightMotorIDs + "," + motorID;

            // widget
            rightMotorWidgets[i]->setStyleSheet("background-color:blue;");
        }
    }
    //-------------------

    //-------------------
    // individual motor ids, group1
    indi1MotorList.clear();
    QList<QWidget *> indi1MotorWidgets = ui->indi1MotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> indi1MotorCBs = ui->indi1MotorGB->findChildren<QCheckBox *>();
    QString indi1MotorIDs;

    // widget reset
    for(int i=0;i<indi1MotorWidgets.size();i++)
        indi1MotorWidgets[i]->setStyleSheet("background-color:light gray;");

    // set list, check box, widget
    for(int i=0;i<indi1MotorCBs.size();i++){
        if(indi1MotorCBs.at(i)->isChecked()){
            motorID = QString::number(i+1);

            // motor list
            indi1MotorList.append(motorID);

            // config parameter string
            if(indi1MotorIDs.length()==0)
                indi1MotorIDs = motorID;
            else
                indi1MotorIDs = indi1MotorIDs + "," + motorID;

            // widget
            indi1MotorWidgets[i]->setStyleSheet("background-color:blue;");
        }
    }
    //-------------------

    //-------------------
    // individual motor ids, group2
    indi2MotorList.clear();
    QList<QWidget *> indi2MotorWidgets = ui->indi2MotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> indi2MotorCBs = ui->indi2MotorGB->findChildren<QCheckBox *>();
    QString indi2MotorIDs;

    // widget reset
    for(int i=0;i<indi2MotorWidgets.size();i++)
        indi2MotorWidgets[i]->setStyleSheet("background-color:light gray;");

    // set list, check box, widget
    for(int i=0;i<indi2MotorCBs.size();i++){
        if(indi2MotorCBs.at(i)->isChecked()){
            motorID = QString::number(i+1);

            // motor list
            indi2MotorList.append(motorID);

            // config parameter string
            if(indi2MotorIDs.length()==0)
                indi2MotorIDs = motorID;
            else
                indi2MotorIDs = indi2MotorIDs + "," + motorID;

            // widget
            indi2MotorWidgets[i]->setStyleSheet("background-color:blue;");
        }
    }
    //-------------------

    // change motor ids
    myJoyThread.changeMotorIDs(leftMotorList,rightMotorList,indi1MotorList, indi2MotorList);


    //----------------------
    // save parameter config
    QSettings settings(configPath,QSettings::IniFormat);

    // motor section
    settings.beginGroup("motor");
    settings.setValue("leftMotors",leftMotorIDs);
    settings.setValue("rightMotors",rightMotorIDs);
    settings.setValue("indi1Motors",indi1MotorIDs);
    settings.setValue("indi2Motors",indi2MotorIDs);
    //----------------------
}

void Dialog::on_motorResetPB_clicked()
{
    // displaying motor ids
    displayMotorIDs();
}

void Dialog::displayMotorIDs()
{
    //----------------------
    // left motor id, widget, checkbox
    QList<QWidget *> leftMotorWidgets = ui->leftMotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> leftMotorCBs = ui->leftMotorGB->findChildren<QCheckBox *>();

    // reset checkbox
    for(int i=0;i<leftMotorCBs.size();i++)
        leftMotorCBs[i]->setChecked(false);

    // set checkbox and widget
    if(leftMotorList.size())
       for(QString id: leftMotorList){
            leftMotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
            leftMotorCBs[id.toInt()-1]->setChecked(true);
       }
    //----------------------

    //----------------------
    // right motor id, widget, checkbox
    QList<QWidget *> rightMotorWidgets = ui->rightMotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> rightMotorCBs = ui->rightMotorGB->findChildren<QCheckBox *>();

    // reset checkbox
    for(int i=0;i<rightMotorCBs.size();i++)
        rightMotorCBs[i]->setChecked(false);

    // set checkbox and widget
    if(rightMotorList.size())
        for(QString id: rightMotorList){
            rightMotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
            rightMotorCBs[id.toInt()-1]->setChecked(true);
        }
    //----------------------

    //----------------------
    // individual motor id, group1, widget, checkbox
    QList<QWidget *> indi1MotorWidgets = ui->indi1MotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> indi1MotorCBs = ui->indi1MotorGB->findChildren<QCheckBox *>();

    // reset checkbox
    for(int i=0;i<indi1MotorCBs.size();i++)
        indi1MotorCBs[i]->setChecked(false);

    // set checkbox and widget
    if(indi1MotorList.size())
        for(QString id: indi1MotorList){
            indi1MotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
            indi1MotorCBs[id.toInt()-1]->setChecked(true);
        }
    //----------------------

    //----------------------
    // individual motor id, group2, widget, checkbox
    QList<QWidget *> indi2MotorWidgets = ui->indi2MotorGB->findChildren<QWidget *>(QRegularExpression(".*Widget"));
    QList<QCheckBox *> indi2MotorCBs = ui->indi2MotorGB->findChildren<QCheckBox *>();

    // reset checkbox
    for(int i=0;i<indi2MotorCBs.size();i++)
        indi2MotorCBs[i]->setChecked(false);

    // set checkbox and widget
    if(indi2MotorList.size())
        for(QString id: indi2MotorList){
            indi2MotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
            indi2MotorCBs[id.toInt()-1]->setChecked(true);
        }
    //----------------------
}



void Dialog::on_connectServerPB_clicked()
{
    //----------------------
    // Socket
    tcpSocket = new QTcpSocket;
    tcpSocket->connectToHost(ipAddress,controlPort);
    bool status=tcpSocket->waitForConnected();
    //----------------------

    if (!status){
        qDebug("Error connecting to server: %d.\n",status);
        ui->connectStateText->setText(QString("Error connecting to server:").append(QString::number(status)));
    }else{
        tcpSocket->moveToThread(&myJoyThread); //tcpSocket in main thread is moved to JoyThread (could be accessed from run())

        // update connect status
        ui->connectStateText->setText(QString("Connected to server"));
        ui->connectIndi->setStyleSheet("background-color:blue;");

        //----------------------
        // Thread
        // stop joystick thread
        if(myJoyThread.isRunning())
            myJoyThread.stop();

        // start joystick thread with joystick id
        myJoyThread.startServerJoyThread(ui->joyComboBox->currentIndex(),tcpSocket,leftMotorList,rightMotorList,indi1MotorList,indi2MotorList,numMotors);
        //----------------------

        // enable disconnect button and disable connect buttons
        ui->connectServerPB->setVisible(false);
        ui->disconnectServerPB->setVisible(true);
        ui->connectDevicePB->setVisible(false);
        ui->disconnectDevicePB->setVisible(false);
    }
}


void Dialog::on_connectDevicePB_clicked()
{
    //----------------------
    // RoboteQ
    roboteqDevice = new RoboteqDevice;
    roboteqDevicePort = ui->serialComboBox->currentText();
    int status = roboteqDevice->Connect(QString("\\\\.\\").append(roboteqDevicePort).toStdString());

    if (status != RQ_SUCCESS)
    {
        qDebug("Error connecting to device: %d.\n",status);
        ui->connectStateText->setText(QString("Error connecting to driver:").append(QString::number(status)));
    }else{
        // stop joystick thread
        if(myJoyThread.isRunning())
            myJoyThread.stop();

        // start joystick thread with joystick id
        myJoyThread.startDeviceJoyThread(ui->joyComboBox->currentIndex(),roboteqDevice,leftMotorList,rightMotorList,indi1MotorList,indi2MotorList,numMotors);
        //----------------------

        // update connect status
        ui->connectStateText->setText(QString("Connected to driver"));
        ui->connectIndi->setStyleSheet("background-color:blue;");

        // enable disconnect button and disable connect buttons
        ui->connectServerPB->setVisible(false);
        ui->disconnectServerPB->setVisible(false);
        ui->connectDevicePB->setVisible(false);
        ui->disconnectDevicePB->setVisible(true);
    }
    //----------------------
}

void Dialog::on_disconnectDevicePB_clicked()
{
    // stop joystick thread
    myJoyThread.stop();

    // enable connect buttons
    ui->connectDevicePB->setVisible(true);
    ui->connectServerPB->setVisible(true);

    // disable disconnect button
    ui->disconnectDevicePB->setVisible(false);

    // update connect status
    ui->connectStateText->setText(QString("Disconnected from driver"));
    ui->connectIndi->setStyleSheet("background-color:gray;");
}

void Dialog::on_disconnectServerPB_clicked()
{
    // stop joystick thread
    myJoyThread.stop();

    // enable connect buttons
    ui->connectDevicePB->setVisible(true);
    ui->connectServerPB->setVisible(true);

    // disable disconnect button
    ui->disconnectServerPB->setVisible(false);

    // update connect status
    ui->connectStateText->setText(QString("Disconnected from server"));
    ui->connectIndi->setStyleSheet("background-color:gray;");

}
