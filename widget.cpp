#include "widget.h"
#include "ui_widget.h"

////==================================-ctrl-rc-============================================

#define RC_CHANNLE_ROLL     0    //正
#define RC_CHANNLE_PITCH    1    //反
#define RC_CHANNLE_THRUST   2    //正
#define RC_CHANNLE_YAW      3    //正
#define RC_CHANNLE_ARMED    4
#define RC_CHANNLE_MODE     5

#define RC_MODE_STABILIZED 1000
#define RC_MODE_POSITION 1500
#define RC_MODE_FORWARD 2000

#define RC_ARMED_NO  1000
#define RC_ARMED_YES 2000

//---------------------------------------------------------------------------------------

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //串口初始化
    SerialPortInit();

    this->RC_Channels[RC_CHANNLE_ROLL] = 1000;
    this->RC_Channels[RC_CHANNLE_PITCH] = 1000;
    this->RC_Channels[RC_CHANNLE_THRUST] = 1000;
    this->RC_Channels[RC_CHANNLE_YAW] = 1000;
    this->RC_Channels[RC_CHANNLE_ARMED] = RC_ARMED_NO;
    this->RC_Channels[RC_CHANNLE_MODE] = RC_MODE_STABILIZED;

    ui->btnArm->setDisabled(false);
    ui->btnDisarm->setDisabled(true);

    ui->btnModeStabilized->setDisabled(true);
    ui->btnModePosition->setDisabled(false);
    ui->btnModeForward->setDisabled(false);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_btnArm_clicked()
{
    this->RC_Channels[RC_CHANNLE_ARMED] = RC_ARMED_YES;
    ui->btnArm->setDisabled(true);
    ui->btnDisarm->setDisabled(false);
    ui->showLabelIsarmed->setText("Armed");
    //改变之后发送数据
    DataSend();
}


void Widget::on_btnDisarm_clicked()
{
    this->RC_Channels[RC_CHANNLE_ARMED] = RC_ARMED_NO;
    ui->btnArm->setDisabled(false);
    ui->btnDisarm->setDisabled(true);
    ui->showLabelIsarmed->setText("Locked");
    //改变之后发送数据
    DataSend();
}


void Widget::on_btnModeStabilized_clicked()
{
    this->RC_Channels[RC_CHANNLE_MODE] = RC_MODE_STABILIZED;
    ui->btnModeStabilized->setDisabled(true);
    ui->btnModePosition->setDisabled(false);
    ui->btnModeForward->setDisabled(false);
    ui->showLabelMode->setText("Stabilized");
    //改变之后发送数据
    DataSend();
}


void Widget::on_btnModePosition_clicked()
{
    this->RC_Channels[RC_CHANNLE_MODE] = RC_MODE_POSITION;
    ui->btnModeStabilized->setDisabled(false);
    ui->btnModePosition->setDisabled(true);
    ui->btnModeForward->setDisabled(false);
    ui->showLabelMode->setText("Position");
    //改变之后发送数据
    DataSend();
}


void Widget::on_btnModeForward_clicked()
{
    this->RC_Channels[RC_CHANNLE_MODE] = RC_MODE_FORWARD;
    ui->btnModeStabilized->setDisabled(false);
    ui->btnModePosition->setDisabled(false);
    ui->btnModeForward->setDisabled(true);
    ui->showLabelMode->setText("Forward");
    //改变之后发送数据
    DataSend();
}

void Widget::on_btnSerialOpen_clicked()
{
    if(serial->isOpen())                                        // 如果串口打开了，先给他关闭
        {
            serial->clear();
            serial->close();
            // 关闭状态，按钮显示“打开串口”
            ui->btnSerialOpen->setText("打开串口");

        }
        else                                                        // 如果串口关闭了，先给他打开
        {
            //当前选择的串口名字
            serial->setPortName(ui->PortBox->currentText());
            //用ReadWrite 的模式尝试打开串口，无法收发数据时，发出警告
            if(!serial->open(QIODevice::ReadWrite))
            {
                QMessageBox::warning(this,tr("提示"),tr("串口打开失败!"),QMessageBox::Ok);
                return;
             }
            // 打开状态，按钮显示“关闭串口”
            ui->btnSerialOpen->setText("关闭串口");

        }

}


void Widget::on_btnSerialRefresh_clicked()
{
    // 刷新串口
    RefreshSerialPort(0);
}

//---------------------------------serial-----------------------------------------

