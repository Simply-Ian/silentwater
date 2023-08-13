#include "Fragment.h"
#include <algorithm> // Для std::find()
using namespace std;

Fragment::operator string() const{
    if(text != "&&&"){
        string for_return = "Style: <";
        for (auto i = styles.begin(); i != styles.end(); i++)
            for_return += to_string((int)(*i)) + ", ";
        for_return += "> ";
        for_return += text;
        return for_return;
    }
    else return "\n";
}

bool Fragment::has_a_style(Styles s){
        return find(styles.begin(), styles.end(), s) != styles.end();
}