#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtNetwork>
#include <QTcpSocket>
#include <QObject>
#include <QByteArray>
#include <QDebug>

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
	void on_startIntercept_clicked();
	void newConn();
	void slotReadClient();


private:
	Ui::Dialog *ui;
	QTcpServer *tcpServer;
	int server_status;
	QMap<int, QTcpSocket*> SClients;
};

#endif // DIALOG_H
