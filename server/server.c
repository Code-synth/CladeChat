#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <uuid/uuid.h>
sqlite3 *db;
SSL *ssl;

char query[128];
int conn;
char buf[256];

struct sessions_ {
	int userid;
	char id[37];
	int time;
} sessions[96];
int s = 0;
int stat = 0;

std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

int number(char *str)
{
	int i = 0, j = 0;
	for (
		; str[j];
		i = (i * 10) + str[j] - '0', j++
	);
	return i;
}

void new_session(char *argv)
{
	uuid_t binuuid;
	uuid_generate_random(binuuid);
	char *uuid = (char*)malloc(37);
	uuid_unparse_lower(binuuid, uuid);
	printf("New UUID = %s\n", uuid);
	strcpy(sessions[s].id, uuid);
	sessions[s].userid = number(argv);
	sessions[s].time = time(0);
	memcpy(buf + 1, sessions[s].id, sizeof(sessions[s].id));
	buf[38] = sessions[s].userid & 0xFF;
	buf[39] = (sessions[s].userid & 0xFF00) >> 8;
	buf[40] = (sessions[s].userid & 0xFF0000) >> 16;
	buf[41] = (sessions[s].userid & 0xFF000000) >> 24;
	s++;
}

int ss = 0;

char check_session(char session[37])
{
	int i = s;
	for (ss = 0;; ss++)
		if (sessions[ss].time) {
			if (!strcmp(sessions[ss].id, session))
				return 1;
			i--;
			if (!i) return 0;
		}
	return 0;
}

int find_known_func(
	void *NotUsed,
	int argc, char **argv,
	char **azColName
)
{
	NotUsed = 0;
	char query2[128];
	memset(query2, 0, sizeof(query2));
	sprintf(
		query2,
		"SELECT COUNT(*) from ignore WHERE `from` = %d AND `to` = %d",
		sessions[ss].userid,
		number(argv[0])
	);
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		puts("error while selecting row from table 'ignore'");
		sqlite3_close(db);
		exit(12);
	}
	sqlite3_step(stmt);
	if (sqlite3_column_int(stmt, 0)) {
		return 0;
	}
	memset(buf, 0, sizeof(buf));
	buf[0] = 1;
	buf[1] = number(argv[0]);
	memset(query2, 0, sizeof(query2));
	sprintf(query2, "SELECT * FROM users WHERE id = %d", number(argv[0]));
	rc = sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		puts("error while selecting row from table 'users'");
		sqlite3_close(db);
		exit(10);
	}
	sqlite3_step(stmt);
	char str[8];
	memset(str, 0, sizeof(str));
	strcpy(str, (char*)sqlite3_column_text(stmt, 1));
	printf("User = %s\n", str);
	int i = 0, j = 2;
	for (
		; str[i];
		buf[j] = str[i], i++, j++
	);
	SSL_write(ssl, buf, sizeof(buf));
	stat = 1;
	return 0;
}

int msg_get_func(
	void *NotUsed,
	int argc, char **argv,
	char **azColName
)
{
	NotUsed = 0;
	printf("msg = %s\n", argv[3]);
	memset(buf, 0, sizeof(buf));
	buf[0] = 1;
	buf[1] = number(argv[1]);
	char i = 0, j = 2;
	for (; argv[3][i]; buf[j] = argv[3][i], i++, j++);
	buf[j] = 0;
	SSL_write(ssl, buf, sizeof(buf));
	if (number(argv[2]) == sessions[ss].userid) {
		char query2[128];
		memset(query2, 0, sizeof(query2));
		strcat(query2, "UPDATE msg SET readen = 1 WHERE id = ");
		strcat(query2, argv[0]);
		if (sqlite3_exec(db, query2, 0, 0, 0) != SQLITE_OK) {
			puts("error while updating row from table 'msg'");
			exit(0);
		}
	}
	stat = 1;
	return 0;
}

int user_find_func(
	void *NotUsed,
	int argc, char **argv,
	char **azColName
)
{
	NotUsed = 0;
	int id = number(argv[0]);
	printf("id = %d\n", id);
	char j = 0;
	buf[0] = 1;
	sqlite3_stmt *stmt;
	char query2[128];
	memset(query2, 0, sizeof(query2));
	sprintf(
		query2,
		"SELECT COUNT(*) FROM msg WHERE `from` = %d AND `to` = %d AND readen = 0",
		number(argv[0]),
		sessions[ss].userid
	);
	int rc = sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		puts("error while selecting row from table 'users'");
		sqlite3_close(db);
		exit(11);
	}
	sqlite3_step(stmt);
	buf[2] = sqlite3_column_int(stmt, 0);
	buf[3] = id & 0xFF;
	buf[4] = (id & 0xFF00) >> 8;
	buf[5] = (id & 0xFF0000) >> 16;
	buf[6] = (id & 0xFF000000) >> 24;
	stat = 1;
	for (; j < 2; j++) {
		if (sessions[j].time && sessions[j].userid == id) {
			buf[1] = 1;
			return 0;
		}
	}
	buf[1] = 0;
	return 0;
}

