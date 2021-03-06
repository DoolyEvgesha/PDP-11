#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1<<1) // = 2
#define HAS_XX (1<<2) // = 3
#define HAS_NN (1<<3) // = 4
#define HAS_R (1<<4)
#define HAS_R_END (1<<5)

#define pc reg[7]
#define sp reg[6]

#define register 1
#define memory 2

typedef unsigned char byte;
typedef unsigned short word;
typedef word adr;

int f_print;
byte r;
word nn;
//word xx;
//unsigned int xx;
int reg_number;
int is_byte_cmd;

byte n, z, v, c;

struct mr {
    word adr;	// address
    word val;
    word res;   // value
    word space; // address in mem[ ] or reg[ ]
} ss, dd;

typedef union s_byte{
    char sby;
    unsigned char uby;
}s_byte;

typedef union s_word{
    short sws;
    unsigned short usw;
}s_word;
s_word xx;

byte mem[64*1024];
word reg[8];
/////////////////////////////////
void b_write(adr a, byte val) {
    mem[a] = val;
}
byte b_read (adr a){
    return mem[a];
};
void w_write(adr a, word val){
    assert (a % 2==0);
    mem[a] = (byte)(val & 0xFF);
    mem[a+1] = (byte)((val >> 8) & 0xFF);
}
word w_read (adr a) {
    word w0 = 0, w1 = 0;
    w0 = mem[a];
    w1 = mem[a+1];
    // printf ("\n////// \n w0 = %x w1 = %x\n//////\n", w0, w1);
    w1 <<= 8;
    return w1 + w0;
}
//////////////////////////////////
void do_xx(word result) {
    if (result == 0)
        z = 1;
    else
        z = 0;
}
/////////////////////////////////////////
/* void do_nz(word result) {     ////////
    if (result == 0)             ////////
        z = 1;
    else
        z = 0;
    if (result < 0)
        n = 1;
    else
        n = 0;
}

void do_c (word result) {        /////////
    if (result & ...)            /////////
} */                             /////////
//////////////////////////////////////////
void do_halt () {
    exit(0);
}
void do_mov () {
    if (dd.space == register) {
        reg[dd.adr] = ss.val;
        do_xx(reg[dd.adr]);
    }
    else {
        w_write(dd.adr, ss.val);
        do_xx(w_read(dd.adr));
    }
}
void do_mov_b () {
    if (dd.space == register) {
        reg[dd.adr] = ss.val;
        if (f_print == 1)
            print_char(ss.val);
        f_print = 0;
        do_xx(reg[dd.adr]);
    }
    else {
        w_write(dd.adr, ss.val);
        if (f_print == 1)
            print_char(ss.val);
        f_print = 0;
        do_xx(w_read(dd.adr));
    }
}
void do_add () {
    reg[dd.adr] = ss.val + dd.val;
    do_xx(reg[dd.adr]);
}
void do_sob () {
    printf("R%d %06o", reg_number, pc - 2*nn);
    reg[reg_number]--;
    if(reg[reg_number] != 0)
        pc = pc - (word)(2)*nn;
}

void do_br() {
    pc = pc + (word)2 * xx.sws;
}


void do_beq() {
    if (z == 1)
        do_br();
}

void do_clear() {
    reg[dd.adr] =0;
}

void do_jsr() {
    //printf("...%o...", dd.adr);
   /* pc = dd.adr;
    reg[6] -= 2;
    reg[reg_number] = pc;
*/
   w_write(sp, reg[reg_number]);
   sp -= 2;
   reg[reg_number] = pc;
   pc = dd.adr;
}

void do_tst(){
    if (dd.val < 0)
        n = 1;
    if (dd.val == 0)
        z = 0;
    v = 0;
    c = 0;

}

void do_tst_b() {
    c = 0;
    if(dd.val == 0)
        z = 1;
    if(dd.val & 0100000)
        n = 1;
}

void do_bpl(){
    if(n == 0)
        do_br();
}

