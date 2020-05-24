#include"my9cc.h"
#include"parser.h"

Type int_type = {INT, nullptr, 0};
Type char_type = {CHAR, nullptr, 0};
Type ptr_type = {PTR, &int_type, 0};

std::optional<bool> comp_type_(const Type* t1, const Type* t2){
    if(t1->ty == ARRAY){
        if(t2->ty == PTR && *(t1->ptr_to) == *(t2->ptr_to)){
            return true;
        }
    }else if(t1->ty == PTR){
    }else if(t1->ty == CHAR){
        if(t2->ty == INT){
            return true;
        }
    }
    return std::nullopt;
}
std::optional<bool> comp_type(const Type* t1, const Type* t2){ //less
    if(*t1 == *t2){
        return false;
    }

    auto ret = comp_type_(t1, t2);
    if(!ret){
        ret  = comp_type_(t2, t1);
        if(ret) *ret = !(*ret);
    }
    return std::move(ret);
}

#ifdef TEST
void test_parser(){
//namespace test{
    const Type int_type = {INT, nullptr, 0};
    const Type char_type = {CHAR, nullptr, 0};
    const Type ptr_type = {PTR, &int_type, 0};
    const Type arr4 = {ARRAY, &int_type, 4};
    const Type arr5 = {ARRAY, &int_type, 5};
    
    assert(*comp_type(&int_type, &char_type) == false);
    assert(!comp_type(&int_type, &ptr_type));
    assert(!comp_type(&arr4, &arr5));
    assert(*comp_type(&arr4, &ptr_type));
//    assert(*comp_type(&int_type, &ptr_type));
}
#endif

size_t sizeof_type(const Type* t){
    if(t->ty == INT) return 4;
    else if(t->ty == CHAR) return 1;
    else if(t->ty == ARRAY) return t->array_size * sizeof_type(t->ptr_to);
    else return 8;
}

std::optional<const Type*> typeof_node(Node* node){
    if(node->kind == ND_NUM || node->kind == ND_FUNC){
        return &int_type;
    }else if(ND_ADD <= node->kind && node->kind <= ND_DIV){
        auto l = typeof_node(node->lhs);
        auto r = typeof_node(node->rhs);
        if(!l || !r) return std::nullopt;
        if(ND_ADD <= node->kind && node->kind <= ND_SUB){
            if(((*l)->ty == INT   && (*r)->ty == PTR  )||
               ((*l)->ty == INT   && (*r)->ty == ARRAY)) return r;
            if(((*l)->ty == PTR   && (*r)->ty == INT  )||
               ((*l)->ty == ARRAY && (*r)->ty == INT  )) return l;            
        }
        if(auto ret = comp_type(*l, *r)){
            if(*ret) return r;
            else     return l;
        }else{
            return std::nullopt;
        }
//        if((*l)->ty == INT && (*r)->ty == INT) return &int_type;
//        else return &ptr_type;
    }else if(ND_EQ <= node->kind && node->kind <= ND_LE){
        return &int_type;
    }else if(node->kind == ND_FUNC){
        return &int_type;
    }else if(node->kind == ND_LVAR){
        return node->vinfo.type;
    }else if(node->kind == ND_ADDR){
        if(auto o_type = typeof_node(node->lhs)){
            return new Type{PTR, o_type.value()}; //解放されていない
        }
    }else if(node->kind == ND_DEREF){
        if(auto o_type = typeof_node(node->lhs)){
            return o_type.value()->ptr_to;
        }
    }//else if(node->kind == ND_ASSIGN){}
    return std::nullopt;
}

void Parser::expect(const std::string &s){
    if(m_itr->kind == TK_RESERVED && m_itr-> str == s){
        m_itr++;
    }else{
        std::stringstream ss;
        ss << "'" << s << "' is expected, but got this.";
        error_at(ss.str());
    }
}
int Parser::expect_number(){
    if(m_itr->kind == TK_NUM){
        return (m_itr++)->val;
    }else{
        error_at("It might need to be a number.");
        return 0;
    }
}
const std::string& Parser::expect_ident(){
    if(m_itr->kind == TK_IDENT){
        return (m_itr++)->str;
    }else{
        error_at("it shall be a identifier.");
        throw "";
    }
}
bool Parser::reserved(const std::string &s){
    if(m_itr->kind == TK_RESERVED && m_itr->str == s){
        m_itr++;
        return true;
    }else{
        return false;
    }
}
std::optional<std::reference_wrapper<const std::string>> Parser::ident(){
    if(m_itr->kind == TK_IDENT){
        return std::make_optional(std::reference_wrapper{(m_itr++)->str});
    }else{
        return std::nullopt;
    }
}
bool Parser::kind(const TokenKind k){
    if(m_itr->kind == k){
        m_itr++;
        return true;
    }else{
        return false;
    }
}

