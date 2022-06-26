#ifndef CLOUD_DISK_USER_INFO_MANAGER
#define CLOUD_DISK_USER_INFO_MANAGER
#include <sql_config.h>

#include <mutex>
#include <mysql/mysql.h>
#include <string>

class UserInfoManager {
public:
	UserInfoManager();
	~UserInfoManager();
	int connect(const SqlConfig &sql_config);

	int add(const std::string &user, const std::string &pass);
	int check(const std::string &user, const std::string &pass);
	int change(const std::string &user, const std::string &pass);
	int del(const std::string &user);

	int initDatabase();
	static const char *error(int error_no);
	const char *getMysqlError();

private:
	MYSQL *sql;
	std::string _mysql_error_msg;
	std::mutex _lock;
};

namespace _UserInfoManager {
const int _RET_OK = 0;
const int _RET_CONNECTED = -1;
const int _RET_NO_CONN = -2;
const int _RET_CONN = -3;
const int _RET_SQL_ERR = -4;
const int _RET_USER_EXIST = -5;
const int _RET_CHECK_FAILED_PASS = -6; // password incorrect
const int _RET_CHECK_FAILED_USER = -7; // user does not exist
} // namespace _UserInfoManager

#endif