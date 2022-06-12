#ifndef CLOUD_DISK_CONFIGS
#define CLOUD_DISK_CONFIGS
struct AppConfig {
	bool is_daemon;
	int port;
	bool debug_mode;
	const char* log_path;
};
struct Config{
	AppConfig app_config;
};

#endif