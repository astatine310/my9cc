#include<numeric>

#include"my9cc.h"

template <unsigned int N>
class Mod{
public:
    int n;

    int calc_mod(int x){
        int tmp = x % N;
        if(tmp < 0) tmp += N;
        return tmp;
    }
    Mod(int x){
        n = calc_mod(x);
    }
    Mod<N> operator+(int x){
        return Mod<N>{n + x};
    }
    Mod<N> operator-(int x){
        return Mod<N>{n - x};
    }
    Mod<N> operator+=(int x){
        n = calc_mod(n + x);
//        std::cout << "    #n = " << n << std::endl;
        return *this;
    }
    Mod<N> operator-=(int x){
        n = calc_mod(n - x);
//        std::cout << "    #n = " << n << std::endl;
        return *this;
    }
    bool operator==(int x){
        return n == calc_mod(x);
    }
};
template <unsigned int N>
std::ostream& operator<<(std::ostream& os, Mod<N> m){
    return os << m.n;
}


//const std::array arg_registers_8B = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
//const std::array arg_registers_4B = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
std::map<const int, const std::array<std::string, 6>> arg_registers = {
    {8, {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}},
    {4, {"edi", "esi", "edx", "ecx", "r8d", "r9d"}},
    {1, {"dil", "sil", "dl",  "dl",  "r8l", "r9l"}}  
};
const std::array no_code_kind = {ND_DECLVAR};

bool no_code(Node* node){
    return std::accumulate(no_code_kind.begin(), no_code_kind.end(), false, [&](bool acc, NodeKind k){
        return acc || node->kind == k;
    });
}

void gen_lval(std::ostream &r, Node* node){
    if(node->kind == ND_LVAR){
        if(node->vinfo.offset != 0){
            r << "  mov rax, rbp\n"
              << "  sub rax, " << node->vinfo.offset << "\n"
              << "  push rax\n";
        }else{
            r << "  lea rax, " << node->label << "[rip]\n"
              << "  push rax\n";
        }
    }else if(node->kind == ND_DEREF){
        gen(r, node->lhs);
    }else{
        throw exception_at(0, "rvalue used on left side of assignment");
    }
}

void load_lval_from_pushedaddr(std::ostream &r, const Type* type){
    if(type->ty != ARRAY){
        if(sizeof_type(type) == 8){
            r << "  pop rax\n"
              << "  mov rax, [rax]\n"
              << "  push rax\n";
        }else if(sizeof_type(type) == 4){
            r << "  pop rdx\n"
              << "  movsx rax, DWORD PTR [rdx]\n"
              << "  push rax\n";
        }else if(sizeof_type(type) == 1){
            r << "  pop rdx\n"
              << "  movsx rax, BYTE PTR [rdx]\n"
              << "  push rax\n";
        }
        //movsxは、ロード後に符号拡張を行う
        //movzxは、ロード後に０で拡張する。unsignedの時に使う。
    }
}

void global_init(std::ostream &r, const Node *node, const size_t size){
    if(node->kind == ND_NUM){
        if(size == 8)      r << "  .quad " << node->val << "\n";
        else if(size == 4) r << "  .long " << node->val << "\n";
        else if(size == 1) r << "  .byte " << node->val << "\n";
        else throw exception_base("error at global_init");
    }else if(node->kind == ND_ADDR){
        r << "  .quad " << node->lhs->label << "\n";
    }else if(node->kind == ND_ADD){
        r << "  .quad ";
        if(node->lhs->kind == ND_LVAR){
            r << node->lhs->label << " + " << node->rhs->val << "\n";
        }else{
            r << node->rhs->label << " + " << node->lhs->val << "\n";
        }
    }else if(node->kind == ND_ARRINIT){
        //size_t s = sizeof_type(node->vinfo.type));
        /*std::string str;
        if(s == 8) str = ".quad";
        else if(s == 4) str = ".long";
        else if(s == 1) str = ".byte";*/
        for(const auto& n : node->nodeList){
            global_init(r, n, size);
        }
    }/*else if(node->lhs->kind == ND_LITERAL){
        for(const char c : node->lhs->label){
            r << "  .byte " << (int)c << "\n";
        }
        r << "  .byte 0\n";
    }*/
}

