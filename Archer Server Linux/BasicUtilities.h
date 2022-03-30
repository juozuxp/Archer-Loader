#pragma once
#ifndef BOTH_BasicUtilities_HEADER_INCLUDED
#define BOTH_BasicUtilities_HEADER_INCLUDED

#include "GeneralErrors.h"

#define GetArraySize(Array) ((sizeof(Array)) / (sizeof(Array[0])))
#define GetStructElementOffset(Struct, Element) ((unsigned long long)(&(((Struct*)(0))->Element)))
#define GetDwordRelativeAddress(RelativeOffsetAddress) (((char*)(RelativeOffsetAddress)) + (*(long*)(RelativeOffsetAddress)) + sizeof(long))
#define GetWordRelativeAddress(RelativeOffsetAddress) (((char*)(RelativeOffsetAddress)) + (*(short*)(RelativeOffsetAddress)) + sizeof(short))
#define GetByteRelativeAddress(RelativeOffsetAddress) (((char*)(RelativeOffsetAddress)) + (*(char*)(RelativeOffsetAddress)) + sizeof(char))
#define AddressToVPN(Address) (((unsigned long long)Address) >> 12)

#define INSERT_INLINECODE_8_BIT_ARRAY(Code, ...) ((unsigned char)((Code) << 8)) | __VA_ARGS__
#define INSERT_INLINECODE_16_BIT_ARRAY(Code, ...) ((unsigned short)((Code) << 16)) | __VA_ARGS__
#define INSERT_INLINECODE_32_BIT_ARRAY(Code, ...) ((unsigned int)((Code) << 32)) | __VA_ARGS__
#define INSERT_INLINECODE_64_BIT_ARRAY(Code, ...) ((unsigned __int64)((Code) << 64)) | __VA_ARGS__

#define RGBA_RGBA(Red, Green, Blue, Alpha) ((((Red) & 0xFF) << 24) | (((Green) & 0xFF) << 16) | (((Blue) & 0xFF) << 8) | ((Alpha) & 0xFF))
#define RGBA_ARGB(Red, Green, Blue, Alpha) ((((Alpha) & 0xFF) << 24) | (((Red) & 0xFF) << 16) | (((Green) & 0xFF) << 8) | ((Blue) & 0xFF))
#define ARGB_RGBA(Alpha, Red, Green, Blue) ((((Red) & 0xFF) << 24) | (((Green) & 0xFF) << 16) | (((Blue) & 0xFF) << 8) | ((Alpha) & 0xFF))
#define ARGB_ARGB(Alpha, Red, Green, Blue) ((((Alpha) & 0xFF) << 24) | (((Red) & 0xFF) << 16) | (((Green) & 0xFF) << 8) | ((Blue) & 0xFF))

//static GeneralErrorCast ArrayLenghtQueryAddress(PSimpleLenghtQuery Array, unsigned long long ArraySize, void * Address, SimpleLenghtQuery ** HitElement)
//{
//	if (!Array)
//		return GENERAL_ERROR_ASSEMBLE(GENERAL_ERROR_HEADER_BASICUTILITIES, STATUS_INVALID_PARAMETER_1) | 1;
//	if (!ArraySize)
//		return GENERAL_ERROR_ASSEMBLE(GENERAL_ERROR_HEADER_BASICUTILITIES, STATUS_INVALID_PARAMETER_2) | 1;
//	if (!HitElement)
//		return GENERAL_ERROR_ASSEMBLE(GENERAL_ERROR_HEADER_BASICUTILITIES, STATUS_INVALID_PARAMETER_4) | 1;
//
//	if (ArraySize % sizeof(SimpleLenghtQuery))
//		return GENERAL_ERROR_ASSEMBLE(GENERAL_ERROR_HEADER_BASICUTILITIES, STATUS_DATATYPE_MISALIGNMENT) | 1;
//
//	unsigned long long ArrayElementCount;
//
//	ArrayElementCount = (ArraySize / sizeof(SimpleLenghtQuery));
//	for (unsigned long long i = 0; i < ArrayElementCount; i++, Array++)
//	{
//		if ((Array->StartAddress <= Address) && ((((char*)Array->StartAddress) + Array->Size) > Address))
//		{
//			(*HitElement) = Array;
//
//			return STATUS_SUCCESS;
//		}
//	}
//
//	return GENERAL_ERROR_ASSEMBLE(GENERAL_ERROR_HEADER_BASICUTILITIES, STATUS_NOT_FOUND) | 1;
//}