[[deprecated]] std::optional<std::reference_wrapper<const std::string>> Parser::type_(){
    if(m_itr->kind == TK_TYPE){
        return std::make_optional(std::reference_wrapper{(m_itr++)->str});
    }else{
        return std::nullopt;
    }
}

//////////////////////////////////////////////////////////

std::optional<VarInfo> Parser::addval(const std::string &s, Type *ty){
    auto o_res = search(s);
    if(!o_res){
        auto& [vs, offset] = m_varData.front();
        offset += sizeof_type(ty);//8; //size of variable
        if(offset > m_max_offset) m_max_offset = offset;
        VarInfo vi = {offset, ty};
        vs.insert(make_pair(s, vi));
        return vi;
    }else{
        error_at("duplicated declaration.");
        return std::nullopt;
    }
}

std::optional<VarInfo> Parser::search(const std::string &s){
    for(const auto& vs : m_varData){
        auto res = vs.first.find(s);
        if(res != vs.first.end()){
            return res->second;
        }
    }
    auto itr = global_var.find(s);
    if(itr != global_var.end()){
        return itr->second;
    }
    return std::nullopt;
}

void Parser::new_scope(){   //スコープ管理が不十分
    Offset used_offset = (m_varData.size() == 0) ? 0 : m_varData.front().second;
    VarList vl{};
    m_varData.push_front(make_pair(vl, used_offset));
}

void Parser::delete_scope(){
    m_varData.pop_front();
    if(m_varData.size() == 0) m_max_offset = 0;
}

std::optional<VarInfo> Parser::addvar_global(const std::string &s, Type *type){
    VarInfo tmp {0, type};
    auto ret = global_var.insert(std::make_pair(s, tmp));
    if(ret.second){
        return tmp;
    }else{
        error_at("multiple definition.");
        return std::nullopt;
    }
}

std::pair<std::string, int> Parser::add_literal(){
    std::pair<std::string, int> ret;
    if(m_itr->kind != TK_LITERAL){
        error_at("unexpected error at add_literal()");
    }
    auto res = m_strliterals.find(m_itr->str);
    if(res == m_strliterals.end()){
        ret = make_pair(m_itr->str, m_strliterals.size());
        m_strliterals.insert(ret);
    }else{
        ret = *res;
    }
    ++m_itr;
    return std::move(ret);
}

void Parser::add_func(const std::string& s){
    auto ret = m_funcData.insert(std::make_pair(s, FuncInfo{}));
    if(ret.second){
        return;
    }else{
        error_at("multiple definition.");
    }
}

std::optional<FuncInfo> Parser::search_func(const std::string& s){
    auto itr = m_funcData.find(s);
    if(itr == m_funcData.end()){
        return std::nullopt;
    }else{
        return itr->second;
    }
}

bool is_addr(Node* node){
    if(node->kind == ND_ADDR) return true;
    if(node->kind == ND_LVAR){
        auto o_type = typeof_node(node);
        if(o_type.value()->ty == PTR || o_type.value()->ty == ARRAY){
            return true;
        }
    }
    return false;
}

Node* eval(Node* node){
    Node *tmp;
    if(ND_ADD <= node->kind && node->kind <= ND_DIV){
        node->lhs = eval(node->lhs);
        node->rhs = eval(node->rhs);
        if(node->lhs->kind == ND_NUM && node->rhs->kind == ND_NUM){
            tmp = node;
            switch(node->kind){
                case ND_ADD: node = new Node(node->lhs->val + node->rhs->val); break;
                case ND_SUB: node = new Node(node->lhs->val - node->rhs->val); break;
                case ND_MUL: node = new Node(node->lhs->val * node->rhs->val); break;
                case ND_DIV: node = new Node(node->lhs->val / node->rhs->val); break;
                default: throw exception_base("unexpected error at eval()"); break;
            }
            delete tmp;
        }
    }
    return node;
}

