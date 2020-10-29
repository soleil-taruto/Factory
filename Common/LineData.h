typedef struct tagRnd_st
{
	char *bgn;
	char *innerBgn;
	char *innerEnd;
	char *end;
}
tagRng_t;

extern tagRng_t lastTagRng;

int updateTagRng(char *str, char *bgnPtn, char *endPtn, int ignoreCase);
int updateAsciiTagRng(char *str, char *bgnPtn, char *endPtn);

