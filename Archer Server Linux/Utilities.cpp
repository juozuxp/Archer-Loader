#include "Utilities.h"

#define CHAR_TO_HEX(Char, Upper) (((Char) >= 0xA) ? (((Char) - 0xA) + (Upper ? 'A' : 'a')) : ((Char) + '0'))
#define BYTE_TO_STR_HEX(Byte, Upper) CHAR_TO_HEX(((unsigned char)(Byte)) >> 4, Upper), CHAR_TO_HEX(((unsigned char)(Byte)) & ((1 << 4) - 1), Upper)
#define BYTE_TO_SHORT_HEX(Byte, Upper) (((unsigned short)CHAR_TO_HEX(((unsigned char)(Byte)) >> 4, Upper)) | (((unsigned short)CHAR_TO_HEX(((unsigned char)(Byte)) & ((1 << 4) - 1), Upper)) << 8))

void Utilities::BufferToString(void* Buffer, unsigned int BufferSize, char* StringBuffer, bool Upper)
{
	char* RunBuffer;
	char* RunStringBuffer;

	RunBuffer = (char*)Buffer;
	RunStringBuffer = StringBuffer;
	for (unsigned int i = 0; i < BufferSize; i++, RunStringBuffer += 2, RunBuffer++)
		*(unsigned short*)RunStringBuffer = BYTE_TO_SHORT_HEX(*RunBuffer, Upper);

	*RunStringBuffer = '\0';
}