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
    0 => Zero Register
    1-31 => GPRs
    32 => Program Counter
*/
int registers[33];
int clockCycleCount;
static int instructionCount = 0;




struct IF_ID
{
    int instruction;
    int atu;

    int prev_instruction;


    unsigned int instruction_id;
    unsigned int prev_instruction_id;
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
    unsigned int prev_instruction_id;
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
    unsigned int prev_instruction_id;

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
    unsigned int prev_instruction_id;

};

struct IF_ID if_id_pipeline;
struct ID_EX id_ex_pipeline;
struct EX_MEM ex_mem_pipeline;
struct MEM_WB mem_wb_pipeline;

int pcChange = 0;

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
            int regNum1 = atoi(reg1);
            int regNum2 = atoi(reg2);
            if (strcmp(instructionName, "ADD") == 0 || strcmp(instructionName, "SUB") == 0)
            {
                char *reg3 = strchr(words[3], 'R');
                if (reg3 != NULL)
                {
                    memmove(reg3, reg3 + 1, strlen(reg3));
                }
                int regNum3 = atoi(reg3);
                instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000) | ((regNum2 << 18) & 0b00000000011111000000000000000000) | ((regNum3 << 13) & 0b00000000000000111110000000000000);
            }
            else if (strcmp(instructionName, "SRL") == 0 || strcmp(instructionName, "SLL") == 0)
            {
                int shamt = atoi(words[3]);
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
            char *immediate = words[3];
            int regNum1 = atoi(reg1);
            int regNum2 = atoi(reg2);
            int imm = atoi(immediate);
            instruction = instruction | ((regNum1 << 23) & 0b00001111100000000000000000000000) | ((regNum2 << 18) & 0b00000000011111000000000000000000) | (imm & 0b00000000000000111111111111111111);
        }
        else if (rORiORj == 2)
        {
            char *address = words[1];
            int addr = atoi(address);
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
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value + id_ex_pipeline.prev_R3_value;
    }
}

void sub()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value - id_ex_pipeline.prev_R3_value;
    }
}

void mulI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value * id_ex_pipeline.prev_immediate;
    }
}

void addI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value + id_ex_pipeline.prev_immediate;
    }
}

void bne()
{
    if(id_ex_pipeline.prev_R2_value != id_ex_pipeline.prev_R3_value){
        int currentPCAddr = id_ex_pipeline.prev_instruction_id;
        int newAddr = currentPCAddr + 1 + id_ex_pipeline.prev_immediate;
        ex_mem_pipeline.register_data = newAddr;
        pcChange = 1;
        ex_mem_pipeline.memory_address = 0;
        ex_mem_pipeline.memory_data = 0;
        ex_mem_pipeline.memoryRead = 0;
        ex_mem_pipeline.memoryWrite = 0;
        ex_mem_pipeline.registerWrite = 0;
        ex_mem_pipeline.register_address = 0;
    }
    else{
        pcChange = 0;
        ex_mem_pipeline.memory_address = 0;
        ex_mem_pipeline.memory_data = 0;
        ex_mem_pipeline.memoryRead = 0;
        ex_mem_pipeline.memoryWrite = 0;
        ex_mem_pipeline.registerWrite = 0;
        ex_mem_pipeline.register_address = 0;
        ex_mem_pipeline.register_data = 0;
    }
}

void ANDI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value & id_ex_pipeline.prev_immediate;
    }
}

void ORI()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value | id_ex_pipeline.prev_immediate;
    }
}

void J()
{
    int currentPCAddr = id_ex_pipeline.prev_instruction_id;
    int newAddr = (currentPCAddr & 0b11110000000000000000000000000000) | (id_ex_pipeline.prev_address & 0b00001111111111111111111111111111);
    ex_mem_pipeline.register_data = newAddr;
    pcChange = 1;
    ex_mem_pipeline.memory_address = 0;
    ex_mem_pipeline.memory_data = 0;
    ex_mem_pipeline.memoryRead = 0;
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.registerWrite = 0;
    ex_mem_pipeline.register_address = 0;
}

void SLL()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = id_ex_pipeline.prev_R2_value << id_ex_pipeline.prev_shamt;
    }
}

