# FileSystemManager

## ����

### FileSystemManager ��

ά���������ݿ���ļ�ϵͳ���ṩ���¹���

- makeFile �½��ļ�
- makeDir �½��ļ���
- list �г��ļ����ļ���
- move �ļ����ļ����ƶ�/����
- copy �ļ����ļ��и���/����
- remove �Ƴ��ļ�/�ļ���
- getHash ���һ���ļ��Ĺ�ϣֵ

### FNode ��

����һ���ļ����У��ڵ㣬���� list �ķ���

```c++
struct FNode {
	std::string name;
	bool is_file;
	std::string file_hash;
	long long file_size;
};
```

name Ϊ�ļ����У����֣�is_file Ϊ true ʱ�ýڵ�Ϊ�ļ���

���ýڵ����ļ�ʱ��file_hash Ϊ�ļ���ϣֵ��file_sizeΪ�ļ���С���ֽڣ���

## ͷ�ļ�

`file_system_manager.h`

## ��Ա����

### �ļ�ϵͳ

�������к������� 0 Ϊ�ɹ������ظ�ֵΪ����

#### �½��ļ�

`int makeFile(const std::string &file_path, const std::string &file_hash);`

������ļ��ߴ��Ƿ���ȷ����������¼�������ͬ�ļ��Ѵ����� file_size �ֶ������á�

#### �½��ļ���

`int makeFolder(const std::string &folder_path);`

#### �г��ļ����ļ���

`int list(const std::string &folder_path, std::vector<FNode> &fnode_list);`

#### �ļ����ļ����ƶ�/����

`int move(const std::string &old_path, const std::string &new_path);`

#### �ļ����ļ��и���/����

`int copy(const std::string &old_path, const std::string &new_path);`

#### �Ƴ��ļ�/�ļ���

`int remove(const std::string &path, std::vector<std::string> &should_be_removed_hashes);`

#### ���һ���ļ��Ĺ�ϣֵ

`int getHash(const std::string &path, std::string &hash);`

### ����

#### ��ô�������Ϣ

`static const char *error(int error_no);`

error_no Ҫ�� _FileSystemManager �����ռ��еĳ�����

#### ��ʼ�����ݿ�

���ݿ⽨��֮��Ĳ�����

`int initDatabase();`

#### ��ȡ���ݿ������Ϣ

��ʱ��������ݿ�������Ϣ���������ȡ��������Ϣ��

`const char* getMysqlError();`