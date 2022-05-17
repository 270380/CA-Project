//مریم شفیع زادگان و زهرا اشرفی

#include <stdio.h>
#include <stdlib.h>

int Memory[16 * 1024], percentage_of_use_every_registers[16];
int pc, percentage_of_use_of_registers, the_amount_of_memory_used, number_of_implementation_instructions, total_use;

int loader () {
    char decimal[32];
    FILE *file = NULL;
    int line = 0, store_in_memory = 0;
    file = fopen("B:\\Information_Of_Project1.txt", "r");
    while (fgets(decimal, 32, file)) {
        Memory[line + 16368] = atoi(decimal);
        if (store_in_memory == 1) Memory[line + 16] = Memory[line + 16368];
        if (Memory[line + 16368] == 234881024) store_in_memory = 1;
        line++;
    }
    //چاپ اطلاعات دریافتی از پروژه اول
    printf("Print the information received from the file:\n");
    for (int i = 0; i < line; ++i)printf("%d\n", Memory[i + 16368]);
    printf("\n");
    return line;
}

int Multiplexer(int signal, int input0, int input1) {
    if (signal == 0) return input0;
    else return input1;
}

int Extension (int ExtOp, int imm) {
    if (ExtOp == 1 && (imm >> 15) == 1)return -65536 | imm; // - 65536 = 11111111111111110000000000000000
    else return imm;
}

int Adder (int input1, int input2) {return input1 + input2;}

int And(int input1, int input2) {
    if (input1 == 1 && input2 == 1) return 1;
    else return 0;
}

int Or(int input1, int input2) {
    if (input1 == 1 || input2 == 1) return 1;
    else return 0;
}

int Alu (int AluCnt, int data1, int data2) {
    if (AluCnt == 0 || AluCnt == 5 || AluCnt == 9 || AluCnt == 10) return data1 + data2;   //add, addi, lw, sw
    else if (AluCnt == 1) return data1 - data2;  //sub
    else if (AluCnt == 2 || AluCnt == 6) {  //slt, slti
        if (data1 < data2) return 1;
        else return 0;
    }
    else if (AluCnt == 3 || AluCnt == 7) return data1 | data2;  //or, ori
    else if (AluCnt == 4) return data1 & data2;  // nand
    else if (AluCnt == 8) return data2 >> 16;  // lui
    else if (AluCnt == 11) { //beq
        if (data1 == data2) return 1;
        else return 0;
    }
    else if (AluCnt == 12) {
        int val = pc;
        pc = data1;
        return val;
    }
    else if (AluCnt == 13) pc = data2;
    return 0;
}

int main() {

    int number_of_order = loader();  //دریافت ورودی
    while (pc < number_of_order) {
        number_of_implementation_instructions++;

        // IF
        int control = 0, ra = Memory[pc + 16368], opcode = (251658240 & ra) >> 24; // 251658240 = 1111000000000000000000000000
        int rs = -1, rt = -1, rd = -1, imm = -1, mem_rs, mem_rt, lw_mem = -1;
        if (opcode == 14) { //halt
            printf("\n%d) halt!!", pc);
            break;
        }
        if (opcode != 13) { //!j
            rs = (ra & 15728640) >> 20; // 15728640 = 111100000000000000000000
            rt = (ra & 983040) >> 16; // 983040 = 11110000000000000000

            total_use += 2;
            percentage_of_use_of_registers += 2;
            percentage_of_use_every_registers[rs]++;
            percentage_of_use_every_registers[rt]++;
        }
        if (opcode < 5) {
            rd = (ra & 61440) >> 12; // 61440 = 1111000000000000

            total_use++;
            percentage_of_use_of_registers++;
            percentage_of_use_every_registers[rd] ++;
        }
        else if (4 < opcode && opcode < 14 && opcode != 12) { //I_Type, j
            control = 1;
            if (opcode == 7 || opcode == 8 || opcode == 13)
                imm = Extension(0, ra & 65535); //65535 = 1111111111111111
            else imm = Extension(1, ra & 65535);
        }
        pc = Adder(pc, 1);

        printf("\n%d)\n-------------------------IF-------------------------\n", pc - 1);
        if (And(opcode != 13, opcode != 13)) { //چاپ مقادیر opcode, rs, rt, rd, imm دستورات
            printf("opcode:%d  address_rs:%d  address_rt:%d  ",opcode,rs, rt);
            if (rd != -1) printf("rd:%d\n", rd);
            else printf("imm:%d\n", imm);
        } else printf("opcode_j:%d  imm:%d\n", opcode, imm);

        // ID
        mem_rs = Memory[rs];
        mem_rt = Memory[rt];

        int write_adder = Multiplexer(Or(control, opcode == 12), rd, rt);
        printf("-------------------------ID-------------------------\nvalue of rs:%d  value of rt:%d  writeAdder:%d\n", mem_rs ,mem_rt ,write_adder);

        //EXE
        int AluOutput = Alu(opcode, mem_rs, Multiplexer(And(control, opcode != 11), mem_rt, imm));
        pc = Multiplexer(And((opcode == 11), AluOutput), pc, Adder(pc, imm)); // دستور beq یا بقیه دستورات
        printf("-------------------------EXE------------------------\nALU output:%d   PC:%d\n", AluOutput, pc);

        //MEM
        printf("-------------------------MEM------------------------\n");
        if (And(opcode == 10, opcode == 10)) { // MemToRegister
            Memory[16 + AluOutput] = mem_rt;
            total_use++;
            the_amount_of_memory_used ++;
            printf("value of rt stores in memory!\nMemory[%d] = rt(%d)\n", AluOutput, mem_rt);
        }
        else if (And(opcode == 9, opcode == 9)) { // MemToRegister
            lw_mem = Memory[16 + AluOutput];
            total_use++;
            the_amount_of_memory_used ++;
            printf("value of memory is load!\nload = Memory[%d]\n", 16 + AluOutput);
        } else printf("No MEM!\n");

        //WB
        if (And(And(opcode != 11, opcode != 13), opcode != 10)) {
            int result = Multiplexer(opcode == 9, AluOutput, lw_mem);
            Memory[write_adder] = result;
            printf("-------------------------WB-------------------------\nWrite Back result in Register!\nresult = %d\n",result);
        } else printf("-------------------------WB-------------------------\nNo WB!(opcode is %d)\n", opcode);


        printf("Number of execution instructions: %d\n", number_of_implementation_instructions);
        printf("Percentage of memory usage: %.2f%%\n", (float)the_amount_of_memory_used / (float)total_use * 100);
        printf("Percentage of use of each register to all registers - total usage:\n");
        for (int i = 0; i < 16; ++i) {
            printf("Register%d: %.2f%%    %.2f%%\n", i + 1, (float)percentage_of_use_every_registers[i] / (float)percentage_of_use_of_registers * 100, (float)percentage_of_use_every_registers[i] / (float)total_use * 100);
        }
    }
    return 0;
}