void SRL()
{
    ex_mem_pipeline.memoryWrite = 0;
    ex_mem_pipeline.memoryRead = 0;
    if (id_ex_pipeline.prev_R1_address == 0)
    {
        ex_mem_pipeline.registerWrite = 0;
    }
    else
    {
        ex_mem_pipeline.registerWrite = 1;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
        ex_mem_pipeline.register_data = (int)((unsigned int)id_ex_pipeline.prev_R2_value >> id_ex_pipeline.prev_immediate);
    }
}

void LW()
{
    if (id_ex_pipeline.prev_R1_address == 0)
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
        ex_mem_pipeline.memory_address = id_ex_pipeline.prev_R2_value + id_ex_pipeline.prev_immediate;
        ex_mem_pipeline.register_address = id_ex_pipeline.prev_R1_address;
    }
}

void SW()
{
    ex_mem_pipeline.memoryWrite = 1;
    ex_mem_pipeline.memoryRead = 0;
    ex_mem_pipeline.registerWrite = 0;
    ex_mem_pipeline.memory_address = id_ex_pipeline.prev_R2_value + id_ex_pipeline.prev_immediate;
    ex_mem_pipeline.memory_data = id_ex_pipeline.prev_R3_value;
}

int noMoreFetch = 0;
int noMoreDecode = 0;
int noMoreExecute = 0;
int noMoreMemoryAccess = 0;
int noMoreWriteBack = 0;

int clockCycleAdd = 0;




void instructionFetch()
{

    if(noMoreFetch == 0){
        if (clockCycleCount % 2 == 1)
        {
            if (if_id_pipeline.instruction_id < instructionCount)
            {
                printf("Fetching instruction: %d\n", registers[32]+1);
                int instruction = memory[registers[32]];
                if_id_pipeline.instruction = instruction;

                if_id_pipeline.instruction_id = registers[32];
                registers[32] = registers[32] + 1;

                printf("Instruction %d fetched \n", if_id_pipeline.instruction_id+1);
            }
            if(if_id_pipeline.instruction_id == instructionCount-1){
                noMoreFetch = 1;
            }
            if_id_pipeline.atu = 1;
        }
        else
        {
            printf("No instruction fetched\n");
        }

    }
    else{
        printf("No more instructions to fetch\n");
    }
}

void instructionDecode()
{
    if(noMoreDecode == 0){
        if (if_id_pipeline.atu == 0 )
        {   
            printf("Instruction %d: Decoding\n", if_id_pipeline.prev_instruction_id+1);
            printf("Instruction: %d\n", if_id_pipeline.prev_instruction);
            int instruction = if_id_pipeline.prev_instruction;
            id_ex_pipeline.opcode = (instruction >> 28) & 0b00000000000000000000000000001111;
            id_ex_pipeline.R1_address = (instruction >> 23) & 0b00000000000000000000000000011111;
            id_ex_pipeline.R2_address = (instruction >> 18) & 0b00000000000000000000000000011111;
            id_ex_pipeline.R3_address = (instruction >> 13) & 0b00000000000000000000000000011111;
            if(id_ex_pipeline.opcode == 4){
                id_ex_pipeline.R2_value = registers[id_ex_pipeline.R2_address];
                id_ex_pipeline.R3_value = registers[id_ex_pipeline.R1_address];
            }
            else if(id_ex_pipeline.opcode == 11){
                id_ex_pipeline.R3_value = registers[id_ex_pipeline.R1_address];
            }
            else{
                id_ex_pipeline.R2_value = registers[id_ex_pipeline.R2_address];
                id_ex_pipeline.R3_value = registers[id_ex_pipeline.R3_address];
            }

            id_ex_pipeline.shamt = instruction & 0b00000000000000000001111111111111;
            id_ex_pipeline.immediate = instruction & 0b00000000000000111111111111111111;
            int sign = id_ex_pipeline.immediate >> 17;
            if (sign == 1)
                id_ex_pipeline.immediate = id_ex_pipeline.immediate | 0b11111111111111000000000000000000;
            id_ex_pipeline.address = instruction & 0b00001111111111111111111111111111;
        }
        else if ((if_id_pipeline.atu == 1) || (noMoreFetch == 1))
        {   

            if (clockCycleCount > (2 + clockCycleAdd))
                id_ex_pipeline.atu = 1;
            if (clockCycleCount % 2 == 1 && clockCycleCount > (2 + clockCycleAdd))
            {
                printf("Instruction %d: Still Decoding\n", (if_id_pipeline.prev_instruction_id+1));
                id_ex_pipeline.instruction_id = if_id_pipeline.prev_instruction_id;
                if(id_ex_pipeline.instruction_id == instructionCount -1){
                    noMoreDecode = 1;
                }
            }
            else
            {
                printf("No instructions decoded\n");
            }

        }
        if_id_pipeline.atu--;
    }
    else {
        if(noMoreExecute == 0 && id_ex_pipeline.atu  < 0 ){
            id_ex_pipeline.atu = 1;
        }
        printf("No more instructions to decode\n");
    }

}

