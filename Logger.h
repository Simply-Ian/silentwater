#include <fstream>
#include <string>

class Logger{
    std::string filename;
    std::ofstream FILE;
    public:
        enum levels{INFO, WARNING, CRITICAL}; 
        Logger(std::string f): filename(f){
            FILE.open(filename);
        }

        ~Logger(){
            FILE.close();
        }

        void log(std::string message, levels level);
};