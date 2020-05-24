#include<fstream>
#include"my9cc.h"
#include"parser.h"

///////////////////////////////////////////
//handling error

std::string exception_base::what() const noexcept{
    return mes;
}

std::string exception_at::what() const noexcept{
    std::stringstream ss;
    if(pos.second == -1){  //いつか直す　ファイル末尾の位置を適切に入れたい
        ss << mes;
        return ss.str();
    }
    for(int i = 0; i < pos.second; i++){
        ss << ' ';
    }
    ss << '^' << mes;
    return ss.str();
}

///////////////////////////////////////////
// handle source file



///////////////////////////////////////////
//main

int main(int argc, char **argv){
    if(argc != 2){
        std::cerr << "error: wrong numbers of arguments." <<std::endl;
        return 1;
    }

#ifdef TEST
    test_parser();
#endif

    std::list<Token> tokens;
    std::list<Token>::iterator itr;
    std::stringstream result_code;
//    std::list<Node*> nodes;
    Node* rootnode = nullptr;    //not deleted
    Textfile tf {argv[1]};

    try{
//        std::string code = read_from_file(argv[1]);
//        tokens = tokenize(code);
        tokens = tokenize_(tf);
/*        for(auto t: tokens){
            std::cout << "Token " << t.kind << " " << t.str << std::endl;
        }*/
        //PRINT("tokenize() has finished.");
        itr = tokens.begin();
        Parser p {itr};
        rootnode = p.program();
#ifdef DEBUG
        {
            std::ofstream ofs{"nodeinfo.txt"};          //テスト用なのでいつか消す
            if(!ofs) throw exception_base("failed to open a file.");
            ofs << "class size:"
                << "\n    sizeof(Token)  = " << sizeof(Token)
                << "\n    sizeof(Node)   = " << sizeof(Node)
                << "\n    sizeof(Parser) = " << sizeof(Parser) << std::endl;
            rootnode->print(ofs);
        }
#endif
        std::cout
            << ".intel_syntax noprefix\n"
            << ".global main" << std::endl;

        gen(std::cout, rootnode);
    }catch(const exception_at& e){
        std::stringstream ss;
        auto line = e.pos.first;
        ss << tf.get_filename() << ":" << (line+1) << ":" << e.pos.second << ": ";
        std::cerr << ss.str() << tf.get_line_of(line) << std::endl;
        for(size_t i = 0; i < ss.str().size(); ++i) std::cerr << ' ';
        std::cerr << e.what() << std::endl;
        return 1;
    }catch(const exception_base& e){
        std::cerr << e.what() << std::endl;
        return 1;
    }catch(const std::runtime_error e){
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