void handleDataHazard(){

    if(id_ex_pipeline.opcode == 4)
    {
        if(id_ex_pipeline.prev_R1_address == ex_mem_pipeline.prev_register_address && ex_mem_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R3_value = ex_mem_pipeline.prev_register_data;
        }
        else if(id_ex_pipeline.prev_R2_address == ex_mem_pipeline.prev_register_address && ex_mem_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R2_value = ex_mem_pipeline.prev_register_data;
        }
        if(id_ex_pipeline.prev_R1_address == mem_wb_pipeline.prev_register_address && mem_wb_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R3_value = mem_wb_pipeline.prev_register_data;
        }
        else if(id_ex_pipeline.prev_R2_address == mem_wb_pipeline.prev_register_address && mem_wb_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R2_value = mem_wb_pipeline.prev_register_data;
        }
    }
    else{
        if(id_ex_pipeline.prev_R3_address == ex_mem_pipeline.prev_register_address && ex_mem_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R3_value = ex_mem_pipeline.prev_register_data;
        }
        else if(id_ex_pipeline.prev_R2_address == ex_mem_pipeline.prev_register_address && ex_mem_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R2_value = ex_mem_pipeline.prev_register_data;
        }
        if(id_ex_pipeline.prev_R3_address == mem_wb_pipeline.prev_register_address && mem_wb_pipeline.prev_register_address!= ex_mem_pipeline.prev_register_address && mem_wb_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R3_value = mem_wb_pipeline.prev_register_data;
        }
        else if(id_ex_pipeline.prev_R2_address == mem_wb_pipeline.prev_register_address && mem_wb_pipeline.prev_register_address!= ex_mem_pipeline.prev_register_address && mem_wb_pipeline.prev_registerWrite == 1){
            id_ex_pipeline.prev_R2_value = mem_wb_pipeline.prev_register_data;
        }
        if(id_ex_pipeline.prev_R1_address == ex_mem_pipeline.prev_register_address && ex_mem_pipeline.prev_registerWrite == 1 && id_ex_pipeline.prev_opcode==11){
            id_ex_pipeline.prev_R3_value = ex_mem_pipeline.prev_register_data;
        }
        if(id_ex_pipeline.prev_R1_address == mem_wb_pipeline.prev_register_address && mem_wb_pipeline.prev_registerWrite == 1 && id_ex_pipeline.prev_opcode==11){
            id_ex_pipeline.prev_R3_value = mem_wb_pipeline.prev_register_data;
        }
    }
}


