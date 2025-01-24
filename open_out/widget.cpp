#include "widget.h"
#include "ui_widget.h"

#include <QProcess>
#include <QDebug>
#include <stdlib.h>
#include <QMessageBox>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->pushButton,&QPushButton::clicked,[=]{
        QString program ="/home/dfy/c++/test/main"; // 替换为你的可执行文件的路径


                process->start("/home/dfy/c++/test/main");
                if (!process->waitForStarted()) {
                           QMessageBox::warning(this, "启动失败", "无法启动可执行文件！");
                       }
    });
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
//    system("/home/dfy/Bash_Script/hello_world.sh");
//    system("gnome-terminal -- bash -c '/home/dfy/Bash_Script/hello_world.sh'&");//更改文件路径
//    QString program = "/home/dfy/Bash_Script/hello_world.sh"; // 替换为你的可执行文件的路径
//            QProcess *process = new QProcess(this);

//            // 启动可执行文件
//            process->start(program);
}

void Widget::on_pushButton_clicked(bool checked)
{
    if(checked){
//            system("/home/dfy/c++/test/main");


    }
}
