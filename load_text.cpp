#include <iconv.h>
#include <sys/stat.h> // Для получения размера файла
#include <sys/types.h> // Аналогично
#include <string>
#include <fstream>
using namespace std;

string* load_text(char* path, char* encoding){
    // Reading the file
    char buffer;
    ifstream file(path);
    struct stat st; // To get the size of the file
    int file_size = 0;
    if (stat(path, &st) == 0)
        file_size = st.st_size;
    else
        throw runtime_error("Problems with getting access to the file");
    char* source = new char[file_size];
    int i = 0;
    do{
        file.get(buffer);
        source[i] = buffer;
        i++;
    }
    while (!file.eof());
    file.close();

    // Re-encoding
    string* for_return = new string;
    //cout << "load_text.cpp: " << path << " " << encoding << "\n";
    iconv_t cd = iconv_open("UTF-8", encoding); // Descriptor
    if (cd == (iconv_t)(-1)) throw runtime_error("Can`t start iconv");

    char* inptr = &source[0];
    size_t in_size = file_size;
    size_t out_size = file_size * 2;
    char* out_buf = new char[out_size];
    char* out_ptr = &out_buf[0];
    size_t result;
    errno = 0;
    result = iconv(cd, &inptr, &in_size, &out_ptr, &out_size);
    if (result == (size_t)(-1) && errno){
        delete[] source;
        delete[] out_buf;
        string message = "Can`t decode. errno ";
        message.append(to_string(errno));
        throw runtime_error(message);
    }
    for_return->append(out_buf);
    delete[] source;
    delete[] out_buf;
    if(iconv_close(cd) != 0) throw runtime_error("Can`t close iconv desc");

    return for_return;
}