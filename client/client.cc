#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <QApplication>
#include "window.hh"

extern SSL_CTX *ctx;
extern char PREFFERED_CIPHERS[];

int main(int argc, char *argv[])
{
	BIO *cbio;
	int len;
	SSL_load_error_strings();
	SSL_library_init();
	ctx = SSL_CTX_new(TLS_client_method());
	if (!ctx) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_CTX_set_options(
		ctx,
		SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION
	) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_CTX_load_verify_locations(
			ctx,
			"domain.crt",
			NULL
	) <= 0) {
		/*
		ERR_print_errors_fp(stderr);
		*/
		exit(1);
	}
	QApplication a(argc, argv);
	a.setStyleSheet("QPushButton { border: 2px outset #777; background: #fff; } QLineEdit { border: 1px solid #000; background: #fff; }");
	Window *window = new Window();
	return a.exec();
}
