#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1<<1) // = 2
#define HAS_XX (1<<2) // = 3
#define HAS_NN (1<<3) // = 4

#define pc reg[7]

typedef unsigned char byte;
typedef unsigned short word;
typedef word adr;
/*
#define  LO(x) ((x) & 0xFF)
#define  HI(x) (((x) >> 8) & 0xFF)
*/

word nn;
struct mr {
    word adr;	// address
    word val;
    word res;// value
    word space; // address in mem[ ] or reg[ ]
} ss, dd;

//word xx;
byte mem[64*1024];
word reg[8];
//////////////////////////////////
void b_write(adr a, byte val) {
    mem[a] = val;
}
byte b_read (adr a){
    return mem[a];
};
void w_write(adr a, word val){
    //val = 0x0b0a
    //a=2
    //mem[2] = 0x0a
    assert (a % 2==0);
    mem[a] = (byte)(val & 0xFF);//(byte(val)
    //mem[3] = 0x0b
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
void do_halt () {
    //printf("HALT\n");
    exit(0);
}
void do_mov () {
    dd = ss;
    // check it
    //printf("MOV\n");
}
void do_add ( ) {
    dd.res = ss.val + dd.val;///////////////////FIX IT
}
void do_sob () {
    w_read(nn);
    pc = pc - (word)(2)*nn;
    //printf("SOB\n");
}
void do_unknown () {
    ;
    //printf("UNKNOWN\n");
}
///////////////////////////////////
word get_nn(word w) {
    return w & 077;// returns NN of the word
}
word get_ss(word w) {
    return w & 07700;// returns SS and DD of the word
}
/*word get_dd(word w) {
    return w & 077;// returns DD of the word
}*/

struct mr get_dd (word w) {
    int n = w & 3;    // register number
    int mode = (w >> 3) & 3;    // mode
    struct mr res;
    switch (mode) {
        case 0:
            res.adr = (word)n;
            res.val = reg[n];
            res.space = (word)reg;
            dprintf(" R%d", n);
            break;
        case 1:
            res.adr = reg[n];
            res.val = mem[res.adr];
            //res.space = res.adr;
            dprintf(" R%d", n);
            break;
        case 2:
            res.adr = reg[n];
            res.val = mem[res.adr];
            if(!(w & 010000) || n == 7 || n == 8)
                reg[n] += 2;
            else
                reg[n] += 1;//it's a byte operation

            break;
        case 3:
            res.adr = mem[reg[n]];
            res.val = mem[res.adr];
            reg[n] += 2;
            dprintf(" R%d", n);
            break;
        case 4:
            if(!(w & 010000) || n == 7 || n == 8)
                res.adr = reg[n] - (word)2;
            else
                res.adr = reg[n] - (word)1;//it's a byte operation
            res.val = mem[res.adr];
            dprintf(" R%d", n);
            break;
        case 5:
            reg[n] -= 2;
            res.adr = reg[n];
            res.space = mem[res.adr];
            res.val = mem[res.space];
            dprintf(" R%d", n);
            break;
        case 6:
            //WRITE IT
            break;
    }
}

struct Command {
    word opcode;
    word mask;
    char *name;
    void (*func)();
    byte param;
}commands[] = {
        {0,       0177777, "halt",    do_halt, NO_PARAM}, //mask is all "1" or "0xFFFF
        {0010000, 0170000, "mov",     do_mov, HAS_SS | HAS_DD},
        {0060000, 0170000, "add",     do_add, HAS_SS | HAS_DD},
        {0077000, 0177000, "sob",     do_sob, HAS_NN},
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
    unsigned int adress = 0;
    unsigned int n = 0;
    unsigned int val = 0;
    while (fscanf (f, "%x%x", &adress, &n) == 2){
        for(int i = 0; i < n; i++) {
            fscanf (f, "%x", &val);
            b_write ((adr)(adress + i), (byte)val);
        }
    }
}

void run (adr pc0) {
    pc = pc0;
    while(1) {
        word w = w_read(pc);
        printf("%06o:%06o ", pc, w);
        pc += 2;
        for(int i = 0; i < (int)sizeof(commands)/ sizeof(struct Command); i++) {
            struct Command cmd = commands[i];
            if ((w & cmd.mask) == cmd.opcode) {
                printf("%s\n", cmd.name);

                if((cmd.param) & HAS_NN) {
                    nn = get_nn(w);
                }
                if((cmd.param) & HAS_SS) {
                    ss.val = get_ss(w);
                }
                if((cmd.param) & HAS_DD) {
                    dd = get_dd(w);
                }
                cmd.func();
                break;
            }
        }
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
    run(512);
    return 0;
}