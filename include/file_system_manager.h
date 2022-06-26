#ifndef CLOUD_DISK_FILE_SYSTEM_MANAGER
#define CLOUD_DISK_FILE_SYSTEM_MANAGER
#include <sql_config.h>

#include <mutex>
#include <mysql/mysql.h>
#include <string>
#include <vector>

struct FNode {
	std::string name;
	bool is_file;
};

class FileSystemManager {
public:
	FileSystemManager();
	~FileSystemManager();
	int connect(const SqlConfig &sql_config);

	int makeFile(const std::string &file_path, const std::string &file_hash);
	int makeFolder(const std::string &folder_path);
	int list(const std::string &folder_path, std::vector<FNode> &fnode_list);
	int move(const std::string &old_path, const std::string &new_path);
	int copy(const std::string &old_path, const std::string &new_path);
	int remove(const std::string &path,
			   std::vector<std::string> &should_be_removed_hashes);
	int getHash(const std::string &path, std::string &hash);

	int initDatabase();
	static const char *error(int error_no);
	const char *getMysqlError();

private:
	MYSQL *sql;
	std::string _mysql_error_msg;
	std::mutex _lock;
};

namespace _FileSystemManager {
const int _RET_OK = 0;
const int _RET_CONNECTED = -1;
const int _RET_NO_CONN = -2;
const int _RET_CONN = -3;
const int _RET_SQL_ERR = -4;
const int _RET_BAD_PATH = -5;
const int _RET_EXIST = -6;
} // namespace _FileSystemManager

#endif