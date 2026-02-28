#ifndef UI_BASIC_LIB_H
#define UI_BASIC_LIB_H

//------------Extension Basic Lib GPL V3------------//
int create_pb(char* Text ,int ShowPercent, int value,int chars, char* FullChar, char* VoidChar, char* Borders, int overwrite, int r,int g, int b);
int CreateGraph(int Height, int value,int Colums, int overwrite, char *bordes,char *BordesStart, char *BordesEnd ,char *full,char *voidChar, int Voidcolor,int TextColor,int FullColor,int BordesColor);
int yesorno(char* Text);

#endif
