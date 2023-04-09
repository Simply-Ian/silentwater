#include "decode_base64.h"

char* decode_base64(const char* start){
    long decoded_len = strlen(start)/4 * 3;
    char* decoded = new char[decoded_len];
    char table[] = "=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char symbol_str[] = " ";
    uint32_t out_buf = 0;

    char* out_cursor = decoded;
    char* in_cursor = const_cast<char*>(start);
    for (long s = 0; s < strlen(start)/4; s++){
        for (char* i = in_cursor; i < in_cursor + 4; i++){
            symbol_str[0] = *i;
            out_buf *= 64;
            int table_index = strstr(table, symbol_str) - table;
            out_buf += table_index > 0 ? table_index-1 : 0; // -1, т.к. "=" в начале
        }
        for (int j = 2; j >= 0; j--){
            *(out_cursor + j) = out_buf % 256;
            out_buf /= 256;
        }
        out_buf = 0;
        in_cursor += 4;
        out_cursor += 3;
    }
    return decoded;
}