void do_rts(){
    pc = reg[reg_number];
    sp += 2;
    pc = w_read(sp);
    //reg[reg_number] = w_read(pc -(pc - sp));
    //printf("pc = %o, sp = %o, pc - sp = %o", pc, sp, pc - sp - 2);
    //reg[reg_number] = w_read(reg[6]+4);
    //printf("..R[%o] = %o, mem[sp] = %o, reg[6] = %o ", reg_number,reg[reg_number], w_read(reg[6]+4), reg[6]+4);
    //printf("pc = %o..", pc);
    //sp += 2;
}

void do_dec(){
    dd.val -= dd.val;
    /*if(dd.val == 0)
        z = 1;
    else
        z = 0;*/
}

void print_char(word val){
    printf(" %c", val);
}

void do_unknown () {
    ;
}
///////////////////////////////////
word get_nn(word w) {
    return w & 077;// returns NN of the word
}

void dump_reg() {
    int i;
    printf("\n");
    for (i = 0; i < 8; i++)
        printf("r%d: %06o ", i, reg[i]);
    printf("\n");
}

int get_reg_number(word w) {
    //printf("%06o -> %03o", w, (w>>6)&07);
    //exit(1);
    return (w>>6) & 07;
}

int get_xx(word w){
    //printf("%o ...",w & 0377);
    union s_byte xx;
    xx.uby = w;
    printf("%o", pc + 2*(xx.sby));
    //printf("%o\n",xx.sby);
    return xx.sby;
}

struct mr get_dd (word w) {
    int n = w & 7;    // register number
    int mode = (w >> 3) & 7;    // mode
    struct mr res = {0, 0, 0 ,0};
    if (mode == 0)
        res.space = register;//if it works with registers
    else
        res.space = memory;//if it works with mem
    switch (mode) {
        case 0:
            res.adr = (word)n;
            res.val = reg[n];
            printf("R%d ", n);
            break;
        case 1:
            res.adr = reg[n];
            res.val = w_read(res.adr);
            printf("(R%d) ", n);
            break;
        case 2:
            res.adr = reg[n];
            if(is_byte_cmd)
            {
                union s_byte xx;
                xx.uby = w_read(res.adr);
                union s_word yy;
                yy.usw = xx.sby;
                res.val = yy.sws;
            }
            else
                res.val = w_read(res.adr);
            if(!is_byte_cmd || n == 7 || n == 6)
                reg[n] += 2;
            else
                reg[n] += 1;//it's a byte operation

            if (n == 7)
                printf("#%o ", res.val);
            else
                printf("(R%d)+ ", n);

            break;
        case 3:
            if (n == 7) {
                res.adr = pc;
                res.val = w_read(res.adr);
                reg[n] += 2;
                printf(" @#%o ", res.val);
                f_print = 1;
            }
            else {
                res.adr = w_read(reg[n]);
                res.val = w_read(res.adr);
                reg[n] += 2;
                printf(" R%d", n);
            }
            break;
        case 4:
            //printf("pc=%o \n", reg[n]);
            if(!is_byte_cmd || n == 6 || n == 7) {
                reg[n] -= (word)2;
                res.adr = reg[n];
            }
            else {
                reg[n] -= (word)1;
                res.adr = reg[n];//it's a byte operation
            }
            res.val =  w_read(res.adr);
            if (n == 7)
                printf("-(pc) ");
            else if (n == 6)
                printf("-(sp) ");
            else
                printf("-(R%d)", n);
            break;
        case 5:
            reg[n] -= 2;
            res.adr = reg[n];
            res.space = mem[res.adr];
            res.val = mem[res.space];
            break;
        case 6:
            if(n == 7) {
                res.adr = w_read(pc) + pc + 2;
                res.val = mem[res.adr];
                printf("%o ,", w_read(pc) + pc + 2);
                printf("[%o] " , res.adr);
                pc+=2;
            }
            else {
                res.adr = reg[n] + w_read(pc);
                res.val = mem[res.adr];
                printf("%o(R%d), ", w_read(pc), n);
                printf("[%o] = %o, ", res.adr, res.val);
                pc+=2;
            }
            break;
        default:
            printf("Mode %d not implemented yet!\n", mode);
            exit(2);
    }
    return res;
}

