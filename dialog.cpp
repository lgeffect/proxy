#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
	ui->setupUi(this);
	on_startProxy_clicked();
}

Dialog::~Dialog()
{
	delete ui;
	serverStatus = false;
}

void Dialog::on_startProxy_clicked()
{
	// Выключение сервера
	if(!serverStatus){
		foreach(int i, SClients.keys()){
			SClients[i]->close();
			SClients.remove(i);
		}
		tcpServer->close();
		serverStatus=false;
		ui->startProxy->setChecked(false);
	}else{ // Иначе включаем
		tcpServer = new QTcpServer(this);
		connect(tcpServer, SIGNAL(newConnection()), SLOT(newConn()));
		if(!tcpServer->listen(QHostAddress::Any, (quint16)ui->proxyPort->value()) && serverStatus){
			ui->proxyTextWindow->append(tcpServer->errorString());
		}else{
			serverStatus=true;
		}
		ui->startProxy->setChecked(true);
	}
}

void Dialog::newConn(){
	// Новое подключение к прокси
	if(serverStatus){
		QTcpSocket* clientSocket=tcpServer->nextPendingConnection();
		int iduser=clientSocket->socketDescriptor();
		SClients[iduser]=clientSocket;
		connect(SClients[iduser], SIGNAL(readyRead()), this, SLOT(slotReadClient()));

	}
}

void Dialog::slotReadClient(){
	// Принимаем данные от клиента и отсылаем их нужному хосту
	QTcpSocket* clientSocket = (QTcpSocket*)sender();
	// Устанавливаем id подключившегося пользователя
	int iduser=clientSocket->socketDescriptor();
	// Читаем присланные данные
	QByteArray clientRequest = clientSocket->readAll();

	// Если очередь запросов пуста
	if(reqQueue.isEmpty()){
		// Показываем содержимое запроса в окне прокси
		ui->proxyTextWindow->setText(clientRequest);
		// Делаем возможным пропустить запрос
		ui->forwardRequest->setEnabled(true);
	}
	// Добавляем запрос в очередь запросов
	reqQueue.enqueue(clientRequest);
	// Добавляем id клиента в очередь запросов
	userIdQueue.enqueue(iduser);
	// Парсим хост из запроса
	QString clientReqStr = QString(clientRequest);
	QString resolveHost = clientReqStr.mid(clientReqStr.indexOf("Host:")+6, clientReqStr.indexOf("Connection:")-24);


	clientSocket->close();
	SClients.remove(iduser);
/*

	QByteArray block;
	QDataStream os(&block, QIODevice::WriteOnly);


	QTcpSocket* clientSocket = (QTcpSocket*)sender();
	int iduser=clientSocket->socketDescriptor();
	os << "HTTP/1.1 200 Ok\r\n"
		  "Content-Type: text/html; charset=\"utf-8\"\r\n"
		  "\r\n"
		  "<em><h1>Nothing to see here!!!</h1></em>\n";

	clientSocket->write(block);

	ui->proxyTextWindow->append((clientSocket->readAll()+"\n"));
	clientSocket->close();
	SClients.remove(iduser);*/
}

void Dialog::replyFinished(){}

void Dialog::on_forwardRequest_clicked()
{
	if(!reqQueue.isEmpty()){
		ui->proxyTextWindow->setText(reqQueue.dequeue());
	}else{
		ui->proxyTextWindow->setText("");
		ui->forwardRequest->setEnabled(false);
	}
}
