#ifndef ALIGNMENT_GROUP
#define ALIGNMENT_GROUP
#include <string>
#include <list>
#include "Fragment.h"

enum alignType{CENTER_BLOCKWISE, CENTER_LINEWISE, RIGHT_LINEWISE, RIGHT_BLOCKWISE, JUSTIFY};

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
    AlignmentGroup(alignType t, int a) : type(t), available_space(a){};
    void setAvailableSpace(int space);
    void addFragment(Fragment* frag){
        this->frags.push_back(frag);
    }
    void align();

    private:
        int available_space;
        std::list<Fragment*> frags;
        void align_center_blockwise();
};
#endif