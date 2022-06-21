#include <file_system_manager.h>

#include <mysql/mysql.h>

using namespace _FileSystemManager;

const char *_SQL_INIT_DATABASE = "\
DROP TABLE IF EXISTS node;\
CREATE TABLE node(\
path char(255) NOT NULL,\
hash char(60),\
PRIMARY KEY (path));";

FileSystemManager::FileSystemManager() {
	sql = nullptr;
	_mysql_error_msg = "";
}
FileSystemManager::~FileSystemManager() {
	if (sql)
		mysql_close(sql);
	return;
}
int FileSystemManager::connect(const SqlConfig &sql_config) {
	if (sql) {
		return _RET_CONNECTED;
	}
	sql = mysql_init(NULL);
	if (!sql) {
		return _RET_CONN;
	}
	if (!mysql_real_connect(sql, sql_config.host, sql_config.user,
							sql_config.pass, sql_config.database_name,
							sql_config.port, NULL, CLIENT_MULTI_STATEMENTS)) {
		_mysql_error_msg = mysql_error(sql);
		mysql_close(sql);
		sql = nullptr;
		return _RET_CONN;
	}
	mysql_set_character_set(sql, sql_config.charset);
	return _RET_OK;
}

int FileSystemManager::initDatabase() {
	if (!sql)
		return _RET_NO_CONN;
	if (mysql_query(sql, _SQL_INIT_DATABASE)) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}
	return _RET_OK;
}
const char *FileSystemManager::getMysqlError() { return _mysql_error_msg; }
const char *FileSystemManager::error(int error_no) {
	switch (error_no) {
	case _RET_OK:
		return "success";
	case _RET_CONNECTED:
		return "be connected";
	case _RET_NO_CONN:
		return "not be connected";
	case _RET_CONN:
		return "connect failed";
	case _RET_SQL_ERR:
		return "mysql query failed";
	default:
		return "unknown error";
	}
	return "unknown error";
}
