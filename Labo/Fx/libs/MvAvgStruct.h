typedef struct MvAvg_st // static member
{
	uint Span;
	char *Pair;
	uint CurrFxTime;
	double CurrTotal;
}
MvAvg_t;

MvAvg_t *CreateMvAvg(uint fxTime, uint span, char *pair);
void ReleaseMvAvg(MvAvg_t *i);

// <-- cdtor

double MA_GetMid(MvAvg_t *i, uint fxTime);

// <-- accessor
