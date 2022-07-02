#include "some_tool_about_path.h"

using namespace std;
string _FileSystemManager::getParent(const string path) {
	size_t pos1 = path.find_last_of('/');
	pos1 = path.find_last_of('/', pos1 - 1);
	return path.substr(0, pos1+1);
}
string _FileSystemManager::getSelf(const string path) {
	size_t pos1 = path.find_last_of('/');
	size_t pos2 = path.find_last_of('/', pos1 - 1);
	return path.substr(pos2 + 1, pos1 - pos2 - 1);
}
