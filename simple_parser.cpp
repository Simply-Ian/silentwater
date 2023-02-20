#include <iostream>
#include "pugixml.hpp"
#include <cstring> // pugixml returns all the string data as C strings
#include "load_text.h"
using namespace std;

int main(){
    char FILE_NAME[] = "RedRing.fb2";
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(FILE_NAME, pugi::parse_declaration);
    if (!result) throw runtime_error(result.description());
    char* encoding_name = (char*)(doc.first_child().last_attribute().value());
    cout << encoding_name << "\n";

    string* re_opened = load_text(FILE_NAME, encoding_name);
    doc.reset();
    result = doc.load_string(re_opened->c_str());
    delete re_opened;

    pugi::xml_node ti = doc.first_element_by_path("/FictionBook/description/title-info");
    for (auto child = ti.first_child(); child; child = child.next_sibling()){
        cout << child.name() << " ";
        for (auto attr = child.first_attribute(); attr; attr = attr.next_attribute()){
            cout << attr.name() << "=" << attr.value() << " ; ";
        }
        if (child.text()) cout << child.text().get();
        cout << "\n";
    }
    
    return 0;
}