#ifndef SW_ABS_PATH
#define SW_ABS_PATH
#include <string>

/// @brief Возвращает абсолютный путь к папке, где находится исполняемый файл
/// @return Абсолютный путь к папке, где находится исполняемый файл, в формате C-строки, без слэша в конце
std::string get_abs_path_to_folder();

#endif