# LogC

## ���

һ�� C ���� C++ ��־�⡣���Ժܿ����ֲ�� C ��

Ŀǰ���߱��̰߳�ȫ��һ����־û�б�ҪӰ�����ܰɣ�

## ���ܺ���

δ������˵��һ�ɷ��� int�� 0 �ɹ��� -1 ʧ�ܡ�

### ���ļ�

`log_open(const char *file_path)`
`log_open(FILE *file)`

���ļ���Ϊ��־���

### �趨����

`log_set(int add_attr, int rm_attr = 0)`

`add_attr` ������ԣ����ȼ��ߣ�

`rm_attr` �Ƴ����ԣ����ȼ��ͣ�

����Ϊ��

- ��־������Ϣ
	- `LOG_FLAG_UTC` ʹ�� UTC ʱ��
	- `LOG_FLAG_DATE` ���ڣ�ȱʡ�У�
	- `LOG_FLAG_TIME` ʱ�䣨ȱʡ�У�
- ��Ҫ�������־����
	- `LOG_FLAG_NORMAL` ���һ����Ϣ��ȱʡ�У�
	- `LOG_FLAG_DEBUG` �������
	- `LOG_FLAG_FATAL` �����������ȱʡ�У�
- �����stdout(2022/6/26�ղ���)
	- `LOG_FLAG_STDOUT` �����stdout


### �����־

`log_printf(const char *fmt, ...)`

### ���������־

`log_println(const char *str)`

ĩβ�Զ���� `'\n'`

### ���������Ϣ

`log_debug(const char *fmt, ...)`

### �����������

`log_fatal(const char *fmt, ...)`

�����Ϣ��Ȼ����� `exit(0)` �˳�����

### �ر��ļ�

`log_close()`

��Ȼ�����ò�����ţ������⹦�ܡ�

## ��Ŀ�ṹ˵��

```
LogC
|-- src Դ����
|   |-- logc
|   |   `-- logc.cpp
|   `-- test.cpp ����/ʾ������
|-- include ͷ�ļ�
|   `-- logc
|       `-- logc.h
|-- .gitignore
|-- LICENCE
|-- makefile
`-- README.md
```

