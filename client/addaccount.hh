#ifndef ADDACCOUNT_HH
#define ADDACCOUNT_HH

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QStringList>
#include <QPainter>

class AddAccount : public QMainWindow
{
	Q_OBJECT
public:
	explicit AddAccount(QObject *parent);
private:
	char user[24];
	void closeEvent(QCloseEvent *event);
	QLabel *label;
	QLineEdit *text;
	QPushButton *addb;
private slots:
	void check();
	void addFunc();
signals:
	void addItem(int i, char *str, char *stro);
};
#endif
