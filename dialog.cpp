#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
	ui->setupUi(this);
}

Dialog::~Dialog()
{
	delete ui;
	serverStatus = false;
}

void Dialog::on_startProxy_clicked()
{
	// Выключение сервера
	if(serverStatus){
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
		if(!tcpServer->listen(QHostAddress::Any, (quint16)ui->proxyPort->value()) && server_status == 0){
			ui->textEdit->append(tcpServer->errorString());
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
	// Добавляем запрос в очередь запросов
	reqQueue.enqueue(clientRequest);
	// Вытаскиваем хост из запроса
	QString clientReqStr = QString(clientRequest);
	QString resolveHost = clientReqStr.mid(clientReqStr.indexOf("Host:")+6, clientReqStr.indexOf("Connection:")-24);


	ui->textEdit->setText(clientReqStr);

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

	ui->textEdit->append((clientSocket->readAll()+"\n"));
	clientSocket->close();
	SClients.remove(iduser);*/
}
