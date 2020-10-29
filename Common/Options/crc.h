#pragma once

#include "C:\Factory\Common\all.h"

uint crc8Start(void);
uint crc8Update(uint counter, uint byte);
uint crc8UpdateBlock(uint counter, void *block, uint blockSize);
uint crc8UpdateLine(uint counter, char *line);
uint crc8Finish(uint counter);
uint crc8CheckBlock(void *block, uint blockSize);
uint crc8CheckLine(char *line);
uint crc8CheckFile(char *file);

uint crc16Start(void);
uint crc16Update(uint counter, uint byte);
uint crc16UpdateBlock(uint counter, void *block, uint blockSize);
uint crc16UpdateLine(uint counter, char *line);
uint crc16Finish(uint counter);
uint crc16CheckBlock(void *block, uint blockSize);
uint crc16CheckLine(char *line);
uint crc16CheckFile(char *file);

uint crc32Start(void);
uint crc32Update(uint counter, uint byte);
uint crc32UpdateBlock(uint counter, void *block, uint blockSize);
uint crc32UpdateLine(uint counter, char *line);
uint crc32Finish(uint counter);
uint crc32CheckBlock(void *block, uint blockSize);
uint crc32CheckLine(char *line);
uint crc32CheckFile(char *file);

// ----

char *coInputLineCRC(void);
