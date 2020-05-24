#include<array>
#include<algorithm>

#include"my9cc.h"

/////////////////////////////////////////////////
//tokenize

bool Token::expect(const std::string &s) const{
    if(kind != TK_RESERVED || str != s){
        std::stringstream ss;
        ss << "it is not '" << s << "'.";
        throw exception_at(pos, ss.str());
    }
    return true;
}
int Token::expect_number() const{
    if(kind != TK_NUM){
        throw exception_at(pos, "it is not a number.");
    }
    return val;
}
std::string Token::expect_ident() const{
    if(kind != TK_IDENT){
        throw exception_at(pos, "it shall be a ident.");
    }
    return str;
}

//read a number located on the front of a string
//no side effect
/*std::string take_front_num_(const std::string &str, std::size_t p){
    std::stringstream ss;
    for(;isdigit(str[p]) && p < str.length(); p++){
        ss << str[p];
    }
    return std::move(ss.str());
}*/

inline bool is_usable_char(const char c){
    return isalpha(c) || isdigit(c) || c == '_';
}

/*
//戻り値も抽象化できる・・・・
std::string take_while(std::function<bool(char)> pred, const std::string &str, std::size_t p){
    std::stringstream ss;
    for(;pred(str[p]) && p < str.length(); p++){
        ss << str[p];
    }
    return std::move(ss.str());
}
auto take_front_alnum_ = [](const std::string &str, std::size_t p){
    return take_while(is_usable_char, str, p);
};*/

std::array reserved_1char = {'+','-','*','/','<','>','=',';', '(', ')', '{', '}', ',', '&', '[', ']', '"'};
std::list<std::string> reserved_2char = {"==", "!=", "<=", ">="};
std::map<std::string, TokenKind> reserved_words = {
    {"return", TK_RETURN},
    {"if",     TK_IF},
    {"while",  TK_WHILE},
    {"for",    TK_FOR},
    {"else",   TK_ELSE},
    {"sizeof", TK_SIZEOF},
    {"int",    TK_TYPE},
    {"char",   TK_TYPE}
};
/*std::map<std::string, Type::Type_enum> reserved_typename = {
    {"int",    INT}
};*/

/*std::optional<std::pair<TokenKind, size_t>> check_words(std::string_view str, size_t p){
    for(const auto& rw : reserved_words){
        size_t size = rw.first.size();
        if(str.substr(p, size) == rw.first && !is_usable_char(str[p+size])){
            return std::make_optional(std::make_pair(rw.second, size));
        }
    }
    return std::nullopt;
}*/

std::optional<std::string> peek_chars(std::istream &is, int l){
    std::stringstream ss;
    auto start = is.tellg();
    for(int i=0; i < l; ++i){
        if(is.eof()){
            is.clear();
            is.seekg(start);
            return std::nullopt;
        }
        ss << (char)is.get();
    }
    is.clear();
    is.seekg(start);
    return ss.str();
}

std::optional<std::string> get_chars(std::istream &is, int l){
    std::stringstream ss;
    for(int i = 0; i < l; ++i){
        if(is.eof()){
            return std::nullopt;
        }
        ss << (char)is.get();
    }
    return ss.str();
}

std::optional<std::pair<TokenKind, size_t>> check_words(std::istream &is){
    auto start = is.tellg();
    for(const auto& rw : reserved_words){
        size_t size = rw.first.size();
        if(*get_chars(is, size) == rw.first && !is_usable_char(is.peek())){
/*        auto s = get_chars(is, size);
        auto b = is_usable_char(is.peek());
        std::cout << *s << " " << b << std::endl;
        if(*s == rw.first && !b){*/
            is.clear();
            is.seekg(start);
            return std::make_pair(rw.second, size);
        }
        is.clear();
        is.seekg(start);
    }
    is.clear();
    is.seekg(start);
    return std::nullopt;
}

auto is_space = [](const char c){
    return elem_of(c, " \n\t\r");
};

std::string take_while(std::function<bool(char)> pred, std::istream &is){
    std::stringstream ss;
    for(;;){
        if(!is.eof() && pred(is.peek())){
            ss << (char)is.get();
            continue;
        }else{
            return ss.str();
        }
    }
}

auto take_front_alnum = [](std::istream &is){
    return take_while(is_usable_char, is);
};

