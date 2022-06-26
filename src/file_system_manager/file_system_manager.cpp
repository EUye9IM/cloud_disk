#include <cstring>
#include <file_system_manager.h>
#include <mysql/mysql.h>
#include <vector>

using namespace _FileSystemManager;

static const char *_SQL_INIT_DATABASE = "\
DROP TABLE IF EXISTS node;\
CREATE TABLE node(\
path char(255) NOT NULL,\
hash char(50),\
PRIMARY KEY (path));\
INSERT INTO node VALUES ('/',NULL);\
";

class TransactionGuard {
public:
	TransactionGuard(MYSQL *mysql) {
		sql = mysql;
		if (mysql_query(sql, "BEGIN;")) {
			status = bad;
			return;
		}
		while (!mysql_next_result(sql))
			;
		status = rollback;
	}
	~TransactionGuard() {
		if (status == commit) {
			mysql_query(sql, "COMMIT;");
			while (!mysql_next_result(sql))
				;
		}
		if (status == rollback) {
			mysql_query(sql, "ROLLBACK;");
			while (!mysql_next_result(sql))
				;
		}
	}
	MYSQL *sql;
	int status;
	enum Status {
		commit,
		rollback,
		bad,
	};
};

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
	std::lock_guard<std::mutex> lock(_lock);
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
int FileSystemManager::makeFile(const std::string &file_path,
								const std::string &file_hash) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string parent_path, recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[2];
	static MYSQL_BIND out;
	static int num;
	size_t pos;
	if (file_path.length() < 2 || file_path[0] != '/')
		return _RET_BAD_PATH;
	if (file_path.back() != '/')
		recent_path = file_path + "/";
	else
		recent_path = file_path;

	pos = recent_path.find_last_of('/');
	pos = recent_path.find_last_of('/', pos - 1);
	parent_path = recent_path.substr(0, pos + 1);

	TransactionGuard transaction(sql);
	if (transaction.status == transaction.bad) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}

	// find father
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt, "SELECT count(*) FROM node WHERE path = ? AND hash is NULL;",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)parent_path.c_str();
	in[0].buffer_length = parent_path.length();

	memset(&out, 0, sizeof(out));
	out.buffer_type = MYSQL_TYPE_LONG;
	out.buffer = (void *)&num;

	if (mysql_stmt_bind_param(stmt, in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_bind_result(stmt, &out)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
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
	if (num != 1)
		return _RET_BAD_PATH;

	// create son
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "INSERT INTO node VALUES(?,?);", -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)recent_path.c_str();
	in[0].buffer_length = recent_path.length();
	in[1].buffer_type = MYSQL_TYPE_STRING;
	in[1].buffer = (void *)file_hash.c_str();
	in[1].buffer_length = file_hash.length();

	if (mysql_stmt_bind_param(stmt, in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
		if (mysql_stmt_errno(stmt) == 1062) {
			mysql_stmt_close(stmt);
			return _RET_EXIST;
		}
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);
	if (num != 1)
		return _RET_BAD_PATH;

	transaction.status = transaction.commit;
	return _RET_OK;
}
int FileSystemManager::makeFolder(const std::string &folder_path) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string parent_path, recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in;
	static MYSQL_BIND out;
	static int num;
	size_t pos;
	if (folder_path.length() < 2 || folder_path[0] != '/')
		return _RET_BAD_PATH;
	if (folder_path.back() != '/')
		recent_path = folder_path + "/";
	else
		recent_path = folder_path;
	pos = recent_path.find_last_of('/');
	pos = recent_path.find_last_of('/', pos - 1);
	parent_path = recent_path.substr(0, pos + 1);

	TransactionGuard transaction(sql);
	if (transaction.status == transaction.bad) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}

	// find father
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt, "SELECT count(*) FROM node WHERE path = ? AND hash is NULL;",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)parent_path.c_str();
	in.buffer_length = parent_path.length();

	memset(&out, 0, sizeof(out));
	out.buffer_type = MYSQL_TYPE_LONG;
	out.buffer = (void *)&num;

	if (mysql_stmt_bind_param(stmt, &in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_bind_result(stmt, &out)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
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
	if (num != 1)
		return _RET_BAD_PATH;

	// create son
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "INSERT INTO node VALUES(?,NULL);", -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)recent_path.c_str();
	in.buffer_length = recent_path.length();

	if (mysql_stmt_bind_param(stmt, &in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
		if (mysql_stmt_errno(stmt) == 1062) {
			mysql_stmt_close(stmt);
			return _RET_EXIST;
		}
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);
	if (num != 1)
		return _RET_BAD_PATH;

	transaction.status = transaction.commit;
	return _RET_OK;
}

int FileSystemManager::list(const std::string &folder_path,
							std::vector<FNode> &fnode_list) {
	// std::lock_guard<std::mutex> lock(_lock);
	// static std::string recent_path;
	// static std::string regex_parm;
	// static MYSQL_STMT *stmt = nullptr;
	// static MYSQL_BIND in;
	// static MYSQL_BIND out;
	// fnode_list.clear();
	// if (folder_path[0] != '/')
	// 	return _RET_BAD_PATH;
	// if (folder_path.back() != '/')
	// 	recent_path = folder_path + "/";
	// else
	// 	recent_path = folder_path;
	// regex_parm = "^" + recent_path + "[^/]/$";
	// // find father
	// stmt = mysql_stmt_init(sql);
	// if (mysql_stmt_prepare(stmt,
	// 					   "SELECT path, hash FROM node WHERE path REGEX "
	// 					   "'^?[^/]/$' AND hash is NULL;",
	// 					   -1)) {
	// 	_mysql_error_msg = mysql_stmt_error(stmt);
	// 	mysql_stmt_close(stmt);
	// 	return _RET_SQL_ERR;
	// }

	return _RET_OK;
}

int FileSystemManager::initDatabase() {
	std::lock_guard<std::mutex> lock(_lock);
	if (!sql)
		return _RET_NO_CONN;
	if (mysql_query(sql, _SQL_INIT_DATABASE)) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}
	while (!mysql_next_result(sql))
		;
	return _RET_OK;
}
const char *FileSystemManager::getMysqlError() {
	std::lock_guard<std::mutex> lock(_lock);
	return _mysql_error_msg.c_str();
}
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
	case _RET_BAD_PATH:
		return "bad path";
	case _RET_EXIST:
		return "file or file folder exist";
	default:
		return "unknown error";
	}
	return "unknown error";
}
#endif