struct Command {
    word opcode;
    word mask;
    char *name;
    void (*func)();
    byte param;
}commands[] = {
        {0,       0177777, "halt",    do_halt,  NO_PARAM}, //mask is all "1" or "0xFFFF
        {0010000, 0170000, "mov",     do_mov,   HAS_SS | HAS_DD},
        {0110000, 0170000, "mov_b",   do_mov_b, HAS_SS | HAS_DD},
        {0060000, 0170000, "add",     do_add,   HAS_SS | HAS_DD},
        {0077000, 0177000, "sob",     do_sob,   HAS_NN | HAS_R},
        {0005000, 0077700, "clr",     do_clear, HAS_DD},
        {0000400,  0xFF00, "br",      do_br,    HAS_XX},
        {0001400,  0xFF00, "beq",     do_beq,   HAS_XX},
        {0105700, 0177700, "tst_b",   do_tst_b, HAS_DD},
        {0005700, 0177700, "tst",     do_tst,   HAS_DD},
        {0100000,  0xFF00, "bpl",     do_bpl,   HAS_XX},
        {0004000, 0177000, "jsr",     do_jsr,   HAS_DD | HAS_R},
        {0000200, 0177770, "rts",     do_rts,   HAS_R_END},
        {0005300, 0177700, "dec",     do_dec,   HAS_DD},
        {0000000, 0000000, "unknown", do_unknown}//MUST BE THE LAST
};

void load_file(char * filename) {
    char* fname = filename;
    //scanf("%ms", &fname);
    FILE * f = fopen (fname, "r");
    if (f == NULL){
        perror (fname);
        exit (1);
    };
    unsigned int address = 0;
    unsigned int n = 0;
    unsigned int val = 0;
    while (fscanf (f, "%x%x", &address, &n) == 2){
        for(int i = 0; i < n; i++) {
            fscanf (f, "%x", &val);
            b_write ((adr)(address + i), (byte)val);
        }
    }
}

void run (adr pc0) {
    pc = pc0;
    int counter =0;
    while(1) {
        counter ++;/////////////////////
        if (counter > 30) break;//////////////
        word w = w_read(pc);
        printf("%06o:%06o ", pc, w);
        pc += 2;
        for(int i = 0; i < (int)sizeof(commands)/ sizeof(struct Command); i++) {
            struct Command cmd = commands[i];
            if ((w & cmd.mask) == cmd.opcode) {
                printf("%s ", cmd.name);
                is_byte_cmd = (w >> 15) & 1;
                if((cmd.param) & HAS_SS) {
                    ss = get_dd(w>>6);
                }
                if((cmd.param) & HAS_DD) {
                    dd = get_dd(w);
                }
                if((cmd.param) & HAS_NN) {
                    nn = get_nn(w);
                    r = w & 000700;
                }
                if((cmd.param) & HAS_R) {
                    reg_number = get_reg_number(w);
                }
                if((cmd.param) & HAS_R_END) {
                    reg_number = get_reg_number(w<<6);
                }
                if((cmd.param) & HAS_XX) {

                    xx.usw = get_xx(w);
                    //printf("..xx = %d, xx * 2 = %d..", xx, xx*2);
                }
                cmd.func();
                dump_reg();
                break;
            }
        }
        printf("\n");
        //break;
    }
}

void mem_dump(adr start, word n) {
    for(adr i = 0; i < n; i = i + (adr)2)
        printf("%06o : %06o\n", start + i, w_read (start + i));
}

void test_mem() {
    byte b0,b1;
    word w, w1;
    w = 0x0d0c;
    w1 = 0x0f0e;
    b0 = 0x0c;
    b1 = 0x0d;
    b_write(4, b0);
    b_write(5, b1);
    w = w_read(4);
    printf ("%04x = %02x%02x\n", w, b1, b0);
    b0 = b_read (4);
    b1 = b_read (5);
    assert(b0 == 0x0c);
    assert(b1 == 0x0d);
    assert (mem[4] == 0x0c);
    assert (mem[5] == 0x0d);
    assert (w == 0x0d0c);
    w_write(6, w);
    w = w_read (6);
    assert (mem[6] == 0x0c);
    assert (mem[7] == 0x0d);
    assert (w == 0x0d0c);
}


int main (int argc, char * argv[]) {
    char* filename = argv[1];
    test_mem ();
    load_file(filename);
    //printf("%d\n\n", mem[512]);
    run(01000);//512
    return 0;
}