/**************************************************************************
 *  MainWindow.h
 *  Created 26/5/2016 by Dimitar T. Dimitrov
 *  mitakatdd@gmail.com
 *
 *  MIT License
 *
 **************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

class QDateTime;
class QSystemTrayIcon;
class QNetworkAccessManager;
class QTimer;

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	QTimer *updateTimer;
	QDateTime lastUpdate;
	QSystemTrayIcon *trayIcon;
	QNetworkAccessManager *networkManager;

	void closeEvent(QCloseEvent *event);
	QString timeAgo(QDateTime date);

public slots:
	void showAbout();
	void refreshGists();
	void gistsFetched(QNetworkReply*);
	void messageClicked();

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
};

#endif // MAINWINDOW_H
