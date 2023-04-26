#include "AlignmentGroup.h"

void AlignmentGroup::align(){
    switch(this->type){
        case alignType::CENTER_BLOCKWISE:
            align_center_blockwise();
            break;
    }
}

void AlignmentGroup::setAvailableSpace(int space){
    if (space > 0)
        this->available_space = space;
    else
        throw LineLenException("Too little available space", space);
}

void AlignmentGroup::align_center_blockwise(){
    int cur_y = -1;
    int cur_line_len = 0;
    int max_line_len = 0;
    for(auto iter = frags.begin(); iter != frags.end(); iter++){
        Fragment* frag_ptr = *iter;
        if (frag_ptr->y != cur_y){
            max_line_len = cur_line_len > max_line_len? cur_line_len : max_line_len;
            cur_line_len = 0;
            cur_y = frag_ptr->y;
        }
        cur_line_len += frag_ptr->width;
    }
    max_line_len = cur_line_len > max_line_len? cur_line_len : max_line_len; // На случай, если последняя строка -- самая длинная
    int offset = (available_space - max_line_len) / 2;

    // Сдвигаем все фрагменты на величину offset
    for(auto iter = frags.begin(); iter != frags.end(); iter++)
        (*iter)->x += offset;
}