
#include <stdio.h>
#include <string.h>

int create_pb(char* Text ,int ShowPercent, int value,int chars, char* FullChar, char* VoidChar, char* Borders, int overwrite){
    //Create Progress bar
    
    if (!chars || !value ||!overwrite){
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
    printf("%s  ", Borders);
    if (ShowPercent == 1)
    {
        if (value <= 0){
            printf("   0%%");
        }
        else{
            printf("   %d", value);
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