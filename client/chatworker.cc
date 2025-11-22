#include "chatworker.hh"
#include <QTimer>
#include <QScrollBar>
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

extern char users[37];
extern char userns[8];
extern char session[37];

extern char bufi[256];

extern SSL_CTX *ctx;

const char PREFFERED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";

char get_msg(char newly, char user[37], char *usern, char *msgt, chat_thread *cthread)
{
	BIO *webg = BIO_new_ssl_connect(ctx);
	if (webg < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_set_conn_hostname(webg, SERVER_HOST) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	SSL *sslg;
	BIO_get_ssl(webg, &sslg);
	if (sslg <= (SSL*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_set_cipher_list(sslg, PREFFERED_CIPHERS) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_set_tlsext_host_name(sslg, SERVER_ADDR) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	BIO *outg = BIO_new_fp(stdout, BIO_NOCLOSE);
	if (outg < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_do_connect(webg) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_do_handshake(webg) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	X509 *cert = SSL_get1_peer_certificate(sslg);
	if (cert) X509_free(cert);
	if (!cert) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	char buf[256];
	memset(buf, 0, sizeof(buf));
	if (!newly) buf[0] = 'g';
	else buf[0] = 'G';
	memcpy(buf + 1, session, sizeof(session));
	memcpy(buf + 38, user, 37);
	BIO_write(webg, buf, sizeof(buf));
	memset(msgt, 0, sizeof(msgt));
	int j = 0;
	memset(buf, 0, sizeof(buf));
	BIO_read(webg, buf, sizeof(buf));
	if (!buf[0]) {
		BIO_free(outg);
		if (webg)
			BIO_free_all(webg);
		return 1;
	}
	if (newly) {
		cthread->msg_text->setPlainText(msgt);
	}
	char user_[37];
	for (;;) {
		char *str;
		memcpy(user_, buf + 1, 37);
		if (strcmp(user_, user)) {
			str = userns;
		} else str = usern;
		int i = 0;
		for (
			; str[i];
			msgt[j] = str[i], i++, j++
		);
		cthread->msg_text->moveCursor(QTextCursor::End);
		cthread->msg_text->setTextColor(QColor(0, 0, 255));
		cthread->msg_text->insertPlainText(str);
		cthread->msg_text->moveCursor(QTextCursor::End);
		cthread->msg_text->setTextColor(QColor(0, 0, 0));
		cthread->msg_text->insertPlainText(": ");
		cthread->msg_text->moveCursor(QTextCursor::End);
		msgt[j] = ':';
		j++;
		msgt[j] = ' ';
		j++;
		char msg2[256];
		memset(msg2, 0, sizeof(msg2));
		int k = 0;
		for (
			i = 38;
			buf[i];
			msgt[j] = buf[i], msg2[k] = buf[i], i++, j++, k++
		);
		msgt[j] = '\n';
		msg2[k] = '\n';
		cthread->msg_text->insertPlainText(msg2);
		cthread->msg_text->moveCursor(QTextCursor::End);
		j++;
		memset(buf, 0, sizeof(buf));
		BIO_read(webg, buf, sizeof(buf));
		if (!buf[0]) break;
	}
	cthread->msg_text->verticalScrollBar()->setValue(
		cthread->msg_text->verticalScrollBar()->maximum()
	);
	BIO_free(outg);
	if (webg)
		BIO_free_all(webg);
	return 0;
}

chatWorker::chatWorker(QObject *parent, chat_thread *threadc)
{
	this->cthread = threadc;
	this->thread = new QThread(parent);
	connect(this->thread, SIGNAL(started()), this, SLOT(doWork()));
	connect(this->thread, SIGNAL(finished()), this, SLOT(doQuit()));
	this->timer = new QTimer(this);
	this->timer->start(1000);
	connect(this->timer, SIGNAL(timeout()), this, SLOT(doWorkLoop()));
	connect(this, SIGNAL(changeText(char*)), parent, SLOT(changeMessage(char*)));
	this->thread->start();
}

void chatWorker::doWork()
{
	if (this->cthread->cont->newly == 2
	|| !this->cthread->cont->newly) {
		char fn[24];
		FILE *fp;
		memset(this->cthread->msg, 0, sizeof(this->cthread->msg));
		if (!get_msg(
			1,
			this->cthread->cont->id,
			this->cthread->cont->name,
			this->cthread->msg,
			this->cthread
		)) {
			sprintf(
				fn,
				"%s/%s.log",
				userns,
				this->cthread->cont->name
			);
			fp = fopen(fn, "w");
			fprintf(fp, "%s", this->cthread->msg);
			fclose(fp);
		}
		memset(fn, 0, sizeof(fn));
		sprintf(
			fn,
			"%s/new.txt",
			userns
		);
		fp = fopen(fn, "r+");
		char str2[12], c;
		memset(str2, 0, sizeof(str2));
		int i = 0;
		for (c=fgetc(fp);;) {
			for (
				i = 0;
				c != '=';
				str2[i] = c, c=fgetc(fp), i++
			);
			str2[i] = 0;
			if (!strcmp(
				this->cthread->cont->name,
				str2
			)) {
				fseek(fp, ftell(fp), 0);
				fprintf(fp, "0");
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
			"%s/%s.log",
			userns,
			this->cthread->cont->name
		);
		fp = fopen(fn, "rb+");
		if (!fp) fp = fopen(fn, "wb");
		fclose(fp);
		this->cthread->cont->newly = 1;
	} else {
		char fn[24];
		memset(fn, 0, sizeof(fn));
		sprintf(
			fn,
			"%s/%s.log",
			userns,
			this->cthread->cont->name
		);
		FILE *fp = fopen(fn, "r");
		if (fp) {
			memset(this->cthread->msg, 0, sizeof(this->cthread->msg));
			int i = 0;
			char c, str_[2];
			memset(str_, 0, sizeof(str_));
			this->cthread->msg_text->setTextColor(QColor(0, 0, 255));
			for (; (c=fgetc(fp)) != EOF; this->cthread->msg[i] = c, i++) {
				if (c == ':') {
					this->cthread->msg_text->setTextColor(QColor(0, 0, 0));
				}
				if (c == '\n') {
					this->cthread->msg_text->setTextColor(QColor(0, 0, 255));
				}
				str_[0] = c;
				this->cthread->msg_text->insertPlainText(str_);
			}
			fclose(fp);
			this->cthread->msg_text->verticalScrollBar()->setValue(
				this->cthread->msg_text->verticalScrollBar()->maximum()
			);
		} else fclose(fp);
	}
	char msg2[4096];
	memset(msg2, 0, sizeof(msg2));
	char fn[24];
	memset(fn, 0, sizeof(fn));
	sprintf(
		fn,
		"%s/%s.log",
		userns,
		this->cthread->cont->name
	);
	FILE *fp;
	if (!this->cthread) return;
}

void chatWorker::doWorkLoop()
{
	char msg2[4096];
	memset(msg2, 0, sizeof(msg2));
	FILE *fp;
	char fn[24];
	memset(fn, 0, sizeof(fn));
	sprintf(
		fn,
		"%s/%s.log",
		userns,
		this->cthread->cont->name
	);
	if (strcmp(this->cthread->cont->id, users) && !get_msg(
		0,
		this->cthread->cont->id,
		this->cthread->cont->name,
		msg2,
		this->cthread
	)) {
		int i = 0, j = strlen(this->cthread->msg);
		for (
			; msg2[i];
			this->cthread->msg[j] = msg2[i], i++, j++
		);
		fp = fopen(fn, "a");
		fprintf(fp, msg2);
		fclose(fp);
		memset(msg2, 0, sizeof(msg2));
	}
}

void chatWorker::doQuit()
{
	delete this->thread;
	delete this->timer;
}
