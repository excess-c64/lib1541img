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

SOEXPORT void petscii_mapUpperGfxToLower(char *str, size_t len)
{
    for (size_t i = 0; i < len; ++i)
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

static unsigned char nppetscii(char utf8char)
{
    switch (utf8char)
    {
	case 0x3: return 0x3;
	case 0x8: return 0x14;
	case 0xa: return 0xd;
	case 0xd: return 0x8d;
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

static unsigned char toPetsciiChar(char screencode)
{
    if ((unsigned char)screencode > 0x7f) return 0;
    if (screencode < 0x20 || screencode >= 0x60) return screencode + 0x40;
    if (screencode >= 0x40) return screencode + 0x20;
    return screencode;
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

static void guessCase(PetsciiCase *casemode, int lowercase)
{
    if (*casemode & PC_GUESS)
    {
	*casemode &= ~PC_GUESS;
	if (lowercase) *casemode |= PC_LOWER;
	else *casemode &= ~PC_LOWER;
    }
}

#define canMapLower(cm) (!(*(cm) & PC_FORCE) || (*(cm) & PC_LOWER))
#define canMapUpper(cm) (!(*(cm) & PC_FORCE) || !(*(cm) & PC_LOWER))

static char lowerOrInvalid(unsigned char c, PetsciiCase *casemode)
{
    guessCase(casemode, 1);
    if (canMapLower(casemode)) return c;
    return (char)-1;
}

static char upperOrInvalid(unsigned char c, PetsciiCase *casemode)
{
    guessCase(casemode, 0);
    if (canMapUpper(casemode)) return c;
    return (char)-1;
}

static char screencodeFromUtf8(
	const char *str, size_t strlen, size_t *strpos, PetsciiCase *casemode)
{
    if (*strpos >= strlen) return (char)-1;
    unsigned char next = (unsigned char)str[(*strpos)++];
    if (next < 0x80)
    {
	if (next > 0x1f && next < 0x40) return next;
	if (next > 0x60 && next < 0x7b) 
	{
	    return lowerOrInvalid(next - 0x60, casemode);
	}
	if (next > 0x40 && next < 0x5b)
	{
	    if (*casemode & PC_LOWER) return next;
	    return next - 0x40;
	}
	if (next == 0x40 || next == 0x5b || next == 0x5d) return next - 0x40;
	return (char)-1;
    }
    if (*strpos >= strlen) return (char)-1;
    switch (next)
    {
	case 0xc2:
	    next = (unsigned char)str[(*strpos)++];
	    if (next == 0xa3) return 0x1c;
	    if (next == 0xa0) return 0x60;
	    break;

	case 0xe2:
	    next = (unsigned char)str[(*strpos)++];
	    if (*strpos >= strlen) return (char)-1;
	    switch (next)
	    {
		case 0x86:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0x91) return 0x1e;
		    if (next == 0x90) return 0x1f;
		    break;

		case 0x94:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0x80) return 0x40;
		    if (next == 0xbc) return 0x5b;
		    if (next == 0x82) return 0x5d;
		    if (next == 0x9c) return 0x6b;
		    if (next == 0x94) return 0x6d;
		    if (next == 0x90) return 0x6e;
		    if (next == 0x8c) return 0x70;
		    if (next == 0xb4) return 0x71;
		    if (next == 0xac) return 0x72;
		    if (next == 0xa4) return 0x73;
		    if (next == 0x98) return 0x7d;
		    break;

		case 0x99:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0xa0) return upperOrInvalid(0x41, casemode);
		    if (next == 0xa5) return upperOrInvalid(0x53, casemode);
		    if (next == 0xa3) return upperOrInvalid(0x58, casemode);
		    if (next == 0xa6) return upperOrInvalid(0x5a, casemode);
		    break;

		case 0x95:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0xae) return upperOrInvalid(0x49, casemode);
		    if (next == 0xb0) return upperOrInvalid(0x4a, casemode);
		    if (next == 0xaf) return upperOrInvalid(0x4b, casemode);
		    if (next == 0xb2) return upperOrInvalid(0x4d, casemode);
		    if (next == 0xb1) return upperOrInvalid(0x4e, casemode);
		    if (next == 0xad) return upperOrInvalid(0x55, casemode);
		    if (next == 0xb3) return upperOrInvalid(0x56, casemode);
		    break;

		case 0x80:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0xa2) return upperOrInvalid(0x51, casemode);
		    break;

		case 0x97:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0x8b) return upperOrInvalid(0x57, casemode);
		    if (next == 0xa5) return upperOrInvalid(0x5f, casemode);
		    if (next == 0xa4) return upperOrInvalid(0x69, casemode);
		    break;

		case 0x96:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0x8c) return 0x61;
		    if (next == 0x84) return 0x62;
		    if (next == 0x94) return 0x63;
		    if (next == 0x81) return 0x64;
		    if (next == 0x8e) return 0x65;
		    if (next == 0x92) return 0x66;
		    if (next == 0x95) return 0x67;
		    if (next == 0x97) return 0x6c;
		    if (next == 0x82) return 0x6f;
		    if (next == 0x8e) return 0x74;
		    if (next == 0x8d) return 0x75;
		    if (next == 0x83) return 0x79;
		    if (next == 0x96) return 0x7b;
		    if (next == 0x9d) return 0x7c;
		    if (next == 0x98) return 0x7e;
		    if (next == 0x9a) return 0x7f;
		    break;

		case 0x9c:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0x93) return lowerOrInvalid(0x7a, casemode);
		    break;
	    }
	    break;

	case 0xf0:
	    next = (unsigned char)str[(*strpos)++];
	    if (*strpos >= strlen) return (char)-1;
	    if (next != 0x9f) break;
	    next = (unsigned char)str[(*strpos)++];
	    if (*strpos >= strlen) return (char)-1;
	    switch (next)
	    {
		case 0xad:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0xb7) return upperOrInvalid(0x44, casemode);
		    if (next == 0xb6) return upperOrInvalid(0x45, casemode);
		    if (next == 0xba) return upperOrInvalid(0x46, casemode);
		    if (next == 0xb1) return upperOrInvalid(0x47, casemode);
		    if (next == 0xb4) return upperOrInvalid(0x48, casemode);
		    if (next == 0xbc) return upperOrInvalid(0x4c, casemode);
		    if (next == 0xbd) return upperOrInvalid(0x4f, casemode);
		    if (next == 0xbe) return upperOrInvalid(0x50, casemode);
		    if (next == 0xbb) return upperOrInvalid(0x52, casemode);
		    if (next == 0xb0) return upperOrInvalid(0x54, casemode);
		    if (next == 0xb5) return upperOrInvalid(0x59, casemode);
		    if (next == 0xbf) return upperOrInvalid(0x7a, casemode);
		    break;

		case 0xae:
		    next = (unsigned char)str[(*strpos)++];
		    if (next == 0x8c) return 0x5c;
		    if (next == 0x8f) return 0x68;
		    if (next == 0x87) return 0x6a;
		    if (next == 0x88) return 0x76;
		    if (next == 0x82) return 0x77;
		    if (next == 0x83) return 0x78;
		    if (next == 0x95) return lowerOrInvalid(0x5e, casemode);
		    if (next == 0x98) return lowerOrInvalid(0x5f, casemode);
		    if (next == 0x99) return lowerOrInvalid(0x69, casemode);
		    break;
	    }
	    break;

	case 0xcf:
	    next = (unsigned char)str[(*strpos)++];
	    if (*strpos >= strlen) return (char)-1;
	    if (next == 0x80) return upperOrInvalid(0x5e, casemode);
	    break;
    }
    while (*strpos < strlen && ((unsigned char)str[*strpos] & 0xc0) == 0x80)
    {
	++(*strpos);
    }
    return (char)-1;
}

