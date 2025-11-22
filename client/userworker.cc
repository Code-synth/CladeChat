#include "userworker.hh"
#include "contact.hh"
#include <QTimer>
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

extern contact contacts[8];
extern char cn;

extern int users;
extern char userns[8];
extern char session[37];

extern char bufi[256];

extern SSL_CTX *ctx;

const char PREFFERED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";

char info_user(char *usern)
{
	BIO *webi = BIO_new_ssl_connect(ctx);
	if (webi < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_set_conn_hostname(webi, SERVER_HOST) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	SSL *ssli;
	BIO_get_ssl(webi, &ssli);
	if (ssli <= (SSL*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_set_cipher_list(ssli, PREFFERED_CIPHERS) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_set_tlsext_host_name(ssli, SERVER_ADDR) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	BIO *outi = BIO_new_fp(stdout, BIO_NOCLOSE);
	if (outi < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_do_connect(webi) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (BIO_do_handshake(webi) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	X509 *cert = SSL_get1_peer_certificate(ssli);
	if (cert) X509_free(cert);
	if (!cert) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	memset(bufi, 0, sizeof(bufi));
	bufi[0] = 'U';
	memcpy(bufi + 1, session, sizeof(session));
	int i = 38, j = 0;
	for (
		; usern[j];
		bufi[i] = usern[j], i++, j++
	);
	BIO_write(webi, bufi, sizeof(bufi));
	memset(bufi, 0, sizeof(bufi));
	BIO_read(webi, bufi, sizeof(bufi));
	BIO_free(outi);
	if (webi)
		BIO_free_all(webi);
	if (!bufi[0]) return 3;
	return 0;
}

userWorker::userWorker(QObject *parent)
{
	this->timer = new QTimer(this);
	this->timer->start(1000);
	connect(this->timer, SIGNAL(timeout()), this, SLOT(doWorkLoop()));
	connect(this, SIGNAL(changeText(int, char*, char*)), parent, SLOT(changeTree(int, char*, char*)));
}

void update_store(int i, userWorker *parent)
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
	parent->changeText(i, str, stro);
}

void userWorker::doWorkLoop()
{
	int i = 0;
	for (; i < cn; i++) {
		if (contacts[i].name[0]) {
			if (info_user(contacts[i].name))
				delete this->timer;
			if (contacts[i].online != bufi[1]) {
				contacts[i].online = bufi[1];
				contacts[i].readen = bufi[2];
				update_store(i, this);
			} else if (contacts[i].readen != bufi[2]) {
				contacts[i].online = bufi[1];
				if (bufi[2] > contacts[i].readen) {
					if (contacts[i].window == 0) {
						printf("New message from %s\n", contacts[i].name);
					}
				}
				contacts[i].readen = bufi[2];
				update_store(i, this);
			}
		}
	}
}

void userWorker::doQuit()
{
	delete this->timer;
}