auto take_front_num = [](std::istream &is){
    return take_while(isdigit, is);
};

//////////////////////////////////////////
/*
TokenList tokenize(const std::string &str){
    TokenList tokens;
    size_t p = 0;
    size_t line = 0;
    //    int p_current_line = 0;

    while(1){
        if(p == str.length()){
            break;   
        }
        const char c = str[p];

        if(is_space(c)){
            p++;
            if(c == '\n'){
                line++;
                //p_current_line = p;
            }
            continue;
        }else if(c == '"'){
            tokens.push_back(Token(TK_RESERVED, str.substr(p, 1), line, p));
            size_t start = ++p;
            while(str[p] != '"'){ ++p; }
            tokens.push_back(Token(TK_LITERAL, str.substr(start, p-start), line, start));
            tokens.push_back(Token(TK_RESERVED, str.substr(p, 1), line, p));
            ++p;
            continue;
        }else if(auto res = check_words(str, p)){
            tokens.push_back(Token(res.value().first, str.substr(p, res.value().second), line, p));
            p += res.value().second;
            continue;
        }else if(elem_of(str.substr(p, 2), reserved_2char)){
            tokens.push_back(Token(TK_RESERVED, str.substr(p, 2), line, p));
            p += 2;
            continue;
        }else if(elem_of(str[p], reserved_1char)){
            tokens.push_back(Token(TK_RESERVED, str.substr(p, 1), line, p));
            p++;
            continue;
        }else if('0' <= c && c <= '9'){
            std::string s_num = take_front_num_(str, p);
            int len = s_num.length();
            int num = std::stoi(s_num);
            tokens.push_back(Token(TK_NUM, num, std::move(s_num), line, p));
            p += len;
            continue;
        }else if(('a' <= c && c <= 'z') || c == '_'){
            std::string slabel = take_front_alnum_(str, p);
            size_t len = slabel.length();
            tokens.push_back(Token(TK_IDENT, str.substr(p, len), line, p));
            p += len;
            continue;
        }
        throw exception_at(p, "unable to tokenize");
    }
    tokens.push_back(Token(TK_EOF, line, p));
    return std::move(tokens);
}
*/
TokenList tokenize_(Textfile &is){
    TokenList tokens;
    Position pos {0, 0};
    Position last {0, 0};

    while(1){
        last = pos;
        if(is.eof()){
            break;   
        }
        const char c = is.peek();
//        std::cout << c << std::endl;
        pos = is.tell_line();
        if(pos.second == -1){
            pos.first  = last.first;
            pos.second = last.second + 1; 
            break;
        }

        if(is_space(c)){
            is.get();
            continue;
        }else if(c == '/'){
            is.get();
            if(is.peek() == '/'){
                take_while([](char c){return c != '\n';}, is);
                continue;
            }else if(is.peek() == '*'){
                for(;;){
                    take_while([](char c){return c != '*' ;}, is);
                    if(*get_chars(is, 2) == "*/") break;
                    is.unget();
                }
                continue;
            }else{
                is.unget();
            }
        }
        if(c == '"'){
            tokens.push_back(Token(TK_RESERVED, *get_chars(is, 1), pos));

            pos = is.tell_line();
            tokens.push_back(Token(TK_LITERAL, take_while([](char c){return c != '"';}, is), pos));
            pos = is.tell_line();
            tokens.push_back(Token(TK_RESERVED, *get_chars(is, 1), pos));
            continue;
        }else if(auto res = check_words(is)){
            tokens.push_back(Token(res.value().first, *get_chars(is, res.value().second), pos));
            continue;
        }else if(elem_of(*peek_chars(is, 2), reserved_2char)){
            tokens.push_back(Token(TK_RESERVED, *get_chars(is, 2), pos));
            continue;
        }else if(elem_of(c, reserved_1char)){
            tokens.push_back(Token(TK_RESERVED, *get_chars(is, 1), pos));
            continue;
        }else if('0' <= c && c <= '9'){
            std::string s_num = take_front_num(is);
            int num = std::stoi(s_num);
            tokens.push_back(Token(TK_NUM, num, std::move(s_num), pos));
            continue;
        }else if(('a' <= c && c <= 'z') || c == '_'){
            std::string slabel = take_front_alnum(is);
            tokens.push_back(Token(TK_IDENT, std::move(slabel), pos));
            continue;
        }
        throw exception_at(pos.second, "unable to tokenize");
    }
//    pos = is.tell_line();
    tokens.push_back(Token(TK_EOF, pos));
    return std::move(tokens);
}