void instructionExecute()
{
    if(noMoreExecute == 0)
    {
        if (id_ex_pipeline.atu == 0)
        {
            printf("Instruction %d: Executing\n", id_ex_pipeline.prev_instruction_id+1);
            handleDataHazard();
            switch (id_ex_pipeline.prev_opcode)
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
        }
        else if (id_ex_pipeline.atu == 1)
        {
            if (clockCycleCount > (4 + clockCycleAdd))
                ex_mem_pipeline.atu = 1;
            if (clockCycleCount % 2 == 1 && ((id_ex_pipeline.atu == 1) || (noMoreDecode == 1)) && clockCycleCount > (4 + clockCycleAdd))
            {
                ex_mem_pipeline.instruction_id = id_ex_pipeline.prev_instruction_id;
                printf("Instruction %d: Still Executing\n", (id_ex_pipeline.prev_instruction_id+1));
                if(ex_mem_pipeline.instruction_id == instructionCount -1){
                    noMoreExecute = 1;
                }
                if(pcChange == 1){
                    registers[32] = ex_mem_pipeline.register_data;
                    if_id_pipeline.atu = 10;
                    id_ex_pipeline.atu = 10;

                    clockCycleAdd = clockCycleCount + 1;

                    if_id_pipeline.instruction = 0;
                    if_id_pipeline.instruction_id = 0;
                    if_id_pipeline.prev_instruction = 0;
                    if_id_pipeline.prev_instruction_id = 0;

                    id_ex_pipeline.opcode = 0;
                    id_ex_pipeline.R1_address = 0;
                    id_ex_pipeline.R2_address = 0;
                    id_ex_pipeline.R3_address = 0;
                    id_ex_pipeline.R2_value = 0;
                    id_ex_pipeline.R3_value = 0;
                    id_ex_pipeline.shamt = 0;
                    id_ex_pipeline.immediate = 0;
                    id_ex_pipeline.address = 0;
                    id_ex_pipeline.prev_opcode = 0;
                    id_ex_pipeline.prev_R1_address = 0;
                    id_ex_pipeline.prev_R2_address = 0;
                    id_ex_pipeline.prev_R3_address = 0;
                    id_ex_pipeline.prev_R2_value = 0;
                    id_ex_pipeline.prev_R3_value = 0;
                    id_ex_pipeline.prev_shamt = 0;
                    id_ex_pipeline.prev_immediate = 0;
                    id_ex_pipeline.prev_address = 0;
                }
            }
            else
            {
                printf("No Execution for this clock cycle\n");
            }
        }
        else{
            printf("No Execution for this clock cycle\n");
        }
        id_ex_pipeline.atu--;
    }
    else{
        printf("No more instructions to execute\n");
    }
}

void instructionMemoryAccess()
{
    if(noMoreMemoryAccess == 0){
        if (ex_mem_pipeline.atu == 0)
        {
            printf("Instruction %d: Memory Access\n", ex_mem_pipeline.prev_instruction_id+1);
            if (ex_mem_pipeline.prev_memoryWrite == 1)
            {
                printf("Writing %d in memory address %d\n",ex_mem_pipeline.prev_memory_data,ex_mem_pipeline.prev_memory_address);
                memory[ex_mem_pipeline.prev_memory_address] = ex_mem_pipeline.prev_memory_data;
            }
            else if (ex_mem_pipeline.prev_memoryRead == 1)
            {
                
                ex_mem_pipeline.prev_register_data = memory[ex_mem_pipeline.prev_memory_address];
                printf("Reading %d from memory address %d\n",ex_mem_pipeline.prev_register_data,ex_mem_pipeline.prev_memory_address);
            }
            mem_wb_pipeline.register_address = ex_mem_pipeline.prev_register_address;
            mem_wb_pipeline.registerWrite = ex_mem_pipeline.prev_registerWrite;
            mem_wb_pipeline.register_data = ex_mem_pipeline.prev_register_data;

            mem_wb_pipeline.atu = 1;
            mem_wb_pipeline.instruction_id = ex_mem_pipeline.prev_instruction_id;
            if(mem_wb_pipeline.instruction_id == instructionCount -1){
                noMoreMemoryAccess = 1;
            }
        }
        else
        {
            printf("No Memory Access for this clock cycle\n");
        }
        ex_mem_pipeline.atu--;
    }
    else{
        printf("No more memory access\n");
    }

}

void instructionRegisterWriteBack()
{
    if(noMoreWriteBack == 0){
        if (mem_wb_pipeline.atu == 0)
        {
            printf("Instruction %d: in Writing Back stage\n", mem_wb_pipeline.prev_instruction_id+1);
            if (mem_wb_pipeline.prev_registerWrite == 1)
            {
                printf("Instruction %d: Writing Back %d in Register %d\n", mem_wb_pipeline.prev_instruction_id+1,mem_wb_pipeline.prev_register_data ,mem_wb_pipeline.prev_register_address);
                registers[mem_wb_pipeline.prev_register_address] = mem_wb_pipeline.prev_register_data;
            }
            if(mem_wb_pipeline.prev_instruction_id == instructionCount - 1){
                noMoreWriteBack = 1;
            }
            if(pcChange == 1){
                ex_mem_pipeline.atu = 10;
                mem_wb_pipeline.atu = 10;
                pcChange = 0;
            }
        }
        else
        {
            printf("No Register Write Back for this clock cycle\n");
        }
        mem_wb_pipeline.atu--;
    }
    else{
        printf("No more register write back\n");
    }
}