int users_func(
	void *NotUsed,
	int argc, char **argv,
	char **azColName
)
{
	NotUsed = 0;
	buf[0] = 1;
	new_session(argv[0]);
	SSL_write(ssl, buf, sizeof(buf));
	stat = 1;
	return 0;
}

int main(void)
{
	if (sqlite3_open("server.db", &db) != SQLITE_OK) {
		puts("error: Unable to open database 'server.db'");
		exit(1);
	}
	if (
		sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users(id INT, name TEXT, passw TEXT);", 0, 0, 0)
		!= SQLITE_OK
	) {
		puts("error: Unable to create table 'users'");
		sqlite3_close(db);
		exit(2);
	}
	if (
		sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS msg(id INT, `from` INT, `to` INT, msg TEXT, readen INT);", 0, 0, 0)
		!= SQLITE_OK
	) {
		puts("error: Unable to create table 'msg'");
		sqlite3_close(db);
		exit(2);
	}
	if (
		sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS ignore(`from` INT, `to` INT);", 0, 0, 0)
		!= SQLITE_OK
	) {
		puts("error: Unable to create table 'ignore'");
		sqlite3_close(db);
		exit(2);
	}
	signal(SIGPIPE, SIG_IGN);
	SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	if (!ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(3);
	}
	if (SSL_CTX_set_options(
		ctx,
		SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION
	) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(3);
	}
	if (SSL_CTX_use_certificate_file(
		ctx,
		"domain.crt",
		SSL_FILETYPE_PEM
	) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(3);
	}
	if (SSL_CTX_use_PrivateKey_file(
		ctx,
		"domain.key",
		SSL_FILETYPE_PEM
	) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(3);
	}
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		puts("error while creating a socket");
		exit(4);
	}
	struct sockaddr_in servaddr, cli;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(26);
	if (bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
		puts("error while binding to address '127.0.0.1'");
		perror("bind");
		exit(4);
	}
	char str[32];
	char i, j , l;
	char session[37];
	for (;; SSL_shutdown(ssl), SSL_free(ssl), close(conn)) {
		if (listen(sock, 5)) {
			puts("error while listening server");
			exit(5);
		}
		int len = sizeof(cli);
		if ((conn = accept(sock, (struct sockaddr*)&cli, (socklen_t*)&len)) < 0) {
			puts("error while server accepting");
			exit(6);
		}
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, conn);
		if (SSL_accept(ssl) > 0) {
			for (;;) {
				memset(buf, 0, sizeof(buf));
				if (!SSL_read(ssl, buf, sizeof(buf))) break;
				printf("Got message from a client = '%c'\n", buf[0]);
				if (buf[0] == 'A') {
					memset(query, 0, sizeof(query));
					strcat(query, "SELECT * FROM users WHERE name = '");
					for (i = 0, j = 1; buf[j]; str[i] = buf[j], i++, j++);
					str[i] = 0;
					strcat(query, str);
					strcat(query, "' AND passw = '");
					for (i = 0, j++; buf[j]; str[i] = buf[j], i++, j++);
					str[i] = 0;
					strcat(query, str);
					strcat(query, "'");
					if (sqlite3_exec(db, query, users_func, 0, 0) != SQLITE_OK) {
						puts("error while selecting row from table 'users'");
						exit(7);
					}
					if (!stat) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
					} else stat = 0;
				}
				if (buf[0] == 'L') {
					memcpy(session, buf + 1, sizeof(session));
					if (!check_session(session))
						continue;
					memset(sessions[ss].id, 0, sizeof(sessions[ss].id));
					sessions[ss].userid = 0;
					sessions[ss].time = 0;
					if (ss == s-1) s--;
				}
				if (buf[0] == 'I') {
					memcpy(session, buf + 1, sizeof(session));
					printf("Session = %s\n", session);
					printf("User ID = %d\n", sessions[ss].userid);
					if (!check_session(session)) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
						continue;
					}
					memset(query, 0, sizeof(query));
					sprintf(
						query,
						"SELECT DISTINCT `from` FROM msg WHERE `to` = %d",
						sessions[ss].userid
					);
					printf("sql = %s\n", query);
					if (sqlite3_exec(db, query, find_known_func, 0, 0) != SQLITE_OK) {
						puts("error while selecting row from table 'msg'");
						exit(9);
					}
					buf[0] = 0;
					SSL_write(ssl, buf, 1);
					if (stat) stat = 0;
				}
				if (buf[0] == 'U') {
					memcpy(session, buf + 1, sizeof(session));
					printf("Session = %s\n", session);
					if (!check_session(session)) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
						continue;
					}
					for (
						i = 0, j = 38;
						buf[j];
						str[i] = buf[j], i++, j++
					);
					str[i] = 0;
					printf("str = %s\n", str);
					memset(query, 0, sizeof(query));
					sprintf(query, "SELECT * FROM users WHERE name = '%s'", str);
					if (sqlite3_exec(db, query, user_find_func, 0, 0) != SQLITE_OK) {
						puts("!!! error while selecting row from table 'users'");
						printf("query = %s\n", query);
						printf("errmsg = %s\n", sqlite3_errmsg(db));
						exit(10);
					}
					if (!stat) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
					} else stat = 0, SSL_write(ssl, buf, sizeof(buf));
				}
				if (buf[0] == 'G' || buf[0] == 'g') {
					memcpy(session, buf + 1, sizeof(session));
					if (!check_session(session)) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
						continue;
					}
					int id =
						buf[38]
						| buf[39] << 8
						| buf[40] << 16
						| buf[41] << 24;
					memset(query, 0, sizeof(query));
					if (buf[0] == 'G') {
						sprintf(
							query,
							"SELECT * FROM msg WHERE `from` = %d AND `to` = %d OR `from` = %d AND `to` = %d",
							id, sessions[ss].userid,
							sessions[ss].userid, id
						);
					} else {
						sprintf(
							query,
							"SELECT * FROM msg WHERE `from` = %d AND `to` = %d AND readen = 0",
							id, sessions[ss].userid
						);
					}
					if (sqlite3_exec(db, query, msg_get_func, 0, 0) != SQLITE_OK) {
						puts("error while selecting row from table 'msg'");
						exit(11);
					}
					buf[0] = 0;
					SSL_write(ssl, buf, 1);
					if (stat) stat = 0;
				}
				if (buf[0] == 'P') {
					memcpy(session, buf + 1, sizeof(session));
					if (!check_session(session)) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
						continue;
					}
					int id =
						buf[38]
						| buf[39] << 8
						| buf[40] << 16
						| buf[41] << 24;
					memset(str, 0, sizeof(str));
					for (
						i = 0, j = 42;
						buf[j];
						str[i] = buf[j], i++, j++
					);
					str[i] = 0;
					memset(query, 0, sizeof(query));
					sprintf(
						query,
						"SELECT COUNT(*) from msg"
					);
					sqlite3_stmt *stmt;
					int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
					if (rc != SQLITE_OK) {
						puts("error while selecting row from table 'msg'");
						sqlite3_close(db);
						exit(12);
					}
					sqlite3_step(stmt);
					std::string strc(str);
					strc = ReplaceAll(strc, std::string("'"), std::string("''"));
					memset(query, 0, sizeof(query));
					sprintf(
						query,
						"INSERT INTO msg(id, `from`, `to`, msg, readen) VALUES(%d, %d, %d, '%s', 0)",
						sqlite3_column_int(stmt, 0),
						sessions[ss].userid, id, strc.c_str()
					);
					if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
						puts("error while inserting a row into table 'msg'");
						exit(13);
					}
				}
				if (buf[0] == 'B') {
					memcpy(session, buf + 1, sizeof(session));
					if (!check_session(session)) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
						continue;
					}
					int id =
						buf[38]
						| buf[39] << 8
						| buf[40] << 16
						| buf[41] << 24;
					memset(str, 0, sizeof(str));
					for (
						i = 0, j = 42;
						buf[j];
						str[i] = buf[j], i++, j++
					);
					str[i] = 0;
					memset(query, 0, sizeof(query));
					sprintf(
						query,
						"INSERT INTO ignore(`from`, `to`) VALUES(%d, %d)",
						sessions[ss].userid, id
					);
					if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
						puts("error while inserting a row into table 'ignore'");
						exit(15);
					}
				}
				if (buf[0] == 'b') {
					memcpy(session, buf + 1, sizeof(session));
					if (!check_session(session)) {
						buf[0] = 0;
						SSL_write(ssl, buf, 1);
						continue;
					}
					int id =
						buf[38]
						| buf[39] << 8
						| buf[40] << 16
						| buf[41] << 24;
					memset(str, 0, sizeof(str));
					for (
						i = 0, j = 42;
						buf[j];
						str[i] = buf[j], i++, j++
					);
					str[i] = 0;
					memset(query, 0, sizeof(query));
					sprintf(
						query,
						"DELETE FROM ignore WHERE `from` = %d AND `to` =  %d",
						sessions[ss].userid, id
					);
					if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
						puts("error while deleting a row from table 'ignore'");
						exit(16);
					}
				}
			}
		} else {
			ERR_print_errors_fp(stderr);
			exit(14);
		}
	}
	exit(0);
}
