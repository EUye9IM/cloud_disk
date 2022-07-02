/**
 * utility ������ò��ֹ��ú���
 **/

#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include <initializer_list>
#include "file_system_manager.h"
#include "sql_config.h"

// ���ݿ��Ƿ��ʼ��
// #define __DATABASE_INIT__

#define UNUSED_VARIABLE(x) ((void)(x))

// ���ݿ�����
// ������Ҫ�������ݿ� disk
static SqlConfig sql_config = {
    .host = "127.0.0.1",
    .port = 3306,
    .user = "root",
    .pass = "root",
    .database_name = "disk",
    .charset = "utf8",
};

// ʵ���ļ��Ĵ洢λ��
const std::string ROOT_PATH = "./DISK";

// ���ú�������ֹ����
inline void unused_variable() {
    UNUSED_VARIABLE(sql_config);
    UNUSED_VARIABLE(ROOT_PATH);
}

// ��ȡ FileSystemManager �ĵ���ģʽ
FileSystemManager& file_system_manager();

// ·��ƴ��
std::string path_join(const std::string user, const std::initializer_list<std::string>);
std::string path_join(const std::initializer_list<std::string>);

#endif
