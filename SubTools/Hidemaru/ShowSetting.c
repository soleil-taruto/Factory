/*
	ShowSetting.exe ���̓t�@�C��

		���̓t�@�C�� ... RegExpToText.exe �̏o�̓t�@�C��
*/

#include "C:\Factory\Common\all.h"

enum
{
	RTL_PATH,  // ���W�X�g���̃p�X
	RTL_TYPE,  // �^
	RTL_VALUE, // �l

	RTL_SIZE, // ��̃Z�b�g�̌�
};

static autoList_t *RTLines;

static char *GetValue(char *path)
{
	uint index;

	for (index = 0; index < getCount(RTLines); index += RTL_SIZE)
		if (!strcmp(path, getLine(RTLines, index + RTL_PATH)))
			return getLine(RTLines, index + RTL_VALUE);

	return NULL;
}
static void ShowColor(char *title, autoBlock_t *bVal, uint baseIdx)
{
	uint ext;
	char *sExt;

	cout("%s ", title);
	cout("��:%3u ", getByte(bVal, baseIdx + 0));
	cout("��:%3u ", getByte(bVal, baseIdx + 1));
	cout("��:%3u ", getByte(bVal, baseIdx + 2));

	ext = getByte(bVal, baseIdx + 3);

	switch (ext)
	{
	case 0x00: sExt = strx("�s����"); break;
	case 0x02: sExt = strx("����"); break;

	default:
		sExt = xcout("�s���Ȓl(%03u)", ext);
		break;
	}
	cout("%s\n", sExt);

	memFree(sExt);
}
static void Main2(char *regTextFile)
{
	RTLines = readLines(regTextFile);

	LOGPOS_T();

	cout("\n");
	cout("---- �f�U�C�� ----\n");
	cout("\n");

	{
		char *value = GetValue("HKCU\\Software\\Hidemaruo\\Hidemaru\\Default\\ColorSet");

		if (!value)
		{
			cout("�����\���̐ݒ肪����܂���B\n");
		}
		else
		{
			autoBlock_t *bVal = ab_fromHexLine(value);
			uint r;

			ShowColor("�����\��1", bVal, 48);
			ShowColor("�����\��2", bVal, 84);
			ShowColor("�����\��3", bVal, 264);
			ShowColor("�����\��4", bVal, 276);

			releaseAutoBlock(bVal);
		}
	}

	cout("\n");
	cout("---- �f�U�C�� / �����\�� ----\n");
	cout("\n");

	{
		char *value = GetValue("HKCU\\Software\\Hidemaruo\\Hidemaru\\Default\\HilightWord");

		if (!value)
		{
			cout("�P��̐ݒ肪����܂���B\n");
		}
		else
		{
			autoBlock_t *bVal = ab_fromHexLine(value);
			uint r;

			for (r = 0; r < getSize(bVal); )
			{
				uint len = getByte(bVal, r++);
				uint hh;
				uint c;
				char *word = strx("");
				char *shh;

				hh = getByte(bVal, r++);

				for (c = 0; c < len; c++)
					word = addChar(word, getByte(bVal, r++));

				hh = ((hh << 2) & 0xff) | (hh >> 6); // ROT: << 2

				cout("�啶�����������:%c ", hh & 0x20 ? 'N' : 'Y');
				cout("�P��̌���:%c ",       hh & 0x80 ? 'Y' : 'N');
				cout("���K�\��:%c ",         hh & 0x40 ? 'Y' : 'N');

				hh &= 0x1f;

				switch (hh)
				{
				case 0x04: shh = strx("�����\��1"); break;
				case 0x05: shh = strx("�����\��2"); break;
				case 0x06: shh = strx("�����\��3"); break;
				case 0x07: shh = strx("�����\��4"); break;

				default:
					shh = xcout("�s���ȕ\�����@(%02u)", hh); // HACK
					break;
				}
				cout("%s ", shh);
				cout("%s\n", word);

				memFree(word);
				memFree(shh);
			}
			releaseAutoBlock(bVal);
		}
	}

	cout("\n");
	cout("----\n");
	cout("\n");
}
int main(int argc, char **argv)
{
	Main2(nextArg());
}
