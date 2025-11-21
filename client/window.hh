#ifndef WINDOW_HH
#define WINDOW_HH

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QStringList>
#include <QPainter>

class userWorker;
#include "userworker.hh"

class Window : public QMainWindow
{
	Q_OBJECT
private:
	QMenu *fileMenu;
	QAction *quitAct;
	QMenu *editMenu;
	QAction *addAct;
	QMenu *helpMenu;
	QAction *aboutAct;
	QString rmUser;
public:
	explicit Window();
	void closeEvent(QCloseEvent *event);
	void initWidgets();
	QTreeWidget *view;
	QLabel *labelu;
	QLineEdit *usert;
	QLabel *labelp;
	QLineEdit *passt;
	QPushButton *signin;
	QPushButton *showb;
	QLabel *welcome;
	QPushButton *signout;
	QThread *thread;
	userWorker *uworker;
private slots:
	void quit();
	void add();
	void aboutProduct();
	void signInFunc();
	void signOutFunc();
	void showChatFunc();
	void prepareMenu(const QPoint &pos);
	void rmFunc();
public slots:
	void changeTree(int i, char *str, char *stro)
	{
		QTreeWidgetItem *item = this->view->topLevelItem(i);
		item->setText(0, str);
		item->setText(1, stro);
	}
	void addItem(int i, char *str, char *stro) {
		QStringList strl;
		strl << str;
		QTreeWidgetItem *j = new QTreeWidgetItem(strl);
		j->setText(1, stro);
		this->view->addTopLevelItem(j);
	}
};
#endif
