#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtNetwork>
#include <QTcpSocket>
#include <QObject>
#include <QByteArray>
#include <QDebug>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QTextCodec>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
	void on_startProxy_clicked();
	void newConn();
	void slotReadClient();


private:
	Ui::Dialog *ui;

	// Создаем объект сервера
	QTcpServer *tcpServer;
	// Статус сервера
	bool serverStatus;
	// Карта соединений с сервером
	QMap<int, QTcpSocket*> SClients;
	// Менеджер http запросов
	QNetworkAccessManager *manager;
	// Очередь запросов
	QQueue<QByteArray> reqQueue;
};

#endif // DIALOG_H
