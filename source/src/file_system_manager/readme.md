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
	std::string file_hash;
	long long file_size;
};
```

name 为文件（夹）名字，is_file 为 true 时该节点为文件。

当该节点是文件时，file_hash 为文件哈希值；file_size为文件大小（字节）。

## 头文件

`file_system_manager.h`

## 成员函数

### 文件系统

以下所有函数返回 0 为成功，返回负值为错误。

#### 新建文件

`int makeFile(const std::string &file_path, const std::string &file_hash);`

不检查文件尺寸是否正确，仅用作记录。如果相同文件已存在则 file_size 字段无作用。

#### 新建文件夹

`int makeFolder(const std::string &folder_path);`

#### 列出文件、文件夹

`int list(const std::string &folder_path, std::vector<FNode> &fnode_list);`

#### 文件、文件夹移动/改名

`int move(const std::string &old_path, const std::string &new_path);`

#### 文件、文件夹复制/改名

`int copy(const std::string &old_path, const std::string &new_path);`

#### 移除文件/文件夹

`int remove(const std::string &path, std::vector<std::string> &should_be_removed_hashes);`

#### 获得一个文件的哈希值

`int getHash(const std::string &path, std::string &hash);`

### 其他

#### 获得错误码信息

`static const char *error(int error_no);`

error_no 要是 _FileSystemManager 命名空间中的常量。

#### 初始化数据库

数据库建表之类的操作。

`int initDatabase();`

#### 获取数据库错误信息

有时候会有数据库错误的信息，这个可以取到错误信息。

`const char* getMysqlError();`