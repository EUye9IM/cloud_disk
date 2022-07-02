# UserInfoManager

## ����

### UserInfoManager ��

ά���û����ݡ���Ҫ�����У�

- ע��
- ��¼
- ����
- ע��

## ͷ�ļ�

`user_info_manager.h`

## ��Ա����

���з���ֵint�ĺ����Գɹ�����0.

����ֵ����ʹ�ó�Ա���� `static const char *error(int error_no)` ��ȡ������Ϣ��

����أ�������ֵΪ `_UserInfoManager::_RET_SQL_ERR` ʱ���� `const char *getMysqlError()` ��ö���Ĵ�����Ϣ��һ���ò��ϣ�����������������д���ˣ��������ݿ�����ʧ�ܣ���������������ڵ��ԡ�

### ���ݿ����

#### �������ݿ�

`int connect(const SqlConfig &sql_config);`

ʾ����

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

#### ��ʼ�����ݿ�

���ݿ⽨��֮��Ĳ�����ϵͳ��ʼ��ʱʹ�á�

`int initDatabase();`

### �û�����

#### ע��

`int add(const std::string &user, const std::string &pass);`

�ɹ����� 0��ʧ�ܻ������û�������

#### ��¼

`int check(const std::string &user, const std::string &pass);`

�ɹ����� 0��ʧ�ܻ������û������������

#### ����

`int change(const std::string &user, const std::string &pass);`

�ɹ����� 0��������ʧ�ܡ���ʹ�û�������Ҳ����0����Ȼ�������κ����á�

#### ע��

`int del(const std::string &user);`

�ɹ����� 0��������ʧ�ܡ���ʹ�û�������Ҳ����0����Ȼ�������κ����á�

### ����

#### ��ô�������Ϣ

`static const char *error(int error_no);`

error_no �� _UserInfoManager �����ռ��еĳ�����

#### ��ȡ���ݿ������Ϣ

��ʱ��������ݿ�������Ϣ���������ȡ��������Ϣ��

`const char* getMysqlError();`