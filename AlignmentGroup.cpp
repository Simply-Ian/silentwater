#include "AlignmentGroup.h"

int AlignmentGroup::offset(){
    if (this->type == alignType::CENTER){
        return (available_space - max_line_len) / 2;
    }
    else return 0; // Stub
}

void AlignmentGroup::updateMaxLineLen(){
    if (line_len >= 0 && line_len <= available_space){
        if (line_len > this->max_line_len)
            max_line_len = line_len;
        line_len = 0;
    }
    else
        throw LineLenException("Failed to set a max line length to an AlignmentGroup object. Invalid line len", line_len);
}

void AlignmentGroup::setAvailableSpace(int space){
    if (space > 0)
        this->available_space = space;
    else
        throw LineLenException("Too little available space", space);
}