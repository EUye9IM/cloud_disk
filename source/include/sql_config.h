#ifndef CLOUD_DISK_SQL_CONFIG
#define CLOUD_DISK_SQL_CONFIG
struct SqlConfig {
	const char *host;
	int port;
	const char *user;
	const char *pass;
	const char *database_name;
	const char *charset;
};
#endif