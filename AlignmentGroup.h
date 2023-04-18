#ifndef ALIGNMENT_GROUP
#define ALIGNMENT_GROUP
#include <string>

enum alignType{CENTER, RIGHT};

class LineLenException{
    std::string message;
    int line_len;
    public:
        LineLenException(std::string m, int l) : message(m), line_len(l){};
        std::string getMessage() const {return message + std::to_string(line_len);}
};

struct AlignmentGroup{
    bool aligned = false; // Выполнено ли выравнивание текста, принадлежащего этой группе
    alignType type;
    AlignmentGroup(alignType t, int a) : type(t), available_space(a) {};
    int line_len = 0; // Длина текущей обрабатываемой строки
    int offset();
    // Этод метод должен вызываться в конце каждой прочитанной строки текста, который нужно выровнять
    void updateMaxLineLen();
    void setAvailableSpace(int space);

    private:
        int max_line_len = 0;
        int available_space;
};
#endif