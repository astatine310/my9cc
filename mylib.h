#ifndef __MYLIB_H_20200510__
#define __MYLIB_H_20200510__

#include<string>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<vector>
#include<iostream>

template<class T, class Container>
bool elem_of (const T &x, const Container &arr){
    return std::any_of(arr.begin(), arr.end(), [&](const T &e){return x == e;});
}

template<int N>
bool elem_of(const char x, const char (&arr)[N]){
    return std::any_of(std::begin(arr), std::end(arr), [&](const char e){return x == e;});
}

std::string read_from_file(const std::string& filename);

class Textfile : public std::ifstream{
    std::vector<std::ifstream::pos_type> lines;
    const std::string filename;
    size_t maxlength = 0;
public:
    template<class ...Args>
    Textfile(const std::string& fname, Args&&...args) : std::ifstream(fname, std::forward<Args>(args)...), filename(fname){
        char c;
        unsigned int len = 0;
        if(!is_open()) throw std::runtime_error("can't open file '" + fname + "'.");
        lines.push_back(0);
        for(;;){
            get(c);
            ++len;
            if(eof()){
                maxlength = (maxlength < len)? len : maxlength;
                break;
            }
            if(c == '\n'){
                int a = static_cast<int>(tellg());
                lines.push_back(a);
                maxlength = (maxlength < len)? len : maxlength;
                len = 0;
            }
        }
        clear();
        seekg(0);
    }
    void seek_line(size_t l){
        if(lines.size() <= l){
            throw std::out_of_range("Designated line number is exceeding.");
        }
        clear();
        seekg(lines[l]);
    }
    std::pair<size_t, int> tell_line() noexcept{
        auto p = tellg();
        if(p < 0){
            return {0, -1};
        }
        for(unsigned int i = 0; ; ++i){
            if(p < lines[i]){
//                std::cout << "Position {" << i-1 << ", " << p - lines[i-1] << "}" << std::endl; 
                return {i-1, p-lines[i-1]};
            }
            if(i == lines.size() - 1){
//                std::cout << "Position {" << i << ", " << p - lines[i] << "}" << std::endl; 
                return {i, p - lines[i]}; 
            }
        }
    }
    size_t num_lines() noexcept{
        return lines.size();
    }
    std::string get_line_of(size_t l){
        seek_line(l);
        char *p = new char[maxlength+1];
        getline(p, maxlength+1);
        std::string str {p};
        delete [] p;
        return std::move(str);
    }
    const std::string& get_filename(){
        return filename;
    }
};

#endif