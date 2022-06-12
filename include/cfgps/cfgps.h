// agps v1.1
#pragma once
#ifndef CFGPS_CFGPS_H
#define CFGPS_CFGPS_H

#include <cfgps/const_str.h>
namespace cfgps {
enum class Type { INT, STR };
union Value {
	int Int;
	const char *Str;
};
const Value VALUE_NONE = Value{0};
class Argument {
public:
	Argument();
	Argument(Argument *next_argument, Type type, const char *section,
			 const char *name, const Value &default_value,
			 bool (*check_func)(Value), bool is_exist,
			 const Value &real_valuel);
	void set(Argument *next_argument, Type type, const char *section,
			 const char *name, const Value &default_value,
			 bool (*check_func)(Value), bool is_exist,
			 const Value &real_valuel);
	Type type;
	ConstStr section;
	ConstStr name;
	ConstStr raw;
	Value default_value;
	bool (*check_func)(Value);
	Argument *next_argument;
	bool is_exist;
	Value real_value;
};
class Parser {
public:
	Parser();
	~Parser();
	void set(Type type, const char *section, const char *key,
			 const Value &value, bool (*check_func)(Value) = nullptr);
	void set(const char *section, const char *key, const char *value,
			 bool (*check_func)(Value) = nullptr);
	void set(const char *section, const char *key, int value,
			 bool (*check_func)(Value) = nullptr);
	void set(const char *key, const char *value,
			 bool (*check_func)(Value) = nullptr);
	void set(const char *key, int value, bool (*check_func)(Value) = nullptr);
	void readFile(const char *file_path);
	bool success() const;
	bool isExist(const char *section, const char *name) const;
	Value get(const char *section, const char *name) const;
	int getInt(const char *section, const char *name) const;
	const char *getStr(const char *section, const char *name) const;
	int getInt(const char *name) const;
	const char *getStr(const char *name) const;

	void reset();
	void clean();

private:
	bool _b_sucess;
	Argument *_argument_list;

}; // class Parser
} // namespace cfgps

#endif
