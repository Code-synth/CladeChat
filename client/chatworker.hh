#ifndef CHATWORKER_HH
#define CHATWORKER_HH
#include <QObject>
#include <QThread>
#include "chatwindow.hh"
class chatWorker : public QObject
{
	Q_OBJECT
public:
	explicit chatWorker(QObject *parent = 0, chat_thread *threadc = 0);
	QThread *thread;
private:
	chat_thread *cthread;
	QTimer *timer;
private slots:
	void doWork();
	void doWorkLoop();
	void doQuit();
signals:
	void changeText(char *str);
};
#endif
