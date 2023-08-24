#ifndef SW_LOAD_TEXT
#define SW_LOAD_TEXT

#include <string>

// Загружает текст из файла в заданной кодировке, перекодирует в UTF-8 и возвращает указатель на строку
std::string* load_text(const char* path, char* encoding);

#endif