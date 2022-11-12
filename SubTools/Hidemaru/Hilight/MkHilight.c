#include "C:\Factory\Common\all.h"

static uint HLSettingToValue(int oomojiKomojiKubetsu, int tangoNoKensaku, int seikihyougen, uint kyouchouHyoujiNo)
{
	uint value = 1;

	value |= !oomojiKomojiKubetsu ? 0x008 : 0;
	value |= tangoNoKensaku       ? 0x020 : 0;
	value |= seikihyougen         ? 0x010 : 0;
	value |= (kyouchouHyoujiNo - 1) << 6;
	value |= (kyouchouHyoujiNo - 1) >> 1 & 2;

	return value;
}
static void ShowSettingOutputToHilight(char *rFile, char *wFile)
{
	autoList_t *lines = readLines(rFile);
	char *line;
	uint index;
	autoList_t *dest = newList();

	foreach (lines, line, index)
	{
		if (lineExp("‘å•¶š¬•¶š‹æ•Ê:<1,NNYY> ’PŒê‚ÌŒŸõ:<1,NNYY> ³‹K•\Œ»:<1,NNYY> ‹­’²•\¦<1,18> <1,,>", line))
		{
			int oomojiKomojiKubetsu = line[17] == 'Y';
			int tangoNoKensaku      = line[30] == 'Y';
			int seikihyougen        = line[41] == 'Y';
			uint kyouchouHyoujiNo   = line[51] - '0';
			char *word              = line + 53;
			char *outLine;

			outLine = xcout("%u,%s", HLSettingToValue(oomojiKomojiKubetsu, tangoNoKensaku, seikihyougen, kyouchouHyoujiNo), word);

			addElement(dest, (uint)outLine);
		}
	}
	writeLines(wFile, dest);

	releaseDim(lines, 1);
	releaseDim(dest, 1);
}
static void MkTestData(char *wFile)
{
	int oomojiKomojiKubetsu;
	int tangoNoKensaku;
	int seikihyougen;
	uint kyouchouHyoujiNo;
	autoList_t *dest = newList();

	for (oomojiKomojiKubetsu = 0; oomojiKomojiKubetsu <= 1; oomojiKomojiKubetsu++)
	for (tangoNoKensaku      = 0; tangoNoKensaku      <= 1; tangoNoKensaku++)
	for (seikihyougen        = 0; seikihyougen        <= 1; seikihyougen++)
	for (kyouchouHyoujiNo    = 1; kyouchouHyoujiNo    <= 8; kyouchouHyoujiNo++)
	{
		addElement(dest, (uint)xcout("%u,_%d%d%d%u",
			HLSettingToValue(oomojiKomojiKubetsu, tangoNoKensaku, seikihyougen, kyouchouHyoujiNo),
			oomojiKomojiKubetsu,
			tangoNoKensaku,
			seikihyougen,
			kyouchouHyoujiNo
			));
	}
	writeLines(wFile, dest);

	releaseDim(dest, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/T"))
	{
		MkTestData(nextArg());
		return;
	}

	ShowSettingOutputToHilight(getArg(0), getArg(1));
}
