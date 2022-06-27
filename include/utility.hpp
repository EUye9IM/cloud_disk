/**
 * utility 负责放置部分公用函数
 **/

#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include "file_system_manager.h"

// 数据库是否初始化
// #define __DATABASE_INIT__

// 数据库配置
// 首先需要创建数据库 disk
static SqlConfig sql_config = {
    .host = "127.0.0.1",
    .port = 3306,
    .user = "root",
    .pass = "root123",
    .database_name = "disk",
    .charset = "UTF-8",
};

// 获取 FileSystemManager 的单例模式
FileSystemManager& file_system_manager();

// 路径拼接
std::string path_join(const std::string user, const std::string path);

#endif