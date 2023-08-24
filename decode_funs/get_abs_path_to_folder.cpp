#include "get_abs_path_to_folder.h"
#include <unistd.h> // Для readlink() -- получения пути к исполняемому файлу
#include <cstring>

std::string get_abs_path_to_folder(){
    char fp_buf[255];
    size_t fp_len = readlink("/proc/self/exe", fp_buf, 255);
    fp_buf[fp_len] = 0; // Символ окончания строки для функции strrchr()
    // Обрезаем путь, оставляя только путь до папки, где лежит бинарник
    size_t folder_len = strrchr(fp_buf, '/') - fp_buf;
    char folder_path[folder_len + 1];
    strncpy(folder_path, fp_buf, folder_len);
    folder_path[folder_len] = 0;

    return std::string(folder_path);
}