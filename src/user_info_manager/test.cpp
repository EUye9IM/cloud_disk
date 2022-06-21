#if 0
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

	if (int ret = um.add("A", "123456")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	if (int ret = um.add("B", "654321")) {
		cout << __LINE__ << um.error(ret) << endl;
	}

	if (int ret = um.check("A", "123456")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	if (int ret = um.check("B", "654321")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	if (int ret = um.check("A", "000000")) {
		cout << __LINE__ << um.error(ret) << endl;
	}

	if (int ret = um.change("A", "000000")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	if (int ret = um.check("A", "000000")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	if (int ret = um.change("C", "000000")) {
		cout << __LINE__ << um.error(ret) << endl;
	}

	if (int ret = um.del("B")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	if (int ret = um.check("B", "654321")) {
		cout << __LINE__ << um.error(ret) << endl;
	}
	return 0;
}
#endif