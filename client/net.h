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

contact contacts[8];
char cn = 0;

int users;
char userns[8];

char logged = 0;

int session = 0;
int sock = 0;

SSL *ssl;
SSL_CTX *ctx;

BIO *web, *out;

const char PREFFERED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";

int connect_server()
{
	if ((web = BIO_new_ssl_connect(ctx)) < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if (BIO_set_conn_hostname(web, SERVER_HOST) <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	BIO_get_ssl(web, &ssl);
	if (ssl <= (SSL*)0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if (SSL_set_cipher_list(ssl, PREFFERED_CIPHERS) <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if (SSL_set_tlsext_host_name(ssl, SERVER_ADDR) <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if ((out = BIO_new_fp(stdout, BIO_NOCLOSE)) < (BIO*)0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if (BIO_do_connect(web) <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if (BIO_do_handshake(web) <= 0) {
		puts("error while handshake with server");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	X509 *cert = SSL_get_peer_certificate(ssl);
	if (cert) X509_free(cert);
	if (!cert) {
		puts("error while getting peer certificate");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	return 0;
}
