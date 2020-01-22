#include <1541img/petscii.h>

#include <string.h>

static const char *screenChars[] =
{
    /* 0x00 - 0x0f */
    "\x40",
    "\x41",
    "\x42",
    "\x43",
    "\x44",
    "\x45",
    "\x46",
    "\x47",
    "\x48",
    "\x49",
    "\x4a",
    "\x4b",
    "\x4c",
    "\x4d",
    "\x4e",
    "\x4f",

    /* 0x10 - 0x1f */
    "\x50",
    "\x51",
    "\x52",
    "\x53",
    "\x54",
    "\x55",
    "\x56",
    "\x57",
    "\x58",
    "\x59",
    "\x5a",
    "\x5b",
    "\xc2\xa3",
    "\x5d",
    "\xe2\x86\x91",
    "\xe2\x86\x90",

    /* 0x20 - 0x2f */
    "\x20",
    "\x21",
    "\x22",
    "\x23",
    "\x24",
    "\x25",
    "\x26",
    "\x27",
    "\x28",
    "\x29",
    "\x2a",
    "\x2b",
    "\x2c",
    "\x2d",
    "\x2e",
    "\x2f",

    /* 0x30 - 0x3f */
    "\x30",
    "\x31",
    "\x32",
    "\x33",
    "\x34",
    "\x35",
    "\x36",
    "\x37",
    "\x38",
    "\x39",
    "\x3a",
    "\x3b",
    "\x3c",
    "\x3d",
    "\x3e",
    "\x3f",

    /* 0x40 - 0x4f */
    "\xe2\x94\x80",
    "\xe2\x99\xa0",
    "\xe2\x94\x82",
    "\xe2\x94\x80",
    "\xf0\x9f\xad\xb7",
    "\xf0\x9f\xad\xb6",
    "\xf0\x9f\xad\xba",
    "\xf0\x9f\xad\xb1",
    "\xf0\x9f\xad\xb4",
    "\xe2\x95\xae",
    "\xe2\x95\xb0",
    "\xe2\x95\xaf",
    "\xf0\x9f\xad\xbc",
    "\xe2\x95\xb2",
    "\xe2\x95\xb1",
    "\xf0\x9f\xad\xbd",

    /* 0x50 - 0x5f */
    "\xf0\x9f\xad\xbe",
    "\xe2\x80\xa2",
    "\xf0\x9f\xad\xbb",
    "\xe2\x99\xa5",
    "\xf0\x9f\xad\xb0",
    "\xe2\x95\xad",
    "\xe2\x95\xb3",
    "\xe2\x97\x8b",
    "\xe2\x99\xa3",
    "\xf0\x9f\xad\xb5",
    "\xe2\x99\xa6",
    "\xe2\x94\xbc",
    "\xf0\x9f\xae\x8c",
    "\xe2\x94\x82",
    "\xcf\x80",
    "\xe2\x97\xa5",

    /* 0x60 - 0x6f */
    "\xc2\xa0",
    "\xe2\x96\x8c",
    "\xe2\x96\x84",
    "\xe2\x96\x94",
    "\xe2\x96\x81",
    "\xe2\x96\x8e",
    "\xe2\x96\x92",
    "\xe2\x96\x95",
    "\xf0\x9f\xae\x8f",
    "\xe2\x97\xa4",
    "\xf0\x9f\xae\x87",
    "\xe2\x94\x9c",
    "\xe2\x96\x97",
    "\xe2\x94\x94",
    "\xe2\x94\x90",
    "\xe2\x96\x82",

    /* 0x70 - 0x7f */
    "\xe2\x94\x8c",
    "\xe2\x94\xb4",
    "\xe2\x94\xac",
    "\xe2\x94\xa4",
    "\xe2\x96\x8e",
    "\xe2\x96\x8d",
    "\xf0\x9f\xae\x88",
    "\xf0\x9f\xae\x82",
    "\xf0\x9f\xae\x83",
    "\xe2\x96\x83",
    "\xf0\x9f\xad\xbf",
    "\xe2\x96\x96",
    "\xe2\x96\x9d",
    "\xe2\x94\x98",
    "\xe2\x96\x98",
    "\xe2\x96\x9a"
};

