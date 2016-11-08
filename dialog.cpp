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

    // reset alive motor list
    aliveMotorList.clear();

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
    ui->webView->setVisible(false);
    //----------------------
    //======================

    //======================
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

    // connect JoyThread::aliveMotorList to Dialog::aliveMotorList
    connect(&myJoyThread, &JoyThread::setAliveMotors, this, &Dialog::setAliveMotors);
    //======================


    //======================
    // ROBOT DEVICE

    // list serial device
    foreach (const QSerialPortInfo &serialInfo, QSerialPortInfo::availablePorts()){

        ui->serialComboBox->addItem(serialInfo.portName());
        /*
        QAction *serialDeviceAction = new QAction(serialInfo.description(), serialDevicesGroup);
        serialDeviceAction->setCheckable(true);
        serialDeviceAction->setData(QVariant::fromValue(serialInfo));

        ui->serialComboBox->addItem(serialInfo.portName(), serialDeviceAction);
        */
    }
    //======================

    //======================
    // CAMERA
    // list camera device and cameraInfo to Qaction data
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {

        // set cameraInfo to Qaction data
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), this);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));

        // add item to cameraComboBox
        ui->cameraComboBox->addItem(cameraInfo.description(),QVariant::fromValue(videoDeviceAction));
    }
    //======================

    //======================
    // disable disconnect button
    ui->disconnectDevicePB->setVisible(false);
    ui->disconnectServerPB->setVisible(false);
    ui->robotStateIndicator->setStyleSheet("background-color:gray;");
    //======================

}

void Dialog::on_connectCameraPB_clicked()
{
    // set the viewfinder of camera and start to display image
    QAction *videoDeviceAction = ui->cameraComboBox->itemData(ui->cameraComboBox->currentIndex(), Qt::UserRole).value<QAction *>();
    QCameraInfo cameraInfo = qvariant_cast<QCameraInfo>(videoDeviceAction->data());
    camera = new QCamera(cameraInfo);
    camera->setViewfinder(ui->viewfinder);
    camera->start();

    // connected
    if(camera->state() == QCamera::ActiveState){
        //ui->connectCameraPB->setVisible(false);
        //ui->disconnectCameraPB->setVisible(true);
        ui->cameraStateTextLabel->setText(cameraInfo.description());
        ui->connectCameraStateTextLabel->setText(QString("Connected to camera"));
    }else{
        ui->cameraStateTextLabel->setText(QString(""));
        qDebug() << camera->errorString() << " " << camera->error();
        ui->connectCameraStateTextLabel->setText(QString("Error connecting to camera:").append(camera->errorString()));
    }
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

    //======================
    // left motor ids
    leftMotorList.clear();
    QList<QCheckBox *> leftMotorCBs = ui->leftMotorGB->findChildren<QCheckBox *>();
    QString leftMotorIDs = "";


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
        }
    }
    //======================

    //======================
    // right motor ids
    rightMotorList.clear();
    QList<QCheckBox *> rightMotorCBs = ui->rightMotorGB->findChildren<QCheckBox *>();
    QString rightMotorIDs = "";

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
        }
    }
    //======================

    //======================
    // individual motor ids, group1
    indi1MotorList.clear();
    QList<QCheckBox *> indi1MotorCBs = ui->indi1MotorGB->findChildren<QCheckBox *>();
    QString indi1MotorIDs;

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
        }
    }
    //======================

    //======================
    // individual motor ids, group2
    indi2MotorList.clear();
    QList<QCheckBox *> indi2MotorCBs = ui->indi2MotorGB->findChildren<QCheckBox *>();
    QString indi2MotorIDs;

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
        }
    }
    //======================

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

// slots for sending motor alive motor list
void Dialog::setAliveMotors(const QStringList &aliveMotorList)
{
    this->aliveMotorList = aliveMotorList;

    qDebug() << "aliveMotorList:" << aliveMotorList.size();

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
    if(aliveMotorList.size())
       for(QString id: aliveMotorList){
            leftMotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
       }

    if(leftMotorList.size())
       for(QString id: leftMotorList){
           qDebug() << "leftMotor:" << id.toInt()-1;
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
    if(aliveMotorList.size())
        for(QString id: aliveMotorList){
            rightMotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
        }

    if(rightMotorList.size())
        for(QString id: rightMotorList){
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
    if(aliveMotorList.size())
        for(QString id: aliveMotorList){
            indi1MotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
        }

    if(indi1MotorList.size())
        for(QString id: indi1MotorList){
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
    if(aliveMotorList.size())
        for(QString id: aliveMotorList){
            indi2MotorWidgets[id.toInt()-1]->setStyleSheet("background-color:blue;");
        }

    if(indi2MotorList.size())
        for(QString id: indi2MotorList){
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
        ui->connectServerStateLabel->setText(QString("Error connecting to server:").append(QString::number(status)));
    }else{
        tcpSocket->moveToThread(&myJoyThread); //tcpSocket in main thread is moved to JoyThread (could be accessed from run())

        // update connect status
        ui->connectServerStateLabel->setText(QString("Connected to server"));
        ui->robotStateTextLabel->setText(QString("Server:").append(ipAddress));
        ui->robotStateIndicator->setStyleSheet("background-color:blue;");

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
        ui->connectDeviceStateTextLabel->setText(QString("Error connecting to device:").append(QString::number(status)));
    }else{
        // stop joystick thread
        if(myJoyThread.isRunning())
            myJoyThread.stop();

        // start joystick thread with joystick id
        myJoyThread.startDeviceJoyThread(ui->joyComboBox->currentIndex(),roboteqDevice,leftMotorList,rightMotorList,indi1MotorList,indi2MotorList,numMotors);
        //----------------------

        // update connect status
        ui->connectDeviceStateTextLabel->setText(QString("Connected to driver"));
        ui->robotStateTextLabel->setText(QString("Device:").append(roboteqDevicePort));
        ui->robotStateIndicator->setStyleSheet("background-color:blue;");

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
    ui->connectDeviceStateTextLabel->setText(QString("Disconnected from driver"));
    ui->robotStateTextLabel->setText(QString(""));
    ui->robotStateIndicator->setStyleSheet("background-color:gray;");
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
    ui->connectServerStateLabel->setText(QString("Disconnected from server"));
    ui->robotStateTextLabel->setText(QString(""));
    ui->robotStateIndicator->setStyleSheet("background-color:gray;");

}


