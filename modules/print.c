
#include "print.h"
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif



void warn(char* str) {
    printf("%s%s%s\n", C_WARN, str,C_RESET);
}

void info(char* str) {
    printf("%s%s%s\n", C_INFO, str,C_RESET);
}
void error(char* str) {
    printf("%s%s%s\n", C_ERROR, str,C_RESET);
}
void done() {
    printf("%s\n", C_DONE);
}


void print_s(const char* format, const char* insert) {
    const char* p = strstr(format, "%s");
    if (p) {
       
        write(1, format, p - format);
        
        write(1, insert, strlen(insert));

        write(1, p + 2, strlen(p + 2));
    } else {
        write(1, format, strlen(format));
    }
}

void print_int(int n) {
    char buf[12]; 
    int i = 10;
    buf[11] = '\0';
    unsigned int num;


    if (n < 0) {
        write(1, "-", 1);
        num = -n;
    } else {
        num = n;
    }


    if (num == 0) {
        write(1, "0", 1);
        return;
    }


    while (num > 0) {
        buf[i--] = (num % 10) + '0';
        num /= 10;
    }


    write(1, &buf[i + 1], 10 - i);
}
void print_float(float f, int precision) {

    int integer_part = (int)f;
    print_int(integer_part);
    
    write(1, ".", 1);
    

    float fractional = f - (float)integer_part;
    for (int i = 0; i < precision; i++) {
        fractional *= 10;
    }
    print_int((int)fractional);
}
void print_long(long n) {
    char buf[21];
    int i = 19;
    buf[20] = '\0';
    if (n == 0) { write(1, "0", 1); return; }
    if (n < 0)  { write(1, "-", 1); n = -n; }
    while (n > 0) {
        buf[i--] = (n % 10) + '0';
        n /= 10;
    }
    write(1, &buf[i + 1], 19 - i);
}