void Parser::check_initializer_global(Node *node, const Type* type){
    if(type->ty == INT){
        if(node->kind == ND_NUM) return;
        error_at("initializing expression to int shall be a number.");
    }else if(type->ty == CHAR){
        if(node->kind == ND_NUM) return;
        error_at("initializing expression to char shall be a number.");
    }else if(type->ty == PTR || type->ty == ARRAY){
        if(node->kind == ND_ADDR) return;
        if(node->kind == ND_ADD){
            if((is_addr(node->lhs) && node->rhs->kind == ND_NUM) ||
               (is_addr(node->rhs) && node->lhs->kind == ND_NUM)){
                return;
            }       
        }
        if(type->ptr_to->ty == CHAR && node->kind == ND_LITERAL) return;
        if(type->ty == ARRAY && node->kind == ND_ARRINIT){
            for(const auto& n : node->nodeList){
                check_initializer_global(n, type->ptr_to);
            }
            return;
        }
        error_at("invalid initializing expression.");
    }
}

//////////////////////////////////////////////////

Node* Parser::block(){
    std::list<Node*> nl;
    while(!reserved("}")){
        Node* tmp = stmt();
        if(tmp) nl.push_back(tmp);
    }
    Node *tmp= new Node(ND_BLOCK, std::move(nl));
    return tmp;
}

std::optional<VarInfo> Parser::declaration(){
    auto o_type = type();

//    auto o_str = type_();
    if(!o_type) return std::nullopt;
/*    auto i_ty_enum = map_type.find(*o_str);
    if(i_ty_enum == map_type.end()) throw exception_base("unexpected error at Parser::declaration()");
    Type *ty = new Type {i_ty_enum->second, nullptr, 0};
    while(reserved("*")){
        ty = new Type {PTR, ty, 0};
    }        */
    const std::string& str = expect_ident();
    Type *type = *o_type;
    while(reserved("[")){
        if(!reserved("]")){
            size_t s = expect_number();
            type = new Type {ARRAY, type, s};
            expect("]");
        }
    }
    auto o_result = addval(str, type);
    return o_result;
}

std::optional<Type*> Parser::type(){
    if(m_itr->kind == TK_TYPE){
        const std::string& typestr = (m_itr++)->str;
        auto itr = map_type.find(typestr);
        Type *type = new Type {itr->second, nullptr, 0};
        while(reserved("*")){
            type = new Type {PTR, type, 0};
        }
        return type;        
    }else{
        return std::nullopt;
    }
}

/////////////////////////////////////////////////

Node* Parser::program(){
    std::list<Node*> nodes;
    while(!kind(TK_EOF)){
        nodes.push_back(global());
    }
    std::list<Node*> tmp;
    for(const auto& lit : m_strliterals){
        tmp.push_back(new Node(ND_DEFLITERAL, lit.second, lit.first));
    }
    nodes.insert(nodes.cbegin(), tmp.begin(), tmp.end());
    return new Node(ND_ROOT, std::move(nodes));
}

Node* Parser::global(){
    std::list<Node*> nodes;
    std::list<VarInfo> argList;
    std::optional<Type *> o_type;

    if(!(o_type = type())){
        error_at("definition shall start with typename.");
    }

    const std::string& name = expect_ident();
    if(reserved("(")){
        new_scope();
        if(!reserved(")")){
            auto o_vinfo = declaration();
            argList.push_back(*o_vinfo);
    
            while(reserved(",")){
                if(argList.size()>5) error_at("Functions can't receive more than 6 arguments.");
                auto o_vinfo = declaration();
                argList.push_back(*o_vinfo);
            }
            expect(")");
        }
        expect("{");
        Node *pn = block();
        Offset tmp = m_max_offset;
        delete_scope();
        add_func(name);
        return new Node(ND_DEFFUNC, pn, name, tmp, std::move(argList));
    }
    std::optional<size_t> s = std::nullopt;
    Type *type = *o_type;
    size_t tmp_size = 0;
    bool first_array = true;
    while(reserved("[")){
        if(first_array){
            if(!reserved("]")){
                s = expect_number();
                expect("]");
            }
            first_array = false;
        }else{
            s = expect_number();
            expect("]");
        }
        if(s) tmp_size = *s;
        type = new Type {ARRAY, type, tmp_size};
    }
    if(reserved("=")){
        Node *node = initializer();
        eval(node);
        check_initializer_global(node, type);
        expect(";");
        if(node->kind == ND_LITERAL && type->ty == ARRAY){
            std::list<Node*> tmp;
            for(char c : node->label){
                tmp.push_back(new Node(static_cast<int>(c)));
            }
            tmp.push_back(new Node(0));
            delete node;
            node = new Node(ND_ARRINIT, std::move(tmp));
        }
        if(node->kind == ND_ARRINIT){
            size_t arr_size = node->nodeList.size();
            if(tmp_size != 0){
                if(tmp_size < arr_size){
                    // 配列初期化子の長さの方が長い場合、単にその部分は無視される
                    for(int i = arr_size - tmp_size; i > 0; --i){
                        node->nodeList.pop_back();
                    }
                }else if(tmp_size > arr_size){
                    //明示された配列の長さの方が名がいい場合、０で埋められる
                    for(int i = tmp_size - arr_size; i > 0; --i){
                        node->nodeList.push_back(new Node(0));
                    }
                }
            }
            if(tmp_size == 0){
                Type* tmp = type;
                type = new Type{ARRAY, tmp->ptr_to, arr_size};
                delete tmp;
            }
        }else{
            if(type->ty == ARRAY){
                if(tmp_size == 0){
                    error_at("array not specified its size shall be initialized.");
                }else if(!is_addr(node)){
                    error_at("array can't be initialized as this.");
                }
            }
        }
        auto o_ret = addvar_global(name, type);
        return new Node(ND_DECLGLOBL, node, name, *o_ret);
    }
    expect(";");
    auto o_ret = addvar_global(name, type);
    return new Node(ND_DECLGLOBL, name, *o_ret);
}