// 串口初始化（参数配置）
void Widget::SerialPortInit()
{
    serial = new QSerialPort;                       //申请内存,并设置父对象

    // 获取计算机中有效的端口号，然后将端口号的名称给端口选择控件
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        serial->setPort(info);                      // 在对象中设置串口
        if(serial->open(QIODevice::ReadWrite))      // 以读写方式打开串口
        {
            ui->PortBox->addItem(info.portName());  // 添加计算机中的端口
            serial->close();                        // 关闭
        } else
        {
            qDebug() << "串口打开失败，请重试";
        }
    }

    // 参数配置
    // 波特率115200
    serial->setBaudRate(QSerialPort::Baud115200);
    // 校验无
    serial->setParity(QSerialPort::NoParity);
    // 数据位8位
    serial->setDataBits(QSerialPort::Data8);
    // 停止位1位
    serial->setStopBits(QSerialPort::OneStop);
    // 控制流无
    serial->setFlowControl(QSerialPort::NoFlowControl);

    // 刷新串口
    RefreshSerialPort(0);

    // 信号
    connect(serial,&QSerialPort::readyRead,this,&Widget::DataReceived);      // 接收数据
    connect(ui->btnTargetUpdate,&QPushButton::clicked,this,&Widget::DataSend); // 发送数据
}

// 刷新串口
void Widget::RefreshSerialPort(int index)
{
    QStringList portNameList;                                        // 存储所有串口名
    if(index != 0)
    {
        serial->setPortName(ui->PortBox->currentText());             //设置串口号
    }
    else
    {
        ui->PortBox->clear();                                        //关闭串口号
        foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts()) //添加新串口
        {
            portNameList.append(info.portName());
        }
        ui->PortBox->addItems(portNameList);
        ui->PortBox->setCurrentIndex(0);                             //  随便设置一个索引，选上一个
        serial->setPortName(ui->PortBox->currentText());             //设置串口号
   }
}

// 接收数据,使用read () / readLine () / readAll ()
void Widget::DataReceived()
{
    QByteArray data = serial->readAll();                      // 读取数据

    if(!data.isEmpty())                                 // 接收到数据
    {
        //接收到的数据的格式： 1.0,1.0,1.0
        QString str = data;
        qDebug() << "str info: " << str;
        QStringList strList = str.split(",");
        //显示到label中
        ui->showLabelRealRoll->setText(strList[0]);
        ui->showLabelRealPitch->setText(strList[1]);
        ui->showLabelRealYaw->setText(strList[2]);

    }
}

// 发送数据，write ()
void Widget::DataSend()
{
    //更新目标姿态显示
    ui->showLabelTargetRoll->setText(ui->lineEditRoll->text());
    ui->showLabelTargetPitch->setText(ui->lineEditPitch->text());
    ui->showLabelTargetYaw->setText(ui->lineEditYaw->text());


    //状态信息
    uint16_t isArmed = this->RC_Channels[RC_CHANNLE_ARMED];
    uint16_t mode = this->RC_Channels[RC_CHANNLE_MODE];
    QByteArray isArmedba;
    QByteArray modeba;
    isArmedba.resize(sizeof(uint16_t));
    modeba.resize(sizeof(uint16_t));
    memcpy(isArmedba.data(), &isArmed, sizeof(uint16_t));
    memcpy(modeba.data(), &mode, sizeof(uint16_t));
    //三轴角度
    float roll = (ui->lineEditRoll->text()).toFloat();
    float pitch = (ui->lineEditPitch->text()).toFloat();
    float yaw = (ui->lineEditYaw->text()).toFloat();
    QByteArray rollba;
    QByteArray pitchba;
    QByteArray yawba;
    rollba.resize(sizeof(float));
    pitchba.resize(sizeof(float));
    yawba.resize(sizeof(float));
    memcpy(rollba.data(), &roll, sizeof(float));
    memcpy(pitchba.data(), &pitch, sizeof(float));
    memcpy(yawba.data(), &yaw, sizeof(float));

    //暂时：
    //uint16_t isarmed
    //uint16_t mode
    //float roll
    //float pitch
    //float yaw
    QByteArray byteArray;
    byteArray.append(isArmedba);
    byteArray.append(modeba);
    byteArray.append(rollba);
    byteArray.append(pitchba);
    byteArray.append(yawba);

    qDebug() << byteArray;
    serial->write(byteArray);      // 串口发送数据
}





