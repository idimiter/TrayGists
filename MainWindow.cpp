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
#include <QSettings>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QtWidgets/QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QImage>
#include <QIcon>
#include <QMessageBox>
#include <QTimer>

static const int updateInterval = 60 * 60000; // Update every hour
#define GISTS_URL "https://api.github.com/gists/public?since="

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	userSettings = new QSettings("traygists_settings");
	lastUpdate = userSettings->value("LastUpdate", QDateTime::currentDateTimeUtc()).toDateTime();
	qDebug() << "Last updated: " << timeAgo(lastUpdate);

	trayIcon = new QSystemTrayIcon(QIcon(":/gists_black.png"), this);
	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	trayIcon->show();

	QAction *refreshAction = new QAction("Refresh", trayIcon);
	connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshGists()));


	QAction *aboutAction = new QAction("About", trayIcon);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));

	QAction *quitAction = new QAction(tr("Exit"), trayIcon );
	quitAction->setShortcut(QKeySequence("CTRL+SHIFT+X"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

	gistsMenu = new QMenu("gists");
	connect(gistsMenu, SIGNAL(triggered(QAction*)), this, SLOT(gistSelected(QAction*)));

	mainMenu = new QMenu;
	mainMenu->addAction(refreshAction);
	mainMenu->addMenu(gistsMenu);
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
	gistsMenu->clear();
	mainMenu->clear();

	delete networkManager;
	delete userSettings;
	delete gistsMenu;
	delete updateTimer;
	delete trayIcon;
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

	gists.clear();

	QString replyData = reply->readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(replyData.toUtf8());
	QJsonArray gistsArray = jsonResponse.array();

	foreach (const QJsonValue &gistValue, gistsArray)
	{
		QJsonObject gistObj = gistValue.toObject();
		Gist gist;

		gist.url = QUrl(gistObj["html_url"].toString());
		gist.iconUrl = QUrl(gistObj["owner"].toObject()["avatar_url"].toString());
		if (gist.iconUrl.isEmpty())
			gist.iconUrl = QUrl("https://avatars2.githubusercontent.com/u/0");

		QString ownerName = gistObj["owner"].isNull()?"anonymous":gistObj["owner"].toObject()["login"].toString();
		gist.description = gistObj["description"].toString();
		gist.updatedAt = QDateTime::fromString(gistObj["updated_at"].toString(), Qt::ISODate);

		QString fileName = "";
		if (gistObj["files"].isArray())
			fileName = gistObj["files"].toArray().first().toObject().keys().first();
		else
			fileName = gistObj["files"].toObject().keys().first();

		gist.name = ownerName.append(" / ").append(fileName);

		gists.push_back(gist);

		trayIcon->showMessage(gist.name, gist.description, QSystemTrayIcon::NoIcon, 10000);
	}

	// Update gists submenu
	gistsMenu->clear();
	for (int i = 0; i < gists.length(); i++)
	{
		Gist *gist = &gists[i];

		QAction *gistAction = new QAction(gist->name, gistsMenu);
		gistAction->setData(QVariant(i));
		gistAction->setToolTip(gist->description);
		gistsMenu->addAction(gistAction);

		if (!gist->description.isEmpty())
		{
			QAction *descriptionAction = new QAction(gist->description, gistsMenu);
			descriptionAction->setEnabled(false);
			gistsMenu->addAction(descriptionAction);
		}

		// Load icon
		loadGistIcon(gist, gistAction);
	}
}

void MainWindow::refreshGists()
{
	// Load gists
	QUrl url(QString(GISTS_URL).append(lastUpdate.toString(Qt::ISODate)));
	networkManager->get(QNetworkRequest(url));

	lastUpdate = QDateTime::currentDateTimeUtc();
	userSettings->setValue("LastUpdate", lastUpdate);
	updateTimer->start(updateInterval);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (event)
		QApplication::quit();
}

void MainWindow::messageClicked()
{

	QDesktopServices::openUrl(QUrl("https://gist.github.com/discover"));
}

void MainWindow::gistSelected(QAction* gistAction)
{
	Gist *gist = &gists[gistAction->data().toInt()];

	QDesktopServices::openUrl(gist->url);
}

void MainWindow::showAbout()
{
	QMessageBox::about(this, "About", tr("Gists reader\n"
										"Autor: Dimitar T. Dimitrov\n\n"
										"mitakatdd@gmail.com\n\n"
										"Version 0.2b\n\n"
										"Sofia, Bulgaria 2016\n"));

}


void MainWindow::loadGistIcon(Gist* gist, QAction *action)
{
	QNetworkAccessManager *netManager = new QNetworkAccessManager(this);
	connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gistIconLoaded(QNetworkReply*)));

	QNetworkReply *reply = netManager->get(QNetworkRequest(gist->iconUrl));
	reply->setUserData(0, (QObjectUserData *) action);
}

void MainWindow::gistIconLoaded(QNetworkReply *reply)
{
	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << "NetworkManager error:: " << reply->errorString();
		return;
	}

	QAction *menuAction = (QAction*)reply->userData(0);

	QPixmap imagePixmap;
	imagePixmap.loadFromData(reply->readAll());
	QIcon icon = QIcon(imagePixmap);
	menuAction->setIcon(icon);
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
