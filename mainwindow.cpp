#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QImageReader>
#include <QFileDialog>
#include <QStandardPaths>
#include <QThread>
#include <QDateTime>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Radio Chat");

    //instância da QSerialPort
    this->serialPort = new QSerialPort;

    //Montagem da lista de portas e alimentação do combobox com a lista de portas disponíveis
    QStringList ports;
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    int i = 0;
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos){
        ports.insert(i,serialPortInfo.portName());
        i++;
    }
    ui->comboBox_port->insertItems(0,ports);

    //Criação da lista de velocidades da comunicação serial e alimentação do combobox baudrate
    QStringList bauds;
    bauds << "9600" << "19200" << "38400" << "57600" << "115200";
    ui->comboBox_baud->insertItems(0,bauds);

    /*Conexão do clique do botão com o slot que inicia a conexão, visto no tutorial 01 de Qt*/
    connect(ui->pushButton_connect,SIGNAL(clicked(bool)),this,SLOT(connectToSerial()));

    /*Conexão do sinal "pronto para leitura" com o slot que faz a leitura. Desse modo, quando
    é feita uma leitura, o método sai ao terminá-la. Adicionalmente estamos escrevendo para a
    porta serial, enviando um Byte para acender ou apagar o LED no Arduino.
    Em alguns casos pode ser necessário utilizar threads, depende de  diversos  fatores,  mas
    normalmente dá pra resolver muita coisa só utilizando SIGNAL & SLOT.*/
    connect(this->serialPort,SIGNAL(readyRead()),this,SLOT(startReading()));

    /*O botão de selecionar imagem chama o slot que abre o widget file chooser*/
    connect(ui->pushButton_select_image,SIGNAL(clicked(bool)),this,SLOT(openImageFileSlot()));

    /*Quando pressionado, o botão alimenta o chat e limpa a lineEdit*/
    connect(ui->pushButton_sendMsg,SIGNAL(clicked(bool)),this,SLOT(feedChat()));

    /*Quando pressionado enter, tem o mesmo efeito do botão*/
    connect(ui->lineEdit_msg,SIGNAL(returnPressed()),this,SLOT(feedChat()));



    this->enableLabels(false);
    this->enableWidgets(false);

    ui->plainTextEdit_chat->setReadOnly(true);

    bool ok;
    this->username = QInputDialog::getText(this, tr("Entre com um username"),
                                             tr("User name:"), QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);

}

void MainWindow::enableLabels(bool activate)
{
    ui->label_lineInfo->setVisible(activate);
    ui->label_percent->setVisible(activate);
    ui->label_percent_symbol->setVisible(activate);
}

void MainWindow::feedChat()
{
    QString msg = QDateTime::currentDateTime().toString() + "\n";
    msg        += this->username.remove("\r\n") + "> " + ui->lineEdit_msg->text().remove("\r\n");

    QString val = msg.split('\n').at(1) + '\n';
    //qDebug() << val;
    this->serialPort->write(val.toUtf8());

    ui->lineEdit_msg->clear();
    ui->plainTextEdit_chat->appendPlainText(msg);

}

void MainWindow::openImageFileSlot()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Selecione o arquivo"),
                                                    QString(QStandardPaths::HomeLocation),
                                                    tr("Images (*.bmp)"));
    if (fileName == NULL){
        qDebug() << "NULL";
        return;
    }
    this->enableLabels(true);
    /*
     * A correspondência de cada pixel no OLED é feita assim:
     * display.drawPixel(x, y, VALOR);
    */
    QImage img;
    img.load(fileName);
    qDebug() << img.width();
    qDebug() << img.height();
    if (img.isGrayscale()){
        qDebug()<< "grayscale";
    }

    QList <uchar> line;
    for (int i=0;i<img.height();i++){
        this->serialPort->write("L\n"); //avisa o envio de linha
        for (int j=0;j<img.width();j++){
            line << img.pixelColor(j,i).value();
            this->serialPort->write(QString(line[j]).toUtf8()+"\n");
            while (!this->serialPort->waitForBytesWritten(1000)){
                QThread::msleep(20);
            }
        }
        qDebug() << line;
        qDebug() << line.length();
        line.clear();
        qDebug() << "-----------";
    }
}

void MainWindow::feedChatFromSerial(QByteArray msg)
{
    QString dt = QDateTime::currentDateTime().toString() + "\n";
    dt += msg;

    ui->plainTextEdit_chat->appendPlainText(dt);
}

void MainWindow::startReading()
{
    //Faz uma leitura. Tenha em mente que esse código é um exemplo e não temos aqui os
    //devidos tratamentos de excessões.
    QByteArray readData = this->serialPort->readAll();
    //Apenas exibindo a leitura em stdout.
    qDebug() << readData;

    this->feedChatFromSerial(readData);

}

void MainWindow::enableWidgets(bool activate)
{

    if (!activate){
        ui->plainTextEdit_chat->clear();
    }
    ui->pushButton_select_image->setEnabled(activate);
    ui->lineEdit_msg->setEnabled(activate);
    ui->pushButton_sendMsg->setEnabled(activate);
    ui->plainTextEdit_chat->setEnabled(activate);
}

void MainWindow::connectToSerial()
{
    /*Na conexão fazemos a abertura ou fechamento da porta, conforme descrito no video 03
     * dos tutoriais relacionados a esse exemplo.*/
    if (this->serialPort->isOpen()){
        this->serialPort->close();
        ui->label_status->setText("Desconectado");
        ui->pushButton_connect->setText("Conectar");
        this->enableLabels(false);
        this->enableWidgets(false);
        return;
    }
    //se for para abrir, pegamos os parâmetros de velocidade e porta a abrir.
    this->serialPort->setPortName(ui->comboBox_port->currentText());
    this->serialPort->setBaudRate(ui->comboBox_baud->currentText().toUInt());

    /*Dentro desse método definimos o modo de abertura. Até o tutorial 03 estavamos
     * usando o modo somente-leitura. Agora precisamos abrir em modo de leitura-e-escrita,
     * porque estamos enviando o comando para acender o LED no Arduino.
     * O exemplo de escrita dentro do mesmo método que lê é apenas para simplificar, mas
     * podemos fazer a escrita de forma assíncrona, de modo que poderiamos criar botões para
     * acender LED, acionar relés, pegar status de sensores etc. O Arduino também poderia
     * responder apenas quando solicitado. Veremos essas variáveis em outro tutorial.
    */
    if (!this->serialPort->open(QIODevice::ReadWrite)){
        ui->label_status->setText("Falha ao tentar conectar");
        return;
    }
    ui->label_status->setText("Conectado");
    ui->pushButton_connect->setText("Desconectar");

    emit startReading(true);
    enableWidgets(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}