static size_t appendUtf8(
	char *buf, size_t bufsz, size_t *bufpos, const char *chrStr)
{
    size_t chrLen = strlen(chrStr);
    if (bufsz > *bufpos + chrLen + 1)
    {
        strcpy(buf + *bufpos, chrStr);
    }
    *bufpos += chrLen;
    return chrLen;
}

SOEXPORT size_t petscii_toUtf8(
        char *buf, size_t bufsz, const char *str, size_t len, int lowercase,
        int onlyPrintable, const char *unknown, const char *shiftspace)
{
    size_t bufpos = 0;
    size_t result = 0;
    for (size_t i = 0; i < len; ++i)
    {
        unsigned char petsciiChar = (unsigned char)str[i];
        if (shiftspace && petsciiChar == 0xa0)
        {
            result += appendUtf8(buf, bufsz, &bufpos, shiftspace);
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
            result += appendUtf8(buf, bufsz, &bufpos, utf8Char);
        }
        else if (unknown)
        {
            result += appendUtf8(buf, bufsz, &bufpos, unknown);
        }
    }
    return result + 1;
}

static size_t appendPetscii(char *buf, size_t bufsz, size_t *bufpos,
	unsigned char petscii)
{
    if (*bufpos + 1 < bufsz)
    {
	buf[(*bufpos)++] = petscii;
	buf[*bufpos] = 0;
    }
    return 1;
}

SOEXPORT size_t petscii_fromUtf8(
	char *buf, size_t bufsz, const char *str, size_t len,
	PetsciiCase casemode, int onlyPrintable, char unknown)
{
    size_t strpos = 0;
    size_t bufpos = 0;
    size_t result = 0;

    while (strpos < len)
    {
	if ((unsigned char)str[strpos] < 0x20)
	{
	    if (!onlyPrintable)
	    {
		unsigned char npp = nppetscii(str[strpos]);
		if (npp) result += appendPetscii(buf, bufsz, &bufpos, npp);
	    }
	    ++strpos;
	}
	else
	{
	    char screencode = screencodeFromUtf8(str, len, &strpos, &casemode);
	    if ((unsigned char)screencode < 0x80)
	    {
		result += appendPetscii(
			buf, bufsz, &bufpos, toPetsciiChar(screencode));
	    }
	    else if (unknown)
	    {
		result += appendPetscii(buf, bufsz, &bufpos, unknown);
	    }
	}
    }
    return result + 1;
}

