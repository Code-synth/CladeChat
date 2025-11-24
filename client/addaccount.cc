#include "addaccount.hh"
#include <QStringList>
#include "contact.hh"
#include <QCloseEvent>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

extern AddAccount *addw;

extern contact contacts[8];
extern char cn;

extern int users;
extern char userns[8];
extern char info_user(char *usern);

extern char bufi[256];

extern void block_user(char id[37], char block);

void update_store_addacc(int i, AddAccount *parent)
{
	char str[24];
	memset(str, 0, sizeof(str));
	sprintf(
		str,
		"%s (%d)",
		contacts[i].name,
		contacts[i].readen
	);
	char stro[24];
	memset(stro, 0, sizeof(stro));
	sprintf(
		stro,
		contacts[i].online ? "On-line" : "Off-line"
	);
	parent->addItem(i, str, stro);
}

void AddAccount::addFunc()
{
	char fn[24];
	memset(fn, 0, sizeof(fn));
	sprintf(
		fn,
		"%s/new.txt",
		userns
	);
	FILE *fp = fopen(fn, "a");
	fprintf(fp, "%s=1\n", this->user);
	fclose(fp);
	memset(fn, 0, sizeof(fn));
	sprintf(
		fn,
		"%s/users.txt",
		userns
	);
	fp = fopen(fn, "a");
	fprintf(fp, "%s\n", this->user);
	fclose(fp);
	int l = 0, i = 2, j = 0;
	for (
		; l < cn && contacts[l].name[0];
		l++
	);
	contacts[l] = contact();
	strcpy(contacts[l].name, this->user);
	info_user(this->user);
	memcpy(contacts[l].id, bufi + 3, 37);
	contacts[l].online = bufi[1];
	contacts[l].readen = bufi[2];
	contacts[l].newly = 0;
	update_store_addacc(l, this);
	l++;
	memset(fn, 0, sizeof(fn));
	block_user(contacts[l].id, 0);
	if (l == cn + 1) {
		cn++;
	}
	this->close();
}

void AddAccount::check()
{
	strcpy(this->user,  this->text->displayText().toLocal8Bit().data());
	if (!info_user(this->user)) {
		this->label->setText("Account exists on server.");
		this->label->adjustSize();
		this->addb->setDisabled(false);
	} else {
		this->label->setText("Account does not exist.");
		this->label->adjustSize();
		this->addb->setDisabled(true);
	}
}

AddAccount::AddAccount(QObject *parent)
{
	this->setWindowTitle("Add account");
	this->setFixedSize(QSize(240, 120));
	this->label = new QLabel("Type account name to add to\nyour contact list.", this);
	this->label->move(10, 10);
	this->label->adjustSize();
	this->text = new QLineEdit(this);
	this->text->move(10, 50);
	this->text->resize(80, 25);
	QPushButton *check = new QPushButton("Check", this);
	check->move(100, 50);
	check->resize(80, 25);
	connect(check, SIGNAL(clicked()), this, SLOT(check()));
	this->addb = new QPushButton("Add", this);
	this->addb->move(10, 90);
	this->addb->resize(80, 25);
	this->addb->setDisabled(true);
	connect(this->addb, SIGNAL(clicked()), this, SLOT(addFunc()));
	connect(this, SIGNAL(addItem(int, char*, char*)), parent, SLOT(addItem(int, char*, char*)));
	this->show();
}

void AddAccount::closeEvent(QCloseEvent *event)
{
	delete addw;
	addw = NULL;
}