#include "window.hh"
#include <QtWidgets/QMenuBar>
#include <QStringList>
#include <QMessageBox>
#include <ctime>
#include "contact.hh"
#include "net.h"
#include "chatwindow.hh"
#include "actions.hh"
#include "userworker.hh"
#include "addaccount.hh"
#include <QCloseEvent>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

char SERVER_ADDR[32] = "127.0.0.1";
char SERVER_HOST[32] = "127.0.0.1:26";

AddAccount *addw;

void block_user(char id[37], char block)
{
	BIO *webp = BIO_new_ssl_connect(ctx);
	if (webp < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_set_conn_hostname(webp, SERVER_HOST) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	SSL *sslp;
	BIO_get_ssl(webp, &sslp);
	if (sslp <= (SSL*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_set_cipher_list(sslp, PREFFERED_CIPHERS) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_set_tlsext_host_name(sslp, SERVER_ADDR) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	BIO *outp = BIO_new_fp(stdout, BIO_NOCLOSE);
	if (outp < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_do_connect(webp) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_do_handshake(webp) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	X509 *cert = SSL_get_peer_certificate(sslp);
	if (cert) X509_free(cert);
	if (!cert) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	char buf[256];
	memset(buf, 0, sizeof(buf));
	if (block) buf[0] = 'B';
	else buf[0] = 'b';
	memcpy(buf + 1, session, sizeof(session));
	memcpy(buf + 38, id, 37);
	BIO_write(webp, buf, sizeof(buf));
	BIO_free(outp);
	if (webp)
		BIO_free_all(webp);
}


void Window::rmFunc()
{
	char user[12];
       	strcpy(user, this->rmUser.toLocal8Bit().data());
	int i = 0;
	for (; user[i] != ' '; i++);
	user[i] = 0;
	for (i = 0; i < cn; i++)
		if (!strcmp(contacts[i].name, user)) {
			block_user(contacts[i].id, 1);
			memset(contacts[i].name, 0, sizeof(contacts[i].name));
			if (i == cn) cn--;
			for (i = 0; i < ct; i++)
				if (chat_threads[i].msg_text
				&& !strcmp(chat_threads[i].cont->name, user))
					delete chat_threads[i].window;
			char fn[24];
			memset(fn, 0, sizeof(fn));
			sprintf(
				fn,
				"%s/new.txt",
				userns
			);
			FILE *fp = fopen(fn, "r+");
			char str2[12], c;
			memset(str2, 0, sizeof(str2));
			i = 0;
			for (c=fgetc(fp);;) {
				for (
					i = 0;
					c != '=';
					str2[i] = c, c=fgetc(fp), i++
				);
				str2[i] = 0;
				if (!strcmp(user, str2)) {
					fseek(fp, ftell(fp), 0);
					fprintf(fp, "1");
					break;
				}
				for (; (c=fgetc(fp)) != '\n';);
				for (; (c=fgetc(fp)) == '\n';);
				if (c == EOF) break;
			}
			fclose(fp);
			memset(fn, 0, sizeof(fn));
			sprintf(
				fn,
				"%s/users.txt",
				userns
			);
			if ((fp = fopen(fn, "r+"))) {
				memset(str2, 0, sizeof(str2));
				i = 0;
				for (c=fgetc(fp);;) {
					for (
						i = 0;
						c != '\n';
						str2[i] = c, c=fgetc(fp), i++
					);
					str2[i] = 0;
					if (!strcmp(user, str2)) {
						fseek(fp, ftell(fp) - 2, 0);
						fprintf(fp, "&");
						break;
					}
					for (; (c=fgetc(fp)) == '\n';);
					if (c == EOF) break;
				}
				fclose(fp);
			}
			fill_view(this);
			return;
		}
}

void Window::prepareMenu(const QPoint &pos)
{
	QTreeWidgetItem *nd = this->view->itemAt(pos);
	if (nd) {
		this->rmUser = nd->text(0);
		QAction *rmAct = new QAction("Remove from list");
		connect(rmAct, SIGNAL(triggered()), this, SLOT(rmFunc()));
		QMenu menu(this);
		menu.addAction(rmAct);
		QPoint pt(pos);
		menu.exec(this->view->mapToGlobal(pos));
	}
}

void Window::showChatFunc()
{
	QModelIndexList index = this->view->selectionModel()->selectedIndexes();
	if (index.size() < 1) {
		puts("No item selected");
		return;
	}
	int in = this->view->currentIndex().row();
	char str[8];
	memset(str, 0, sizeof(str));
	int j = 0;
	for (
		; contacts[in].name[j];
		str[j] = contacts[in].name[j], j++
	);
	int l = 0;
	for (; l < cn; l++)
		if (contacts[l].name[0]
		&& !strcmp(contacts[l].name, str)) break;
	if (contacts[l].window == 1) {
		return;
	}
	/*
	for (j = 0; j < ct; j++)
		if (chat_threads[j].msg_text
		&& chat_threads[j].cont->id == contacts[l].id)
			return;
	*/
	int i = 0;
	for (; i < ct && chat_threads[i].msg_text; i++);
	contacts[l].window = 1;
	chat_threads[i] = chat_thread();
	chat_threads[i].cont = &contacts[in];
	chat_threads[i].window = new chatWindow(&chat_threads[i], i, &ct);
	chat_threads[i].i = i;
	if (i == ct) ct++;
}

void logout()
{
	if (addw) {
		delete addw;
		addw = NULL;
	}
	int i = 0, j = 0;
	for (
		; i < ct;
		j++
	) {
		if (chat_threads[j].msg_text) {
			chat_threads[j].window->close();
			chat_threads[j].msg_text = 0;
		}
	}
	ct = 0;
	if (connect_server()) {
		BIO_free(out);
		if (web)
			BIO_free_all(web);
		return;
	}
	char buf[256];
	memset(buf, 0, sizeof(buf));
	buf[0] = 'L';
	memcpy(buf + 1, session, sizeof(session));
	BIO_write(web, buf, sizeof(buf));
	BIO_free(out);
	if (web)
		BIO_free_all(web);
	cn = 0;
	logged--;
}

void Window::signOutFunc()
{
	logout();
	view->clear();
	delete this->welcome;
	delete this->signout;
	this->initWidgets();
}

void make_directory()
{
	struct stat st = {};
	if (stat(userns, &st) == -1) mkdir(userns, 0700);
}

void Window::signInFunc()
{
	if (connect_server()) {
		BIO_free(out);
		if (web)
			BIO_free_all(web);
		QMessageBox::critical(
			this,
			"Clade Chat",
			"Unable to connect to the server.",
			QMessageBox::Ok
		);
		return;
	}
	char pass[24];
	memset(userns, 0, sizeof(userns));
	memset(pass, 0, sizeof(pass));
	strcpy(userns, this->usert->displayText().toLocal8Bit().data());
	strcpy(pass, this->passt->text().toLocal8Bit());
	char buf[256];
	memset(buf, 0, sizeof(buf));
	buf[0] = 'A';
	int i = 0, j = 1;
	for (
		; userns[i];
		buf[j] = userns[i], i++, j++
	);
	j++;
	for (
		i = 0;
		pass[i];
		buf[j] = pass[i], i++, j++
	);
	BIO_write(web, buf, sizeof(buf));
	memset(buf, 0, sizeof(buf));
	BIO_read(web, buf, sizeof(buf));
	if (buf[0]) {
		memcpy(session, buf + 1, sizeof(session));
		users = buf[38]
			| buf[39] << 8
			| buf[40] << 16
			| buf[41] << 24;
		memset(buf, 0, sizeof(buf));
		buf[0] = 'I';
		memcpy(buf + 1, session, sizeof(session));
		BIO_write(web, buf, sizeof(buf));
		for (;;) {
			memset(buf, 0, sizeof(buf));
			BIO_read(web, buf, sizeof(buf));
			if (!buf[0]) break;
			int l = 0, i = 38, j = 0;
			for (
				; l < cn && contacts[l].name[0];
				l++
			);
			contacts[l] = contact();
			for (
				; buf[i];
				contacts[l].name[j] = buf[i], i++, j++
			);
			contacts[l].name[j] = 0;
			contacts[l].newly = 0;
			if (l == cn) cn++;
		}
		BIO_free(out);
		if (web)
			BIO_free_all(web);
		make_directory();
		char fn[24];
		memset(fn, 0, sizeof(fn));
		sprintf(
			fn,
			"%s/users.txt",
			userns
		);
		FILE *fp = fopen(fn, "r");
		if (fp) {
			char c=fgetc(fp), str2[8];
			int j = 0;
			for (i = cn;; i++) {
				for (
					memset(str2, 0, sizeof(str2)), j = 0;
					c != '\r' && c != '\n' && c != EOF;
					str2[j] = c, c=fgetc(fp), j++
				);
				if (!c && !str2[0]) break;
				for (
					j = 0;
					j < cn;
					j++
				) {
					if (contacts[j].name[0]
					&& !strcmp(str2, contacts[j].name))
						break;
				}
				if (j == cn) {
					if (!info_user(str2)) {
						contacts[j] = contact();
						strcpy(contacts[(int)cn].name, str2);
						cn++;
					}
				}
				if (c == EOF) break;
				for (;; c=fgetc(fp))
					if (c == '\r' || c == '\n' || c == EOF) break;
				if (c == '\r') fgetc(fp);
				if (c == EOF) break;
				c=fgetc(fp);
			}
			fclose(fp);
		}
		char msg[48];
		memset(msg, 0, sizeof(msg));
		for (
			i = 0;
			i < cn;
			i++
		) {
			if (contacts[i].name[0]) {
				info_user(contacts[i].name);
				memcpy(contacts[i].id, bufi + 3, 37);
				contacts[i].online = bufi[1];
				contacts[i].readen = bufi[2];
				if (contacts[i].readen) {
					sprintf(
						msg,
						"%d unread messages from %s\n",
						contacts[i].readen,
						contacts[i].name
					);
					printf("%s", msg);
					this->trayIcon->showMessage(
						"Clade Chat",
						msg
					);
				}
			}
		}
		memset(fn, 0, sizeof(fn));
		sprintf(
			fn,
			"%s/new.txt",
			userns
		);
		if ((fp = fopen(fn, "r"))) {
			char c, str2[8];
			int j = 0;
			for (i = cn, c=fgetc(fp);;) {
				for (
					memset(str2, 0, sizeof(str2)), j = 0;
					c != '=' && c != EOF;
					str2[j] = c, c=fgetc(fp), j++
				);
				if (c == EOF) break;
				str2[j] = 0;
				for (
					j = 0;
					j < cn;
					j++
				) {
					if (contacts[j].name[0]
					&& !strcmp(str2, contacts[j].name))
						contacts[j].newly = fgetc(fp) - '0' + 1;
				}
				for (; (c=fgetc(fp)) != '\n';);
				for (; (c=fgetc(fp)) == '\n';);
				if (c == EOF) break;
			}
			fclose(fp);
		}
		fp = fopen(fn, "a");
		for (i = 0; i < cn; i++) {
			if (!contacts[i].newly)
				fprintf(
					fp,
					"%s=1\n",
					contacts[i].name
				);
		}
		fclose(fp);
		fill_view(this);
		delete this->labelu;
		delete this->usert;
		delete this->labelp;
		delete this->passt;
		delete this->signin;
		char str[64];
		memset(str, 0, sizeof(str));
		sprintf(str, "Welcome back,\n%s", userns);
		this->welcome = new QLabel(str, this);
		this->welcome->move(10, 30);
		this->welcome->setFixedWidth(230);
		this->welcome->setFixedHeight(50);
		this->welcome->show();
		this->signout = new QPushButton("Sign out", this);
		this->signout->move(90, 55);
		this->signout->resize(80, 25);
		connect(this->signout, &QPushButton::clicked, this, &Window::signOutFunc);
		this->signout->show();
		this->showb->setEnabled(true);
		this->uworker = new userWorker(this);
		logged++;
	} else {
		QMessageBox::critical(
			this,
			"Clade Chat",
			"Invalid username or password entered.",
			QMessageBox::Ok
		);
		BIO_free(out);
		if (web)
			BIO_free_all(web);
	}
}

void Window::aboutProduct()
{
	QMessageBox::information(
		this,
		"Clade Chat",
		"Version 0.8\n2025 (c) Code-synth",
		QMessageBox::Close
	);
}

void Window::add()
{
	if (logged && !addw) {
		addw = new AddAccount(this);
	}
}

void Window::quit()
{
	puts("Quit application");
	this->close();
}

void Window::initWidgets()
{
	labelu = new QLabel("username:", this);
	labelu->move(10, 25);
	labelu->show();
	usert = new QLineEdit(0, this);
	usert->resize(90, 25);
	usert->move(95, 30);
	usert->show();
	labelp = new QLabel("password:", this);
	labelp->move(10, 55);
	labelp->show();
	passt = new QLineEdit(0, this);
	passt->setEchoMode(QLineEdit::Password);
	passt->resize(90, 25);
	passt->move(95, 60);
	passt->show();
	signin = new QPushButton("Sign in", this);
	QPixmap signinp("key.png");
	signin->setIcon(signinp);
	connect(signin, &QPushButton::clicked, this, &Window::signInFunc);
	signin->resize(75, 25);
	signin->move(10, 90);
	signin->show();
	showb->setEnabled(false);
}

Window::Window()
	: trayIcon(new QSystemTrayIcon(this))
{
	this->setWindowTitle("Clade Chat");
	this->setFixedSize(QSize(200, 270));
	this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	showb = new QPushButton("View chat", this);
	QPixmap showp("view.png");
	showb->setIcon(showp);
	showb->resize(100, 25);
	showb->move(90, 90);
	this->initWidgets();
	view = new QTreeWidget(this);
	view->resize(180, 138);
	view->move(10, 120);
	view->setColumnCount(2);
	QStringList strl;
	strl << "Accounts" << "State";
	view->setHeaderLabels(strl);
	view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this->view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(prepareMenu(const QPoint &)));
	connect(showb, &QPushButton::clicked, this, &Window::showChatFunc);
	fileMenu = this->menuBar()->addMenu(tr("&File"));
	quitAct = fileMenu->addAction(tr("Quit"));
	QPixmap quitp("exit.png");
	quitAct->setIcon(quitp);
	connect(quitAct, &QAction::triggered, this, &Window::quit);
	editMenu = this->menuBar()->addMenu(tr("&Edit"));
	addAct = editMenu->addAction(tr("&Add account"));
	connect(addAct, &QAction::triggered, this, &Window::add);
	helpMenu = this->menuBar()->addMenu(tr("&Help"));
	aboutAct = helpMenu->addAction(tr("&About Messager"));
	QPixmap aboutp("about.png");
	aboutAct->setIcon(aboutp);
	connect(aboutAct, &QAction::triggered, this, &Window::aboutProduct);
	this->show();

	auto appIcon = QIcon("exit.png");
	this->trayIcon->setIcon(appIcon);
	this->setWindowIcon(appIcon);
	this->trayIcon->show();
}

void Window::closeEvent(QCloseEvent *event)
{
	for (int i = 0; i < ct; i++)
		if (chat_threads[i].msg_text)
			delete chat_threads[i].window;
	event->accept();
}
