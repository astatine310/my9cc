int sup_root(int x){
    int i;
    for(i=0; i*i<= x; i=i+1){}
    return i;
}
int is_prime(int x){
    int i;
    for(i=2; i<sup_root(x); i=i+1){
        int a;
        a = x/i;
        if(a*i == x){
            return 0;
        }
    }
    return 1;
}

int add_six(int a1,int a2,int a3,int a4,int a5,int a6){return a1+a2+a3+a4+a5+a6;}

int square(int x){return x*x;}
int twice(int x){return x*2;} 

int func1(int x){x=x+3; return x;}
int func2(int *x){*x=*x+3; return *x;}

int fact(int x){if(x == 1)return 1; else return x*fact(x-1);}

int g1;
int *gp;

int g2 = 2;
int *gp2 = &g2;
int garr[3];
int *gp3 = garr + 2;

int g3[] = {1,2,3,4,5};
int g4[3] = {6,7};

char g5[] = "good bye.";
char g6[10] = "abc";

// this is comment.

int main(){
    if(12 + 34 - 5 != 41) printf("error.");
    if(5+6*7 != 47) printf("error.");
    if(5*(9-6) != 15){printf("error.");}
    if( (3+5) / 2 != 4){printf("error.");}
    if(-2+4!=2){printf("error.");}
    if(((6/2)==3) != 1) printf("error.");
    if((5+1<-2) != 0) printf("error");
    if(5<=(7-2) != 1) printf("error.");
    int a;
    int b;
    a = 3; b = 5*6-8; 
    if(a+b/2 != 14) printf("error No.10\n");
    int foo;
    foo = 4;
    if(foo+11 != 15) printf("error No.11\n");
    int teihen; int takasa;
    teihen = 5; takasa = 6;
    if(teihen*takasa/2 != 15) printf("error No.12\n");
    int ret;
    a = 2; b = 5;
    if(a*2 == b) ret = 6; else ret = 10;
    if(ret != 10) printf("error No.13\n");
    a = 1;
    while(a < 10)a=a+a;
    if(a != 16) printf("error No.14\n");
    int i; 
    a = 0;
    for(i=0; i<=20; i=i+1)a=a+i;
    if(a != 210) printf("error No.15\n");
    if(add_six(1,2,3,4,5,6) != 21) printf("error No.16\n");
    if(square(3)-twice(3)-3 != 0) printf("error No.17\n");
    if(is_prime(27)) printf("error No.18\n");
    if(is_prime(3571) == 0) printf("error No.19\n");
    int x;
    int *p;
    x = 3;
    p = &x;
    if(*p != 3){printf("error No.20\n");}
    *p = 5;
    if(x != 5){printf("error No.21\n");}
    func1(x);
    if(x != 5){printf("error No.22\n");}
    func2(&x);
    if(x != 8){printf("error No.23\n");}
    if(fact(5) != 120){printf("error No.24\n");}
    if(sizeof(x) != 4){printf("error No.25\n");}
    if(sizeof(x+3) != 4){printf("error No.26\n");}
    if(sizeof(&x) != 8){printf("error No.27\n");}
    if(sizeof(p) != 8){printf("error No.28\n");}
    if(sizeof(*p) != 4){printf("error No.29\n");}
    if(sizeof(sizeof(p)) != 4){printf("error No.30\n");}
    int arr[10];
    if(sizeof(arr) != 40){printf("error No.31\n");}
    int arr2[2][3];
    if(sizeof(arr2) != 24){printf("error No.32\n");}
    *arr = 1;
    *(arr+1) = 2;
    p = arr;
    if(*p+*(p+1) != 3) printf("error No.33\n");
    arr[0]=8; arr[1]=4;
    if(0[arr]/1[arr] != 2) printf("error No.34\n");
    g1 = 4;
    if(g1 != 4) printf("error No.35\n");
    gp = &x;
    x = 3;
    if(*gp+g1 != 7) printf("error No.36\n");
    if(sizeof(arr)/sizeof(arr[0]) != 10) printf("error No.37\n");
    arr[3] = 10;
    p = arr;
    p = p+3;
    if(*p != 10) printf("error No.38\n");
    char c;
    c = 5;
    if(c != 5) printf("error No.39\n");
    char cs[3];
    cs[0] = -1; cs[1] = 2; x = 4;
    if(x+cs[0] != 3) printf("error No.40\n");
    if(g2 != 2) printf("error No.41\n");
    if(*gp2 != 2) printf("error No.42\n");
    garr[2] = 5;
    if(*gp3 != 5) printf("error No.43\n");
    if(g3[3] != 4) printf("error No.44\n");
    if(g4[2] != 0) printf("error No.45\n");
    if(g5[4] != 32) printf("error No.46\n");
    if(g6[9] != 0) printf("error No.47\n");
    g5[4] = 43;
    if(g5[4] != 43) printf(g5); // 43 = '+'
        
    //
    //comment

    printf("hello world\n");
    return 0;
}

/*
# assert 20 "char func(){return 20;} char a; int main(){a = func(); return a;}"
*/