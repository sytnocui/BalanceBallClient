#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QMessageBox>
// 引入串口通信的两个头文件(第一步)
#include <QtSerialPort/QSerialPort>         // 提供访问串口的功能
#include <QtSerialPort/QSerialPortInfo>     // 提供系统中存在的串口信息

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // 串口功能
    void        SerialPortInit();                      // 串口初始化（参数配置）
    void        RefreshSerialPort(int index);          // 刷新串口
    void        DataReceived();                        // 接收数据
    void        DataSend();                            // 发送数据

private slots:
    void on_btnArm_clicked();

    void on_btnDisarm_clicked();

    void on_btnModeStabilized_clicked();

    void on_btnModePosition_clicked();

    void on_btnModeForward_clicked();

    void on_btnSerialOpen_clicked();

    void on_btnSerialRefresh_clicked();

private:
    Ui::Widget *ui;


    QSerialPort     *serial;                            // 定义全局的串口对象（第三步）

    uint16_t RC_Channels[6] = {0};
};
#endif // WIDGET_H