void gen(std::ostream &r, Node *node){
    static int label_num = 0;
    const int ln = label_num;
    static Mod<16> pushed = 0;

    if(!node){
        r << "  push 0\n";
        pushed += 8;
        return;
    }

    switch(node->kind){
        case ND_ROOT:
            for(auto&& n : node->nodeList) gen(r, n);
            return;
        case ND_RETURN:
            gen(r, node->lhs);
            r << "  pop rax\n"
              << "  mov rsp, rbp\n"
              << "  pop rbp\n"
              << "  ret" << std::endl;
            r << "  push rax\n";
            return;
        case ND_IF:
            gen(r, node->lhs);
            r << "  pop rax\n"
              << "  cmp rax, 0\n";
            pushed -= 8;
            if(node->rhs->rhs == nullptr){
/*
        if(A)B
            A
            pop rax
            cmp rax, 0
            je .Lend
            B
        .Lend

    Bに何も入っていないコード
    例えば、if(1);
    のような書き方だと、スタックがずれるので、調整が必要
    但し、現在は、上のようなコードはコンパイルできない。
*/
                label_num++;
                r << "  push rax\n"
                  << "  je  .Lend" << ln << "\n"
                  << "  pop rax\n";
                gen(r, node->rhs->lhs);
                r << ".Lend" << ln << ":\n";
            }else{
/*      if(A)B else C
            A
            pop rax
            cmp rax, 0
            je .Lelse
            B
            jmp .Lend
        .Lelse
            C
        .Lend
*/
                label_num++;
                r << "  je  .Lelse" << ln << std::endl;
                gen(r, node->rhs->lhs);
                r << "  jmp .Lend"  << ln << std::endl
                  << ".Lelse" << ln << ":\n";
                gen(r, node->rhs->rhs);
                r << ".Lend" << ln << ":\n";
            }
            return;
        case ND_WHILE:
/*  while(A)B
        .Lbegin
            A
            pop rax
            cmp rax, 0
            je .Lend
            B
            jmp .Lbegin
        .Lend
*/
            label_num++;
            r << ".Lbegin" << ln << ":\n";
            gen(r, node->lhs);
            r << "  pop rax\n"
              << "  cmp rax, 0\n"
              << "  je  .Lend" << ln << "\n";
            pushed -= 8;
            gen(r, node->rhs);
            r << "  jmp .Lbegin" << ln << "\n"
              << ".Lend" << ln << ":\n";
            return;
        case ND_FOR:
/*  for(A;B;C)D
            A
            pop rax
        .Lbegin
            B   (if empty, push 1)
            pop rax
            cmp rax, 0
            je .Lend
            D
            pop rax
            C
            jmp .Lbegin
        .Lend
*/
            label_num++;
            gen(r, node->lhs->lhs);
            r << "  pop rax\n";
            r << ".Lbegin" << ln << ":\n";
            pushed -= 8;
            if(node->lhs->rhs == nullptr) r << "  push 1\n";
            else {gen(r, node->lhs->rhs); pushed -= 8;}
            r << "  pop rax\n"
              << "  cmp rax, 0\n"
              << "  je  .Lend" << ln << "\n";
            gen(r, node->rhs->rhs);
            r << "  pop rax\n";
            pushed -= 8;
            gen(r, node->rhs->lhs);
            r << "  jmp .Lbegin" << ln << "\n"
              << ".Lend" << ln << ":\n";
            return;
        case ND_BLOCK:
/*
    {A,B,...}
        A
        pop rax
        B

    {}
        push 0
*/
            if(node->nodeList.size() != 0){
                auto itr = node->nodeList.begin();
                auto iEnd = node->nodeList.end();
                iEnd--;
                while(itr != iEnd){
                    if(!*itr){
                        itr++;
                        continue;
                    }
                    gen(r, *(itr++));
                    r << "  pop rax\n";
                    pushed -= 8;
                }
                gen(r, *itr);
//                pushed -= 8;
            }else{
                r << "  push 0\n";
                pushed += 8;
            }
            return;
        case ND_DEFFUNC:
            {
                Mod<16> tmp = pushed;
                Offset modified = node->offset;
                if(modified % 16 < 16) modified += 16 - (modified % 16);
                r << "  .text\n"
                  << node->label << ":\n"
                  << "  push rbp\n"
                  << "  mov rbp, rsp\n"
                  << "  sub rsp, " << modified << "\n";
                
                size_t idx = 0;
                for(const auto& arg : node->argList){
                    r << "  mov rax, rbp\n"
                      << "  sub rax, " << arg.offset << "\n";
                    size_t size = sizeof_type(arg.type);
                    r << "  mov [rax], " << arg_registers[size][idx] << "\n";
/*                    if(size == 8){
                        r << "  mov [rax], " << arg_registers_8B[idx] << "\n";
                    }else if(size == 4){
                        r << "  mov [rax], " << arg_registers_4B[idx] << "\n";
                    }else{
                        throw exception_base("unexpected error at ND_DEFFUNC in gen");
                    }*/
                    idx++;
                }
                pushed = 0;
                gen(r, node->lhs);
                r << "  pop rax\n"
                  << "  mov rsp, rbp\n"
                  << "  pop rbp\n"
                  << "  ret\n";
                pushed = tmp;
            }
            return;
        case ND_FUNC:
            {
                auto iStr = arg_registers[8].cbegin();
                for(const auto& arg_expr: node->nodeList){
                    gen(r, arg_expr);
                    r << "  pop " << *iStr << "\n";
                    pushed -= 8;
                    iStr++;
                }
                Mod<16> tmp = pushed;
                if(tmp == 8) r << "  push 0\n";
 //↓可変数引数を取る関数を呼ぶときは、alに浮動小数点の引数の個数を入れておく→とりあえずすべて０にしておく
                r << "  xor rax, rax\n"; 
                if(node->plt){
                    r << "  call " << node->label << "@PLT\n";
                }else{
                    r << "  call " << node->label << "\n";
                }
                if(tmp == 8) r << "  pop rdx\n";
                r << "  push rax\n";
                pushed += 8;
            }
//        r << "  call " << node->label << "@PLT\n"

            return;
        case ND_NUM:
            r << "  push " << node->val << "\n";
            pushed += 8;
            return;
        case ND_LVAR:
            gen_lval(r, node);
            pushed += 8;
            load_lval_from_pushedaddr(r, node->vinfo.type);
            return;
        case ND_ASSIGN:
            gen_lval(r, node->lhs);
            if(node->lhs->kind == ND_LVAR) pushed += 8; // gen_lvarでは加算できない
            gen(r, node->rhs);
            r << "  pop rdi\n"
              << "  pop rax\n";
            if(sizeof_type(*typeof_node(node->lhs)) == 8){
                r << "  mov [rax], rdi\n"
                  << "  push rdi\n";
            }else if(sizeof_type(*typeof_node(node->lhs)) == 4){
                r << "  mov [rax], edi\n"
                  << "  push rdi\n";
            }else if(sizeof_type(*typeof_node(node->lhs)) == 1){
                r << "  mov [rax], dil\n"
                  << "  push rdi\n";
            }
            pushed -= 8;
            return;
        case ND_ADDR:
            gen_lval(r, node->lhs);
            pushed += 8;
            return;
        case ND_DEREF:
            gen(r, node->lhs);
            load_lval_from_pushedaddr(r, typeof_node(node->lhs).value()->ptr_to);
            return;
        case ND_DECLGLOBL:
            //r << "  .globl  " << node->label << "\n"
            if(node->lhs == nullptr){
                r << "  .bss\n"
                  << node->label << ":\n"
                  << "  .zero " << sizeof_type(node->vinfo.type) << "\n";
            }else{
                r << "  .data\n"
                  << node->label << ":\n";
                size_t size;
                if(node->vinfo.type->ty == ARRAY && node->lhs->kind == ND_ARRINIT){
                    size = sizeof_type(node->vinfo.type->ptr_to);
                }else{
                    size = sizeof_type(node->vinfo.type);
                }
                global_init(r, node->lhs, size);
            }
            return;
        case ND_DEFLITERAL:
            r << "  .section    .rodata\n"
              << ".LC" << node->val << ":\n"
              << "  .string \"" << node->label << "\"\n";
            return;
        case ND_LITERAL:
            r << "  lea rax, .LC" << node->val << "[rip]\n"
              << "  push rax\n";
            pushed += 8;
            return;
        default:
            if(node->kind == ND_DECLVAR) return;
            break;
    }
    gen(r, node->lhs);
    gen(r, node->rhs);

    r << "  pop rdi\n"
      << "  pop rax\n";

    switch(node->kind){
        case ND_ADD:
            r << "  add rax, rdi\n";
            break;
        case ND_SUB:
            r << "  sub rax, rdi\n";
            break;
        case ND_MUL:
            r << "  imul rax, rdi\n";
            break;
        case ND_DIV:
            r << "  cqo\n";
            r << "  idiv rdi\n";
            break;
        case ND_EQ:
            r << "  cmp rax, rdi\n";
            r << "  sete al\n";
            r << "  movzb rax, al\n";
            break;
        case ND_NE:
            r << "  cmp rax, rdi\n";
            r << "  setne al\n";
            r << "  movzb rax, al\n";
            break;
        case ND_LT:
            r << "  cmp rax, rdi\n";
            r << "  setl al\n";
            r << "  movzb rax, al\n";
            break;
        case ND_LE:
            r << "  cmp rax, rdi\n";
            r << "  setle al\n";
            r << "  movzb rax, al\n";
            break;
        default:
            throw exception_at(0, "unexpected node");
    }
    r << "  push rax\n";
    pushed += 8;
}
