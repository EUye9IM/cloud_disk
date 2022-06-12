#pragma once
#ifndef AGPS_AGPS_H
#define AGPS_AGPS_H

#include <agps/const_str.h>

#include <cstdio>

namespace agps {
enum class Type { FLAG, INT, STR };
union Value {
	bool Exist;
	int Int;
	const char *Str;
};
const Value VALUE_NONE = Value{0};
class Argument {
public:
	Argument();
	Argument(Argument *next_argument, Type type, char short_name,
			 const char *long_name, const char *infomation, bool is_required,
			 const Value &default_value, bool (*check_func)(Value),
			 bool is_exist, const Value &real_valuel);
	void set(Argument *next_argument, Type type, char short_name,
			 const char *long_name, const char *infomation, bool is_required,
			 const Value &default_value, bool (*check_func)(Value),
			 bool is_exist, const Value &real_valuel);
	Type type;
	char short_name;
	ConstStr long_name;
	ConstStr infomation;
	bool is_required;
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
	/** @brief add rule
	 *  @param type argument's type. (agps::Type::FLAG/INT/STRING)
	 */
	void add(Type type, char short_name, const char *long_name,
			 const char *infomation, bool is_required = true,
			 const Value &default_value = VALUE_NONE,
			 bool (*check_func)(Value) = nullptr);
	void parse(int argc, const char **argv);
	bool success() const;
	Value get(char short_name) const;
	Value get(const char *long_name) const;
	bool isExist(char short_name) const;
	bool isExist(const char *long_name) const;
	const char *rest(int index);
	int restCnt();
	void printUsage(const char *name = nullptr,
					FILE *out_stream = stdout) const;
	const char *error() const;
	void reset(); // delete the arg's value and rules
	void clean(); // delete the arg's value

private:
	bool _b_sucess;
	Argument *_argument_list;
	ConstStr *_raw_list;
	ConstStr _error_str;
	int *_rest_index;
	int _rest_num;
}; // class Parser
} // namespace agps

#endif
