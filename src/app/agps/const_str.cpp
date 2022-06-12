#include <agps/const_str.h>
#include <cstring>
namespace agps {
ConstStr::ConstStr() { _str_pointer = nullptr; }
ConstStr::ConstStr(const char *str) {
	_str_pointer = nullptr;
	set(str);
};
ConstStr::~ConstStr() {
	if (_str_pointer)
		delete _str_pointer;
}
void ConstStr::set(const char *str) {
	if (_str_pointer)
		delete _str_pointer;
	if (!str)
		_str_pointer = nullptr;
	else {
		int str_len = strlen(str) + 1;
		_str_pointer = new char[str_len];
		strcpy(_str_pointer, str);
	}
	return;
}
void ConstStr::append(const char *str) {
	if (!str || str[0] == 0) // nullptr || ""
		return;
	if (!_str_pointer)
		set(str);
	else {
		int str_len = strlen(str) + strlen(_str_pointer) + 1;
		char *new_str = new char[str_len];
		strcpy(new_str, _str_pointer);
		strcat(new_str, str);
		delete _str_pointer;
		_str_pointer = new_str;
	}
	return;
}
const char *ConstStr::cstr() const { return (const char *)(_str_pointer); }
} // namespace agps