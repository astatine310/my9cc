#ifndef __PARSER_H_20200427__
#define __PARSER_H_20200427__

#include<vector>

#include"my9cc.h"

///////////////////////////////////////////////////
// parser

using VarList = std::map<std::string, VarInfo>;
typedef std::list<std::string> FuncList;

class Parser{
    std::list<Token>::iterator &m_itr;
    std::list<std::pair<VarList, Offset>> m_varData;
    Offset m_max_offset = 0;
    std::map<std::string, VarInfo> global_var;
    std::map<std::string, int> m_strliterals;
    std::map<std::string, FuncInfo> m_funcData;

public:
    Parser(std::list<Token>::iterator &itr) : m_itr(itr){}

    Node* program();
    Node* global();
    Node* stmt();
    Node* initializer();
    Node* expr();
    Node* assign();
    Node* equality();
    Node* relational();
    Node* add();
    Node* mul();
    Node* unary();
    Node* postfix();
    Node* primary();

    Node* block();
    std::optional<VarInfo> declaration();
    Node* addpointer(Node* lhs, Node* rhs, NodeKind addkind = ND_ADD);

    void expect(const std::string &s);
    int expect_number();
    const std::string& expect_ident();
    bool reserved(const std::string &s);
    std::optional<std::reference_wrapper<const std::string>> ident();
    bool kind(const TokenKind k);
    std::optional<std::reference_wrapper<const std::string>> type_();
    std::optional<Type*> type();

    std::optional<VarInfo> addval(const std::string &s, Type *ty);
    std::optional<VarInfo> search(const std::string &s);
    void new_scope();
    void delete_scope();
    std::optional<VarInfo> addvar_global(const std::string &s, Type *type);
    std::pair<std::string, int> add_literal();
    void add_func(const std::string& s);
    std::optional<FuncInfo> search_func(const std::string& s);

    void check_initializer_global(Node* node, const Type *type);

    inline void error_at(const std::string &s){
        throw exception_at(Position {m_itr->line, m_itr->pos}, s);
    }
};

//////////////////////////////////////////////////////////////
/*文法構造
program     = global*
global      = type ident ( "(" (type ident)?, ("," type ident)* ")" "{" stmt* "}"
                         | ("[" num? "]")? ("=" initializer) ";" )
stmt        = expr ";" 
            | "{" stmt* "}"
            | type ident ";"
            | type ident "[" num "]"
            | "return" expr ";"
            | "if" "(" expr ")" stmt ("else" stmt)?
            | "while" "(" expr ")" stmt
            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
initializer = "{" expr ("," expr)* "}"
            | expr 
expr        = assign
assing      = equality ("=" assign)?
equality    = relational ("==" relational | "!=" relational)*
relational  = add ("<" add | "<=" add | ">" add | ">=" add)*
add         = mul ("+" mul | "-" mul)*
mul         = unary ("*" unary | "/" unary)*
unary       = "sizeof" unary
            | ("+"|"-")? postfix
            | "*" unary
            | "&" unary
postfix     = primary ("[" expr "]")?
primary     = num 
            | literal
            | ident ("(" expr ("," expr)* ")")? 
            | "(" expr ")"
*/

#endif

