#ifndef SW_COMMONDATA
#define SW_COMMONDATA
#include <string>
using namespace std;

/// Предназначена для передачи данных от контроллера представлению и обратно
struct commonData{
    struct controller_to_view{
        int bookFontSize;
        int lineInterval;
        string defaultFontName;
        string preview;

        controller_to_view(string name, int fs, int li, string pre) : defaultFontName(name), bookFontSize(fs), lineInterval(li), preview(pre){}
    };

    commonData(string name, int fs, int li, string pre) : c_to_v(name, fs, li, pre){}

    controller_to_view c_to_v;
};

#endif