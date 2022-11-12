#include "BigInt.h"

static void AddInt(autoList_t *out, uint index, uint64 value)
{
	for (; ; )
	{
		value += refElement(out, index);
		putElement(out, index, (uint)value);
		value >>= 32;

		if (!value)
		{
			break;
		}
		index++;
	}
}

autoList_t *addBigInt(autoList_t *i, autoList_t *j)
{
	uint bound = m_max(getCount(i), getCount(j));
	uint index;
	autoList_t *out;

	out = createAutoList(bound + 1);

	for (index = 0; index < bound; index++)
	{
		AddInt(out, index, (uint64)refElement(i, index) + refElement(j, index));
	}
//	removeTrailZero(out);
	return out;
}
autoList_t *subBigInt(autoList_t *i, autoList_t *j) // i < j ‚Ì‚Æ‚« NULL ‚ð•Ô‚·B
{
	uint bound = m_max(getCount(i), getCount(j));
	uint index;
	autoList_t *out;

	out = createAutoList(bound + 1);

	for (index = 0; index < bound; index++)
	{
		AddInt(out, index, ((uint64)1 << 32) + refElement(i, index) - refElement(j, index) - (index ? 1 : 0));
	}
	if (!refElement(out, bound)) // ? i < j
	{
		releaseAutoList(out);
		return NULL;
	}
	putElement(out, bound, 0);

	removeTrailZero(out);
	return out;
}
int compBigInt(autoList_t *i, autoList_t *j) // ret: (-1, 0, 1) as strcmp()-ly condition
{
	autoList_t *out = subBigInt(i, j);
	uint count;

	if (!out)
		return -1; // i < j

	count = getCount(out);
	releaseAutoList(out);

	if (!count)
		return 0; // i == j

	return 1; // i > j
}
autoList_t *mulBigInt(autoList_t *i, autoList_t *j)
{
	autoList_t *out = createAutoList(getCount(i) + getCount(j));
	uint indx;
	uint jndx;

	for (indx = 0; indx < getCount(i); indx++)
	for (jndx = 0; jndx < getCount(j); jndx++)
	{
		AddInt(out, indx + jndx, (uint64)getElement(i, indx) * getElement(j, jndx));
	}
//	removeTrailZero(out);
	return out;
}
autoList_t *idivBigInt(autoList_t *i, uint j, uint *remain) // remain == NULL: –³Ž‹‚·‚éB
{
	autoList_t *out = createAutoList(getCount(i));
	uint index;
	uint64 mixer = 0;

	errorCase(!j); // ƒ[ƒœŽZ

	if (getCount(i))
	{
		for (index = getCount(i) - 1; ; index--)
		{
			mixer += getElement(i, index);
			putElement(out, index, (uint)(mixer / j));
			mixer %= j;

			if (!index)
				break;

			mixer <<= 32;
		}
		removeTrailZero(out);
	}
	if (remain)
		*remain = (uint)mixer;

	return out;
}
autoList_t *divBigInt(autoList_t *i, autoList_t *j, autoList_t **remain)
{
	autoList_t *out = createAutoList(getCount(i));
	uint count;
	uint index;

	i = copyAutoList(i);
	j = copyAutoList(j);

	removeTrailZero(i);
	removeTrailZero(j);

	errorCase(!getCount(j)); // ƒ[ƒœŽZ

	count = getCount(i) < getCount(j) ? 0 : getCount(i) - getCount(j) + 1;

	for (index = 0; index < count; index++)
		insertElement(j, 0, 0);

	count *= 32;

	while (count)
	{
		autoList_t *wout;

		count--;

		wout = idivBigInt(j, 2, NULL);
		releaseAutoList(j);
		j = wout;

		wout = subBigInt(i, j);

		if (wout)
		{
			releaseAutoList(i);
			i = wout;

			AddInt(out, count / 32, 1u << (count % 32));
		}
	}
	removeTrailZero(out);

	if (remain)
		*remain = i;
	else
		releaseAutoList(i);

	releaseAutoList(j);
	return out;
}
autoList_t *modBigInt(autoList_t *i, autoList_t *j)
{
	autoList_t *out;
	autoList_t *remain;

	out = divBigInt(i, j, &remain);

	releaseAutoList(out);
	return remain;
}

autoList_t *modPowerBigInt(autoList_t *i, autoList_t *j, autoList_t *mod) // mod == NULL: no mod
{
	autoList_t *out = createOneElement(1);
	autoList_t *wout;
	uint index;
	uint bit;

	for (index = getCount(j); ; )
	{
		index--;

		for (bit = 32; bit; )
		{
			bit--;

			if (getElement(j, index) & (1 << bit))
			{
				wout = mulBigInt(out, i);
				releaseAutoList(out);
				out = wout;

				if (mod)
				{
					wout = modBigInt(out, mod);
					releaseAutoList(out);
					out = wout;
				}
			}
			if (!bit && !index)
				goto endLoop;

			wout = mulBigInt(out, out);
			releaseAutoList(out);
			out = wout;

			if (mod)
			{
				wout = modBigInt(out, mod);
				releaseAutoList(out);
				out = wout;
			}
		}
	}
endLoop:
	return out;
}
autoList_t *powerBigInt(autoList_t *i, autoList_t *j)
{
	return modPowerBigInt(i, j, NULL);
}
