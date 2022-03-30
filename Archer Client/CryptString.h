#pragma once

#define CRYPT_STRING_TIME_KEY ((((unsigned short)(__TIME__[0])) << 5) ^ (((unsigned short)(__TIME__[1])) << 4) ^ (((unsigned short)(__TIME__[3])) << 3) ^ (((unsigned short)(__TIME__[4])) << 2) ^ (((unsigned short)(__TIME__[6])) << 1) ^ ((unsigned short)(__TIME__[7])))
#define CRYPT_STRING_ENCRYPTA(CHAR, INDEX, TIMEKEY) ((char)(((unsigned char)(CHAR)) ^ ((unsigned char)(INDEX)) ^ ((unsigned char)(TIMEKEY))))
#define CRYPT_STRING_ENCRYPTW(CHAR, INDEX, TIMEKEY) ((wchar_t)(((unsigned short)(CHAR)) ^ ((unsigned short)(INDEX)) ^ ((unsigned char)(TIMEKEY))))

#define __CS(STRING) ([]{ constexpr unsigned short TimeKey = CRYPT_STRING_TIME_KEY; constexpr ACRYPT_STRING<sizeof(STRING), TimeKey> Local = ACRYPT_STRING<sizeof(STRING), TimeKey>(STRING); return Local; }().Decrypt())
#define __CWS(STRING) ([]{ constexpr unsigned short TimeKey = CRYPT_STRING_TIME_KEY; constexpr WCRYPT_STRING<sizeof(STRING) / sizeof(wchar_t), TimeKey> Local = WCRYPT_STRING<sizeof(STRING) / sizeof(wchar_t), TimeKey>(STRING); return Local; }().Decrypt())

template<unsigned long Size, unsigned short TimeKey>
class ACRYPT_STRING
{
public:
	constexpr ACRYPT_STRING(const char* String) : String{}
	{
		for (unsigned long i = 0; i < Size; i++)
			this->String[i] = CRYPT_STRING_ENCRYPTA(String[i], i, TimeKey);
	}

	constexpr const char* Decrypt() const
	{
		char* Result = const_cast<char*>(String);
		for (unsigned long i = 0; i < Size; i++)
			Result[i] = CRYPT_STRING_ENCRYPTA(String[i], i, TimeKey);

		return Result;
	}

private:
	char String[Size];
};

template<unsigned long Size, unsigned short TimeKey>
class WCRYPT_STRING
{
public:
	constexpr WCRYPT_STRING(const wchar_t* String) : String{}
	{
		for (unsigned long i = 0; i < Size; i++)
			this->String[i] = CRYPT_STRING_ENCRYPTW(String[i], i, TimeKey);
	}

	constexpr const wchar_t* Decrypt() const
	{
		wchar_t* Result = const_cast<wchar_t*>(String);
		for (unsigned long i = 0; i < Size; i++)
			Result[i] = CRYPT_STRING_ENCRYPTW(String[i], i, TimeKey);

		return Result;
	}

private:
	wchar_t String[Size];
};