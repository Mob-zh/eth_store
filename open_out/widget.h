#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
                QProcess *process = new QProcess(this);
private slots:
 void on_pushButton_clicked();

 void on_pushButton_clicked(bool checked);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
