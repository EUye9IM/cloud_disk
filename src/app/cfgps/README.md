# cfgps

a parser for config file

目前用作平时网络课作业，用于解析配置文件。

## 部分接口约定

```
// 设定参数
void Parser.set(const char *key, const char *value, bool (*check_func)(Value) = nullptr);

void Parser.set(const char *key, int value, bool (*check_func)(Value) = nullptr);

void Parser.set(const char *section, const char *key, const char *value, bool (*check_func)(Value) = nullptr);

void Parser.set(const char *section, const char *key, int value, bool (*check_func)(Value) = nullptr);

// 读取配置文件
void Parser.readFile(const char *file_path);

// 配置文件无错误
int Parser.success();

// 参数存在于配置文件
int Parser.isExist(const char *section, const char *key);

// 获取值
int Parser.getInt(const char *section, const char *key);

const char* Parser.getStr(const char *section, const char *key);

int Parser.getInt(const char *key);

const char* Parser.getStr(const char *key);
```

暂定。

## 例子

./src/test.cpp

## 目前进度

不考虑鲁棒性。