static const char *lcChars[] =
{
    "\x61",
    "\x62",
    "\x63",
    "\x64",
    "\x65",
    "\x66",
    "\x67",
    "\x68",
    "\x69",
    "\x6a",
    "\x6b",
    "\x6c",
    "\x6d",
    "\x6e",
    "\x6f",
    "\x70",
    "\x71",
    "\x72",
    "\x73",
    "\x74",
    "\x75",
    "\x76",
    "\x77",
    "\x78",
    "\x79",
    "\x7a"
};

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

static const char* nonprintable(unsigned char petsciiChar)
{
    if ((petsciiChar & 0x7f) > 0x1f) return 0;
    switch (petsciiChar)
    {
        case 0x3: return "\x03";
        case 0xd: return "\x0a";
        case 0x14: return "\x08";
        case 0x8d: return "\x0d";
        default: return 0;
    }
}

static char toScreencode(unsigned char petsciiChar)
{
    if ((petsciiChar & 0x7f) < 0x20) return -1;
    if (petsciiChar >= 0x20 && petsciiChar < 0x40) return petsciiChar;
    if (petsciiChar >= 0x40 && petsciiChar < 0x60) return petsciiChar - 0x40;
    if (petsciiChar >= 0x60 && petsciiChar < 0x80) return petsciiChar - 0x20;
    if (petsciiChar >= 0xa0 && petsciiChar < 0xc0) return petsciiChar - 0x40;
    return petsciiChar - 0x80;
}

static const char *lcScreenChar(char screencode)
{
    if (screencode == 0x5e) return "\xf0\x9f\xae\x95";
    if (screencode == 0x5f) return "\xf0\x9f\xae\x98";
    if (screencode == 0x69) return "\xf0\x9f\xae\x99";
    if (screencode == 0x7a) return "\xe2\x9c\x93";
    if (screencode > 0 && screencode < 0x1b) return lcChars[screencode-1];
    if (screencode > 0x40 && screencode < 0x5b)
        return screenChars[screencode-0x40];
    return screenChars[(unsigned char)screencode];
}

static const char *screencodeToUtf8(char screencode, int lowercase)
{
    if (screencode < 0) return 0;
    if (lowercase) return lcScreenChar(screencode);
    return screenChars[(unsigned char)screencode];
}

static int appendUtf8(char *buf, int bufsz, int *bufpos, const char *chrStr)
{
    int chrLen = (int)strlen(chrStr);
    if (bufsz - *bufpos - 1 > chrLen)
    {
        strcpy(buf + *bufpos, chrStr);
    }
    *bufpos += chrLen;
    return chrLen;
}

SOEXPORT int petscii_toUtf8(
        char *buf, int bufsz, const char *str, uint8_t len, int lowercase,
        int onlyPrintable, const char *unknown, const char *shiftspace)
{
    int bufpos = 0;
    for (int i = 0; i < len; ++i)
    {
        unsigned char petsciiChar = (unsigned char)str[i];
        if (shiftspace && petsciiChar == 0xa0)
        {
            appendUtf8(buf, bufsz, &bufpos, shiftspace);
            continue;
        }
        char screencode = toScreencode(petsciiChar);
        const char *utf8Char = 0;
        if (screencode < 0)
        {
            if (!onlyPrintable) utf8Char = nonprintable(petsciiChar);
        }
        else
        {
            utf8Char = screencodeToUtf8(screencode, lowercase);
        }
        if (utf8Char)
        {
            appendUtf8(buf, bufsz, &bufpos, utf8Char);
        }
        else if (unknown)
        {
            appendUtf8(buf, bufsz, &bufpos, unknown);
        }
    }
    return bufpos + 1;
}
