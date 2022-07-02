# LogC

## 简介

一个 C 风格的 C++ 日志库。可以很快地移植到 C 。

目前不具备线程安全（一个日志没有必要影响性能吧）

## 功能函数

未作特殊说明一律返回 int。 0 成功。 -1 失败。

### 打开文件

`log_open(const char *file_path)`
`log_open(FILE *file)`

打开文件作为日志输出

### 设定属性

`log_set(int add_attr, int rm_attr = 0)`

`add_attr` 添加属性（优先级高）

`rm_attr` 移除属性（优先级低）

属性为：

- 日志行首信息
	- `LOG_FLAG_UTC` 使用 UTC 时间
	- `LOG_FLAG_DATE` 日期（缺省有）
	- `LOG_FLAG_TIME` 时间（缺省有）
- 需要输出的日志级别
	- `LOG_FLAG_NORMAL` 输出一般信息（缺省有）
	- `LOG_FLAG_DEBUG` 输出调试
	- `LOG_FLAG_FATAL` 输出致命错误（缺省有）
- 输出到stdout(2022/6/26日补充)
	- `LOG_FLAG_STDOUT` 输出到stdout


### 输出日志

`log_printf(const char *fmt, ...)`

### 输出单行日志

`log_println(const char *str)`

末尾自动添加 `'\n'`

### 输出调试信息

`log_debug(const char *fmt, ...)`

### 输出致命错误

`log_fatal(const char *fmt, ...)`

输出信息，然后调用 `exit(0)` 退出进程

### 关闭文件

`log_close()`

虽然基本用不到大概，但有这功能。

## 项目结构说明

```
LogC
|-- src 源代码
|   |-- logc
|   |   `-- logc.cpp
|   `-- test.cpp 测试/示例代码
|-- include 头文件
|   `-- logc
|       `-- logc.h
|-- .gitignore
|-- LICENCE
|-- makefile
`-- README.md
```

