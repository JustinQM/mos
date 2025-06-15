#include "string.h"
#include <stdint.h>

void* strncpy(char* dest, char* src, size_t length)
{
	if (length != 0)
	{
		uint8_t* ptr = (uint8_t*)dest;
		do
		{
			uint8_t chr = *(uint8_t*)src++;
			*ptr++ = chr;
			if (!chr)
				break;
		}
		while (--length != 0);
	}
	return dest;
}

