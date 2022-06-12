#pragma once
#ifndef CFGPS_CONST_STR_H
#define CFGPS_CONST_STR_H
namespace cfgps {
class ConstStr {
public:
	ConstStr();
	ConstStr(const char *str);
	~ConstStr();
	void set(const char *str);
	const char *cstr() const;

private:
	char *_str_pointer;
}; // class ConstStr
} // namespace agps
#endif