static void* Detailedmemcmp(void* Buffer1, void* Buffer2, unsigned long long Size)
{
	unsigned long long Int64s;
	unsigned long long Int32s;
	unsigned long long Int16s;
	unsigned long long Int8s;

	Int64s = (Size / sizeof(unsigned long long));
	Int32s = ((Size - (sizeof(unsigned long long) * Int64s)) / sizeof(unsigned long));
	Int16s = ((Size - ((sizeof(unsigned long long) * Int64s) + (sizeof(unsigned long) * Int32s))) / sizeof(unsigned short));
	Int8s = (Size - ((sizeof(unsigned long long) * Int64s) + (sizeof(unsigned long) * Int32s) + (sizeof(unsigned short) * Int16s)));

	while (Size)
	{
		if (Int64s)
		{
			if (((*((unsigned long long*)Buffer1)) != (*((unsigned long long*)Buffer2))))
				for (unsigned char i = 0; i < sizeof(unsigned long long); i++, Buffer1 = (((char*)Buffer1) + sizeof(unsigned char)), Buffer2 = (((char*)Buffer2) + sizeof(unsigned char)))
					if (((*((unsigned char*)Buffer1)) != (*((unsigned char*)Buffer2))))
						return Buffer1;

			Int64s--;
			Size -= sizeof(unsigned long long);

			Buffer1 = (((char*)Buffer1) + sizeof(unsigned long long));
			Buffer2 = (((char*)Buffer2) + sizeof(unsigned long long));
		}
		else if (Int32s)
		{
			if (((*((unsigned long*)Buffer1)) != (*((unsigned long*)Buffer2))))
				for (unsigned char i = 0; i < sizeof(unsigned long); i++, Buffer1 = (((char*)Buffer1) + sizeof(unsigned char)), Buffer2 = (((char*)Buffer2) + sizeof(unsigned char)))
					if (((*((unsigned char*)Buffer1)) != (*((unsigned char*)Buffer2))))
						return Buffer1;

			Int32s--;
			Size -= sizeof(unsigned long);

			Buffer1 = (((char*)Buffer1) + sizeof(unsigned long));
			Buffer2 = (((char*)Buffer2) + sizeof(unsigned long));
		}
		else if (Int16s)
		{
			if (((*((unsigned short*)Buffer1)) != (*((unsigned short*)Buffer2))))
				for (unsigned char i = 0; i < sizeof(unsigned short); i++, Buffer1 = (((char*)Buffer1) + sizeof(unsigned char)), Buffer2 = (((char*)Buffer2) + sizeof(unsigned char)))
					if (((*((unsigned char*)Buffer1)) != (*((unsigned char*)Buffer2))))
						return Buffer1;

			Int16s--;
			Size -= sizeof(unsigned short);

			Buffer1 = (((char*)Buffer1) + sizeof(unsigned short));
			Buffer2 = (((char*)Buffer2) + sizeof(unsigned short));
		}
		else if (Int8s)
		{
			if (((*((unsigned char*)Buffer1)) != (*((unsigned char*)Buffer2))))
				return Buffer1;

			Int8s--;
			Size--;

			Buffer1 = (((char*)Buffer1) + sizeof(unsigned char));
			Buffer2 = (((char*)Buffer2) + sizeof(unsigned char));
		}
	}

	return 0;
}

#endif