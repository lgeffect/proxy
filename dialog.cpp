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

void Dialog::on_pushButton_clicked()
{
	tcpServer = new QTcpServer(this);
	connect(tcpServer, SIGNAL(newConnection()), SLOT(newUser()));
	if(!tcpServer->listen(QHostAddress::Any, 8080) && server_status == 0){
		ui->textEdit->append(tcpServer->errorString());
	}else{
		server_status=1;
		ui->textEdit->append("Server starts...");
	}

}

void Dialog::on_pushButton_2_clicked()
{
	if(server_status==1){
		foreach(int i, SClients.keys()){
			SClients[i]->write("Bye!\n");
			SClients[i]->close();
			SClients.remove(i);
		}
	}
	tcpServer->close();
	ui->textEdit->append("Server stopped!");
	server_status=0;
}

void Dialog::newUser(){
	if(server_status==1){
		ui->textEdit->append("NEw connection!");
		QTcpSocket* clientSocket=tcpServer->nextPendingConnection();
		int iduser=clientSocket->socketDescriptor();
		SClients[iduser]=clientSocket;
		connect(SClients[iduser], SIGNAL(readyRead()), this, SLOT(slotReadCLient()));

	}
}

void Dialog::slotReadCLient(){
	QByteArray block;
	QDataStream os(&block, QIODevice::WriteOnly);


	QTcpSocket* clientSocket = (QTcpSocket*)sender();
	int iduser=clientSocket->socketDescriptor();
	os << "HTTP/1.1 200 Ok\r\n"
		  "Content-Type: text/html; charset=\"utf-8\"\r\n"
		  "\r\n"
		  "<em><h1>Nothing to see here!!!</h1></em>"
	   << QDateTime::currentDateTime().toString() << "\n";

	clientSocket->write(block);

	ui->textEdit->append(("ReadClient:"+clientSocket->readAll()+"\n\r"));
	clientSocket->close();
	SClients.remove(iduser);
}
