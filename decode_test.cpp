#include "decode_base64.h"
#include <fstream>
#include <cstring>
#include <string>

int main(int argc, char** argv){
    if (argc > 1){
        std::ifstream FILE(argv[1]);
        std::string encoded;
        std::getline(FILE, encoded);
        char prefix[] = "Icons/decoded_";
        char path[strlen(prefix) + strlen(argv[1]) + 1];
        strcpy(path, prefix);
        strcat(path, argv[1]);
        std::ofstream OUT(path, std::ios::binary);
        
        char* decoded = decode_base64(encoded.c_str());
        long decoded_len = encoded.size()/4 * 3;
        for (char* i = decoded; i < decoded + decoded_len; i++){
            OUT << *i;
        }
        delete[] decoded;
    }
    return 0;
}