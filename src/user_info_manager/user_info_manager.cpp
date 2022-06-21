#include <cstring>
#include <mysql/mysql.h>
#include <user_info_manager.h>

using namespace _UserInfoManager;

static const char *_SQL_INIT_DATABASE = "\
DROP TABLE IF EXISTS userinfo;\
CREATE TABLE userinfo(\
name char(255) NOT NULL,\
pass char(50),\
PRIMARY KEY (name));";

UserInfoManager::UserInfoManager() {
	sql = nullptr;
	_mysql_error_msg = "";
}
UserInfoManager::~UserInfoManager() {
	if (sql)
		mysql_close(sql);
	return;
}
int UserInfoManager::connect(const SqlConfig &sql_config) {
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
int UserInfoManager::add(const std::string &user, const std::string &pass) {
	if (!sql)
		return _RET_NO_CONN;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND bind[2];
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "INSERT INTO userinfo VALUES(?,SHA1(?))",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(bind, 0, sizeof(bind));

	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (void *)user.c_str();
	bind[0].buffer_length = user.length();

	bind[1].buffer_type = MYSQL_TYPE_STRING;
	bind[1].buffer = (void *)pass.c_str();
	bind[1].buffer_length = pass.length();

	if (mysql_stmt_bind_param(stmt, bind)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
		if (mysql_stmt_errno(stmt) == 1062) {
			mysql_stmt_close(stmt);
			return _RET_USER_EXIST;
		}

		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);
	return _RET_OK;
}
int UserInfoManager::check(const std::string &user, const std::string &pass) {
	if (!sql)
		return _RET_NO_CONN;
	static MYSQL_STMT *stmt = nullptr;
	stmt = mysql_stmt_init(sql);
	static MYSQL_BIND bind[2];
	static MYSQL_BIND res;
	static int num;
	if (mysql_stmt_prepare(
			stmt,
			"SELECT count(*) FROM userinfo WHERE name = ? AND pass = SHA1(?)",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(bind, 0, sizeof(bind));
	memset(&res, 0, sizeof(res));

	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (void *)user.c_str();
	bind[0].buffer_length = user.length();

	bind[1].buffer_type = MYSQL_TYPE_STRING;
	bind[1].buffer = (void *)pass.c_str();
	bind[1].buffer_length = pass.length();

	res.buffer_type = MYSQL_TYPE_LONG;
	res.buffer = (void *)&num;

	if (mysql_stmt_bind_param(stmt, bind)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_bind_result(stmt, &res)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	if (mysql_stmt_execute(stmt)) {
		if (mysql_stmt_errno(stmt) == 1062) {
			mysql_stmt_close(stmt);
			return _RET_USER_EXIST;
		}
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_fetch(stmt)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);
	if (num == 1)
		return _RET_OK;
	return _RET_CHECK_FAILED;
}
int UserInfoManager::change(const std::string &user, const std::string &pass) {
	if (!sql)
		return _RET_NO_CONN;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND bind[2];
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt,
			"UPDATE userinfo SET pass = SHA1(?) WHERE name = ?",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(bind, 0, sizeof(bind));

	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (void *)pass.c_str();
	bind[0].buffer_length = pass.length();

	bind[1].buffer_type = MYSQL_TYPE_STRING;
	bind[1].buffer = (void *)user.c_str();
	bind[1].buffer_length = user.length();


	if (mysql_stmt_bind_param(stmt, bind)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	if (mysql_stmt_execute(stmt)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	mysql_stmt_close(stmt);
	return _RET_OK;
}
int UserInfoManager::del(const std::string &user) {
	if (!sql)
		return _RET_NO_CONN;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND bind;
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt, "DELETE FROM userinfo WHERE name = ?", -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(&bind, 0, sizeof(bind));

	bind.buffer_type = MYSQL_TYPE_STRING;
	bind.buffer = (void *)user.c_str();
	bind.buffer_length = user.length();

	if (mysql_stmt_bind_param(stmt, &bind)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	if (mysql_stmt_execute(stmt)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	mysql_stmt_close(stmt);
	return _RET_OK;
}
int UserInfoManager::initDatabase() {
	if (!sql)
		return _RET_NO_CONN;
	if (mysql_query(sql, _SQL_INIT_DATABASE)) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}
	mysql_next_result(sql);
	return _RET_OK;
}
const char *UserInfoManager::getMysqlError() {
	return _mysql_error_msg.c_str();
}
const char *UserInfoManager::error(int error_no) {
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
	case _RET_USER_EXIST:
		return "user exist";
	case _RET_CHECK_FAILED:
		return "username or password incorrect";

	default:
		return "unknown error";
	}
	return "unknown error";
}
