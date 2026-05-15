#pragma once
#include <Windows.h>

enum class LogLevel : int {
	INFO = 0,
	WARN,
	ERR
};

class INSSys {
	public:
		virtual void Log(HMODULE handle, LogLevel level, const char* msg) = 0;
};