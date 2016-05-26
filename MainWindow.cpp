/**************************************************************************
 *  MainWindow.cpp
 *  Created 26/5/2016 by Dimitar T. Dimitrov
 *  mitakatdd@gmail.com
 *
 *  MIT License
 *
 **************************************************************************/
#include "MainWindow.h"
#include <QtCore>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QtWidgets/QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTimer>

static const int updateInterval = 60 * 60000; // Update every hour
#define GISTS_URL "https://api.github.com/gists?since="

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	trayIcon = new QSystemTrayIcon(QIcon(":/gists_black.png"), this);
	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	trayIcon->show();

	QAction *refreshAction = new QAction("Refresh", trayIcon);
	connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshGists()));


	QAction *aboutAction = new QAction("About", trayIcon);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));

	QAction *quitAction = new QAction(tr("Exit"), trayIcon );
	quitAction->setShortcut(QKeySequence("CTRL+SHIFT+X"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

	QMenu *mainMenu = new QMenu;
	mainMenu->addAction(refreshAction);
	mainMenu->addSeparator();
	mainMenu->addAction(aboutAction);
	mainMenu->addAction(quitAction);
	trayIcon->setContextMenu(mainMenu);

	networkManager = new QNetworkAccessManager(this);
	connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gistsFetched(QNetworkReply*)));

	// Start update routine
	updateTimer = new QTimer(this);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(refreshGists()));
	updateTimer->start();
}

MainWindow::~MainWindow()
{
}


void MainWindow::gistsFetched(QNetworkReply *reply)
{

	QList<QByteArray> headerList = reply->rawHeaderList();
	int remainingUpdates = 5000;
	foreach(QByteArray header, headerList)
	{
		if (header.toStdString() == "X-RateLimit-Remaining")
		{
			remainingUpdates = reply->rawHeader(header).toInt();
			break;
		}
	}
	if (remainingUpdates == 0)
		updateTimer->stop();

	qDebug("%d remaining updates", remainingUpdates);

	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << "NetworkManager error:: " << reply->errorString();
		return;
	}


	QString replyData = reply->readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(replyData.toUtf8());
	QJsonArray gistsArray = jsonResponse.array();

	foreach (const QJsonValue &gistValue, gistsArray)
	{
		QJsonObject gist = gistValue.toObject();
		QUrl url = QUrl(gist["url"].toString());
		QUrl ownerAvatar = QUrl(gist["owner"].toObject()["avatar_url"].toString());
		QString ownerName = gist["owner"].toObject()["login"].toString();
		QString description = gist["description"].toString();

		trayIcon->showMessage(ownerName, description, QSystemTrayIcon::NoIcon, 10000);
	}
}

void MainWindow::refreshGists()
{
	// Load gists
	QUrl url(QString(GISTS_URL).append(lastUpdate.toString(Qt::ISODate)));
	networkManager->get(QNetworkRequest(url));

	qDebug() << timeAgo(lastUpdate);
	lastUpdate = QDateTime::currentDateTimeUtc();
	updateTimer->start(updateInterval);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (event)
		QApplication::quit();
}

void MainWindow::messageClicked()
{
	qDebug() << "Who's bad?";
}

void MainWindow::showAbout()
{
	QMessageBox::about(this, "About", tr("Gists reader\n"
										"Autor: Dimitar T. Dimitrov\n\n"
										"mitakatdd@gmail.com\n\n\n"
										"Sofia, Bulgaria 2016\n"));

}


QString MainWindow::timeAgo(QDateTime date)
{
	QString ret("");
	double diff = date.secsTo(QDateTime::currentDateTimeUtc());

	const QVector<QString> periods( { "sec", "min", "h", "day", "week", "month", "year", "decade" } );
	const QVector<double> lengths( { 60, 60, 24, 7, 4.35, 12, 10 } );

	int i = 0;
	for(i = 0; i < 7 && diff >= lengths.at(i); i++)
		diff /=  lengths.at(i);

	diff = qFloor(diff);
	ret.sprintf("%d %s%s ago", (int)diff, periods.at(i).toStdString().c_str(), (i > 2 && diff != 1)?"s":"");

	return ret;
}
