#ifndef CLOUD_DISK_CONFIGS
#define CLOUD_DISK_CONFIGS
#include <sql_config.h>
struct AppConfig {
	bool is_daemon;
	int port;
	bool debug_mode;
	const char* log_path;
};
struct Config{
	AppConfig app_config;
	SqlConfig sql_config;
};

#endif