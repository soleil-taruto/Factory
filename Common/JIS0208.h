int isSJISChar(int mbchr);
int isNECSpecialChar(int mbchr);
int isNECSelectIBMExtendChar(int mbchr);
int isIBMExtendChar(int mbchr);
int isJChar(int mbchr);
int isJCharP(char *p);

int convCharCP932ToUTF16(int mbchr);
int convCharUTF16ToCP932(int mbchr);
int convCharSJISToUTF16(int mbchr);
int convCharUTF16ToSJIS(int mbchr);
