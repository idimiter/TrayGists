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
class QMenu;
class QAction;
class QNetworkAccessManager;
class QTimer;
class QNetworkReply;
class QSettings;

struct Gist {
	QString name;
	QString description;
	QUrl url;
	QUrl iconUrl;
	QDateTime updatedAt;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	QSettings *userSettings;
	QList<Gist> gists;

	QTimer *updateTimer;
	QDateTime lastUpdate;
	QSystemTrayIcon *trayIcon;
	QMenu *mainMenu;
	QMenu *gistsMenu;
	QNetworkAccessManager *networkManager;

	void closeEvent(QCloseEvent *event);
	QString timeAgo(QDateTime date);
	void loadGistIcon(Gist*, QAction*);


public slots:
	void showAbout();
	void refreshGists();
	void gistsFetched(QNetworkReply*);
	void messageClicked();
	void gistSelected(QAction*);
	void gistIconLoaded(QNetworkReply*);

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
};

#endif // MAINWINDOW_H
