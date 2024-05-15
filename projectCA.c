#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For parsing the text file
char **read_lines(const char *filename, int *num_lines)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return NULL;
    }

    char **lines;
    int max_lines = 16;
    int num_allocated_lines = 0;
    char line[1024];

    lines = malloc(max_lines * sizeof(char *));
    if (lines == NULL)
    {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    // Read the lines from the file and store them in an array
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (num_allocated_lines == max_lines)
        {
            max_lines *= 2;
            lines = realloc(lines, max_lines * sizeof(char *));
            if (lines == NULL)
            {
                printf("Error reallocating memory\n");
                fclose(file);
                free(lines);
                return NULL;
            }
        }

        lines[num_allocated_lines] = malloc(strlen(line) + 1);
        if (lines[num_allocated_lines] == NULL)
        {
            printf("Error allocating memory\n");
            fclose(file);
            for (int i = 0; i < num_allocated_lines; i++)
            {
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

char **split_string(const char *str, int *num_words)
{
    char **words;
    int max_words = 16;
    int num_allocated_words = 0;
    char *word;

    words = malloc(max_words * sizeof(char *));
    if (words == NULL)
    {
        printf("Error allocating memory\n");
        return NULL;
    }

    word = strtok(str, " \t\n");
    while (word != NULL)
    {
        if (num_allocated_words == max_words)
        {
            max_words *= 2;
            words = realloc(words, max_words * sizeof(char *));
            if (words == NULL)
            {
                printf("Error reallocating memory\n");
                for (int i = 0; i < num_allocated_words; i++)
                {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }
        }

        words[num_allocated_words] = malloc(strlen(word) + 1);
        if (words[num_allocated_words] == NULL)
        {
            printf("Error allocating memory\n");
            for (int i = 0; i < num_allocated_words; i++)
            {
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
int memory[2048];
/*
    31 GPRs
    1-31 => GPRs
    0 => Zero Register
    32 => Program Counter
*/
int registers[33];
int clockCycleCount;
static int instructionCount = 0;

struct ALU_out
{
    int result;
    int zeroFlag;
};

// struct decode_out{

// }

struct IF_ID
{
    int instruction;
    int atu;

    unsigned int instruction_id;
};

struct ID_EX
{
    unsigned int opcode;
    unsigned int R1_address;
    unsigned int R2_address;
    unsigned int R3_address;
    unsigned int R2_value;
    unsigned int R3_value;
    unsigned int shamt;
    int immediate;
    unsigned int address;

    unsigned int prev_opcode;
    unsigned int prev_R1_address;
    unsigned int prev_R2_address;
    unsigned int prev_R3_address;
    unsigned int prev_R2_value;
    unsigned int prev_R3_value;
    unsigned int prev_shamt;
    int prev_immediate;
    unsigned int prev_address;

    int atu;

    unsigned int instruction_id;
};

struct EX_MEM
{
    unsigned int register_address;
    int register_data;
    int registerWrite;

    unsigned int memory_address;
    int memory_data;
    int memoryWrite;
    int memoryRead;

    unsigned int prev_register_address;
    int prev_register_data;
    int prev_registerWrite;

    unsigned int prev_memory_address;
    int prev_memory_data;
    int prev_memoryWrite;
    int prev_memoryRead;

    int atu;

    unsigned int instruction_id;
};

struct MEM_WB
{
    unsigned int register_address;
    int register_data;
    int registerWrite;

    unsigned int prev_register_address;
    int prev_register_data;
    int prev_registerWrite;

    int atu;

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
void init()
{
    int num_lines;
    char **lines = read_lines("input.txt", &num_lines);
    instructionCount = 0;

    for (int i = 0; i < num_lines; i++)
    {
        int num_words;
        char **words = split_string(lines[i], &num_words);
        int instruction = 0b00000000000000000000000000000000;
        int rORiORj;

        char *instructionName = words[0];
        if (strcmp(instructionName, "ADD") == 0)
        {
            instruction = instruction | 0b00000000000000000000000000000000;
            rORiORj = 0;
        }
        else if (strcmp(instructionName, "SUB") == 0)
        {
            instruction = (instruction) | 0b00010000000000000000000000000000;
            rORiORj = 0;
        }
        else if (strcmp(instructionName, "MULI") == 0)
        {
            instruction = instruction | 0b00100000000000000000000000000000;
            rORiORj = 1;
        }
        else if (strcmp(instructionName, "ADDI") == 0)
        {
            instruction = instruction | 0b00110000000000000000000000000000;
            rORiORj = 1;
        }
        else if (strcmp(instructionName, "BNE") == 0)
        {
            instruction = instruction | 0b01000000000000000000000000000000;
            rORiORj = 1;
        }
        else if (strcmp(instructionName, "ANDI") == 0)
        {
            instruction = instruction | 0b01010000000000000000000000000000;
            rORiORj = 1;
        }
        else if (strcmp(instructionName, "ORI") == 0)
        {
            instruction = instruction | 0b01100000000000000000000000000000;
            rORiORj = 1;
        }
        else if (strcmp(instructionName, "J") == 0)
        {
            instruction = instruction | 0b01110000000000000000000000000000;
            rORiORj = 2;
        }
        else if (strcmp(instructionName, "SLL") == 0)
        {
            instruction = instruction | 0b10000000000000000000000000000000;
            rORiORj = 0;
        }
        else if (strcmp(instructionName, "SRL") == 0)
        {
            instruction = instruction | 0b10010000000000000000000000000000;
            rORiORj = 0;
        }
        else if (strcmp(instructionName, "LW") == 0)
        {
            instruction = instruction | 0b10100000000000000000000000000000;
            rORiORj = 1;
        }
        else if (strcmp(instructionName, "SW") == 0)
        {
            instruction = instruction | 0b10110000000000000000000000000000;
            rORiORj = 1;
        }
        else
        {
            return;
        }

        if (rORiORj == 0)
        {
            char *reg1 = strchr(words[1], 'R');
            if (reg1 != NULL)
            {
                memmove(reg1, reg1 + 1, strlen(reg1));
            }
            char *reg2 = strchr(words[2], 'R');
            if (reg2 != NULL)
            {
                memmove(reg2, reg2 + 1, strlen(reg2));
            }
            // char * reg2 = words[2];
            // char * reg3 = words[3];
            int regNum1 = atoi(reg1);
            int regNum2 = atoi(reg2);
            // printf("RegNum1 --> %i , RegNum2 --> %i  ,",regNum1 , regNum2);
            if (strcmp(instructionName, "ADD") == 0 || strcmp(instructionName, "SUB") == 0)
            {
                char *reg3 = strchr(words[3], 'R');
                if (reg3 != NULL)
                {
                    memmove(reg3, reg3 + 1, strlen(reg3));
                }
                int regNum3 = atoi(reg3);
                // printf(" RegNum3 --> %i \n",regNum3);
                instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000) | ((regNum2 << 18) & 0b00000000011111000000000000000000) | ((regNum3 << 13) & 0b00000000000000111110000000000000);
            }
            else if (strcmp(instructionName, "SRL") == 0 || strcmp(instructionName, "SLL") == 0)
            {
                int shamt = atoi(words[3]);
                // printf(" Shamt --> %i \n" , shamt);
                instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000) | ((regNum2 << 18) & 0b00000000011111000000000000000000) | ((shamt) & 0b00000000000000000001111111111111);
            }
        }
        else if (rORiORj == 1)
        {
            char *reg1 = strchr(words[1], 'R');
            if (reg1 != NULL)
            {
                memmove(reg1, reg1 + 1, strlen(reg1));
            }
            char *reg2 = strchr(words[2], 'R');
            if (reg2 != NULL)
            {
                memmove(reg2, reg2 + 1, strlen(reg2));
            }
            // char * reg1 = words[1];
            // char * reg2 = words[2];
            char *immediate = words[3];
            int regNum1 = atoi(reg1);
            int regNum2 = atoi(reg2);
            // printf("RegNum1 --> %i , RegNum2 --> %i  ,",regNum1 , regNum2);
            int imm = atoi(immediate);
            // printf(" Immediate --> %i \n" , imm);
            instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000) | ((regNum2 << 18) & 0b00000000011111000000000000000000) | (imm & 0b00000000000000111111111111111111);
        }
        else if (rORiORj == 2)
        {
            char *address = words[1];
            int addr = atoi(address);
            // printf("Address --> %i \n" , addr);
            instruction = instruction | (addr);
        }

        memory[instructionCount] = instruction;
        instructionCount++;

        // Free the memory for the words
        for (int j = 0; j < num_words; j++)
        {
            free(words[j]);
        }
        free(words);
    }

    // Free the memory
    for (int i = 0; i < num_lines; i++)
    {
        free(lines[i]);
    }
    free(lines);

    registers[0] = 0;
    clockCycleCount = 1;
    if_id_pipeline.instruction_id = 0;
    id_ex_pipeline.instruction_id = 0;
    ex_mem_pipeline.instruction_id = 0;
    mem_wb_pipeline.instruction_id = 0;
    if_id_pipeline.atu = 10;
    id_ex_pipeline.atu = 10;
    ex_mem_pipeline.atu = 10;
    mem_wb_pipeline.atu = 10;
    registers[32] = 0;
}

void add()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.R2_value + id_ex_pipeline.R3_value;
    }
}

void sub()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.R2_value - id_ex_pipeline.R3_value;
    }
}

void mulI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.R2_value * id_ex_pipeline.immediate;
    }
}