void switchPrevNew()
{
    if_id_pipeline.prev_instruction = if_id_pipeline.instruction;
    if_id_pipeline.prev_instruction_id = if_id_pipeline.instruction_id;
    
    id_ex_pipeline.prev_opcode = id_ex_pipeline.opcode;
    id_ex_pipeline.prev_R1_address = id_ex_pipeline.R1_address;
    id_ex_pipeline.prev_R2_address = id_ex_pipeline.R2_address;
    id_ex_pipeline.prev_R3_address = id_ex_pipeline.R3_address;
    id_ex_pipeline.prev_R2_value = id_ex_pipeline.R2_value;
    id_ex_pipeline.prev_R3_value = id_ex_pipeline.R3_value;
    id_ex_pipeline.prev_shamt = id_ex_pipeline.shamt;
    id_ex_pipeline.prev_immediate = id_ex_pipeline.immediate;
    id_ex_pipeline.prev_address = id_ex_pipeline.address;
    id_ex_pipeline.prev_instruction_id = id_ex_pipeline.instruction_id;

    ex_mem_pipeline.prev_register_address = ex_mem_pipeline.register_address;
    ex_mem_pipeline.prev_register_data = ex_mem_pipeline.register_data;
    ex_mem_pipeline.prev_registerWrite = ex_mem_pipeline.registerWrite;
    ex_mem_pipeline.prev_memory_address = ex_mem_pipeline.memory_address;
    ex_mem_pipeline.prev_memory_data = ex_mem_pipeline.memory_data;
    ex_mem_pipeline.prev_memoryWrite = ex_mem_pipeline.memoryWrite;
    ex_mem_pipeline.prev_memoryRead = ex_mem_pipeline.memoryRead;
    ex_mem_pipeline.prev_instruction_id = ex_mem_pipeline.instruction_id;

    mem_wb_pipeline.prev_register_address = mem_wb_pipeline.register_address;
    mem_wb_pipeline.prev_register_data = mem_wb_pipeline.register_data;
    mem_wb_pipeline.prev_registerWrite = mem_wb_pipeline.registerWrite;
    mem_wb_pipeline.prev_instruction_id = mem_wb_pipeline.instruction_id;
}

void printMemoryAndRegisters() {
    int i,j;
    printf("--------------------\n");
    printf("Instruction Memory:\n");
    for (i = 0; i < 1024; i+=10) {
        for (j = 0; j < 10; j++) {
            if(i+j == 1024) break;
            printf("Cell %d: %d ", i+j ,memory[i + j]);
        }
        printf("\n");
    }

    printf("\nData Memory:\n");
    for (i = 1024; i < 2048; i+=10) {
        for (j = 0; j < 10; j++) {
            if(i+j == 2048) break;
            printf("Cell %d: %d ", i+j ,memory[i + j]);
        }
        printf("\n");
    }

    printf("\nRegister File:\n");
    for (i = 0; i < 32; i+=5) {
        for (j = 0; j < 5; j++) {
            if(i+j == 32) break;
            if(i+j == 0){
                printf("Zero Register: %d \n", registers[i]);
            }
            else
            printf("R%-2d: %d|", i+j, registers[i + j]);
        }
        printf("\n");
    }
    printf("Program Counter: %d\n", registers[32]);
    printf("--------------------\n");
}

void runProgram() {
    while(noMoreWriteBack == 0){   
        printf("Clock Cycle %d\n", clockCycleCount); 
        instructionFetch();
        instructionDecode();
        instructionExecute();
        instructionMemoryAccess();
        instructionRegisterWriteBack();
        switchPrevNew();
        printMemoryAndRegisters();
        clockCycleCount++;
    }
    printf("Final State of Registers and Memory-----\n");
    printMemoryAndRegisters();
}

int main()
{
    init();
    runProgram();
    return 0;
}