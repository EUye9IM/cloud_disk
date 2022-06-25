#if 0
#include <file_system_manager.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
	SqlConfig sql_config = {
		.host = "127.0.0.1",
		.port = 3306,
		.user = "root",
		.pass = "root",
		.database_name = "disk",
		.charset = "UTF-8",
	};

	FileSystemManager fs;

	if (int ret = fs.connect(sql_config)) {
		cout << fs.error(ret) << endl;
		return 0;
	}
	if (int ret = fs.initDatabase()) {
		cout << fs.error(ret) << endl;
		if (ret == _FileSystemManager::_RET_SQL_ERR)
			cout << fs.getMysqlError() << endl;
		return 0;
	}
	// FIXME debug code
	printf("%s:%d\n", __FILE__, __LINE__);
	if (int ret = fs.makeFile("/123", "123")) {
		cout << fs.error(ret) << endl;
		if (ret == _FileSystemManager::_RET_SQL_ERR)
			cout << fs.getMysqlError() << endl;
		return 0;
	} 
	// FIXME debug code
	printf("%s:%d\n", __FILE__, __LINE__);
	if (int ret = fs.makeFolder("/123")) {
		cout << fs.error(ret) << endl;
		if (ret == _FileSystemManager::_RET_SQL_ERR)
			cout << fs.getMysqlError() << endl;
	}
	// FIXME debug code
	printf("%s:%d\n", __FILE__, __LINE__);
	if (int ret = fs.makeFolder("/1234")) {
		cout << fs.error(ret) << endl;
		if (ret == _FileSystemManager::_RET_SQL_ERR)
			cout << fs.getMysqlError() << endl;
		return 0;
	}
	// FIXME debug code
	printf("%s:%d\n", __FILE__, __LINE__);
	if (int ret = fs.makeFile("/1234/123", "123")) {
		cout << fs.error(ret) << endl;
		if (ret == _FileSystemManager::_RET_SQL_ERR)
			cout << fs.getMysqlError() << endl;
		return 0;
	}
	return 0;
}
#endif