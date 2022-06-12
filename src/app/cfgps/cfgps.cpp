#include <cfgps/cfgps.h>
#include <cstdio>
#include <cstring>

const int BUFSIZE = 128;

namespace cfgps {

Argument::Argument() {
	set(nullptr, Type::STR, nullptr, nullptr, VALUE_NONE, nullptr, false,
		VALUE_NONE);
}
Argument::Argument(Argument *next_argument, Type type, const char *section,
				   const char *name, const Value &default_value,
				   bool (*check_func)(Value), bool is_exist,
				   const Value &real_value) {
	set(next_argument, type, section, name, default_value, check_func, is_exist,
		real_value);
}
void Argument::set(Argument *next_argument, Type type, const char *section,
				   const char *name, const Value &default_value,
				   bool (*check_func)(Value), bool is_exist,
				   const Value &real_value) {
	this->type = type;
	this->section.set(section);
	this->name.set(name);
	this->raw.set(nullptr);
	this->default_value = default_value;
	this->check_func = check_func;
	this->next_argument = next_argument;
	this->is_exist = is_exist;
	this->real_value = real_value;
}

Parser::Parser() {
	_b_sucess = false;
	_argument_list = nullptr;
}
Parser::~Parser() { reset(); }
void Parser::set(Type type, const char *section, const char *key,
				 const Value &value, bool (*check_func)(Value)) {
	if ((key == nullptr || key[0] == 0))
		return;
	Argument *p_new_arg;
	p_new_arg = new Argument(nullptr, type, section, key, value, check_func,
							 false, value);
	if (_argument_list == nullptr)
		_argument_list = p_new_arg;
	else {
		Argument *pointer = _argument_list;
		while (pointer->next_argument != nullptr) {
			pointer = pointer->next_argument;
		}
		pointer->next_argument = p_new_arg;
	}
	return;
}
void Parser::set(const char *section, const char *key, const char *value,
				 bool (*check_func)(Value)) {
	return this->set(Type::STR, section, key, Value{.Str = value}, check_func);
}
void Parser::set(const char *section, const char *key, int value,
				 bool (*check_func)(Value)) {
	return this->set(Type::INT, section, key, Value{.Int = value}, check_func);
}
void Parser::set(const char *key, const char *value,
				 bool (*check_func)(Value)) {
	return this->set(Type::STR, nullptr, key, Value{.Str = value}, check_func);
}
void Parser::set(const char *key, int value,
				 bool (*check_func)(Value)) {
	return this->set(Type::INT, nullptr, key, Value{.Int = value}, check_func);
}
static bool _matchKay(const char *sec1, const char *sec2, const char *key1,
					  const char *key2) {
	if (nullptr == key1 || nullptr == key2)
		return false;
	if (strcmp(key1, key2))
		return false;
	if (nullptr == sec1 || 0 == *sec1)
		if (nullptr == sec2 || 0 == *sec2)
			return true;
	if (nullptr == sec2 || 0 == *sec2)
		return false;
	if (nullptr == sec1 || 0 == *sec1)
		return false;
	if (!strcmp(sec1, sec2))
		return true;
	return false;
}
void Parser::readFile(const char *file_path) {
	FILE *fp;
	char buf[BUFSIZE], buf2[BUFSIZE], buf3[BUFSIZE];
	ConstStr cur_section;
	int ret;
	char tmpc;
	// parse arguments
	_b_sucess = true;
	fp = fopen(file_path, "r");
	if (fp == nullptr) {
		_b_sucess = false;
		return;
	}

	while (!feof(fp)) {
		if (!fgets(buf, sizeof(buf), fp))
			break;
		// remove #...
		ret = sscanf(buf, " %[^#\n]#", buf2);
		if (ret < 1)
			continue;
		// now data in buf2
		ret = sscanf(buf2, " [ %[^] \t] %c", buf, &tmpc);
		if (ret == 2 && ']' == tmpc) {
			// buf is section
			cur_section.set(buf);
		} else if (sscanf(buf2, " %[^= \t] = %[^\n]", buf, buf3) == 2) {
			// clear blank
			int len = strlen(buf3);
			while (buf3[len - 1] == ' ' || buf3[len - 1] == '\t')
				len--;
			buf3[len] = 0;
			// buf is key
			// buf3 is value
			Argument *p_rule = _argument_list;
			while (p_rule != nullptr) {
				if (_matchKay(cur_section.cstr(), p_rule->section.cstr(), buf,
							  p_rule->name.cstr())) {
					p_rule->is_exist = true;
					switch (p_rule->type) {
					case Type::INT:
						int ret, val;
						char c;
						ret = sscanf(buf3, "%d%c", &val,&c);
						if (1 != ret)
							_b_sucess = false;
						else {
							if (p_rule->check_func == nullptr ||
								p_rule->check_func(Value{.Int = val})) {
								p_rule->raw.set(buf3);
								p_rule->real_value.Int = val;
							}
						}
						break;

					case Type::STR:
					default:
						if (p_rule->check_func == nullptr ||
							p_rule->check_func(Value{.Str = buf3})) {
							p_rule->raw.set(buf3);
							p_rule->real_value.Str = p_rule->raw.cstr();
						}
						break;

					} // switch rule type
					break;
				}
				p_rule=p_rule->next_argument;
			}
		} else {
			_b_sucess = false;
			// something wrong
		}
	} // file line loop
	fclose(fp);
}
bool Parser::success() const { return _b_sucess; }

Value Parser::get(const char *section, const char *name) const {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		if (_matchKay(section, pointer->section.cstr(), name,
					  pointer->name.cstr()))
			return pointer->real_value;
		pointer = pointer->next_argument;
	}
	return VALUE_NONE;
}
int Parser::getInt(const char *section, const char *name) const {
	return this->get(section, name).Int;
}
const char *Parser::getStr(const char *section, const char *name) const {
	return this->get(section, name).Str;
}
int Parser::getInt(const char *name) const {
	return this->get(nullptr, name).Int;
}
const char *Parser::getStr(const char *name) const {
	return this->get(nullptr, name).Str;
}
bool Parser::isExist(const char *section, const char *name) const {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		if (!strcmp(pointer->section.cstr(), section) &&
			!strcmp(pointer->name.cstr(), name))
			return pointer->is_exist;
		pointer = pointer->next_argument;
	}
	return false;
}
void Parser::reset() {
	Argument *head = _argument_list;
	while (head != nullptr) {
		_argument_list = head->next_argument;
		delete head;
		head = _argument_list;
	}
	_b_sucess = false;
	return;
}
void Parser::clean() {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		pointer->is_exist = false;
		pointer->real_value = pointer->default_value;
		pointer->raw.set(nullptr);
		pointer = pointer->next_argument;
	}
	_b_sucess = false;
	return;
}
} // namespace cfgps
