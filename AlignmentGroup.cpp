#include "AlignmentGroup.h"

void AlignmentGroup::align(){
    switch(this->type){
        case alignType::CENTER_BLOCKWISE:
            align_center_blockwise();
            break;
        case alignType::CENTER_LINEWISE:
            align_center_linewise();
            break;
        case alignType::RIGHT_LINEWISE:
            align_right_linewise();
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

void AlignmentGroup::align_center_linewise(){
    int cur_y = -1;
    int cur_offset = 0;
    int cur_line_len = 0;
    list<Fragment*> cur_line;
    auto shift_line = [&](){
        cur_offset = (available_space - cur_line_len) / 2;
        for (auto shifter = cur_line.begin(); shifter != cur_line.end(); shifter++){
            (*shifter)->x += cur_offset;
        }
        cur_line.clear();
    };
    for(auto iter = frags.begin(); iter != frags.end(); iter++){
        Fragment* frag_ptr = *iter;

        if (frag_ptr->y != cur_y){
            shift_line();
            cur_y = frag_ptr->y;
            cur_line_len = 0;
        }
        cur_line_len += frag_ptr->width;
        cur_line.push_back(frag_ptr);
    }
    shift_line();
}

void AlignmentGroup::align_right_linewise(){
    int cur_y = -1;
    int cur_offset = 0;
    int cur_line_len = 0;
    list<Fragment*> cur_line;
    auto shift_line = [&](){
        if (!cur_line.empty()){
            cur_offset = available_space - cur_line_len - (*cur_line.begin())->x; // Группа выравнивания м. б. назначена уже отцентрированному тексту
            for (auto shifter = cur_line.begin(); shifter != cur_line.end(); shifter++){
                (*shifter)->x += cur_offset;
            }
            cur_line.clear();
        }
    };
    for(auto iter = frags.begin(); iter != frags.end(); iter++){
        Fragment* frag_ptr = *iter;

        if (frag_ptr->y != cur_y){
            shift_line();
            cur_y = frag_ptr->y;
            cur_line_len = 0;
        }
        cur_line_len += frag_ptr->width;
        cur_line.push_back(frag_ptr);
    }
    shift_line();
}