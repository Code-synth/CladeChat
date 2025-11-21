#include "chatwindow.hh"
#include <QtWidgets/QTextEdit>
#include <unistd.h>
#include <sys/stat.h>
#include <QAction>
chat_thread chat_threads[8] = {};
char ct;

void fill_view(Window *parent)
{
	parent->view->clear();
	char str[24], stro[24];
	int i = 0;
	for (; i < cn; i++) {
		if (contacts[i].name[0]) {
			memset(str, 0, sizeof(str));
			sprintf(
				str,
				"%s (%d)",
				contacts[i].name,
				contacts[i].readen
			);
			QStringList strl;
			strl << str;
			QTreeWidgetItem *j = new QTreeWidgetItem(strl);
			memset(stro, 0, sizeof(stro));
			sprintf(
				stro,
				"%s",
				contacts[i].online ? "On-line" : "Off-line"
			);
			j->setText(1, stro);
			parent->view->addTopLevelItem(j);
		}
	}
}

char info_user(char *usern);
char bufi[256];
