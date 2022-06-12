#pragma once
#ifndef AGPS_CONST_STR_H
#define AGPS_CONST_STR_H
namespace agps {
class ConstStr {
public:
	ConstStr();
	ConstStr(const char *str);
	~ConstStr();
	void set(const char *str);
	void append(const char *str);
	const char *cstr() const;

private:
	char *_str_pointer;
}; // class ConstStr
} // namespace agps
#endif