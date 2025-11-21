#include "contact.hh"
#include "chatwindow.hh"
#include <QtWidgets/QMenuBar>
#include <QStringList>
#include <QtConcurrent>
#include <QScrollBar>
#include <QColor>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

extern char SERVER_ADDR[32];
extern char SERVER_HOST[32];

char buf[256];
extern char userns[8];
extern char session[37];

extern chat_thread chat_threads[8];

extern SSL_CTX *ctx;

const char PREFFERED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";

void put_msg(char id, char *msg2, char *name, chat_thread *cthread)
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
	X509 *cert = SSL_get1_peer_certificate(sslp);
	if (cert) X509_free(cert);
	if (!cert) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	char buf[256];
	memset(buf, 0, sizeof(buf));
	buf[0] = 'P';
	memcpy(buf + 1, session, sizeof(session));
	/*
	buf[1] = session & 0xFF;
	buf[2] = (session & 0xFF00) >> 8;
	buf[3] = (session & 0xFF0000) >> 16;
	buf[4] = (session & 0xFF000000) >> 24;
	*/
	buf[38] = id & 0xFF;
	buf[39] = (id & 0xFF00) >> 8;
	buf[40] = (id & 0xFF0000) >> 16;
	buf[41] = (id & 0xFF000000) >> 24;
	int i = 0, j = 42;
	for (
		; msg2[i];
		buf[j] = msg2[i], i++, j++
	);
	BIO_write(webp, buf, sizeof(buf));
	BIO_free(outp);
	if (webp)
		BIO_free_all(webp);
	for (
		i = 0, j = strlen(cthread->msg);
		userns[i];
		cthread->msg[j] = userns[i], i++, j++
	);
	for (
		cthread->msg[j] = ':', j++, cthread->msg[j] = ' ',
		i = 0, j++;
		msg2[i];
		cthread->msg[j] = msg2[i], i++, j++
	);
	cthread->msg[j] = '\n';
	char fn[24];
	memset(fn, 0, sizeof(fn));
	sprintf(fn, "%s/%s.log", userns, name);
	FILE *fp = fopen(fn, "a+");
	fprintf(fp, "%s: %s\n", userns, msg2);
	fclose(fp);
	cthread->msg_text->moveCursor(QTextCursor::End);
	cthread->msg_text->setTextColor(QColor(0, 0, 255));
	cthread->msg_text->insertPlainText(userns);
	cthread->msg_text->moveCursor(QTextCursor::End);
	cthread->msg_text->setTextColor(QColor(0, 0, 0));
	cthread->msg_text->insertPlainText(": ");
	cthread->msg_text->moveCursor(QTextCursor::End);
	cthread->msg_text->insertPlainText(msg2);
	cthread->msg_text->moveCursor(QTextCursor::End);
	cthread->msg_text->insertPlainText("\n");
	cthread->msg_text->moveCursor(QTextCursor::End);
}

void chatWindow::sendMessage()
{
	put_msg(
		this->cthread->cont->id,
		this->textb->displayText().toLocal8Bit().data(),
		this->cthread->cont->name,
		this->cthread
	);
	this->cthread->msg_text->verticalScrollBar()->setValue(
		this->cthread->msg_text->verticalScrollBar()->maximum()
	);
}

chatWindow::chatWindow(chat_thread *cthread, int thread, char *ct)
{
	this->cthread = cthread;
	this->thread = thread;
	this->ct = ct;
	char title[64];
	memset(title, 0, sizeof(title));
	sprintf(title, "%s - Chat window", this->cthread->cont->name);
	this->setWindowTitle(title);
	this->setFixedSize(QSize(180, 285));
	msg_text = new QTextEdit(this);
	msg_text->setFont(QFont("Georgia", 9));
	msg_text->setReadOnly(true);
	msg_text->resize(160, 240);
	msg_text->move(10, 10);
	msg_text->setTextColor(QColor(0, 0, 0));
	cthread->msg_text = this->msg_text;
	textb = new QLineEdit(this);
	textb->resize(80, 25);
	textb->move(10, 250);
	sendb = new QPushButton("Send", this);
	sendb->resize(80, 25);
	sendb->move(90, 250);
	connect(sendb, &QPushButton::clicked, this, &chatWindow::sendMessage);
	this->cworker = new chatWorker(this, this->cthread);
	this->show();
}

void chatWindow::closeEvent(QCloseEvent *event)
{
	if (this->cworker->thread) {
		this->cworker->thread->terminate();
		this->cworker->thread->wait();
	}
	this->cthread->cont->window = 0;
	this->cthread->msg_text = 0;
	this->cthread = 0;
	if (this->thread == this->ct[0]-1) {
		char i = ct[0] - 2;
		for (;; i--) {
			if (i < 0) {
				ct[0] = 0;
				break;
			}
			if (chat_threads[i].msg_text) {
				ct[0] = i + 1;
				break;
			}
		}
	}
}
