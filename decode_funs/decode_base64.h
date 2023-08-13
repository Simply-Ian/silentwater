#ifndef DECODE_BASE_64
#define DECODE_BASE_64

#include <cstdint>
#include <cstring>

/*!
    \brief Функция для декодирования base64
    \param [in] start Ссылка на начало закодированной C-строки
    \returns Возвращает ссылку на начало последовательности байтовых данных.
*/
char* decode_base64(const char* start);

#endif