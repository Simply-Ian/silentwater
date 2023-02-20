#include <string>

// Загружает текст из файла в заданной кодировке, перекодирует в UTF-8 и возвращает указатель на строку
std::string* load_text(char* path, char* encoding);