#ifndef CHATWINDOW_HH
#define CHATWINDOW_HH

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QStringList>
#include <QPainter>
#include <QScrollBar>
#include "contact.hh"

class chatWorker;
class chat_thread;
#include "chatworker.hh"

class chatWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit chatWindow(chat_thread *cthread, int thread, char *ct);
	void closeEvent(QCloseEvent *event);
	QTextEdit *msg_text;
	QLineEdit *textb;
	QPushButton *sendb;
private:
	chatWorker *cworker;
	chat_thread *cthread;
	int thread;
	char *ct;
	void sendMessage();
public slots:
	void changeMessage(char *str)
	{
		printf("STR = %s\n", str);
		puts("Done 1");
		this->msg_text->setText(str);
		puts("Done 2");
		this->msg_text->verticalScrollBar()->setValue(
			this->msg_text->verticalScrollBar()->maximum()
		);
		puts("Done 3");
	}
signals:
	void finished();
};

class chat_thread {
public:
	chat_thread() { }
	char msg[4096];
	int i;
	chatWorker *cworker;
	contact *cont;
	chatWindow *window;
	QTextEdit *msg_text;
	QLineEdit *textb;
	QPushButton *send;
};
#endif
