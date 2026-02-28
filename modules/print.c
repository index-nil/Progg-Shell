
#include <stdio.h>

void warn(char* Text){
    printf("\033[38;2;252;186;3m[Warning]: %s \033[0m\n",Text);
}
void info(char* Text){
    printf("\033[38;2;0;45;191m[Info]: %s \033[0m\n",Text);
}
void error(char* Text){
    printf("\033[38;2;209;0;0m[Error]: %s \033[0m\n",Text);
}
void done(){
    printf("\033[38;2;33;198;0mDone\033[0m\n");
}