#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For parsing the text file
char** read_lines(const char *filename, int *num_lines) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }

    char **lines;
    int max_lines = 16;
    int num_allocated_lines = 0;
    char line[1024];

    lines = malloc(max_lines * sizeof(char*));
    if (lines == NULL) {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    // Read the lines from the file and store them in an array
    while (fgets(line, sizeof(line), file)!= NULL) {
        if (num_allocated_lines == max_lines) {
            max_lines *= 2;
            lines = realloc(lines, max_lines * sizeof(char*));
            if (lines == NULL) {
                printf("Error reallocating memory\n");
                fclose(file);
                free(lines);
                return NULL;
            }
        }

        lines[num_allocated_lines] = malloc(strlen(line) + 1);
        if (lines[num_allocated_lines] == NULL) {
            printf("Error allocating memory\n");
            fclose(file);
            for (int i = 0; i < num_allocated_lines; i++) {
                free(lines[i]);
            }
            free(lines);
            return NULL;
        }

        strcpy(lines[num_allocated_lines], line);
        num_allocated_lines++;
    }

    *num_lines = num_allocated_lines;
    fclose(file);
    return lines;
}



char** split_string(const char *str, int *num_words) {
    char **words;
    int max_words = 16;
    int num_allocated_words = 0;
    char *word;

    words = malloc(max_words * sizeof(char*));
    if (words == NULL) {
        printf("Error allocating memory\n");
        return NULL;
    }

    word = strtok(str, " \t\n");
    while (word!= NULL) {
        if (num_allocated_words == max_words) {
            max_words *= 2;
            words = realloc(words, max_words * sizeof(char*));
            if (words == NULL) {
                printf("Error reallocating memory\n");
                for (int i = 0; i < num_allocated_words; i++) {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }
        }

        words[num_allocated_words] = malloc(strlen(word) + 1);
        if (words[num_allocated_words] == NULL) {
            printf("Error allocating memory\n");
            for (int i = 0; i < num_allocated_words; i++) {
                free(words[i]);
            }
            free(words);
            return NULL;
        }

        strcpy(words[num_allocated_words], word);
        num_allocated_words++;
        word = strtok(NULL, " \t\n");
    }

    *num_words = num_allocated_words;
    return words;
}



//  0-1023 => Instructions  , 1024-2047 => Data
int memory [2048];
/*
    31 GPRs
    0-30 => GPRs
    31 => Zero Register
    32 => Program Counter
*/
int registers [33];
int clockCycleCount;
static int instructionCount = 0;





struct ALU_out{
    int result;
    int zeroFlag;
};

// struct decode_out{
    
// }

struct IF_ID{
    int instruction;
    unsigned int atu;

    unsigned int instruction_id;

};

struct ID_EX{
    unsigned int opcode; 
    unsigned int R1; 
    unsigned int R2;
    unsigned int R3;
    unsigned int shamt;
    int immediate;
    unsigned int address;

    unsigned int atu;

    unsigned int instruction_id;

};

struct EX_MEM{
    unsigned int register_address;
    int register_data;
    int registerWrite;

    unsigned int memory_address;
    int memory_data;
    int memoryWrite;

    unsigned int atu;

    unsigned int instruction_id;

};

struct MEM_WB{
    unsigned int register_address;
    int register_data;
    int registerWrite;

    unsigned int atu;

    unsigned int instruction_id;

};

struct IF_ID if_id_pipeline;


struct ID_EX id_ex_pipeline;


struct EX_MEM ex_mem_pipeline;

struct MEM_WB mem_wb_pipeline;



// struct I_Instruction{
//     int opcode : 4;
//     int r1 : 5;
//     int r2 : 5;
//     int immediate : 18;
// };

// struct J_Instruction{
//     int opcode : 4;
//     int address : 28;
// };


// struct R_Instruction{
//     int opcode : 4;
//     int r1 : 5;
//     int r2 : 5;
//     int r3 : 5;
//     int shamt : 13;
// };
void init(){
    int num_lines;
    char **lines = read_lines("input.txt", &num_lines);
    instructionCount = 0;

    for (int i = 0; i < num_lines; i++) {
        int num_words;
        char **words = split_string(lines[i], &num_words);
        int instruction = 0b00000000000000000000000000000000;
        int rORiORj;

        char * instructionName = words[0];
        if(strcmp(instructionName,"ADD") == 0){
            instruction = instruction | 0b00000000000000000000000000000000;
            rORiORj = 0;
        }
        else if(strcmp(instructionName,"SUB") == 0){
            instruction = (instruction ) | 0b00010000000000000000000000000000;
            rORiORj = 0;
        }
        else if(strcmp(instructionName,"MULI") == 0){
            instruction = instruction | 0b00100000000000000000000000000000;
            rORiORj = 1;
        }
        else if(strcmp(instructionName,"ADDI") == 0){
            instruction = instruction | 0b00110000000000000000000000000000;
            rORiORj = 1;
        }
        else if(strcmp(instructionName,"BNE") == 0){
            instruction = instruction | 0b01000000000000000000000000000000;
            rORiORj = 1;
        }
        else if(strcmp(instructionName,"ANDI") == 0){
            instruction = instruction | 0b01010000000000000000000000000000;
            rORiORj = 1;
        }
        else if(strcmp(instructionName,"ORI") == 0){
            instruction = instruction | 0b01100000000000000000000000000000;
            rORiORj = 1;
        }
        else if(strcmp(instructionName,"J") == 0){
            instruction = instruction | 0b01110000000000000000000000000000;
            rORiORj = 2;
        }
        else if(strcmp(instructionName,"SLL") == 0){
            instruction = instruction | 0b10000000000000000000000000000000;
            rORiORj = 0;
        }
        else if(strcmp(instructionName,"SRL") == 0){
            instruction = instruction | 0b10010000000000000000000000000000;
            rORiORj = 0;
        }
        else if(strcmp(instructionName,"LW") == 0){
            instruction = instruction | 0b10100000000000000000000000000000;
            rORiORj = 1;
        }
        else if(strcmp(instructionName,"SW") == 0){
            instruction = instruction | 0b10110000000000000000000000000000;
            rORiORj = 1;
        }
        else{
            return;
        }

        if(rORiORj == 0 ){
            char * reg1 = strchr(words[1],'R');
            if(reg1 != NULL){
                memmove(reg1,reg1+1,strlen(reg1));
            }
            char * reg2 = strchr(words[2],'R');
            if(reg2 != NULL){
                memmove(reg2,reg2+1,strlen(reg2));
            }
            // char * reg2 = words[2];
            // char * reg3 = words[3];
            int regNum1 = atoi(reg1);
            int regNum2 = atoi(reg2);
            // printf("RegNum1 --> %i , RegNum2 --> %i  ,",regNum1 , regNum2);
            if (strcmp(instructionName,"ADD") == 0 ||strcmp(instructionName,"SUB") == 0){
                char * reg3 = strchr(words[3],'R');
                if(reg3 != NULL){
                    memmove(reg3,reg3+1,strlen(reg3));
                }
                int regNum3 = atoi(reg3);
                // printf(" RegNum3 --> %i \n",regNum3);
                instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000)  
                                          | ((regNum2 << 18) & 0b00000000011111000000000000000000)
                                          | ((regNum3 << 13) & 0b00000000000000111110000000000000); 
            }
            else if (strcmp(instructionName,"SRL") == 0 || strcmp(instructionName,"SLL") == 0){
                int shamt = atoi(words[3]);
                // printf(" Shamt --> %i \n" , shamt);
                instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000)  
                                          | ((regNum2 << 18) & 0b00000000011111000000000000000000)
                                          | ((shamt) & 0b00000000000000000001111111111111); 
            }
        }
        else if(rORiORj == 1){
            char * reg1 = strchr(words[1],'R');
            if(reg1 != NULL){
                memmove(reg1,reg1+1,strlen(reg1));
                }
            char * reg2 = strchr(words[2],'R');
            if(reg2 != NULL){
                memmove(reg2,reg2+1,strlen(reg2));
            }
            // char * reg1 = words[1];
            // char * reg2 = words[2];
            char * immediate = words[3];
            int regNum1 = atoi(reg1);
            int regNum2 = atoi(reg2);
            // printf("RegNum1 --> %i , RegNum2 --> %i  ,",regNum1 , regNum2);
            int imm = atoi(immediate);
            // printf(" Immediate --> %i \n" , imm);
            instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000)  
                                      | ((regNum2 << 18) & 0b00000000011111000000000000000000)
                                      | (imm & 0b00000000000000111111111111111111); 
             }
        else if (rORiORj == 2){
            char * address = words[1];
            int addr = atoi(address);
            // printf("Address --> %i \n" , addr);
            instruction = instruction | (addr);
             }
        
        memory[instructionCount] = instruction;
        instructionCount++;

        

        // Free the memory for the words
        for (int j = 0; j < num_words; j++) {
            free(words[j]);
        }
        free(words);

    }

    // Free the memory
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);

    
    registers[0] = 0;
    clockCycleCount = 1;
    if_id_pipeline.instruction_id = 0;
    id_ex_pipeline.instruction_id = 0;
    ex_mem_pipeline.instruction_id = 0;
    mem_wb_pipeline.instruction_id = 0;
    if_id_pipeline.atu = 0;
    id_ex_pipeline.atu = 0;
    ex_mem_pipeline.atu = 0;
    mem_wb_pipeline.atu = 0;

    
}

