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
		.charset = "gbk",
	};

	FileSystemManager fs;

	if (fs.connect(sql_config)) {
		cout << fs.getMysqlError() << endl;
		return 0;
	}
	if (fs.initDatabase()) {
		cout << fs.getMysqlError() << endl;
		return 0;
	}
	int ret = 1;
	while (1) {
		if (ret <= 0) {
			cout << fs.error(ret) << endl;
			if (ret == _FileSystemManager::_RET_SQL_ERR)
				cout << fs.getMysqlError() << endl;
		}
		cout << " > ";
		cout.flush();
		string in;
		cin >> in;
		if (in == "ls") {
			string args;
			cin >> args;
			vector<FNode> node_list;
			ret = fs.list(args, node_list);
			printf("%10s%5s%10s%10s%40s\n", "name", "is_f", "hash", "size",
				   "mtime");
			for (auto i = node_list.begin(); i != node_list.end(); i++) {
				auto r = (*i);
				printf("%10s%5d%10s%10lld%40s", r.name.c_str(), r.is_file,
					   r.file_hash.c_str(), r.file_size,
					   asctime(localtime(&r.modufy_time)));
			}
			continue;
		}
		if (in == "mkdir") {
			string args;
			cin >> args;
			ret = fs.makeFolder(args);
			continue;
		}
		if (in == "mkfile") {
			string args[2];
			int size;
			cin >> args[0] >> args[1] >> size;
			ret = fs.makeFile(args[0], args[1], size);
			continue;
		}
		if (in == "mv") {
			string args[2];
			cin >> args[0] >> args[1];
			ret = fs.move(args[0], args[1]);
			continue;
		}
		if (in == "cp") {
			string args[2];
			cin >> args[0] >> args[1];
			ret = fs.copy(args[0], args[1]);
			continue;
		}
		if (in == "hash") {
			string args;
			bool is_exist;
			cin >> args;
			ret = fs.hashExist(args, is_exist);
			cout << is_exist << endl;
			continue;
		}
		if (in == "rm") {
			string args;
			cin >> args;
			vector<string> list;
			ret = fs.remove(args, list);
			cout << "list:" << endl;
			for (auto i = list.begin(); i != list.end(); i++)
				cout << (*i) << endl;
		}
		if (in == "find") {
			string args[2];
			cin >> args[0] >> args[1];
			bool res;
			ret = fs.include(args[0], args[1], res);
			cout << res << endl;
		}
		if (in == "cat") {
			string args;
			FNode r;
			cin >> args;
			ret = fs.getFile(args, r);
			printf("%10s%5d%10s%10lld%40s", r.name.c_str(), r.is_file,
				   r.file_hash.c_str(), r.file_size,
				   asctime(localtime(&r.modufy_time)));
			continue;
		}
		if (in == "quit") {
			break;
		}
		ret = 1;
	}

	return 0;
}
#endif