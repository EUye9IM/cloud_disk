#include "some_tool_about_path.h"

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
PRIMARY KEY (path)) DEFAULT CHARSET=gbk;\
INSERT INTO node VALUES ('/',NULL,NULL);\
DROP TABLE IF EXISTS file;\
CREATE TABLE file(\
hash char(50),\
size bigint(22) NOT NULL,\
PRIMARY KEY (hash)) DEFAULT CHARSET=gbk;\
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
	if (mysql_stmt_prepare(stmt,
						   "SELECT count(*) FROM node WHERE path = "
						   "CONVERT(? USING 'gbk') AND hash is NULL;",
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
	if (mysql_stmt_prepare(
			stmt, "INSERT INTO node VALUES(CONVERT(? USING 'gbk'),?,?);", -1)) {
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
	if (mysql_stmt_prepare(stmt,
						   "SELECT count(*) FROM node WHERE path = CONVERT(? "
						   "USING 'gbk') AND hash is NULL;",
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
	if (mysql_stmt_prepare(
			stmt, "INSERT INTO node VALUES(CONVERT(? USING 'gbk'),NULL,?);",
			-1)) {
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
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[5];
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
	fnode_list.clear();

	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt,
			"SELECT CONVERT(path USING 'utf8'), node.hash, mtime, size FROM "
			"node LEFT JOIN file ON node.hash = file.hash WHERE "
			"SUBSTR(path,1,CHAR_LENGTH(CONVERT(? USING 'gbk')))=CONVERT(? "
			"USING 'gbk') AND POSITION('/' in "
			"SUBSTR(path,CHAR_LENGTH(CONVERT(? USING "
			"'gbk'))+1))=CHAR_LENGTH(path)-CHAR_LENGTH(CONVERT(? USING 'gbk')) "
			"AND path!=CONVERT(? USING 'gbk') ORDER BY path;",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)recent_path.c_str();
	in[0].buffer_length = recent_path.length();
	in[1].buffer_type = MYSQL_TYPE_STRING;
	in[1].buffer = (void *)recent_path.c_str();
	in[1].buffer_length = recent_path.length();
	in[2].buffer_type = MYSQL_TYPE_STRING;
	in[2].buffer = (void *)recent_path.c_str();
	in[2].buffer_length = recent_path.length();
	in[3].buffer_type = MYSQL_TYPE_STRING;
	in[3].buffer = (void *)recent_path.c_str();
	in[3].buffer_length = recent_path.length();
	in[4].buffer_type = MYSQL_TYPE_STRING;
	in[4].buffer = (void *)recent_path.c_str();
	in[4].buffer_length = recent_path.length();

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

	if (mysql_stmt_bind_param(stmt, in)) {
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
	memset(buffer_hash, 0, sizeof(buffer_hash));
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
		memset(buffer_hash, 0, sizeof(buffer_hash));
	}
	mysql_stmt_close(stmt);

	return _RET_OK;
}
int FileSystemManager::move(const std::string &old_path,
							const std::string &new_path) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string parent_path, the_old_path, name_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[5];
	static std::time_t mtime;
	if (old_path.length() < 1 || old_path[0] != '/')
		return _RET_BAD_PATH;
	if (new_path.length() < 1 || new_path[0] != '/')
		return _RET_BAD_PATH;
	if (old_path.back() != '/')
		the_old_path = old_path + "/";
	else
		the_old_path = old_path;
	if (new_path.back() != '/')
		parent_path = new_path + "/";
	else
		parent_path = new_path;

	TransactionGuard transaction(sql);
	if (transaction.status == transaction.bad) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}

	int ret = this->_checkMovePath(the_old_path, parent_path, name_path);
	if (ret != _RET_OK)
		return ret;
	parent_path += name_path;

	mtime = std::time(nullptr);
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "UPDATE node SET mtime = ?,path = "
						   "CONCAT(?,SUBSTR(path,CHAR_LENGTH(CONVERT(? USING "
						   "'gbk'))+1)) where "
						   "SUBSTR(path,1,CHAR_LENGTH(CONVERT(? USING "
						   "'gbk')))=CONVERT(? USING 'gbk');",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_LONG;
	in[0].buffer = (void *)&mtime;
	in[1].buffer_type = MYSQL_TYPE_STRING;
	in[1].buffer = (void *)parent_path.c_str();
	in[1].buffer_length = parent_path.length();
	in[2].buffer_type = MYSQL_TYPE_STRING;
	in[2].buffer = (void *)the_old_path.c_str();
	in[2].buffer_length = the_old_path.length();
	in[3].buffer_type = MYSQL_TYPE_STRING;
	in[3].buffer = (void *)the_old_path.c_str();
	in[3].buffer_length = the_old_path.length();
	in[4].buffer_type = MYSQL_TYPE_STRING;
	in[4].buffer = (void *)the_old_path.c_str();
	in[4].buffer_length = the_old_path.length();

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
	mysql_stmt_close(stmt);

	ret = this->_updateModifyTime(parent_path, mtime);
	if (ret != _RET_OK)
		return ret;

	transaction.status = transaction.commit;
	return _RET_OK;
}

int FileSystemManager::copy(const std::string &old_path,
							const std::string &new_path) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string parent_path, the_old_path, name_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[4];
	static std::time_t mtime;
	if (old_path.length() < 1 || old_path[0] != '/')
		return _RET_BAD_PATH;
	if (new_path.length() < 1 || new_path[0] != '/')
		return _RET_BAD_PATH;
	if (old_path.back() != '/')
		the_old_path = old_path + "/";
	else
		the_old_path = old_path;
	if (new_path.back() != '/')
		parent_path = new_path + "/";
	else
		parent_path = new_path;

	TransactionGuard transaction(sql);
	if (transaction.status == transaction.bad) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}

	int ret = this->_checkMovePath(the_old_path, parent_path, name_path);
	if (ret != _RET_OK)
		return ret;
	parent_path += name_path;

	mtime = std::time(nullptr);
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "INSERT INTO node(path,hash,mtime) SELECT "
						   "CONCAT(CONVERT(? USING "
						   "'gbk'),SUBSTR(path,CHAR_LENGTH(CONVERT(? USING "
						   "'gbk'))+1)),hash,mtime FROM "
						   "node where SUBSTR(path,1,CHAR_LENGTH(CONVERT(? "
						   "USING 'gbk')))=CONVERT(? USING 'gbk');",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)parent_path.c_str();
	in[0].buffer_length = parent_path.length();
	in[1].buffer_type = MYSQL_TYPE_STRING;
	in[1].buffer = (void *)the_old_path.c_str();
	in[1].buffer_length = the_old_path.length();
	in[2].buffer_type = MYSQL_TYPE_STRING;
	in[2].buffer = (void *)the_old_path.c_str();
	in[2].buffer_length = the_old_path.length();
	in[3].buffer_type = MYSQL_TYPE_STRING;
	in[3].buffer = (void *)the_old_path.c_str();
	in[3].buffer_length = the_old_path.length();

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
	mysql_stmt_close(stmt);

	ret = this->_updateModifyTime(parent_path, mtime);
	if (ret != _RET_OK)
		return ret;

	transaction.status = transaction.commit;
	return _RET_OK;
}
int FileSystemManager::remove(
	const std::string &path,
	std::vector<std::string> &should_be_removed_hashes) {
	static std::string recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in[2];
	static MYSQL_BIND out;
	static char buffer_hash[HASH_LEN];

	if (path.length() < 2 || path[0] != '/')
		return _RET_BAD_PATH;
	if (path.back() != '/')
		recent_path = path + "/";
	else
		recent_path = path;

	should_be_removed_hashes.clear();
	TransactionGuard transaction(sql);
	if (transaction.status == transaction.bad) {
		_mysql_error_msg = mysql_error(sql);
		return _RET_SQL_ERR;
	}
	// remove
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt,
			"DELETE FROM node WHERE SUBSTR(path,1,CHAR_LENGTH(CONVERT(? USING "
			"'gbk')))=CONVERT(? USING 'gbk');",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(in, 0, sizeof(in));
	in[0].buffer_type = MYSQL_TYPE_STRING;
	in[0].buffer = (void *)recent_path.c_str();
	in[0].buffer_length = recent_path.length();
	in[1].buffer_type = MYSQL_TYPE_STRING;
	in[1].buffer = (void *)recent_path.c_str();
	in[1].buffer_length = recent_path.length();

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
	mysql_stmt_close(stmt);

	// list
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "SELECT hash FROM file WHERE hash NOT IN(SELECT "
						   "DISTINCT hash FROM node WHERE hash IS NOT NULL);",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	memset(&out, 0, sizeof(out));
	out.buffer_type = MYSQL_TYPE_STRING;
	out.buffer = (void *)buffer_hash;
	out.buffer_length = sizeof(buffer_hash);
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

	memset(buffer_hash, 0, sizeof(buffer_hash));
	while (!mysql_stmt_fetch(stmt)) {
		should_be_removed_hashes.push_back(std::string(buffer_hash));
		memset(buffer_hash, 0, sizeof(buffer_hash));
	}
	mysql_stmt_close(stmt);

	// delete
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "DELETE FROM file WHERE hash NOT IN(SELECT DISTINCT "
						   "hash FROM node WHERE hash IS NOT NULL);",
						   -1)) {
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
	transaction.status = transaction.commit;
	return _RET_OK;
}
int FileSystemManager::getFile(const std::string &path, FNode &file) {
	std::lock_guard<std::mutex> lock(_lock);
	static std::string recent_path;
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in;
	static MYSQL_BIND out[4];
	static char buffer_hash[HASH_LEN];
	static char buffer_name[PATH_LEN];
	static time_t buffer_mtime;
	static long long buffer_size;
	static size_t pos1, pos2;

	if (path.length() < 1 || path[0] != '/')
		return _RET_BAD_PATH;
	if (path.back() != '/')
		recent_path = path + "/";
	else
		recent_path = path;

	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "SELECT path, node.hash, mtime, size FROM node LEFT "
						   "JOIN file ON node.hash = file.hash WHERE path = "
						   "CONVERT(? USING 'gbk');",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)recent_path.c_str();
	in.buffer_length = recent_path.length();

	memset(out, 0, sizeof(out));
	out[0].buffer_type = MYSQL_TYPE_STRING;
	out[0].buffer = (void *)buffer_name;
	out[0].buffer_length = sizeof(buffer_name);
	out[1].buffer_type = MYSQL_TYPE_STRING;
	out[1].buffer = (void *)buffer_hash;
	out[1].buffer_length = sizeof(buffer_hash);
	out[2].buffer_type = MYSQL_TYPE_LONG;
	out[2].buffer = (void *)&buffer_mtime;
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
	memset(buffer_hash, 0, sizeof(buffer_hash));
	if (mysql_stmt_fetch(stmt)) {
		if (mysql_stmt_errno(stmt) == 0) {
			mysql_stmt_close(stmt);
			return _RET_NO_EXIST;
		}
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	mysql_stmt_close(stmt);
	file.name = buffer_name;
	file.is_file = (buffer_hash[0] != 0);
	file.file_hash = buffer_hash;
	file.modufy_time = buffer_mtime;
	file.file_size = buffer_size;

	pos1 = file.name.find_last_of('/');
	pos2 = file.name.find_last_of('/', pos1 - 1);
	file.name = file.name.substr(pos2 + 1, pos1 - pos2 - 1);
	return _RET_OK;
}
int FileSystemManager::hashExist(const std::string &hash, bool &is_exist) {
	std::lock_guard<std::mutex> lock(_lock);
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in;
	static MYSQL_BIND out;
	static int num;

	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt, "SELECT count(*) FROM file WHERE hash = ?;",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)hash.c_str();
	in.buffer_length = hash.length();

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
	is_exist = (num == 1);
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
	case _RET_NO_EXIST:
		return "file or file folder does not exist";
	case _RET_RECUR:
		return "recursive copy/move";
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
	if (mysql_stmt_prepare(
			stmt,
			"UPDATE node SET mtime = ? where path = "
			"SUBSTR(CONVERT(? USING 'gbk'),1,CHAR_LENGTH(path));",
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

	mysql_stmt_close(stmt);
	return 0;
}

int FileSystemManager::_checkMovePath(const std::string &from_path,
									  std::string &to_par,
									  std::string &to_name) {
	static MYSQL_STMT *stmt = nullptr;
	static MYSQL_BIND in;
	static MYSQL_BIND out;
	static char buffer_hash[HASH_LEN];
	// if from exist --
	// find if parent dir exist --
	//     if is_file
	//          error
	//     else
	//          name = getself oldpath
	// else
	//     name = getself parent
	//     parent = getparent parent
	//     if parent dir not exist --
	//         err
	//     if isfile
	//         err

	// find if from exist
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt, "SELECT hash FROM node WHERE path = CONVERT(? USING 'gbk');",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)from_path.c_str();
	in.buffer_length = from_path.length();

	memset(&out, 0, sizeof(out));
	out.buffer_type = MYSQL_TYPE_STRING;
	out.buffer = (void *)buffer_hash;
	out.buffer_length = sizeof(buffer_hash);

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
	memset(buffer_hash, 0, sizeof(buffer_hash));
	if (mysql_stmt_fetch(stmt)) {
		if (mysql_stmt_errno(stmt) == 0) {
			mysql_stmt_close(stmt);
			return _RET_NO_EXIST;
		}
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);

	// find if parent dir exist
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(
			stmt, "SELECT hash FROM node WHERE path = CONVERT(? USING 'gbk');",
			-1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)to_par.c_str();
	in.buffer_length = to_par.length();

	memset(&out, 0, sizeof(out));
	out.buffer_type = MYSQL_TYPE_STRING;
	out.buffer = (void *)buffer_hash;
	out.buffer_length = sizeof(buffer_hash);

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
	memset(buffer_hash, 0, sizeof(buffer_hash));
	if (!mysql_stmt_fetch(stmt)) {
		mysql_stmt_close(stmt);
		// parent exist
		//	parent is_file
		if (buffer_hash[0] != '\0') {
			return _RET_BAD_PATH;
		}
		to_name = getSelf(from_path) + "/";

		if (from_path != to_par.substr(0, from_path.length()))
			return _RET_OK;
		return _RET_RECUR;
	}

	if (mysql_stmt_errno(stmt) != 0) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);

	// parent not exist
	//     name = getself parent
	to_name = getSelf(to_par) + "/";
	//     parent = getparent parent
	to_par = getParent(to_par);

	//     if parent dir not exist
	stmt = mysql_stmt_init(sql);
	if (mysql_stmt_prepare(stmt,
						   "SELECT hash FROM node WHERE path = CONVERT(? USING "
						   "'gbk')quit AND hash is NULL;",
						   -1)) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}

	memset(&in, 0, sizeof(in));
	in.buffer_type = MYSQL_TYPE_STRING;
	in.buffer = (void *)to_par.c_str();
	in.buffer_length = to_par.length();

	memset(&out, 0, sizeof(out));
	out.buffer_type = MYSQL_TYPE_STRING;
	out.buffer = (void *)buffer_hash;
	out.buffer_length = sizeof(buffer_hash);

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
	memset(buffer_hash, 0, sizeof(buffer_hash));
	if (!mysql_stmt_fetch(stmt)) {
		mysql_stmt_close(stmt);
		// parent exist
		//	parent is_dir
		if (from_path != to_par.substr(0, from_path.length()))
			return _RET_OK;
		return _RET_RECUR;
	}
	// parent not exist

	if (mysql_stmt_errno(stmt) != 0) {
		_mysql_error_msg = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return _RET_SQL_ERR;
	}
	mysql_stmt_close(stmt);
	return _RET_BAD_PATH;
}