void addI()
{
    ex_mem_pipeline.prev_memoryWrite = 0;
    ex_mem_pipeline.prev_memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.prev_registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.prev_registerWrite = 1;
        ex_mem_pipeline.prev_register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.prev_register_data = id_ex_pipeline.prev_R2_value + id_ex_pipeline.prev_immediate;
        printf("Register Data = %d\n", ex_mem_pipeline.prev_register_data);
    }
}

void bne()
{
    // if(registers[r1] != registers[r2]){
    //     registers[32] = registers[32] + imm;
    // }
}

void ANDI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.R2_value & id_ex_pipeline.immediate;
    }
}

void ORI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.R2_value | id_ex_pipeline.immediate;
    }
}

void J()
{ //! TODO update it to match the new convention
  // if(addr<1024){
  //     registers[32] = addr;
  // }
  // else{
  //     printf("Invalid address");
  // }
}

void SLL()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.R2_value << id_ex_pipeline.shamt;
    }
}

void SRL()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
        ex_mem_pipeline.register_data = (int)((unsigned int)id_ex_pipeline.R2_value >> id_ex_pipeline.immediate);
    }
}
void LW()
{
    if (id_ex_pipeline.R1_address == 0)
    {
        ex_mem_pipeline.memoryWrite = 0;
        ex_mem_pipeline.registerWrite = 0;
        ex_mem_pipeline.memoryRead = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.memoryWrite = 0;
        ex_mem_pipeline.memoryRead = 1;
        ex_mem_pipeline.memory_address = id_ex_pipeline.R2_value + id_ex_pipeline.immediate;
        ex_mem_pipeline.register_address = id_ex_pipeline.R1_address;
    }
}

