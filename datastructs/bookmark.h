#ifndef SW_BOOKMARK
#define SW_BOOKMARK
#include <string>

namespace sw{ // Существует контейнер стандартной библиотеки std::Bookmark. В моем коде повсеместно используется using namespace std
    struct Bookmark{
        std::string preview;
        std::string chapter;
        int page;
        Bookmark(std::string pre, std::string ch, int p): preview(pre), chapter(ch), page(p){};
        Bookmark(){};
    };
}
#endif