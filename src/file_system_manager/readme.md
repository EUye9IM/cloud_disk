# FileSystemManager

## 类型

### FileSystemManager 类

维护基于数据库的文件系统。提供以下功能

- makeFile 新建文件
- makeDir 新建文件夹
- list 列出文件、文件夹
- move 文件、文件夹移动/改名
- copy 文件、文件夹复制/改名
- remove 移除文件/文件夹
- getHash 获得一个文件的哈希值

### FNode 类

描述一个文件（夹）节点，用于 list 的返回

```c++
struct FNode {
	std::string name;
	bool is_file;
};
```

name 为文件（夹）名字，is_file 为 true 时该节点为文件。

### Path

是 `std::vector<std::string>` 的别名。

一般第一个元素是用户id

## 头文件

`file_system_manager.h`

## 构造函数

`FileSystemManager(SqlConfig *sql_config);`

需要提供数据库连接配置。

## 成员函数

### 文件系统

以下所有函数返回 0 为成功，返回负值为错误。

#### 新建文件

`int makeFile(const Path &path, const std::string &file_name, const std::string &file_hash);`

#### 新建文件夹

`int makeFolder(const Path &path, const std::string &folder_name);`

#### 列出文件、文件夹

`int list(const Path &path, std::vector<FNode> &fnode_list);`

#### 文件、文件夹移动/改名

`int move(const Path &old_path, const std::string &old_name, const Path &new_path, const std::string &new_name);`

#### 文件、文件夹复制/改名

`int copy(const Path &old_path, const std::string &old_name, const Path &new_path, const std::string &new_name);`

#### 移除文件/文件夹

`int remove(const Path &path, const std::string &name, std::vector<std::string> &should_be_removed_hashes);`

#### 获得一个文件的哈希值

`int getHash(const Path &path, std::string &hash);`

### 其他

#### 获得错误码信息

`static const char *error(int errno);`