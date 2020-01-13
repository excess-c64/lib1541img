#include <1541img/petscii.h>

SOEXPORT void petscii_mapUpperGfxToLower(char *str, uint8_t len)
{
    for (uint8_t i = 0; i < len; ++i)
    {
	if (str[i] == (char)0x62 || str[i] == (char)0xc2)
	{
	    str[i] = (char)0x7d;
	}
	else if (str[i] == (char)0x63 || str[i] == (char)0xc3)
	{
	    str[i] = (char)0x60;
	}
    }
}