void add(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] + registers[id_ex_pipeline.R3];

    }
}

void sub(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] - registers[id_ex_pipeline.R3];

    }
}

void mulI(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] * id_ex_pipeline.immediate;
    }
}

void addI(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] + id_ex_pipeline.immediate;
    }
}

void bne(int r1 , int r2 , int imm){
    if(registers[r1] != registers[r2]){
        registers[32] = registers[32] + imm;
    }
}

void ANDI(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] & id_ex_pipeline.immediate;
    }
}

void ORI(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] | id_ex_pipeline.immediate;
    }
}

void J(int addr){ //!TODO update it to match the new convention
    if(addr<1024){
        registers[32] = addr;
    }
    else{
        printf("Invalid address");
    }
}

void SLL(){
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] << id_ex_pipeline.shamt;
    }
}

void SRL(){
    // if(r1 == 0){
    //     return;
    // }
    // else{
    //     if(r2 == 0){
    //         registers[r1] = 0 ;
    //         return;
    //     }
    //     else{
    //         registers[r1] = registers[r2] >> shamt;
    //     }
    //     int bitMsk = 0;
    //     int i;
    //     for(i = 0 ; i< shamt ; i++){
    //         bitMsk = bitMsk | 1 << i;
    //     }
    //     registers[r1] = registers[r1] & bitMsk;
    ex_mem_pipeline.memoryWrite = 0;
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        
        ex_mem_pipeline.register_data = registers[id_ex_pipeline.R2] >> id_ex_pipeline.immediate;
        int bitMsk = 0;
        int i;
        for(i = 0 ; i< id_ex_pipeline.shamt ; i++){
            bitMsk = bitMsk | 1 << i;
        }
        // int bitMsk = 2^
        ex_mem_pipeline.register_data = ex_mem_pipeline.register_data & bitMsk;
    }
    
}
void LW(){
    if(id_ex_pipeline.R1 == 0){
        ex_mem_pipeline.memoryWrite = 0;
        ex_mem_pipeline.registerWrite = 0;
    }
    else{
        ex_mem_pipeline.registerWrite = 1;
        // ex_mem_pipeline.register_data = memory[registers[id_ex_pipeline.R2] + id_ex_pipeline.immediate];
        ex_mem_pipeline.memory_address = registers[id_ex_pipeline.R2] + id_ex_pipeline.immediate;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1;
        
    }
}

