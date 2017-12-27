#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSerialPort *serialPort;

    bool ON = false;

    void enableLabels(bool activate);
    void enableWidgets(bool activate);
    void feedChatFromSerial(QByteArray msg);

    QString username = "None";

public slots:
    void connectToSerial();
    void startReading();
    void openImageFileSlot();
    void feedChat();

signals:
    void startReading(bool ok);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
