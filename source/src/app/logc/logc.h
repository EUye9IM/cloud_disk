#ifndef LOGC
#define LOGC
#include <cstdio>
namespace LogC {
// flags
const int LOG_FLAG_UTC = 1 << 0;
const int LOG_FLAG_DATE = 1 << 1;
const int LOG_FLAG_TIME = 1 << 2;
const int LOG_FLAG_NORMAL = 1 << 3;
const int LOG_FLAG_DEBUG = 1 << 4;
const int LOG_FLAG_FATAL = 1 << 5;
// 补充flag:写入文件的同时写入标准输出
const int LOG_FLAG_STDOUT = 1 << 6;

int log_open(const char *file_path);
int log_open(FILE *file);
int log_set(int add_attr, int rm_attr = 0);

int log_printf(const char *fmt, ...);
int log_println(const char *str);
int log_debug(const char *fmt, ...);
int log_fatal(const char *fmt, ...);
int log_close();
} // namespace LogC
#endif