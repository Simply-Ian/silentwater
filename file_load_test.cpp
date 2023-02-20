#include "load_text.h"
#include <iostream>
#include <string>
using namespace std;

int main(){
    char path[] = "cp1251test.txt";
    char encoding[] = "windows-1251";
    string* cyrillic = load_text(path, encoding);
    cout << *cyrillic << "\n";
    return 0;
}