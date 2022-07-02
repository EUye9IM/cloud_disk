#include "logc/logc.h"
// #include <agps/agps.h>
// #include <cfgps/cfgps.h>
// #include <cfgps/check.h>
// #include <configs.h>
#include <iostream>

#include "command_handler.h"

using namespace std;

// int init(int argc, char **argv, Config &config) {
// 	// parse arguments
// 	agps::Parser ap;
// 	ap.add(agps::Type::STR, 'c', "conf-path", "config file path", false,
// 		   agps::Value{.Str = "config.ini"});
// 	ap.add(agps::Type::FLAG, 'h', "help", "show usage", false);

// 	ap.parse(argc, (const char **)argv);

// 	if (!ap.success()) {
// 		cout << ap.error() << endl;
// 		ap.printUsage();
// 		return -1;
// 	}
// 	if (ap.isExist("help")) {
// 		ap.printUsage();
// 		return -1;
// 	}

// 	const char *config_file_path = ap.get("conf-path").Str;

// 	// read config file
// 	cfgps::Parser cp;

// 	cp.set(cfgps::Type::STR, "app", "is_daemon", cfgps::Value{.Str = "false"},
// 		   CHECK_STR_VERIFY_YNTF);
// 	cp.set(cfgps::Type::INT, "app", "port", cfgps::Value{.Int = 80},
// 		   CHECK_INT_BETWEEN(0, 65535));
// 	cp.set(cfgps::Type::STR, "app", "debug_mode", cfgps::Value{.Str = "false"},
// 		   CHECK_STR_VERIFY_YNTF);
// 	cp.set(cfgps::Type::STR, "app", "log_path",
// 		   cfgps::Value{.Str = "cloud_disk.log"});

// 	cp.readFile(config_file_path);

// 	config.app_config.is_daemon =
// 		CHECK_STR_VERIFY_Y(cp.get("app", "is_daemon")) ||
// 		CHECK_STR_VERIFY_T(cp.get("app", "is_daemon"));
// 	config.app_config.port = cp.getInt("app", "port");
// 	config.app_config.debug_mode =
// 		CHECK_STR_VERIFY_Y(cp.get("app", "debug_mode")) ||
// 		CHECK_STR_VERIFY_T(cp.get("app", "debug_mode"));
// 	config.app_config.log_path = cp.getStr("app", "log_path");

// 	// open log
// 	if (config.app_config.debug_mode) {
// 		LogC::log_open(stdout);
// 		LogC::log_set(LogC::LOG_FLAG_DEBUG);
// 	} else {
// 		LogC::log_open(config.app_config.log_path);
// 	}

// 	return 0;
// }

int main(int argc, char **argv) {
	// Config config;
	// if (init(argc, argv, config) < 0)
	// 	return 0;
	// LogC::log_println("initial success");
	// AppConfig app_config=config.app_config;
	// LogC::log_debug("\napp-config:\n"
	// 				"\tis_daemon:%s\n"
	// 				"\tport:%d\n"
	// 				"\tdebug_mode:%s\n"
	// 				"\tlog_path:%s\n\n",
	// 				app_config.is_daemon ? "true" : "false", app_config.port,
	// 				app_config.debug_mode ? "true" : "false", app_config.log_path);

#ifdef __ANAKIN_DEBUG__
	CommandHandlerDebug();
#endif

	return 0;
}