void SW(unsigned int r1 , unsigned int r2 , int immediate){
    if(r1 == 0){
        return;
    }
    else{
        if(r2 == 0){
            memory[immediate] = registers[r1];
        }
        else{
            memory[registers[r2] + immediate] = registers[r1];
        }
    }
}


struct ALU_out ALU(int opcode , int operandA, int operandB){
    int output = 0;
    int zeroFlag = 0;
    switch(opcode) {
        case 0: //Add
            output = operandA + operandB;
            break;
        case 1: //Subtract
            output = operandA - operandB;
            break;
        case 2: //Multiply
            output = operandA * operandB;
            break;
        case 3: //Shift left logical
            output = operandA << operandB;
            break;
        case 4: //Shift right logical
            output = operandA >> operandB;
            break;
    }
    struct ALU_out result;
    result.result = output;
    result.zeroFlag = (output == 0) ? 1 : 0;
    printf("Operation = %d\n", opcode);
    printf("First Operand = %d\n", operandA);
    printf("Second Operand = %d\n", operandB);
    printf("Result = %d\n", result.result);
    printf("Zero Flag = %d\n", result.zeroFlag);
    return result;
    // return output;
}

void instructionFetch(){
    if(clockCycleCount % 2 == 1){
        int instruction = memory[registers[31]];
        
        // printf("Instruction %i -> %b \n" , i , registers[32]);
        if_id_pipeline.instruction = instruction;
        if_id_pipeline.atu = 1;
        if_id_pipeline.instruction_id = registers[32];
        registers[32] = registers[32] + 1;

    }
    // instructionDecode();

}