void SW()
{
    ex_mem_pipeline.memoryWrite = 1;
    ex_mem_pipeline.memoryRead = 0;
    ex_mem_pipeline.registerWrite = 0;
    ex_mem_pipeline.memory_address = id_ex_pipeline.R2_value + id_ex_pipeline.immediate;
    ex_mem_pipeline.memory_data = id_ex_pipeline.R1_address;
}

struct ALU_out ALU(int opcode, int operandA, int operandB)
{
    int output = 0;
    int zeroFlag = 0;
    switch (opcode)
    {
    case 0: // Add
        output = operandA + operandB;
        break;
    case 1: // Subtract
        output = operandA - operandB;
        break;
    case 2: // Multiply
        output = operandA * operandB;
        break;
    case 3: // Shift left logical
        output = operandA << operandB;
        break;
    case 4: // Shift right logical
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

void
instructionFetch()
{

    if (clockCycleCount % 2 == 1)
    {

        if (instructionCount > 0)
        {
            printf("Fetching instruction: %d\n", registers[32]);
            int instruction = memory[registers[32]];
            if_id_pipeline.instruction = instruction;

            if_id_pipeline.instruction_id = registers[32];
            registers[32] = registers[32] + 1;

            printf("Instruction %d fetched \n", if_id_pipeline.instruction_id);
        }
        if_id_pipeline.atu = 1;
        instructionCount--;
    }
    else
    {
        printf("No instruction fetched\n");
    }
}

void instructionDecode()
{
    printf("Decode found if_id = %d\n", if_id_pipeline.atu);
    if (if_id_pipeline.atu == 0)
    {
        printf("Instruction %d: Decoding\n", if_id_pipeline.instruction_id);
        printf("Instruction: %d\n", if_id_pipeline.instruction);
        int instruction = if_id_pipeline.instruction;
        id_ex_pipeline.opcode = (instruction >> 28) & 0b00000000000000000000000000001111;
        id_ex_pipeline.R1_address = (instruction >> 23) & 0b00000000000000000000000000011111;
        id_ex_pipeline.R2_address = (instruction >> 18) & 0b00000000000000000000000000011111;
        id_ex_pipeline.R3_address = (instruction >> 13) & 0b00000000000000000000000000011111;
        id_ex_pipeline.R2_value = registers[id_ex_pipeline.R2_address];
        id_ex_pipeline.R3_value = registers[id_ex_pipeline.R3_address];
        id_ex_pipeline.shamt = instruction & 0b00000000000000000001111111111111;
        printf("Shamt = %d\n", id_ex_pipeline.shamt);
        id_ex_pipeline.immediate = instruction & 0b00000000000000111111111111111111;
        printf("Immediate = %d\n", id_ex_pipeline.immediate);
        int sign = id_ex_pipeline.immediate >> 17;
        printf("Sign = %d\n", sign);
        if (sign == 1)
            id_ex_pipeline.immediate = id_ex_pipeline.immediate | 0b11111111111111000000000000000000;
        printf("Immediate = %d\n", id_ex_pipeline.immediate);
        id_ex_pipeline.address = instruction & 0b00001111111111111111111111111111;

        id_ex_pipeline.instruction_id = if_id_pipeline.instruction_id;

        // id_ex_pipeline.atu = 2;
        printf("Instruction %d decoded\n opcode = %d, R1 = %d, R2 = %d, R3 = %d, shamt = %d, immediate = %d, address = %d, R2_value = %d, R3_value = %d \n", id_ex_pipeline.instruction_id, id_ex_pipeline.opcode, id_ex_pipeline.R1_address, id_ex_pipeline.R2_address, id_ex_pipeline.R3_address, id_ex_pipeline.shamt, id_ex_pipeline.immediate, id_ex_pipeline.address, id_ex_pipeline.R2_value, id_ex_pipeline.R3_value);
    }
    else if (if_id_pipeline.atu == 1)
    {
        if (clockCycleCount > 2)
            id_ex_pipeline.atu = 1; // initial condition
        if (clockCycleCount % 2 == 1 && clockCycleCount > 2)
        {
            printf("Instruction %d: Still Decoding\n", (if_id_pipeline.instruction_id - 1));
        }
        else
        {
            printf("Decoded Nothing\n");
        }
    }
    if_id_pipeline.atu--;
    // instructionExecute(opcode,R1,R2,R3,shamt,immediate,address);
}

void instructionExecute()
{
    printf("idex %d \n", id_ex_pipeline.atu);
    if (id_ex_pipeline.atu == 0)
    {
        printf("Instruction %d: Executing\n", id_ex_pipeline.instruction_id);
        switch (id_ex_pipeline.opcode)
        {
        case 0:
            add();
            break;
        case 1:
            sub();
            break;
        case 2:
            mulI();
            break;
        case 3:
            addI();
            break;
        case 4:
            bne();
            break;
        case 5:
            ANDI();
            break;
        case 6:
            ORI();
            break;
        case 7:
            J();
            break;
        case 8:
            SLL();
            break;
        case 9:
            SRL();
            break;
        case 10:
            LW();
            break;
        case 11:
            SW();
            break;
        }
        // ex_mem_pipeline.atu = 2;
        ex_mem_pipeline.instruction_id = id_ex_pipeline.instruction_id;
    }
    else if (id_ex_pipeline.atu == 1)
    {
        if (clockCycleCount > 4)
            ex_mem_pipeline.atu = 1;
        if (clockCycleCount % 2 == 1 && id_ex_pipeline.atu == 1 && clockCycleCount > 4)
        {
            printf("Instruction %d: Still Executing\n", (id_ex_pipeline.instruction_id - 1));
        }
        else
        {
            printf("Execution did nothing\n");
        }
    }
    id_ex_pipeline.atu--;
}

void instructionMemoryAccess()
{
    printf("exmem atu %d\n", ex_mem_pipeline.atu);
    if (ex_mem_pipeline.atu == 0)
    {
        printf("Instruction %d: Memory Access\n", ex_mem_pipeline.instruction_id);
        if (ex_mem_pipeline.prev_memoryWrite == 1)
        {
            memory[ex_mem_pipeline.prev_memory_address] = ex_mem_pipeline.prev_memory_data;
        }
        else if (ex_mem_pipeline.prev_memoryRead == 1)
        {
            ex_mem_pipeline.prev_register_data = memory[ex_mem_pipeline.prev_memory_address];
        }
        mem_wb_pipeline.prev_register_address = ex_mem_pipeline.prev_register_address;
        mem_wb_pipeline.prev_registerWrite = ex_mem_pipeline.prev_registerWrite;
        mem_wb_pipeline.prev_register_data = ex_mem_pipeline.prev_register_data;

        mem_wb_pipeline.atu = 1;
        mem_wb_pipeline.instruction_id = ex_mem_pipeline.instruction_id;
    }
    else
    {
        printf("No Memory Access for this clock cycle\n");
    }
    ex_mem_pipeline.atu--;
}

void instructionRegisterWriteBack()
{

    if (mem_wb_pipeline.atu == 0)
    {
        printf("Instruction %d: Writing Back in Register %d\n", mem_wb_pipeline.instruction_id, mem_wb_pipeline.register_address);
        printf("Register Data = %d\n", mem_wb_pipeline.prev_register_data);
        printf("Register Write = %d\n", mem_wb_pipeline.prev_registerWrite);
        if (mem_wb_pipeline.prev_registerWrite == 1)
        {
            registers[mem_wb_pipeline.prev_register_address] = mem_wb_pipeline.prev_register_data;
        }
    }
    else
    {

        printf("No Register Write Back for this clock cycle\n");
    }
    mem_wb_pipeline.atu--;
}

void switchPrevNew()
{
    ex_mem_pipeline.prev_memory_address = ex_mem_pipeline.memory_address;
    ex_mem_pipeline.prev_memory_data = ex_mem_pipeline.memory_data;
    ex_mem_pipeline
        .prev_memoryRead = ex_mem_pipeline.memoryRead;
    ex_mem_pipeline.prev_memoryWrite = ex_mem_pipeline.memoryWrite;
    ex_mem_pipeline.prev_register_address = ex_mem_pipeline.register_address;
    ex_mem_pipeline.prev_register_data = ex_mem_pipeline.register_data;
    ex_mem_pipeline.prev_registerWrite = ex_mem_pipeline.registerWrite;

    mem_wb_pipeline.prev_register_address = mem_wb_pipeline.register_address;
    mem_wb_pipeline.prev_register_data = mem_wb_pipeline.register_data;
    mem_wb_pipeline.prev_registerWrite = mem_wb_pipeline.registerWrite;

    id_ex_pipeline.prev_opcode = id_ex_pipeline.opcode;
    id_ex_pipeline.prev_R1_address = id_ex_pipeline.R1_address;
    id_ex_pipeline.prev_R2_address = id_ex_pipeline.R2_address;
    id_ex_pipeline.prev_R3_address = id_ex_pipeline.R3_address;
    id_ex_pipeline.prev_R2_value = id_ex_pipeline.R2_value;
    id_ex_pipeline.prev_R3_value = id_ex_pipeline.R3_value;
    id_ex_pipeline.prev_shamt = id_ex_pipeline.shamt;
    id_ex_pipeline.prev_immediate = id_ex_pipeline.immediate;
    id_ex_pipeline.prev_address = id_ex_pipeline.address;

}

int main()
{
    init();
    printf("Instruction Count: %d\n", instructionCount);
    while (1)
    {
        printf("all atus: %d %d %d %d\n", if_id_pipeline.atu, id_ex_pipeline.atu, ex_mem_pipeline.atu, mem_wb_pipeline.atu);
        printf("--------------\n");
        printf("Clock Cycle: %d\n", clockCycleCount);
        instructionFetch();
        instructionDecode();
        instructionExecute();
        instructionMemoryAccess();
        instructionRegisterWriteBack();
        switchPrevNew();
        clockCycleCount++;
        printf("--------------------\n");
        if (clockCycleCount == 10)
        {
            break;
        }
        printf("---------------\n");
        printf("all atus: %d %d %d %d\n", if_id_pipeline.atu, id_ex_pipeline.atu, ex_mem_pipeline.atu, mem_wb_pipeline.atu);
    }
    printf("Final State of Registers and Memory-----\n");
    for (int i = 0; i < 5; i++)
    {
        printf("Register %d: %d\n", i, registers[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        printf("Memory %d: %d\n", i, memory[i]);
    }
    return 0;
}
