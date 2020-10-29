#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"
#include "C:\Factory\Common\Options\autoTable.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\Meteor\BmpToCsv.h"

#define IMAGE_ID_NUM 1024

void ConvImageFile(char *rFile, char *wFile);
void LoadImageFile(char *file);
void SaveImageFile(char *file);

void SetImageId(uint id);
void SetImageJpegQuality(uint quality);
void SetImageSize(uint w, uint h);
uint GetImg_W(void);
uint GetImg_H(void);

extern uint Img_A;
extern uint Img_R;
extern uint Img_G;
extern uint Img_B;

void GetImgDot(uint x, uint y);
void SetImgDot(uint x, uint y);
