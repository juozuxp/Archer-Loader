#pragma once

class ExceptionHandler
{
public:
	void Initialize();

public:
	static ExceptionHandler Instance;

private:
	static bool Handleable(unsigned long ExceptionCode);
	static long Handler(struct _EXCEPTION_POINTERS* ExceptionInfo);
};

