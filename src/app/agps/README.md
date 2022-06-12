# agps

## 简介

a parser to parse arguments

一个平时作业用的参数解析器。

目前支持的类型：整形、标志、字符串


## 主要类名

- Type 枚举类。标识参数类型，目前包含 FLAG、INT、STR
- value 联合体，记录以上的一种信息，对应值为 Exist、Int、Str
- Parser 分析器。

## 使用方法

### 定义分析器

```c++
Parser pa;
```

### 添加参数

Parser::add 设定参数规则
- Type type 参数类型
- char short_name 短名字（ NULL 表示不含短名）
- const char *long_name 长名字（ NULL 表示不含长名）
- const char *infomation 提示信息
- bool is_required = true 必填参数（ 对 Type::FLAG 无效）
- const Value &default_value = VALUE_NONE 默认值（ 对 Type::FLAG 无效）
- bool (*check_func)(Value) = nullptr 检查合法性。返回true 则合法（ 对 Type::FLAG 无效）

例：

- 添加一个整型参数，默认值80，在 0 到 65535 之间。

```c++
pa.add(Type::INT, 0, "port", "port number [0-65535] default 80", false, Value{.Int = 80}, CHECK_INT_BETWEEN(0, 65535));
```

- 添加一个字符串，必填且符合 IPv4 格式

```c++
pa.add(Type::STR, 0, "debug", "open debug mode [y/n]", false, Value{.Str = "no"}, CHECK_STR_VERIFY);
```

### 分析参数

- int argc 参数个数
- const char \*\*argv 参数数组

```c++
pa.parse(argc, argv);
```

### 分析结果

- Parser::success() 解析成功并参数值全合法
- Parser::get(char short_name) 用短名获取值
- Parser::get(const char \*long_name) 用长名获取值
- Parser::isExist(char short_name) 参数在命令行设定过
- Parser::isExist(const char \*long_name) 参数存在于命令行
- Parser::printUsage(const char \*name = nullptr, FILE \*out_stream = stdout) 输出提示
- Parser::error() 返回错误信息（如果有）
- pa.restCnt() 获取剩余参数数量
- pa.rest(int index) 获取剩余参数
- Parser::reset() 重置解析器
- Parser::clean() 清空参数为默认值

## 示例

```c++
#include <agps/agps.h>
#include <agps/check.h>
#include <cstring>
#include <iostream>
using namespace std;
using namespace agps;

int main(int argc, const char **argv) {
	Parser pa;
	pa.add(Type::FLAG, 'h', "help", "show ussage page");
	pa.add(Type::INT, 0, "port", "port number [0-65535] default 80", false,
		   Value{.Int = 80}, CHECK_INT_BETWEEN(0, 65535));
	pa.add(Type::STR, 'p', "ip", "ip address", true, VALUE_NONE,
		   CHECK_STR_IPADDR);
	pa.add(Type::STR, 0, "debug", "debug mode [y/n]", false,
		   Value{.Str = "no"}, CHECK_STR_VERIFY);
	pa.parse(argc, argv);
	
	if (pa.get("help").Exist) {
		pa.printUsage();
		return 0;
	} 
	if (!pa.success()) {
		cout << "Error : " << endl;
		cout << pa.error() << endl << endl;
		pa.printUsage();
		return 0;
	} else {
		cout << "success." << endl << endl;
		cout << "deamon " << pa.get("deamon").Exist << endl;
		cout << "port   " << pa.get("port").Int << endl;
		cout << "ip     " << pa.get("ip").Str << endl;
		cout << "debug  " << pa.get("debug").Str << endl << endl;

		cout << "rest cnt : " << pa.restCnt() << endl;
		for (int i = 0; i < pa.restCnt(); i++) {
			cout << pa.rest(i) << endl;
		}
	}
	return 0;
}
```