//////////////////////////////////////////////////////////////////
//parse

Node::~Node(){
    if(!lhs){
        delete lhs;
    }
    if(!rhs){
        delete rhs;
    }
    for(auto& n : nodeList){
        delete n;
    }
}

#ifdef DEBUG
void Node::print(std::ostream& os){
    static int level = 0;
    auto print_spaces = [&](){
        for(int i = 0; i < level; i++){
            os << "    ";
        }
    };

//    if(this == nullptr) return;
    if(kind != ND_FORR && kind != ND_FORL && kind != ND_THEN && kind != ND_DECLVAR){
        print_spaces();
    }
    switch(kind){
        case ND_ROOT:
            os << "(program)" << std::endl;
            break;
        case ND_NUM:
            os << "(num: " << val << ")" << std::endl;
            break;
        case ND_ADD:
            os << "(+)" << std::endl;
            break;
        case ND_SUB:
            os << "(-)" << std::endl;
            break;
        case ND_MUL:
            os << "(*)" << std::endl;
            break;
        case ND_DIV:
            os << "(/)" << std::endl;
            break;
        case ND_EQ:
            os << "(==)" << std::endl;
            break;
        case ND_NE:
            os << "(!=)" << std::endl;
            break;
        case ND_LT:
            os << "(<)" << std::endl;
            break;
        case ND_LE:
            os << "(<=)" << std::endl;
            break;
        case ND_ADDR:
            os << "(address of)" << std::endl;
            break;
        case ND_DEREF:
            os << "(dereference to)" << std::endl;
            break;
        case ND_ASSIGN:
            os << "(assign)" << std::endl;
            break;
        case ND_LVAR:
            os << "(variable named '" << label << "', its offset is " << vinfo.offset << ")" << std::endl;
            break;
        case ND_RETURN:
            os << "(return)" << std::endl;
            break;
        case ND_IF:
            os << "(if)" << std::endl;
            break;
        case ND_FOR:
            os << "(for)" << std::endl;
            break;
        case ND_WHILE:
            os << "(while)" << std::endl;
            break;
        case ND_BLOCK:
            os << "{" << std::endl;
            break;
        case ND_FUNC:
            os << "(call function named '" << label << "' with below arguments)" << std::endl;
            break;
        case ND_DEFFUNC:
            os << "(define function named '" << label << "', offsets of arguments is [ ";
            for(const auto& arg : argList) os << arg.offset << " ";
            os << "])" << std::endl;
            break;
        default:
            os << "(node: " << kind << ")" << std::endl;
            break;
    }
    level++;
    if(kind == ND_IF){
        print_spaces();
        os << "condition:" << std::endl;
        if(lhs) lhs->print(os);
        print_spaces();
        os << "then:" << std::endl;
        if(rhs->lhs) rhs->lhs->print(os);
        print_spaces();
        os << "else:" << std::endl;
        if(rhs->rhs) rhs->rhs->print(os);
    }else if(kind == ND_FOR){
        print_spaces();
        os << "initialization:" << std::endl;
        if(lhs->lhs) lhs->lhs->print(os);
        print_spaces();
        os << "condition:" << std::endl;
        if(lhs->rhs) lhs->rhs->print(os);
        print_spaces();
        os << "end process:" << std::endl;
        if(rhs->lhs) rhs->lhs->print(os);
        print_spaces();
        os << "loop statement:" << std::endl;
        if(rhs->rhs) rhs->rhs->print(os);
    }else if(kind == ND_WHILE){
        print_spaces();
        os << "condition:" << std::endl;
        if(lhs) lhs->print(os);
        print_spaces();
        os << "loop statement:" << std::endl;
        if(rhs) rhs->print(os);
    }else{
        if(lhs) lhs->print(os);
        if(rhs) rhs->print(os);
    }
    for(auto pNode : nodeList) if(pNode) pNode->print(os);
    level--;
    if(kind == ND_BLOCK){
        print_spaces();
        os << "}" << std::endl;
    }
}
#endif

