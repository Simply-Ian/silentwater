#include "Controller.h"
#include <iostream>

void Controller::load_book(char* path){
    model.load_fb2(path);
    model.split_into_words();
}

int main(){
    Controller cont;
    char FILE_NAME[] = "RedRing.fb2";
    cont.load_book(FILE_NAME);
    for (Fragment frag: cont.model.fragments){
        cout << (string)frag;
    }
    return 0;
}