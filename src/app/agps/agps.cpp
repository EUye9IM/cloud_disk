#include <agps/agps.h>
#include <cstdio>
#include <cstring>
namespace agps {
Argument::Argument() {
	set(nullptr, Type::FLAG, 0, nullptr, nullptr, false, Value{0}, nullptr,
		false, Value{0});
}
Argument::Argument(Argument *next_argument, Type type, char short_name,
				   const char *long_name, const char *infomation,
				   bool is_required, const Value &default_value,
				   bool (*check_func)(Value), bool is_exist,
				   const Value &real_value) {
	set(next_argument, type, short_name, long_name, infomation, is_required,
		default_value, check_func, is_exist, real_value);
}
void Argument::set(Argument *next_argument, Type type, char short_name,
				   const char *long_name, const char *infomation,
				   bool is_required, const Value &default_value,
				   bool (*check_func)(Value), bool is_exist,
				   const Value &real_value) {
	this->type = type;
	this->short_name = short_name;
	this->long_name.set(long_name);
	this->infomation.set(infomation);
	this->is_required = is_required;
	this->default_value = default_value;
	this->check_func = check_func;
	this->next_argument = next_argument;
	this->is_exist = is_exist;
	this->real_value = real_value;
}
Parser::Parser() {
	_b_sucess = false;
	_argument_list = nullptr;
	_raw_list = nullptr;
	_rest_index = nullptr;
	_rest_num = 0;
	_error_str.set(nullptr);
}
Parser::~Parser() { reset(); }
void Parser::add(Type type, char short_name, const char *long_name,
				 const char *infomation, bool is_required,
				 const Value &default_value, bool (*check_func)(Value)) {
	if (short_name == 0 && (long_name == nullptr || long_name[0] == 0))
		return;
	Argument *p_new_arg;
	if (Type::FLAG == type)
		p_new_arg =
			new Argument(nullptr, type, short_name, long_name, infomation,
						 false, VALUE_NONE, nullptr, false, VALUE_NONE);
	else
		p_new_arg = new Argument(nullptr, type, short_name, long_name,
								 infomation, is_required, default_value,
								 check_func, false, default_value);
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
bool static _checkIfArgumentIsMatchRule(const char *arg, const Argument *rule) {
	if (strlen(arg) < 2 || arg[0] != '-')
		return false;
	if (0 != rule->short_name && arg[1] == rule->short_name && 0 == arg[2])
		return true;
	if (0 != rule->short_name && arg[1] == rule->short_name &&
		rule->type != Type::FLAG)
		return true;
	if ('-' == arg[1] && nullptr != rule->long_name.cstr()) {
		if (!strcmp(arg + 2, rule->long_name.cstr()))
			return true;
	}
	return false;
}

static void _err_clear(ConstStr &err) { err.set(nullptr); }
static void _err_missing(ConstStr &err, const Argument *arg) {
	err.append("Argument '");
	if ((nullptr != arg->long_name.cstr() && arg->long_name.cstr()[0] != 0))
		err.append(arg->long_name.cstr());
	else {
		char k[2] = {arg->short_name, char(0)};
		err.append(k);
	}
	err.append("' is missing a value.\n");
}
static void _err_type_int(ConstStr &err, const Argument *arg) {
	err.append("Argument '");
	if ((nullptr != arg->long_name.cstr() && arg->long_name.cstr()[0] != 0))
		err.append(arg->long_name.cstr());
	else {
		char k[2] = {arg->short_name, char(0)};
		err.append(k);
	}
	err.append("' requires an integer value.\n");
}
static void _err_unexpect(ConstStr &err, const Argument *arg) {
	err.append("Unexpected error at argument '");
	if ((nullptr != arg->long_name.cstr() && arg->long_name.cstr()[0] != 0))
		err.append(arg->long_name.cstr());
	else {
		char k[2] = {arg->short_name, char(0)};
		err.append(k);
	}
	err.append("'.\n");
}
static void _err_invalid(ConstStr &err, const Argument *arg) {
	err.append("Argument '");
	if ((nullptr != arg->long_name.cstr() && arg->long_name.cstr()[0] != 0))
		err.append(arg->long_name.cstr());
	else {
		char k[2] = {arg->short_name, char(0)};
		err.append(k);
	}
	err.append("' received an invalid value.\n");
}
static void _err_required(ConstStr &err, const Argument *arg) {
	err.append("Argument '");
	if ((nullptr != arg->long_name.cstr() && arg->long_name.cstr()[0] != 0))
		err.append(arg->long_name.cstr());
	else {
		char k[2] = {arg->short_name, char(0)};
		err.append(k);
	}
	err.append("' required but not exist.\n");
}
static void _err_unknown(ConstStr &err, const char *key_long) {
	err.append("Unkown argument : '");
	err.append(key_long);
	err.append("'.\n");
}
static void _err_multiple(ConstStr &err, const Argument *arg) {
	err.append("Argument '");
	if ((nullptr != arg->long_name.cstr() && arg->long_name.cstr()[0] != 0))
		err.append(arg->long_name.cstr());
	else {
		char k[2] = {arg->short_name, char(0)};
		err.append(k);
	}
	err.append("' is setted multiple times.\n");
}

void Parser::parse(int argc, const char **argv) {
	_b_sucess = true;
	_err_clear(_error_str);
	int pos = 1;
	// save the args
	_raw_list = new ConstStr[argc];
	_rest_index = new int[argc];
	_rest_num = 0;
	for (int i = 0; i < argc; i++)
		_raw_list[i].set(argv[i]);
	// parse arguments
	while (pos < argc /*&& _b_sucess*/) {
		const char *the_arg = _raw_list[pos].cstr();
		// check if it is -*
		if (strlen(the_arg) >= 2 && the_arg[0] == '-') {
			// find the rule
			Argument *p_rule = _argument_list;
			bool no_match = true;
			while (p_rule != nullptr) {
				if (_checkIfArgumentIsMatchRule(the_arg, p_rule)) {
					no_match = false;
					if (p_rule->is_exist)
						_err_multiple(_error_str, p_rule);
					p_rule->is_exist = true;
					if (p_rule->type == Type::FLAG) {
						p_rule->real_value.Exist = true;
					} else {
						const char *val_str;
						if (0 != p_rule->short_name &&
							the_arg[1] == p_rule->short_name &&
							0 != the_arg[2]) {
							val_str = the_arg + 2;
						} else {
							pos++;
							if (pos >= argc) {
								_b_sucess = false;
								_err_missing(_error_str, p_rule);
								break;
							}
							val_str = _raw_list[pos].cstr();
							if (val_str[0] == '-') {
								_b_sucess = false;
								_err_missing(_error_str, p_rule);
								pos--;
								break;
							}
						}
						Value temp_val = VALUE_NONE;
						switch (p_rule->type) {
						case Type::INT:
							int ret, val;
							char c;
							ret = sscanf(val_str, "%d%c", &val, &c);
							if (1 != ret) {
								_b_sucess = false;
								_err_type_int(_error_str, p_rule);
							} else
								temp_val.Int = val;
							break;

						case Type::STR:
							temp_val.Str = val_str;
							break;

						case Type::FLAG:
						default:
							_b_sucess = false;
							_err_unexpect(_error_str, p_rule);
							break;
						} // switch rule type
						// if value is invalid
						if (p_rule->check_func != nullptr &&
							!(p_rule->check_func(temp_val))) {
							_b_sucess = false;
							_err_invalid(_error_str, p_rule);
						} else {
							p_rule->real_value = temp_val;
						}
					}
					break;
				}
				p_rule = p_rule->next_argument;
			} // rule_list loop : to find which rule to match
			  // match faild
			if (no_match) {
				_b_sucess = false;
				_err_unknown(_error_str, the_arg);
			}
		} // check if arg[pos] like -*
		else {
			_rest_index[_rest_num] = pos;
			_rest_num++;
		}
		pos++;
	} // arg_list loop

	// check all argument is ok
	Argument *p_rule = _argument_list;
	while (p_rule != nullptr) {
		if (p_rule->is_required && p_rule->is_exist == false) {
			_b_sucess = false;
			_err_required(_error_str, p_rule);
		}
		p_rule = p_rule->next_argument;
	}
}
bool Parser::success() const { return _b_sucess; }
Value Parser::get(char short_name) const {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		if (pointer->short_name == short_name)
			return pointer->real_value;
		pointer = pointer->next_argument;
	}
	return VALUE_NONE;
}
Value Parser::get(const char *long_name) const {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		if (pointer->long_name.cstr() != nullptr &&
			!strcmp(pointer->long_name.cstr(), long_name))
			return pointer->real_value;
		pointer = pointer->next_argument;
	}
	return VALUE_NONE;
}
bool Parser::isExist(char short_name) const {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		if (pointer->short_name == short_name)
			return pointer->is_exist;
		pointer = pointer->next_argument;
	}
	return false;
}
bool Parser::isExist(const char *long_name) const {
	Argument *pointer = _argument_list;
	while (pointer != nullptr) {
		if (!strcmp(pointer->long_name.cstr(), long_name))
			return pointer->is_exist;
		pointer = pointer->next_argument;
	}
	return false;
}
const char *Parser::rest(int index) {
	if (index < _rest_num && index >= 0)
		return _raw_list[_rest_index[index]].cstr();
	return nullptr;
}
int Parser::restCnt() { return _rest_num; }
void Parser::printUsage(const char *name, FILE *out_stream) const {
	Argument *pointer;
	if (!name) {
		name = _raw_list[0].cstr();
	}
	fprintf(out_stream, "ussage : %s", name == nullptr ? "" : name);
	pointer = _argument_list;
	while (pointer != nullptr) {
		if (pointer->is_required) {
			if (nullptr != pointer->long_name.cstr()) {
				switch (pointer->type) {
				case Type::STR:
					fprintf(out_stream, " --%s {string}",
							pointer->long_name.cstr());
					break;
				case Type::INT:
					fprintf(out_stream, " --%s {integer}",
							pointer->long_name.cstr());
					break;

				case Type::FLAG:
				default:
					fprintf(out_stream, " --%s", pointer->long_name.cstr());
					break;
				}
			} else {
				switch (pointer->type) {
				case Type::STR:
					fprintf(out_stream, " -%c {string}", pointer->short_name);
					break;
				case Type::INT:
					fprintf(out_stream, " -%c {integer}", pointer->short_name);
					break;

				case Type::FLAG:
				default:
					fprintf(out_stream, " -%c", pointer->short_name);
					break;
				}
			}
		}
		pointer = pointer->next_argument;
	}
	fprintf(out_stream, " [options] ...\noptions :\n");
	pointer = _argument_list;
	while (pointer != nullptr) {
		const char *val_type = "";
		int n;

		switch (pointer->type) {
		case Type::STR:
			val_type = "  {string}";
			break;
		case Type::INT:
			val_type = "  {integer}";
			break;
		default:
			break;
		}

		if (nullptr != pointer->long_name.cstr() &&
			pointer->long_name.cstr()[0] != 0) {
			if (0 != pointer->short_name)
				fprintf(out_stream, "    -%c,--%s%s%n", pointer->short_name,
						pointer->long_name.cstr(), val_type, &n);
			else
				fprintf(out_stream, "       --%s%s%n",
						pointer->long_name.cstr(), val_type, &n);
		} else {
			if (0 != pointer->short_name)
				fprintf(out_stream, "    -%c %s%n", pointer->short_name,
						val_type, &n);
			else
				fprintf(out_stream, "     %s%n", val_type, &n);
		}

		if (nullptr != pointer->infomation.cstr() &&
			pointer->infomation.cstr()[0] != 0) {
			if (n < 24) {
				for (int i = n; i < 26; i++) {
					fprintf(out_stream, " ");
				}
			} else {
				fprintf(out_stream, "\n");
				for (int i = 0; i < 26; i++) {
					fprintf(out_stream, " ");
				}
			}
			fprintf(out_stream, "%s", pointer->infomation.cstr());
		}
		fprintf(out_stream, "\n");
		pointer = pointer->next_argument;
	}
	return;
}
const char *Parser::error() const { return _error_str.cstr(); };
void Parser::reset() {
	Argument *head = _argument_list;
	if (_raw_list != nullptr)
		delete[] _raw_list;
	if (_rest_index != nullptr)
		delete[] _rest_index;
	while (head != nullptr) {
		_argument_list = head->next_argument;
		delete head;
		head = _argument_list;
	}
	_b_sucess = false;
	_raw_list = nullptr;
	_rest_index = nullptr;
	_rest_num = 0;
	_error_str.set(nullptr);
	return;
}
void Parser::clean() {
	Argument *pointer = _argument_list;
	if (_raw_list != nullptr)
		delete[] _raw_list;
	if (_rest_index != nullptr)
		delete[] _rest_index;
	while (pointer != nullptr) {
		pointer->is_exist = false;
		pointer->real_value = pointer->default_value;
		pointer = pointer->next_argument;
	}
	_b_sucess = false;
	_raw_list = nullptr;
	_rest_index = nullptr;
	_rest_num = 0;
	_error_str.set(nullptr);
	return;
}
} // namespace agps