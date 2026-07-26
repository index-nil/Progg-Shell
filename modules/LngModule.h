
#ifndef LANGUAGE_MODULE_H
#define LANGUAGE_MODULE_H





char* getSection(char* SectionName, const char* langFile, signed char mode) ;
int WriteToSection(const char* SectionName, const char* Value, const char* path);

// char* getlangline(int n, const char* langfile);




#endif