typedef struct Price_st // static member
{
	double Bid;
	double Ask;
}
Price_t;

Price_t *CreatePrice(double bid, double ask);
Price_t *CopyPrice(Price_t *i);
void ReleasePrice(Price_t *i);

// <-- cdtor

double GetBid(Price_t *i);
double GetAsk(Price_t *i);
double GetMid(Price_t *i);
double GetSpread(Price_t *i);

// <-- accessor
