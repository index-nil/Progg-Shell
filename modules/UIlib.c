

/*---------------------------Progg Shell "UIlib" Module MIT License---------------------------*/

#include <string.h>
#include "print.h"
#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

// __int8_t create_pb(char* Text ,int ShowPercent, int value,int chars, char* FullChar, char* VoidChar, char* Borders, int overwrite, int r,int g, int b){
    
//     //Create Progress bar
    
//     if (!chars){
//         return -1;
//     }
//     if (overwrite == 1 || overwrite == 2)
//     {
//         write(1,"\r",1);
//     }
    
//     int Steps = (value * chars) / 100;
    

//     print_s("%s   ",Text);
//     if (!Borders){
//         return -1;
//     }
    
//     print_s("%s", Borders);
//     write(1, "\033[38;2;", 7);
//     print_int(r);
//     write(1, ";", 1);
//     print_int(g);
//     write(1, ";", 1);
//     print_int(b);
//     write(1, "m", 1);
//     for (int i = 0; i < chars; i++)
//     {
//         if (Steps > 0){
//             if (!FullChar || FullChar[0] == ' '){
//                 write(1,"|",1);
//             }
//             else{
//                 write(1,FullChar,strlen(FullChar));
//             }
//             Steps -= 1;
//         }
//         else{
//             if (!VoidChar || VoidChar[0] == ' ')
//             {
//                 write(1,"·",1);
//             }
//             else{write(1,VoidChar,strlen(VoidChar));}
            
//         }

        
//     }
//     write(1,"\033[0m",4);
//     print_s("%s  ", Borders);
//     if (ShowPercent == 1)
//     {
//         if (value <= 0){
//             write(1,"   0/100%",9);
//         }
//         else{
//             write(1,"   ",3);
//             print_int(value);
//             write(1,"/100%",5);
//         }
//     }
//     else if (ShowPercent == 2)
//     {
//         print_int(Steps);
//         write(1,"/",1);
//         print_int(chars);
//     }
//     if (overwrite != 1)
//     {
//         write(1,"\n",1);
        
//     }
//     else if (overwrite == 2)
//     {   
        
//         write(1,"\n",1);
//     }
    
    
//     return 0;
    
// }

// __int8_t CreateGraph(int Height, int value, int Colums, int overwrite, char *bordes, char *BordesStart, char *BordesEnd, char *full, char *voidChar, int Voidcolor, int TextColor, int FullColor, int BordesColor) {

//     int cells_to_fill = (value * Height) / 100;
    
//     if (Colums <= 1) {

//         write(1, "\n\033[", 3);
//         print_int(BordesColor);
//         write(1, "m", 1);
//         if (BordesEnd) write(1, BordesEnd, strlen(BordesEnd));

//         for (int i = 0; i < (Height - cells_to_fill); i++) {
//             write(1, "\n\033[", 3);
//             print_int(BordesColor);
//             write(1, "m", 1);
//             if (bordes) write(1, bordes, strlen(bordes));
            
//             write(1, "\033[", 2);
//             print_int(Voidcolor);
//             write(1, "m ", 2);

//             if (voidChar) {
//                 size_t vLen = strlen(voidChar);
//                 write(1, voidChar, vLen);
//                 write(1, voidChar, vLen);
//                 write(1, voidChar, vLen);
//             }
//         }

//         write(1, "\r\033[", 2);
//         print_int(BordesColor);
//         write(1, "m", 1);
//         if (bordes) write(1, bordes, strlen(bordes));
        
//         write(1, "\033[", 2);
//         print_int(TextColor);
//         write(1, "m ", 2);
//         print_int(value);
//         write(1, "%%", 2); 

//         for (int i = 0; i < cells_to_fill; i++) {
//             write(1, "\n\033[", 3);
//             print_int(BordesColor);
//             write(1, "m", 1);
//             if (bordes) write(1, bordes, strlen(bordes));
            
//             write(1, "\033[", 2);
//             print_int(FullColor);
//             write(1, "m ", 2); 
            
//             if (full) {
//                 size_t fLen = strlen(full);
//                 write(1, full, fLen);
//                 write(1, full, fLen);
//                 write(1, full, fLen);
//             }
//         }

//         write(1, "\n\033[", 3);
//         print_int(BordesColor);
//         write(1, "m", 1);
//         if (BordesStart) write(1, BordesStart, strlen(BordesStart));
//         write(1, "\n\033[0m", 5);

//     } else {
//         write(1, "\nThis function is currently unavailable :(\n", 43);
//     }
//     return 0;   
// }

int yesorno(char* Text){
    if (Text == NULL){
        write(1,"Continue? [Y/N]: ",17);
        Text = "Continue?";
    }
    else{
        print_s("%s [Y/N]: ", Text);
    }
    char Answer = 'q';
    
    
    while (Answer != 'y' && Answer != 'n')
    {
        if (read(0, &Answer, 1) <= 0) return 0; 

        if (Answer != 'y' && Answer != 'n'){
            print_s("%s [Y/N]: ", Text);
        }
    }
    write(1,"\n",1);
    if (Answer == 'y'){
        return 1;
    }
    return 0;
    



}
