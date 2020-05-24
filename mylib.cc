#include"mylib.h"

std::string read_from_file(const std::string& filename){
    std::stringstream ss;
    char c;
    std::ifstream ifs{filename};
    if(!ifs.is_open()) throw std::runtime_error("can't open file '" + filename + "'.");
    while(!ifs.eof()){
        ifs.get(c);
        ss << c;
    }
    ss << '\n';
    return ss.str();
}
