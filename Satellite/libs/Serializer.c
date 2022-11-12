/*
	バイト列とリストしか扱わない。
	リストは、バイト列までの深さが均一でなければならない。

	データの破損 -> error();
*/

#include "Serializer.h"

#define KIND_BYTES 'B'
#define KIND_LIST 'L'

static autoBlock_t *WBuff;

static void AddInt(uint value)
{
	/*
		big-endian
	*/
	addByte(WBuff, value >> 24 & 0xff);
	addByte(WBuff, value >> 16 & 0xff);
	addByte(WBuff, value >>  8 & 0xff);
	addByte(WBuff, value >>  0 & 0xff);
}
static void SerializerMain(void *data, uint depth)
{
	errorCase(!data);

	if (!depth)
	{
		autoBlock_t *block = (autoBlock_t *)data;

		addByte(WBuff, KIND_BYTES);
		AddInt(getSize(block));
		addBytes(WBuff, block);
	}
	else
	{
		autoList_t *subDataList = (autoList_t *)data;
		void *subData;
		uint index;

		addByte(WBuff, KIND_LIST);
		AddInt(getCount(subDataList));

		foreach (subDataList, subData, index)
		{
			SerializerMain(subData, depth - 1);
		}
	}
}

/*
	depth == 0 ... data == BYTES
	depth == 1 ... data == { BYTES ... }
	depth == 2 ... data == {{ BYTES ... } ... }
*/
autoBlock_t *Stllt_Serializer(void *data, uint depth)
{
	WBuff = newBlock();
	SerializerMain(data, depth);
	return WBuff;
}
autoBlock_t *Stllt_Serializer_0(autoBlock_t *data)
{
	return Stllt_Serializer(data, 0);
}

static autoBlock_t *RBuff;
static uint RIndex;

static int ReadByte(void)
{
	return getByte(RBuff, RIndex++);
}
static uint ReadInt(void)
{
	uint ret = ReadByte() << 8;

	ret |= ReadByte();
	ret <<= 8;
	ret |= ReadByte();
	ret <<= 8;
	ret |= ReadByte();

	return ret;
}
static void ReadBytes(void *out_p, uint size)
{
	memcpy(out_p, directGetBlock(RBuff, RIndex, size), size);
	RIndex += size;
}
static void *DeserializerMain(uint depth)
{
	if (!depth)
	{
		autoBlock_t *buff;
		uint size;

		errorCase(ReadByte() != KIND_BYTES);

		size = ReadInt();
		buff = nobCreateBlock(size);
		ReadBytes(directGetBuffer(buff), size);

		return buff;
	}
	else
	{
		autoList_t *wList;
		uint count;
		uint index;

		errorCase(ReadByte() != KIND_LIST);

		count = ReadInt();
		wList = createAutoList(count);

		for (index = 0; index < count; index++)
		{
			addElement(wList, (uint)DeserializerMain(depth - 1));
		}
		return wList;
	}
}

/*
	depth == 0 ... ret == BYTES                 -> releaseDim_BR(ret, 0, releaseAutoBlock); or releaseAutoBlock(ret);
	depth == 1 ... ret == { BYTES ... }         -> releaseDim_BR(ret, 1, releaseAutoBlock);
	depth == 2 ... ret == {{ BYTES ... } ... }  -> releaseDim_BR(ret, 2, releaseAutoBlock);

	data と depth が一致しない。-> error();
*/
void *Stllt_Deserializer(autoBlock_t *data, uint depth)
{
	void *ret;

	RBuff = data;
	RIndex = 0;
	ret = DeserializerMain(depth);

	errorCase(RIndex < getSize(RBuff));

	return ret;
}
autoBlock_t *Stllt_Deserializer_0(autoBlock_t *data)
{
	return (autoBlock_t *)Stllt_Deserializer(data, 0);
}
