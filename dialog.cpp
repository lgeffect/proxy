#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
	ui->setupUi(this);
	on_startProxy_clicked();
	// Создаем объект для запросов
	manager = new QNetworkAccessManager(this);
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
	ui->debugEdit->addItem(QString::number(iduser));
	// Читаем присланные данные
	QByteArray clientRequest = clientSocket->readAll();
	QList<QByteArray> clReqList = clientRequest.split(' ');

	QString resolveHost = clReqList[1];
	QMap<QString, QString> reqTypes;
	// Если очередь запросов пуста
	if(reqQueue.isEmpty()){
		// Показываем содержимое запроса в окне прокси
		ui->proxyTextWindow->setText(clientRequest);

		// Показываем запрашиваемый хост
		ui->reqHost->setText(resolveHost);

		// Делаем возможным пропустить запрос
		ui->forwardRequest->setEnabled(true);
	}
	// Добавляем запрос в очередь запросов
	reqQueue.enqueue(clientRequest);
	// Добавляем id клиента в очередь запросов
	userIdQueue.enqueue(iduser);



	QUrl url(resolveHost);
	QNetworkRequest request(url);
	//request.set
	QNetworkReply* reply =  manager->get(request);
	connect( reply, SIGNAL(finished()), this, SLOT(replyFinished()) );
}

void Dialog::replyFinished(){
	int userid = userIdQueue.dequeue();
	QNetworkReply *reply= qobject_cast<QNetworkReply *>(sender());
	QByteArray content;

	if (reply->error() == QNetworkReply::NoError)
	{
		content = reply->readAll();
		SClients[userid]->write(content);
	}else{
		content = reply->errorString().toStdString().c_str();
		SClients[userid]->write(content);
	}
	SClients[userid]->close();
	SClients.remove(userid);
	reply->deleteLater();
}

void Dialog::on_forwardRequest_clicked()
{
	if(!reqQueue.isEmpty()){
		// Берем новый запрос из очереди
		QString clientRequest = reqQueue.dequeue();
		// Парсим хост из запроса
		QString resolveHost = clientRequest.mid(clientRequest.indexOf("GET")+3, clientRequest.indexOf("HTTP/1.")-3);

		// Показываем информацию о запросе
		ui->proxyTextWindow->setText(clientRequest);
		ui->reqHost->setText(resolveHost.trimmed());
	}else{
		// Если в очереди нет запросов очищаем все
		ui->proxyTextWindow->setText("");
		ui->reqHost->setText("");
		ui->forwardRequest->setEnabled(false);
	}
}
