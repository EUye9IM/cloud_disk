#pragma once
#ifndef AGPS_CHECK_H
#define AGPS_CHECK_H

#include <agps/agps.h>
#include <cstring>
namespace agps {
bool _checkStrVerify(agps::Value val);
bool _checkStrVerify_Y(agps::Value val);
bool _checkStrVerify_N(agps::Value val);

bool _checkStrIpaddr(agps::Value val);
} // namespace agps

#ifndef CHECK_INT_BETWEEN
#define CHECK_INT_BETWEEN(left, right)                                         \
	[](agps::Value val) { return val.Int >= left && val.Int <= right; }
#endif

#ifndef CHECK_STR_IN
#define CHECK_STR_IN(...)                                                      \
	[](agps::Value val) {                                                      \
		if (!val.Str)                                                          \
			return false;                                                      \
		const char *_str_list[] = {__VA_ARGS__, NULL};                         \
		for (int i = 0; _str_list[i]; i++) {                                   \
			if (!strcmp(val.Str, _str_list[i]))                                \
				return true;                                                   \
		}                                                                      \
		return false;                                                          \
	}
#endif

#ifndef CHECK_STR_VERIFY
#define CHECK_STR_VERIFY agps::_checkStrVerify
#endif

#ifndef CHECK_STR_VERIFY_Y
#define CHECK_STR_VERIFY_Y agps::_checkStrVerify_Y
#endif

#ifndef CHECK_STR_VERIFY_N
#define CHECK_STR_VERIFY_N agps::_checkStrVerify_N
#endif

#ifndef CHECK_STR_IPADDR
#define CHECK_STR_IPADDR agps::_checkStrIpaddr
#endif

#endif
