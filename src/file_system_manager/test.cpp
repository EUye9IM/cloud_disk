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

	int ret;
	ret = fs.connect(sql_config);
	cout << __LINE__<< fs.error(ret) << endl;

	ret = fs.initDatabase();
	cout << __LINE__ << fs.error(ret) << endl;

	ret = fs.makeFolder("/123");
	cout << __LINE__ << fs.error(ret) << endl;

	ret = fs.makeFile("/123/234", "123", 213);
	cout << __LINE__ << fs.error(ret) << endl;
	ret = fs.makeFile("/123/244", "123", 2113);
	cout << __LINE__ << fs.error(ret) << endl;

	return 0;
}
#endif