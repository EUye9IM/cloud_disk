#include <cfgps/cfgps.h>
#include <cfgps/check.h>

#include <cctype>
#include <cstdio>
#include <cstring>

static char *__strlwr(char *str) {
	unsigned char *p = (unsigned char *)str;
	while (*p) {
		*p = tolower((unsigned char)*p);
		p++;
	}
	return str;
}
namespace cfgps {
bool _checkStrVerify_YNTF(cfgps::Value val) {
	if (!val.Str)
		return false;
	return _checkStrVerify_Y(val) || _checkStrVerify_N(val) ||
		   _checkStrVerify_T(val) || _checkStrVerify_F(val);
}
bool _checkStrVerify_Y(cfgps::Value val) {
	if (!val.Str)
		return false;
	char *temp = new char[strlen(val.Str)];
	strcpy(temp, val.Str);
	__strlwr(temp);
	bool ans = !strcmp(temp, "yes") || !strcmp(temp, "y");
	delete[] temp;
	return ans;
}

bool _checkStrVerify_N(cfgps::Value val) {
	if (!val.Str)
		return false;
	char *temp = new char[strlen(val.Str)];
	strcpy(temp, val.Str);
	__strlwr(temp);
	bool ans = !strcmp(temp, "no") || !strcmp(temp, "n");
	delete[] temp;
	return ans;
}

bool _checkStrVerify_T(cfgps::Value val) {
	if (!val.Str)
		return false;
	char *temp = new char[strlen(val.Str)];
	strcpy(temp, val.Str);
	__strlwr(temp);
	bool ans = !strcmp(temp, "true") || !strcmp(temp, "t");
	delete[] temp;
	return ans;
}

bool _checkStrVerify_F(cfgps::Value val) {
	if (!val.Str)
		return false;
	char *temp = new char[strlen(val.Str)];
	strcpy(temp, val.Str);
	__strlwr(temp);
	bool ans = !strcmp(temp, "false") || !strcmp(temp, "f");
	delete[] temp;
	return ans;
}

bool _checkStrIpaddr(cfgps::Value val) {
	if (!val.Str)
		return false;
	int a, b, c, d;
	char e;
	if (4 == sscanf(val.Str, "%d.%d.%d.%d%c", &a, &b, &c, &d, &e)) {
		if (0 <= a && a <= 255 && 0 <= b && b <= 255 && 0 <= c && c <= 255 &&
			0 <= d && d <= 255)
			return true;
	}
	return false;
}
} // namespace cfgps