#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef word adr;
/*
#define  LO(x) ((x) & 0xFF)
#define  HI(x) (((x) >> 8) & 0xFF)
*/

void do_halt () {
    printf("HALT\n");
    exit(0);
}

void do_mov () {
    printf("MOV\n");
    //write it
}

void do_add () {
    printf("ADD\n");
    //write it
}

void do_unknown () {
    printf("UNKNOWN\n");
    //write it
}

struct Command {
    word opcode;
    word mask;
    char *name;
    void (*func)();

}commands[] = {
        {0,       0177777, "halt",    do_halt}, //mask is all "1" or "0xFFFF
        {0010000, 0170000, "mov",     do_mov},
        {0060000, 0170000, "add",     do_add},
        {0170000, 0177777, "unknown", do_unknown}//MUST BE THE LAST
};



byte mem[64*1024];

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

void load_file() {
    FILE * f = fopen ("sum.txt", "r");
    if (f == NULL){
        perror ("sum.txt");
        exit (1);
    };
    unsigned int adress;
    unsigned int n;
    unsigned int val = 0;
    while (fscanf (f, "%x%x", &adress, &n) == 2){
    //fscanf (f, "%x%x", &adress, &n);
        for(int i = 0; i < n; i++) {
            fscanf (f, "%x", &val);
            b_write ((adr)(adress + i), (byte)val);
        }
    }
}

void run (adr pc0) {
    adr pc = pc0;
    while(1) {
        word w = w_read(pc);
        printf("%06o:%06o", pc, w);
        pc += 2;
        for(int i = 0; i < 64*1024; i++) {
            struct Command cmd = commands[i];
            if ((w & cmd.mask) == *(cmd.name)) {
                cmd.func;
                //if (i == 0) // if it is halt
                    //exit(0);
            }
        }
        break;
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

int main () {
    test_mem ();
    return 0;
    // test
}