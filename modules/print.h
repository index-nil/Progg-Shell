#ifndef PRINT_TOOLS_H
#define PRINT_TOOLS_H



void warn(char* Text);
void info(char* Text);
void error(char* Text);
void done();
void print_s(const char* format, const char* insert);
void print_ull(unsigned long long n);
void print_int(int n);
void print_float(float f, int precision);
void print_long(long n);




#define C_WARN "\033[38;2;252;186;3m[Warning]: "
#define C_INFO "\033[38;2;0;45;191m[Info]: "
#define C_ERROR "\033[38;2;209;0;0m[Error]: "
#define C_RESET " \033[0m\n"
#define C_DONE "\033[38;2;33;198;0mDone\033[0m\n"





#endif