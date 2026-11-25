#ifndef __LCD1602_H__
#define __LCD1602_H__

//用户调用函数：
void LC_Init();
void LC_ShowChar(unsigned char Line,unsigned char Column,char Char);
void LC_ShowString(unsigned char Line,unsigned char Column,char *String);
void LC_ShowNum(unsigned char Line,unsigned char Column,unsigned int Number,unsigned char Length);
void LC_ShowSignedNum(unsigned char Line,unsigned char Column,int Number,unsigned char Length);
void LC_ShowHexNum(unsigned char Line,unsigned char Column,unsigned int Number,unsigned char Length);
void LC_ShowBinNum(unsigned char Line,unsigned char Column,unsigned int Number,unsigned char Length);

#endif
