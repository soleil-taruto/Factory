#include "wav.h"

static struct
{
	char Riff[5];
	uint Size;
	char Wave[5];
}
Header;

static struct
{
	int Loaded;
	uint FormatID;
	uint ChannelNum;
	uint Hz;
	uint BytePerSec;
	uint BlockSize;
	uint BitPerSample;
}
Fmt;

static autoBlock_t *RawData;

uint lastWAV_Hz;

autoList_t *readWAVFile(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	autoList_t *wavData;
	uint index;

	zeroclear(Header);
	zeroclear(Fmt);
	RawData = NULL;

	Header.Riff[0] = readChar(fp);
	Header.Riff[1] = readChar(fp);
	Header.Riff[2] = readChar(fp);
	Header.Riff[3] = readChar(fp);

	Header.Size = readValue(fp);

	Header.Wave[0] = readChar(fp);
	Header.Wave[1] = readChar(fp);
	Header.Wave[2] = readChar(fp);
	Header.Wave[3] = readChar(fp);

	errorCase(strcmp(Header.Riff, "RIFF"));
	errorCase(strcmp(Header.Wave, "WAVE"));

	for (; ; )
	{
		int chr = readChar(fp);
		char name[5];
		uint size;

		if (chr == EOF)
			break;

		name[0] = chr;
		name[1] = readChar(fp);
		name[2] = readChar(fp);
		name[3] = readChar(fp);
		name[4] = '\0';
		size = readValue(fp);

		errorCase_m(IMAX < size, "チャンクサイズが大きすぎます。"); // HACK: 厳密な上限ではない。実際の上限はもっと低いはず。

		if (!strcmp(name, "fmt "))
		{
			errorCase_m(Fmt.Loaded, "複数のフォーマットチャンクは処理できません。"); // ? 2回目のフォーマットチャンク
			errorCase(size < 16);

			Fmt.Loaded       = 1;
			Fmt.FormatID     = readValueWidth(fp, 2);
			Fmt.ChannelNum   = readValueWidth(fp, 2);
			Fmt.Hz           = readValue(fp);
			Fmt.BytePerSec   = readValue(fp);
			Fmt.BlockSize    = readValueWidth(fp, 2);
			Fmt.BitPerSample = readValueWidth(fp, 2);

			fileSeek(fp, SEEK_CUR, (sint64)size - 16);
		}
		else if (!strcmp(name, "data"))
		{
			errorCase_m(RawData, "複数のデータチャンクは処理できません。"); // ? 2回目のデータチャンク

			RawData = neReadBinaryBlock(fp, size);
		}
		else
		{
			fileSeek(fp, SEEK_CUR, size);
		}
	}
	fileClose(fp);

	errorCase(Fmt.FormatID != 1); // ? ! PCM
	errorCase(!m_isRange(Fmt.ChannelNum, 1, 2));
	errorCase(!m_isRange(Fmt.Hz, 1, IMAX));
	errorCase(!m_isRange(Fmt.BitPerSample, 8, 16) || Fmt.BitPerSample % 8);
	errorCase(Fmt.BlockSize != Fmt.ChannelNum * (Fmt.BitPerSample >> 3));
	errorCase(Fmt.BytePerSec != Fmt.Hz * Fmt.BlockSize);
	errorCase(!RawData);

	/*
		波形値: 0x8000 を波形高ゼロ, 値域 0x0000 ～ 0xffff (波形高さ -32728 ～ 32767)
		要素: { 上位16ビット, 下位16ビット } == { 左側の波形値, 右側の波形値 }
	*/
	wavData = newList();

	if (Fmt.BitPerSample == 8)
	{
		setAllocCount(wavData, getSize(RawData));

		for (index = 0; index < getSize(RawData); index++)
		{
			addElement(wavData, getByte(RawData, index) * 0x0100); // 8ビットの場合は符号なし整数
		}
	}
	else // 16
	{
		errorCase(getSize(RawData) & 1);

		setAllocCount(wavData, getSize(RawData) / 2);

		for (index = 0; index < getSize(RawData); index += 2)
		{
			addElement(wavData, (getByte(RawData, index) | getByte(RawData, index + 1) << 8) ^ 0x8000); // 16ビットの場合は符号あり整数
		}
	}
	if (Fmt.ChannelNum == 1) // monoral
	{
		for (index = 0; index < getCount(wavData); index++)
		{
			*directGetPoint(wavData, index) *= 0x00010001;
		}
	}
	else // stereo
	{
		errorCase(getCount(wavData) & 1);

		for (index = 0; index * 2 < getCount(wavData); index++)
		{
			*directGetPoint(wavData, index) = getElement(wavData, index * 2) << 16 | getElement(wavData, index * 2 + 1); // { 上位16ビット, 下位16ビット } == { 左側の波形値, 右側の波形値 }
		}
		setCount(wavData, index);
	}
	releaseAutoBlock(RawData);

	lastWAV_Hz = Fmt.Hz;

	return wavData;
}
void writeWAVFile(char *file, autoList_t *wavData, uint hz)
{
	FILE *fp = fileOpen(file, "wb");
	uint value;
	uint index;

	writeToken(fp, "RIFF");
	writeValue(fp, 4 + 4 + 4 + 16 + 4 + 4 + getCount(wavData) * 4);
	writeToken(fp,
		"WAVE"
		"fmt\40"
		);
	writeValue(fp, 16);
	writeValueWidth(fp, 1, 2);
	writeValueWidth(fp, 2, 2);
	writeValue(fp, hz);
	writeValue(fp, hz * 4);
	writeValueWidth(fp, 4, 2);
	writeValueWidth(fp, 16, 2);
	writeToken(fp, "data");
	writeValue(fp, getCount(wavData) * 4);

	foreach (wavData, value, index)
	{
		uint v1 = value >> 16;
		uint v2 = value & 0xffff;

		writeValueWidth(fp, v1 ^ 0x8000, 2); // 左側の波形値
		writeValueWidth(fp, v2 ^ 0x8000, 2); // 右側の波形値
	}
	fileClose(fp);
}
