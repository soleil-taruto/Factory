typedef struct Sabun_st
{
	uint Pos;
	uint Length;
	uchar *Data;
}
Sabun_t;

Sabun_t *CreateSabun(uint pos, uint length, uchar *data);
void ReleaseSabun(Sabun_t *i);
void ReleaseSabunList(autoList_t *list);

void MakeSabun(autoBlock_t *lBlock, autoBlock_t *rBlock, autoList_t *lDiff, autoList_t *rDiff, int lWithData, int rWithData);