void instructionDecode(){
    // instructionFetch();
    if(if_id_pipeline.atu == 0){
        int instruction = if_id_pipeline.instruction;
        id_ex_pipeline.opcode = (instruction >> 28) & 0b00000000000000000000000000001111;
        id_ex_pipeline.R1 = (instruction >> 23) & 0b00000000000000000000000000011111;
        id_ex_pipeline.R2 = (instruction >> 18) & 0b00000000000000000000000000011111;
        id_ex_pipeline.R3 = (instruction >> 13) & 0b00000000000000000000000000011111;
        id_ex_pipeline.shamt = instruction & 0b00000000000000000001111111111111;
        int immediate = instruction & 0b00000000000000111111111111111111;
        int sign = immediate >>17;
        if(sign == 1)id_ex_pipeline.immediate=immediate|0b11111111111111000000000000000000;
        id_ex_pipeline.address = instruction & 0b00001111111111111111111111111111;

        id_ex_pipeline.instruction_id = if_id_pipeline.instruction_id;

        id_ex_pipeline.atu = 2;
    }
    else{
        if_id_pipeline.atu--;
        printf("Instruction :  didn't execute");
    }
    // instructionExecute(opcode,R1,R2,R3,shamt,immediate,address);

}

void instructionExecute(){
    if(id_ex_pipeline.atu == 0){
        switch(id_ex_pipeline.opcode) {
            case 1:
                add(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.R3);
                break;
            case 2:
                sub(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.R3);
                break;
            case 3:
                mulI(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;
            case 4:
                addI(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;
            case 5:
                bne(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;
            case 6:
                ANDI(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;
            case 7:
                ORI(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;
            case 8:
                J(id_ex_pipeline.address);
                break;
            case 9:
                SLL(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.shamt);
                break;
            case 10:
                SRL(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.shamt);
                break;
            case 11:
                LW(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;
            case 12:
                SW(id_ex_pipeline.R1,id_ex_pipeline.R2,id_ex_pipeline.immediate);
                break;    
            ex_mem_pipeline.atu = 2;
            ex_mem_pipeline.instruction_id = id_ex_pipeline.instruction_id;
        }
    }
    else{
        id_ex_pipeline.atu--;
        // printf("Instruction %id :  didn't execute");
    
    }
}

void instructionMemoryAccess(){
    if(ex_mem_pipeline.atu == 0){
        if(ex_mem_pipeline.memoryWrite == 1){
            memory[ex_mem_pipeline.memory_address] = ex_mem_pipeline.memory_data;
        }
        // else if(ex_mem_pipeline.registerWrite == 1){
        //     registers[ex_mem_pipeline.register_address] = ex_mem_pipeline.register_data;
        // }
        mem_wb_pipeline.register_address = ex_mem_pipeline.register_address;
        mem_wb_pipeline.register_data = ex_mem_pipeline.register_data;
        mem_wb_pipeline.registerWrite = ex_mem_pipeline.registerWrite;

        mem_wb_pipeline.atu = 1;
        mem_wb_pipeline.instruction_id = ex_mem_pipeline.instruction_id;
    }
    else{
        ex_mem_pipeline.atu--;
        // printf("Instruction %id :  didn't execute");
    }
}

void instructionRegisterWriteBack(){
    if(mem_wb_pipeline.atu == 0){
        if(mem_wb_pipeline.registerWrite == 1){
            registers[mem_wb_pipeline.register_address] = mem_wb_pipeline.register_data;
        }

        // mem_wb_pipeline.atu = 1;
        // mem_wb_pipeline.instruction_id = ex_mem_pipeline.instruction_id;
    }
    else{
        mem_wb_pipeline.atu--;
        // printf("Instruction %id :  didn't execute");
    }
}


int main(){
    init();
    // instructionFetch();
    // instructionDecode();
    // instructionExecute();
    // instructionMemoryAccess();
    // instructionRegisterWriteBack();





    // registers[2] = 5;
    // registers[3] = 6;
    // instructionFetch();
    // int s = 3;
    // int shamt = 3;

    // s = s >> shamt;
    // printf("%i\n" , s);
    // int bitMsk = 0;
    // int i;
    // for(i = 0 ; i< shamt ; i++){
    //     bitMsk = bitMsk | 1 << i;
    // }
    // s = s & bitMsk;
    // printf("%i\n" , s);
}