Node *Parser::stmt(){
    Node *node;
    if(kind(TK_RETURN)){
        node = new Node(ND_RETURN, expr(), nullptr);
        expect(";");
    }else if(kind(TK_IF)){
        expect("(");
        Node *tmp1 = expr();
        expect(")");
        Node *tmp2 = stmt();
        if(kind(TK_ELSE)){
            node = new Node(ND_IF, tmp1, new Node(ND_THEN, tmp2, stmt()));
        }else{
            node = new Node(ND_IF, tmp1, new Node(ND_THEN, tmp2, nullptr));
        }
    }else if(kind(TK_WHILE)){
        expect("(");
        Node *tmp1 = expr();
        expect(")");
        node = new Node(ND_WHILE, tmp1, stmt());
    }else if(kind(TK_FOR)){
        expect("(");
        Node *tmp1 = nullptr;
        Node *tmp2 = nullptr;
        Node *tmp3 = nullptr;
        if(!reserved(";")){
            tmp1 = expr();
            expect(";");
        }
        if(!reserved(";")){
            tmp2 = expr();
            expect(";");
        }
        if(!reserved(")")){
            tmp3 = expr();
            expect(")");
        }
        node = new Node(ND_FOR, 
                        new Node(ND_FORL, tmp1, tmp2),
                        new Node(ND_FORR, tmp3, stmt()));
    }else if(reserved("{")){
        return block();
    }else if(declaration()){
        reserved(";");
        return nullptr;
    }else{
        node = expr();
        expect(";");
    }
    return node;
}

Node* Parser::initializer(){
    Node *node;
    if(reserved("{")){
        std::list<Node*> nodelist;
        node = expr();
        nodelist.push_back(node);
        while(reserved(",")){
            node = expr();
            nodelist.push_back(node);
        }
        expect("}");
        return new Node(ND_ARRINIT, std::move(nodelist));
    }
    node = expr();
    return node;
}

Node* Parser::expr(){
    return assign();
}

