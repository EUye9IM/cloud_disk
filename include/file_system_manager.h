#ifndef CLOUD_DISK_FILE_SYSTEM_MANAGER
#define CLOUD_DISK_FILE_SYSTEM_MANAGER
#include <sql_config.h>

#include <string>
#include <vector>

using Path = std::vector<std::string>;

struct FNode {
	std::string name;
	bool is_file;
};

class FileSystemManager {
public:
	FileSystemManager(SqlConfig *sql_config);
	~FileSystemManager();

	int makeFile(const Path &path, const std::string &file_name,
				 const std::string &file_hash);
	int makeFolder(const Path &path, const std::string &folder_name);
	int list(const Path &path, std::vector<FNode> &fnode_list);
	int move(const Path &old_path, const std::string &old_name,
			 const Path &new_path, const std::string &new_name);
	int copy(const Path &old_path, const std::string &old_name,
			 const Path &new_path, const std::string &new_name);
	int remove(const Path &path, const std::string &name,
			   std::vector<std::string> &should_be_removed_hashes);
	int getHash(const Path &path, std::string &hash);

	static const char *error(int errno);
};

namespace _FileSystemManager {
const int _RET_OK = 0;
}

#endif