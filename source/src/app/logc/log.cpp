#include "logc.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace LogC {

static FILE *_file = nullptr;
static int _flag = 0;

const int OK = 0;
const int ER = -1;

static const int _FALG_DEFAULT =
	LOG_FLAG_DATE | LOG_FLAG_TIME | LOG_FLAG_NORMAL 
	| LOG_FLAG_FATAL | LOG_FLAG_STDOUT;

static void _log_head();

int log_open(const char *file_path) {
	if (_file)
		return ER;
	_file = fopen(file_path, "a");
	if (!_file)
		return ER;
	log_set(_FALG_DEFAULT);
	return OK;
}
int log_open(FILE *file) {
	_file = file;
	log_set(_FALG_DEFAULT);
	return OK;
}
int log_set(int add_attr, int rm_attr) {
	_flag &= ~rm_attr;
	_flag |= add_attr;
	return OK;
}

int log_printf(const char *fmt, ...) {
	if (!_file)
		return ER;
	if (!(_flag & LOG_FLAG_NORMAL))
		return OK;
	_log_head();
	va_list ap;
	va_start(ap, fmt);
	vfprintf(_file, fmt, ap);
	va_end(ap);
	fflush(_file);
	if (_flag & LOG_FLAG_STDOUT) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		fflush(stdout);
	}

	return OK;
}
int log_println(const char *str) {
	if (!_file)
		return ER;
	if (!(_flag & LOG_FLAG_NORMAL))
		return OK;
	_log_head();
	fprintf(_file, "%s\n", str);
	fflush(_file);
	// 补充输出到 stdout
	if (_flag & LOG_FLAG_STDOUT) {
		printf("%s\n", str);
		fflush(stdout);
	}
	return OK;
}
int log_debug(const char *fmt, ...) {
	if (!_file)
		return ER;
	if (!(_flag & LOG_FLAG_DEBUG))
		return OK;
	_log_head();
	fprintf(_file, "Debug : ");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(_file, fmt, ap);
	va_end(ap);
	fflush(_file);
	return OK;
}
int log_fatal(const char *fmt, ...) {
	if (!_file)
		return ER;
	if (!(_flag & LOG_FLAG_FATAL))
		return OK;
	_log_head();
	fprintf(_file, "Fatal : ");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(_file, fmt, ap);
	va_end(ap);
	fflush(_file);
	// 输出到 stdout
	if (_flag & LOG_FLAG_STDOUT) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		fflush(stdout);
	}
	exit(0);
	return OK;
}
int log_close() {
	if (!_file)
		return ER;
	fclose(_file);
	_file = nullptr;
	return OK;
}

static void _log_head() {
	static time_t cur;
	static tm *c_tm;

	if (_flag & (LOG_FLAG_DATE | LOG_FLAG_TIME)) {
		cur = time(NULL);
		if (_flag & LOG_FLAG_UTC)
			c_tm = gmtime(&cur);
		else
			c_tm = localtime(&cur);
	}

	if (_flag & LOG_FLAG_DATE) {
		fprintf(_file, "%04d/%02d/%02d ", c_tm->tm_year + 1900,
				c_tm->tm_mon + 1, c_tm->tm_mday);
		if (_flag & LOG_FLAG_STDOUT) {
			printf("%04d/%02d/%02d ", c_tm->tm_year + 1900,
				c_tm->tm_mon + 1, c_tm->tm_mday);
			fflush(stdout);
		}
	}
	if (_flag & LOG_FLAG_TIME) {
		fprintf(_file, "%02d:%02d:%02d ", c_tm->tm_hour, c_tm->tm_min,
				c_tm->tm_sec);
		if (_flag & LOG_FLAG_STDOUT) {
			printf("%02d:%02d:%02d ", c_tm->tm_hour, c_tm->tm_min,
				c_tm->tm_sec);
			fflush(stdout);
		}
	}
}
} // namespace LogC