Node* Parser::assign(){
    Node *node = equality();
    if(reserved("=")){
        node = new Node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node* Parser::equality(){
    Node *node = relational();
    for(;;){
        if(reserved("==")){
            node = new Node(ND_EQ, node, relational());
        }else if(reserved("!=")){
            node = new Node(ND_NE, node, relational());
        }else{
            return node;
        }
    }
}

Node* Parser::relational(){
    Node *node = add();
    for(;;){
        if(reserved("<")){
            node = new Node(ND_LT, node, add());
        }else if(reserved("<=")){
            node = new Node(ND_LE, node, add());
        }else if(reserved(">")){
            node = new Node(ND_LT, add(), node);
        }else if(reserved(">=")){
            node = new Node(ND_LE, add(), node);
        }else{
            return node;
        }
    }
}

Node* Parser::addpointer(Node* lhs, Node* rhs, NodeKind addkind){
    auto type1 = typeof_node(lhs).value();
    auto type2 = typeof_node(rhs).value();
    TypeEnum ty1 = type1->ty;
    TypeEnum ty2 = type2->ty;
    if(ty1 == ARRAY) ty1 = PTR;
    if(ty2 == ARRAY) ty2 = PTR;

    if(ty1 == PTR && ty2 == INT){
        return new Node(addkind, lhs, new Node(ND_MUL, rhs, new Node(sizeof_type(type1->ptr_to))));
    }else if(ty1 == INT && ty2 == PTR){
        return new Node(addkind, rhs, new Node(ND_MUL, lhs, new Node(sizeof_type(type2->ptr_to))));
    }else if(ty1 == PTR && ty2 == PTR){
        error_at("invalid operation");
        return nullptr;
    }else{
        return new Node(addkind, lhs, rhs);
    }
}

Node* Parser::add(){
    Node *node = mul();
    NodeKind addkind;
    for(;;){
        if(reserved("+")){
            addkind = ND_ADD;
        }else if(reserved("-")){
            addkind = ND_SUB;
        }else{
            return node;
        }
        Node *tmp = mul();
/*        auto type_node = typeof_node(node);
        auto type_tmp = typeof_node(tmp);
        auto ty1 = type_node.value()->ty;
        auto ty2 = type_tmp.value()->ty;
        if(ty1 == ARRAY) ty1 = PTR;
        if(ty2 == ARRAY) ty2 = PTR;

        if(ty1 == PTR && ty2 == INT){
            node = new Node(addkind, node, new Node(ND_MUL, tmp, new Node(sizeof_type(type_node.value()->ptr_to))));
        }else if(ty1 == INT && ty2 == PTR){
            node = new Node(addkind, tmp, new Node(ND_MUL, node, new Node(sizeof_type(type_node.value()->ptr_to))));
        }else if(ty1 == PTR && ty2 == PTR){
            error_at("invalid operation");
        }else{
            node = new Node(addkind, node, tmp);
        }*/
        node = addpointer(node, tmp, addkind);
    }
}
Node* Parser::mul(){
    Node *node = unary();
    NodeKind mulkind;
    for(;;){
        if(reserved("*")){
            mulkind = ND_MUL;
        }else if(reserved("/")){
            mulkind = ND_DIV;
        }else{
            return node;
        }
        Node *tmp = unary();
        auto ty1 = typeof_node(node).value()->ty;
        auto ty2 = typeof_node(tmp) .value()->ty;
        if(ty1 == PTR || ty1 == ARRAY || ty2 == PTR || ty2 == ARRAY){
            error_at("invalid operation");
        }
        node = new Node(mulkind, node, tmp);

    }
}
Node* Parser::unary(){
    if(reserved("+")){
        Node *tmp = primary();
        auto ty1 = typeof_node(tmp).value()->ty;
        if(ty1 != INT) error_at("invalid operation");
        return tmp;
    }else if(reserved("-")){
        Node *tmp = primary();
        auto ty1 = typeof_node(tmp).value()->ty;
        if(ty1 != INT) error_at("invalid operation");
        return new Node(ND_SUB, new Node(0), tmp);
    }else if(reserved("&")){
        return new Node(ND_ADDR, unary(), nullptr);
    }else if(reserved("*")){
        return new Node(ND_DEREF, unary(), nullptr);
    }else if(kind(TK_SIZEOF)){
        Node *node = unary();
        try{
            size_t s = sizeof_type(typeof_node(node).value());
            return new Node(s);
        }catch(std::bad_optional_access e){
            error_at("this expression has no type");
        }
    }
    return postfix();
}
Node* Parser::postfix(){
    Node *node = primary();
    if(reserved("[")){
        Node *tmp = expr();
        expect("]");
//        return new Node(ND_DEREF, new Node(ND_ADD, node, tmp), nullptr);
        return new Node (ND_DEREF, addpointer(node, tmp), nullptr);
    }
    return node;
}
Node* Parser::primary(){
    if(reserved("(")){
        Node *node = expr();
        expect(")");
        return node;
    }
    if(auto o_name = ident()){
        if(reserved("(")){
            std::list<Node*> nl;
            if(!reserved(")")){
                Node* tmp = expr();
                nl.push_back(tmp);
                while(!reserved(")")){
                    expect(",");
                    tmp = expr();
                    if(nl.size()>5){
                        error_at("Functions can't receive more than 6 arguments.");
                    }
                    nl.push_back(tmp);
                }
            }
            bool plt = true;
            if(search_func(*o_name)) plt = false;
            return new Node(ND_FUNC, *o_name, std::move(nl), plt);
        }

        auto o_varinfo = search(*o_name);
        if(!o_varinfo){
            error_at("undefined identifier.");
        }else{
            return new Node(ND_LVAR, *o_name, o_varinfo.value());
        }
    }
    if(reserved("\"")){
        auto lit_info = add_literal();
        expect("\"");
        return new Node(ND_LITERAL, lit_info.second, std::move(lit_info.first));
    }
    return new Node(expect_number());
}

