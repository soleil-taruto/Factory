#include "CRandom.h"

char *MakeUUID(int bracket)
{
	char *buff = strx("");
	char *p;

	if (bracket)
	{
		buff = addChar(buff, '{');
	}
	for (p = "HHHH-HH-4H-8H-HHHHHH"; *p; p++)
	{
		switch (*p)
		{
		case 'H':
			buff = addLine_x(buff, xcout("%02x", getCryptoByte()));
			break;

		case '4':
			buff = addLine_x(buff, xcout("%02x", getCryptoByte() & 0x0f | 0x40));
			break;

		case '8':
			buff = addLine_x(buff, xcout("%02x", getCryptoByte() & 0x3f | 0x80));
			break;

		case '-':
			buff = addChar(buff, '-');
			break;

		default:
			error();
		}
	}
	if (bracket)
	{
		buff = addChar(buff, '}');
	}
	return buff;
}
int GetRandIDChar(void)
{
	int chr;

	do
	{
		chr = getCryptoByte();
	}
	while (252 <= chr);

	chr %= 36;
	chr = m_i2c(chr);

	return chr;
}
char *MakeRandID(void)
{
	char *buff = strx("");
	uint index;

	for (index = 0; index < 25; index++)
		buff = addChar(buff, GetRandIDChar());

	return buff;
}
char *MakeRandHexID(void)
{
	char *buff = strx("");
	uint index;

	for (index = 0; index < 16; index++)
		buff = addLine_x(buff, xcout("%02x", getCryptoByte()));

	return buff;
}
autoBlock_t *GetConcreteRawKey(void)
{
	static autoBlock_t *rawKey;

	if (!rawKey)
		rawKey = ab_fromLine("DancingStarsOnMe"); // 16 bytes

	return rawKey;
}
