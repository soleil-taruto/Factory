extern int coutOff;

void setCoutWrFile(char *file, char *mode);
void unsetCoutWrFile(void);
void setCoutLogFile(char *fileBase);
void setCoutLogFileAdd(char *fileBase);
void cout(char *format, ...);
char *xcout(char *format, ...);
char *vxcout(char *format, va_list marker);
void coutJLine(char *line);
void coutJLine_x(char *line);
void coutLongText(char *text);
void coutLongText_x(char *text);
