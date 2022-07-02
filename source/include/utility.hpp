/**
 * utility 负责放置部分公用函数
 **/

#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include <initializer_list>
#include "file_system_manager.h"
#include "sql_config.h"

// 数据库是否初始化
// #define __DATABASE_INIT__

#define UNUSED_VARIABLE(x) ((void)(x))

// 数据库配置
// 首先需要创建数据库 disk
static SqlConfig sql_config = {
    .host = "127.0.0.1",
    .port = 3306,
    .user = "root",
    .pass = "root",
    .database_name = "disk",
    .charset = "utf8",
};

// 实际文件的存储位置
const std::string ROOT_PATH = "./DISK";

// 无用函数，防止警告
inline void unused_variable() {
    UNUSED_VARIABLE(sql_config);
    UNUSED_VARIABLE(ROOT_PATH);
}

// 获取 FileSystemManager 的单例模式
FileSystemManager& file_system_manager();

// 路径拼接
std::string path_join(const std::string user, const std::initializer_list<std::string>);
std::string path_join(const std::initializer_list<std::string>);

#endif
