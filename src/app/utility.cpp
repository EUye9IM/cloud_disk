#include "utility.hpp"
#include <mutex>
#include <string>
#include "logc/logc.h"

static std::once_flag flag;

// 获取 FileSystemManager 的单例模式
FileSystemManager& file_system_manager()
{
    static FileSystemManager fsm;
    std::call_once(flag, []() {
        int ret;
        ret = fsm.connect(sql_config);
        LogC::log_printf("FileSystemManager connect %s\n", fsm.error(ret));
#ifdef __DATABASE_INIT__
        ret = fsm.initDatabase();
        LogC::log_printf("FileSystemManager init %s\n", fsm.error(ret));
#endif
    });

    return fsm;
}

// 路径拼接
std::string path_join(const std::string user, const std::string path)
{
    std::string new_path = '/' + user;
    if (path[0] == '/') {
        new_path += path;
    } else {
        new_path += '/' + path;
    }
    return new_path;
}
