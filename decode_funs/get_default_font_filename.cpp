#include <filesystem>
#include "get_default_font_filename.h"
#include "exec.h"
#include <sstream>
using namespace std;

/// @brief Функция для поиска пути к системному шрифту по умолчанию
/// @return Путь к системному шрифту с засечками по умолчанию
filesystem::path get_default_font_filename(){
    // Получаем имя файла шрифта с засечками по умолчанию
    string raw_output = exec("fc-match serif");
    string fontname = raw_output.substr(0, raw_output.find(":"));
    // Получаем имя шрифта
    fontname = fontname.substr(0, fontname.find("."));
    // Получаем список путей ко всем файлам шрифтов из этого семейства с заданным стилем
    stringstream file_paths(exec("fc-list :family=" + fontname + ":style=Book"));
    string final_path;
    // Отфильтровываем файлы с узким начертанием и выбираем первый попавшийся "широкий" шрифт
    while (getline(file_paths, final_path, '\n')){
        if (final_path.find("Condensed") == string::npos)
            break;
    }
    return final_path.substr(0, final_path.find(":"));
}