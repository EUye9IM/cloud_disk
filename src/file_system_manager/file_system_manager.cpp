#include <cstring>
#include <file_system_manager.h>
#include <mysql/mysql.h>
#include <vector>

using namespace _FileSystemManager;

const int PATH_LEN = 520;
const int HASH_LEN = 60;

static const char *_SQL_INIT_DATABASE = "\
DROP TABLE IF EXISTS node;\
CREATE TABLE node(\
path VARCHAR(512) NOT NULL,\
hash char(50),\
mtime int(11),\
PRIMARY KEY (path));\
INSERT INTO node VALUES ('/',NULL,NULL);\
DROP TABLE IF EXISTS file;\
CREATE TABLE file(\
hash char(50),\
size bigint(22) NOT NULL,\
PRIMARY KEY (hash));\
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
								const std::string &file_hash,
								const long long file_size) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string parent_path, recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[3];
	static MYSQL_BIND out;
	static std::time_t mtime;
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
	mtime = std::time(nullptr);
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "INSERT INTO node VALUES(?,?,?);", -1)) {
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
	in[2].buffer_type = MYSQL_TYPE_LONG;
	in[2].buffer = (void *)&mtime;

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
	// file size
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "INSERT INTO file VALUES(?,?);", -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)file_hash.c_str();
	in[0].buffer_length = file_hash.length();
	in[1].buffer_type = MYSQL_TYPE_LONGLONG;
	in[1].buffer = (void *)&file_size;

	if (mysql_stmt_bind_param(stmt, in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
		if (mysql_stmt_errno(stmt) != 1062) {
			_mysql_error_msg = mysql_stmt_error(stmt);
			mysql_stmt_close(stmt);
			return _RET_SQL_ERR;
		}
	}
	mysql_stmt_close(stmt);

	// update mtime
	if (int ret = this->_updateModifyTime(file_path, mtime)) {
		return ret;
	}
	transaction.status = transaction.commit;
	return _RET_OK;
}
int FileSystemManager::makeFolder(const std::string &folder_path) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string parent_path, recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[2];
	static MYSQL_BIND out;
	static int num;
	static std::time_t mtime;
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
	mtime = std::time(nullptr);
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "INSERT INTO node VALUES(?,NULL,?);", -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(&in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)recent_path.c_str();
	in[0].buffer_length = recent_path.length();
	in[1].buffer_type = MYSQL_TYPE_LONG;
	in[1].buffer = (void *)&mtime;

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

	// update mtime
	if (int ret = this->_updateModifyTime(folder_path, mtime)) {
		return ret;
	}
	transaction.status = transaction.commit;
	return _RET_OK;
}

int FileSystemManager::list(const std::string &folder_path,
							std::vector<FNode> &fnode_list) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string recent_path;
	static std::string regex_parm;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in;
	static MYSQL_BIND out[4];
	static FNode buf_node;

	static char buffer_path[PATH_LEN];
	static char buffer_hash[HASH_LEN];
	static std::time_t mtime;
	static long long buffer_size;

	if (folder_path.length() < 1 || folder_path[0] != '/')
		return _RET_BAD_PATH;
	if (folder_path.back() != '/')
		recent_path = folder_path + "/";
	else
		recent_path = folder_path;
	regex_parm = "^" + recent_path + "[^/]+/$";
	fnode_list.clear();

	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt,
			"SELECT path, node.hash, mtime, size FROM node LEFT JOIN file "
			"ON node.hash = file.hash WHERE path REGEXP "
			"? ORDER BY path",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)regex_parm.c_str();
	in.buffer_length = regex_parm.length();

	memset(out, 0, sizeof(out));
	out[0].buffer_type = MYSQL_TYPE_STRING;
	out[0].buffer = (void *)&buffer_path;
	out[0].buffer_length = sizeof(buffer_path);
	out[1].buffer_type = MYSQL_TYPE_STRING;
	out[1].buffer = (void *)&buffer_hash;
	out[1].buffer_length = sizeof(buffer_hash);
	out[2].buffer_type = MYSQL_TYPE_LONG;
	out[2].buffer = (void *)&mtime;
	out[3].buffer_type = MYSQL_TYPE_LONGLONG;
	out[3].buffer = (void *)&buffer_size;

	if (mysql_stmt_bind_param(stmt, &in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_bind_result(stmt, out)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	while (!mysql_stmt_fetch(stmt)) {
		static size_t pos1, pos2;
		buf_node.name = buffer_path;
		buf_node.is_file = (buffer_hash[0] != 0);
		buf_node.file_hash = buffer_hash;
		buf_node.modufy_time = mtime;
		buf_node.file_size = buffer_size;

		pos1 = buf_node.name.find_last_of('/');
		pos2 = buf_node.name.find_last_of('/', pos1 - 1);
		buf_node.name = buf_node.name.substr(pos2 + 1, pos1 - pos2 - 1);

		fnode_list.push_back(buf_node);
	}
	mysql_stmt_close(stmt);

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
int FileSystemManager::_updateModifyTime(const std::string &path,
										 std::time_t mtime) {
	static std::string recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[2];
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "UPDATE node SET mtime = ? where path = "
						   "SUBSTR(?,1,CHAR_LENGTH(path));",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_LONG;
	in[0].buffer = (void *)&mtime;
	in[1].buffer_type = MYSQL_TYPE_STRING;
	in[1].buffer = (void *)path.c_str();
	in[1].buffer_length = path.length();

	if (mysql_stmt_bind_param(stmt, in)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	if (mysql_stmt_execute(stmt)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	printf("%s\n", path.c_str());

	mysql_stmt_close(stmt);
	return 0;
}