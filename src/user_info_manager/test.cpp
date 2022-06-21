#if 1
#include <cstdlib>
#include <iostream>
#include <user_info_manager.h>

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

	UserInfoManager um;

	if (int ret = um.connect(sql_config)) {
		cout << um.error(ret) << endl;
		if (ret == _UserInfoManager::_RET_SQL_ERR)
			cout << um.getMysqlError() << endl;
		return 0;
	}
	if (int ret = um.initDatabase()) {
		cout << um.error(ret) << endl;
		if (ret == _UserInfoManager::_RET_SQL_ERR)
			cout << um.getMysqlError() << endl;
		return 0;
	}

	if (int ret = um.add("hi", "hello")) {
		cout << um.error(ret) << endl;
		if (ret == _UserInfoManager::_RET_SQL_ERR)
			cout << um.getMysqlError() << endl;
		return 0;
	}

}
#endif