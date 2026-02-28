
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
/*---------------------------Progg Shell "UIlib" Module MIT License---------------------------*/


int create_pb(char* Text ,int ShowPercent, int value,int chars, char* FullChar, char* VoidChar, char* Borders, int overwrite, int r,int g, int b){
    //Create Progress bar
    
    if (!chars){
        return -1;
    }
    if (overwrite == 1 || overwrite == 2)
    {
        printf("\r");
    }
    
    int Steps = (value * chars) / 100;
    

    printf("%s   ",Text);
    if (!Borders){
        return -1;
    }
    
    printf("%s", Borders);
    printf("\033[38;2;%d;%d;%dm",r,g,b);
    for (int i = 0; i < chars; i++)
    {
        if (Steps > 0){
            if (!FullChar || FullChar[0] == ' '){
                printf("|");
            }
            else{
                printf("%s",FullChar);
            }
            Steps -= 1;
        }
        else{
            if (!VoidChar || VoidChar[0] == ' ')
            {
                printf("·");
            }
            else{printf("%s",VoidChar);}
            
        }

        
    }
    printf("\033[0m");
    printf("%s  ", Borders);
    if (ShowPercent == 1)
    {
        if (value <= 0){
            printf("   0/100%%");
        }
        else{
            printf("   %d/100%%", value);
        }
    }
    else if (ShowPercent == 2)
    {
        printf("%d/%d",Steps,chars);
    }
    if (overwrite != 1)
    {
        printf("\n");
        
    }
    else if (overwrite == 2)
    {   
        fflush(stdout);
        printf("\n");
    }
    
    else{fflush(stdout);}
    
    
    return 0;
    
}

int CreateGraph(int Height, int value,int Colums, int overwrite, char *bordes,char *BordesStart, char *BordesEnd ,char *full,char *voidChar, int Voidcolor,int TextColor,int FullColor,int BordesColor){
    
    int cells_to_fill = (int)round(((float)value / 100.0) * Height);
    int MoreOne = (Colums > 1) ? 1 : 0;
    if (MoreOne == 0) {
            
        printf("\n\033[%dm%s",BordesColor, BordesEnd);
        
        
        
        for (int i = 0; i < (Height - cells_to_fill); i++) {
            printf("\n\033[%dm%s\033[%dm %s%s%s",BordesColor,bordes,Voidcolor, voidChar,voidChar,voidChar);
        }
        
        
        printf("\r\033[%dm%s\033[%dm %d%%",BordesColor,bordes,TextColor, value); 
        
        
        for (int i = 0; i < cells_to_fill; i++) {
            printf("\n\033[%dm%s\033[%dm %s%s%s",BordesColor,bordes,FullColor, full,full,full);
        }
        printf("\n\033[%dm%s\n\033[0m",BordesColor, BordesStart);
    }
    else {
        printf("\nThis function is currently unavailable  :( \n"); //Place holder
    }
    return 0;   
}

int yesorno(char* Text){
    if (!Text){
        printf("Continue? [Y/N]: ");
    }
    else{
        printf("%s [Y/N]: ", Text);
    }
    char Answer = 'q';
    
    
    while (Answer != 'y' && Answer != 'n')
    {
        scanf(" %c",&Answer);
        Answer = tolower(Answer);
    }
    printf("\n");
    if (Answer == 'y'){
        return 1;
    }
    return 0;
    



}