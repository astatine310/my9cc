#ifndef __MY9CC_H_20200404__
#define __MY9CC_H_20200404__

#include<optional>
#include<sstream>
#include<string>
#include<list>
#include<iostream>
#include<map>
#include<functional>
#include<string_view>
#include<utility>
#include<cassert>

#include"mylib.h"

#define DEBUG
#define TEST

#define ENUM_STR(var) #var

using Position = std::pair<size_t, int>;

///////////////////////////////////////////////////////////
// handling error
#ifdef  DEBUG
#define PRINT(mes) std::cerr << "test: " << mes << std::endl
#endif

class exception_base{
protected:
    std::string mes;
public:
    exception_base(std::string str) : mes(str){}
    virtual ~exception_base(){}
    virtual std::string what() const noexcept;
};

class exception_at : public exception_base{
public:
    Position pos;
//    int pos;
public:
    exception_at(const Position &p, const std::string &str) : exception_base(str), pos(p){}
    exception_at(const int p, const std::string &str) : exception_base(str), pos(std::make_pair(0, p)){}
    std::string what() const noexcept override;
};

////////////////////////////////////////////////////////
// tokenize

enum TokenKind{
    TK_RESERVED,  
    TK_IDENT,
    TK_NUM,
    TK_TYPE,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_FOR,
    TK_WHILE,
    TK_SIZEOF,
    TK_LITERAL,
    TK_EOF,
};

class Token{
public:
    const TokenKind kind;
    const int val;
    const std::string str;
    const size_t line;
    const size_t pos; //position where this token was read

    Token(TokenKind k, int v, std::string s, Position p) : kind(k), val(v), str(s), line(p.first), pos(p.second){}
    Token(TokenKind k, std::string s, Position p) : kind(k), val(0), str(s), line(p.first), pos(p.second){}
    Token(TokenKind k, Position p) : kind(k), val(0), str(""), line(p.first), pos(p.second){}

    Token(TokenKind k, int v, std::string s, size_t l, size_t p) : kind(k), val(v), str(s), line(l), pos(p){}
    Token(TokenKind k, std::string s, size_t l, size_t p) : Token(k, 0, s, l, p){}
    Token(TokenKind k, size_t l, size_t p) : Token(k, 0, "", l, p){}
    bool expect(const std::string &s) const;
    int expect_number() const;
    std::string expect_ident() const;
};

typedef std::list<Token> TokenList;

TokenList tokenize(const std::string &str);
TokenList tokenize_(Textfile &is);

///////////////////////////////////////////////////////
//type

enum TypeEnum{
    INT, CHAR, PTR, ARRAY
};

struct Type{
    const TypeEnum ty;
    const Type *ptr_to;
    const size_t array_size;
    bool operator==(const Type& t) const{
        if(ty == PTR){
            return t.ty == PTR && *ptr_to == *(t.ptr_to);
        }else if(ty == ARRAY){
            return t.ty == ARRAY && array_size == t.array_size && *ptr_to == *(t.ptr_to);
        }else{
            return t.ty == ty;
        }
    }
};
// operator == が必要
// 可変長引数テンプレートで実装できないか？<PTR, PTR, INT>みたいなイメージで

const std::map<std::string, TypeEnum> map_type = {
    {"int", INT},
    {"char", CHAR}
};

//std::optional<Type*> typeof_node(Node* node);
size_t sizeof_type(const Type* t);

typedef unsigned int Offset;

struct VarInfo{
    const Offset offset;
    const Type *type;
/*    ~VarInfo(){
        delete type;
    }*/
};

struct FuncInfo{

};

////////////////////////////////////////////////
// parse

enum NodeKind{
    ND_ROOT,
    ND_NUM,
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ADDR,    //operator &
    ND_DEREF,   //operator *
    ND_ASSIGN,
    ND_LVAR,
    ND_LITERAL,
    ND_DEFLITERAL,
    ND_RETURN,
    ND_IF,      //left is condition, right is ND_THEN
    ND_THEN,    //left is stmt when true, right is when false
    ND_WHILE,
    ND_FOR, 
    ND_FORL,    //left is initializing, right is condition
    ND_FORR,    //left is end process, right is loop stmt
    ND_BLOCK,   //nlist  //old -> left is stmt, right is next code in block
    ND_FUNC,    //call function  nlist is argument list
    ND_DEFFUNC, //definition of function. lhs is block
    ND_DECLVAR,
    ND_DECLGLOBL,
    ND_ARRINIT,
};

class Node{
public:
    const NodeKind kind;
    Node *lhs = nullptr;
    Node *rhs = nullptr;
    const int val = 0;  //ND_NUM 
    const std::string label = "";  //ND_LVAR name of lvar
    const VarInfo vinfo = {0, nullptr};
//    const size_t size = 0;
    const Offset offset = 0;         //ND_LVAR offset of address of variables
                                //ND_DEFFUNC total of offsets of local variables
    std::list<Node*> nodeList;
    std::list<VarInfo> argList;
    bool plt;

    std::string_view strv;

    Node(NodeKind k, Node *l, Node *r) : kind(k), lhs(l), rhs(r){}
    Node(int v) : kind(ND_NUM), lhs(nullptr), rhs(nullptr), val(v){}      //for ND_NUM
    Node(const NodeKind k, const int num) : kind(k), val(num){} 
    Node(NodeKind k, const std::string& str, Offset n) : kind(k), label(str), offset(n){}
    Node(NodeKind k, const std::string& str, const VarInfo& vi) : kind(k), label(str), vinfo(vi){}   //for ND_LVAR
    Node(NodeKind k, Node *l, const std::string& str, const VarInfo& vi) : kind(k), lhs(l), label(str), vinfo(vi){}   //for ND_LVAR with initializer
    Node(NodeKind k, const std::string& str, const std::list<Node*>& nl, bool b) : kind(k), label(str), nodeList(nl), plt(b){}
    Node(NodeKind k, Node *l, const std::string& str, Offset n, std::list<VarInfo> al) : kind(k), lhs(l), label(str), offset(n), argList(al){}
    Node(NodeKind k, const std::list<Node*>& nl) : kind(k), nodeList(nl){}
    Node(const NodeKind k, const int v, const std::string& s) : kind(k), val(v), label(s){}
    Node(NodeKind k) : kind(k){}
    ~Node();
#ifdef DEBUG
    void print(std::ostream& os);
#endif
private:
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;
};

std::optional<const Type*> typeof_node(Node* node);

///////////////////////////////////////////////////
//code generator

void gen(std::ostream &r, Node *node);


///////////////////////////////////////////////////
#ifdef TEST

void test_parser();

#endif


#endif


