typedef struct stampData_st
{
	uint year;  // 1970 �` 3001
	uint month; // 1 �` 12
	uint day;   // 1 �` 31
	uint weekday; // 0 �` 6 == �� �` �y
	uint hour;   // 0 �` 23
	uint minute; // 0 �` 59
	uint second; // 0 �` 59
}
stampData_t;

char *getEMonth(uint month);
char *getEWeekDay(uint weekday);
char *getJWeekDay(uint weekday);

char *makeStamp(time_t t);

stampData_t *getStampData(char *stamp);
stampData_t *getStampDataTime(time_t t);

int isAllowStampData(stampData_t *i);

char *makeJStamp(stampData_t *i, int nonWeekDay);
char *makeCompactStamp(stampData_t *i);
time_t stampDataToTime(stampData_t *i);
time_t compactStampToTime(char *stamp);

// _x
time_t compactStampToTime_x(char *stamp);

// c_
char *c_makeStamp(time_t t);
char *c_makeJStamp(stampData_t *i, int nonWeekDay);
char *c_makeCompactStamp(stampData_t *i);
