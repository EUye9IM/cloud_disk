# UserInfoManager

## 类型

### UserInfoManager 类

维护用户数据。主要功能有：

- 注册
- 登录
- 改密
- 注销

## 头文件

`user_info_manager.h`

## 成员函数

所有返回值int的函数皆成功返回0.

返回值可以使用成员函数 `static const char *error(int error_no)` 获取错误信息。

特殊地，当返回值为 `_UserInfoManager::_RET_SQL_ERR` 时调用 `const char *getMysqlError()` 获得额外的错误信息。一般用不上，出了这个错基本是我写错了（或者数据库连接失败），所以这基本用于调试。

### 数据库相关

#### 连接数据库

`int connect(const SqlConfig &sql_config);`

示例：

```c++
SqlConfig sql_config = {
	.host = "127.0.0.1",
	.port = 3306,
	.user = "root",
	.pass = "root",
	.database_name = "disk",
	.charset = "UTF-8",
};
UserInfoManager um;
if (int ret = um.connect(sql_config)) {
	cout << um.error(ret) << endl;
	if (ret == _UserInfoManager::_RET_SQL_ERR)
		cout << um.getMysqlError() << endl;
	return 0;
}
```

#### 初始化数据库

数据库建表之类的操作，系统初始化时使用。

`int initDatabase();`

### 用户管理

#### 注册

`int add(const std::string &user, const std::string &pass);`

成功返回 0，失败基本是用户重名。

#### 登录

`int check(const std::string &user, const std::string &pass);`

成功返回 0，失败基本是用户名或密码错误。

#### 改密

`int change(const std::string &user, const std::string &pass);`

成功返回 0，基本不失败。即使用户不存在也返回0，虽然不会有任何作用。

#### 注销

`int del(const std::string &user);`

成功返回 0，基本不失败。即使用户不存在也返回0，虽然不会有任何作用。

### 其他

#### 获得错误码信息

`static const char *error(int error_no);`

error_no 是 _UserInfoManager 命名空间中的常量。

#### 获取数据库错误信息

有时候会有数据库错误的信息，这个可以取到错误信息。

`const char* getMysqlError();`