#ifndef USERWORKER_HH
#define USERWORKER_HH
#include "window.hh"
#include <QObject>
#include <QThread>
class userWorker : public QObject
{
	Q_OBJECT
public:
	explicit userWorker(QObject *parent);
private:
	QTimer *timer;
private slots:
	void doWorkLoop();
	void doQuit();
signals:
	void changeText(int i, char *str, char *stro);
};
#endif
