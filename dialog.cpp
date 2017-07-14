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
	server_status = 0;
}

void Dialog::on_startIntercept_clicked()
{
	// Выключение сервера
	if(server_status==1){
		foreach(int i, SClients.keys()){
			SClients[i]->close();
			SClients.remove(i);
		}
		tcpServer->close();
		server_status=0;
		ui->startIntercept->setChecked(false);
	}else{ // Иначе включаем
		tcpServer = new QTcpServer(this);
		connect(tcpServer, SIGNAL(newConnection()), SLOT(newConn()));
		if(!tcpServer->listen(QHostAddress::Any, (quint16)ui->proxyPort->value()) && server_status == 0){
			ui->textEdit->append(tcpServer->errorString());
		}else{
			server_status=1;
		}
		ui->startIntercept->setChecked(true);
	}
}

void Dialog::newConn(){
	if(server_status==1){
		ui->textEdit->append("New connection!");
		QTcpSocket* clientSocket=tcpServer->nextPendingConnection();
		int iduser=clientSocket->socketDescriptor();
		SClients[iduser]=clientSocket;
		connect(SClients[iduser], SIGNAL(readyRead()), this, SLOT(slotReadClient()));

	}
}

void Dialog::slotReadClient(){
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
	SClients.remove(iduser);
}
