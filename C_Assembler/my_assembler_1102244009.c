


#define _CRT_SECURE_NO_WARNINGS    // Disable warnings for using strtok
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "my_assembler_1102244009.h"

void allocMem(token** tokens, int input_length, literal* literal_table[], symbol* symbol_table[]); // Allocate memory for token array
int isComma(char* buf);                                                 // Check if the buffer contains a comma
void RemoveFirst(char* buf);                                            // Remove the first character from the buffer
void RemoveEnd(char* buf);                                              // Remove the newline character from the buffer
// Processes token array, SYMTAB, and LITTAB in location_counter function
void location_counter(const inst* inst_table[], int inst_table_length, token* token, int tokens_length,
    int* memoryAddress, symbol* symbol, int* sym_index, literal* literal_table[], int literal_table_length, char* area);
// Converts operand to address in SYMTAB
int addrSymbol(const symbol* symbol_table[], int symbol_table_length, char* ptrName, const char* ptrArea);
void addLiteral(token* token, literal* literal_table[], int* literal_table_length);                // Add literal to LITTAB
int meetLTORG(token* tokens[], literal* literal_table[], int* literal_table_length, int* index);   // Handle LTORG directive and process literals
void binary_print(int a);                                               // Print nixbpe as binary
char numberRegister(char* operand);                                     // Convert register name to its corresponding number
int pcRelativeAddressing(const int pcValue, const char* operand, const char* area,  // Handle PC-relative addressing
    const symbol* symbol_table[], int symbol_table_length, const literal* literal_table[], int literal_table_length);
int opcodeTransfer(int a);                                              // Convert opcode from ASCII to hex
void equalLocator(token* tok, symbol* symbol_table[], int symbolIndex); // Handle EQU operator
int isTherePlusMinus(char* p);                                          // Check if operand contains + or -
int returnLiteralAddress(char* p, const symbol* symbol_table[], int symbol_table_length); // Convert literal to address
void makingObject(const token* tok, int tok_object, object_code* obj, int* obj_index,
    const symbol* symbol_table[], int symbol_table_length, int* JLT30,
    const inst* inst_table[], int inst_table_length, int* text_index,
    int* modify_index, int* loopIndex, int* EndOfToken);                // Create object program from token array
int isFourFormat(int a);                                                // Check if instruction is format 4
int isTwoFormat(int a);                                                 // Check if instruction is format 2




/**
 * @brief Main function to convert SIC/XE assembly code into object code.
 *
 * @details
 * This main function orchestrates the conversion of SIC/XE assembly code
 * into object code. The input file "input2.txt" is used as an example.
 */
int main(int argc, char** argv) {
    /** SIC/XE instruction table */
    inst* inst_table[MAX_INST_TABLE_LENGTH];
    int inst_table_length;

    /** SIC/XE input lines */
    char* input[MAX_INPUT_LINES];
    int input_length;

    /** Token array generated from the input lines */
    token* tokens[MAX_INPUT_LINES];
    int tokens_length;

    /** Symbol table for storing labels and their addresses */
    symbol* symbol_table[MAX_TABLE_LENGTH];
    int symbol_table_length;

    /** Literal table for storing literals and their addresses */
    literal* literal_table[MAX_TABLE_LENGTH];
    int literal_table_length;

    /** Object code for the assembled program */
    object_code* obj_code = (object_code*)calloc(1, sizeof(object_code));

    int err = 0;

    if ((err = init_inst_table(inst_table, &inst_table_length,
        "inst_table.txt")) < 0) {
        fprintf(stderr,
            "init_inst_table: Error initializing instruction table. "
            "(error_code: %d)\n",
            err);
        return -1;
    }



    if ((err = init_input(input, &input_length, "input2.txt")) < 0) {
        fprintf(stderr,
            "init_input: Error reading input file. (error_code: %d)\n",
            err);
        return -1;
    }

    if ((err = assem_pass1((const inst**)inst_table, inst_table_length,
        (const char**)input, input_length, tokens,
        &tokens_length, symbol_table, &symbol_table_length,
        literal_table, &literal_table_length)) < 0) {
        fprintf(stderr,
            "assem_pass1: Error in pass 1. (error_code: %d)\n",
            err);
        return -1;
    }

    if ((err = make_symbol_table_output("output_symtab.txt",
        (const symbol**)symbol_table,
        symbol_table_length)) < 0) {
        fprintf(stderr,
            "make_symbol_table_output: Error writing symbol table. (error_code: %d)\n",
            err);
        return -1;
    }

    printf("\n");

    if ((err = make_literal_table_output("output_littab.txt",
        (const literal**)literal_table,
        literal_table_length)) < 0) {
        fprintf(stderr,
            "make_literal_table_output: Error writing literal table. (error_code: %d)\n",
            err);
        return -1;
    }

    printf("\n");

    if ((err = assem_pass2((const token**)tokens, tokens_length,
        (const inst**)inst_table, inst_table_length,
        (const symbol**)symbol_table, symbol_table_length,
        (const literal**)literal_table,
        literal_table_length, obj_code)) < 0) {
        fprintf(stderr,
            "assem_pass2: Error in pass 2. (error_code: %d)\n",
            err);
        return -1;
    }

    if ((err = make_objectcode_output("output_objectcode.txt",
        (const object_code*)obj_code)) < 0) {
        fprintf(stderr,
            "make_objectcode_output: Error writing object code. (error_code: %d)\n",
            err);
        return -1;
    }

    for (int i = 0; i < input_length + 50; i++) {
        free(tokens[i]);
    }

    return 0;
}
/**
 * @brief Reads the instruction table from a file and initializes the inst_table array.
 *
 * @param inst_table Pointer to the instruction table array
 * @param inst_table_length Pointer to the length of the instruction table
 * @param inst_table_dir Directory of the instruction table file
 * @return Error code (0 if successful)
 *
 * @details
 * This function reads the instruction table from a file and initializes the inst_table array.
 */

int init_inst_table(inst* inst_table[], int* inst_table_length,
    const char* inst_table_dir) {
    FILE* fp;
    int err;

    err = 0;                              // Initialize error to 0
    fp = fopen(inst_table_dir, "r");      // Open the file specified by inst_table_dir

    if (fp == NULL) {                     // If the file failed to open
        printf("failed to open file\n");
        err = -1;                         // Set error to -1
        return err;
    }

    *inst_table_length = 0;               // Initialize instruction table length to 0

    char str[MAX_INPUT_LEN];              // Input buffer
    memset(str, 0, MAX_INPUT_LEN);        // Initialize input buffer

    char* ptr;

    while (fgets(str, MAX_INPUT_LEN, fp) != NULL) {                     
        inst_table[*inst_table_length] = calloc(1, sizeof(inst) + 1);                      
        ptr = strtok(str, "\t");                                        
        strcpy(inst_table[*inst_table_length]->str, ptr);               
        ptr = strtok(NULL, "\t");                                       
        strcpy(inst_table[*inst_table_length]->opcode, ptr);
        ptr = strtok(NULL, "\t");
        inst_table[*inst_table_length]->format = (*ptr) - 48;
        ptr = strtok(NULL, "\t");
        inst_table[*inst_table_length]->ops = (*ptr) - 48;
        (*inst_table_length)++;                                          
    }

    fclose(fp);

    return err;

    return 0;
}

/**
 * @brief Initializes the input instructions (input.txt) for SIC/XE.
 *
 * @param input Array to store input instructions.
 * @param input_length Variable to store the length of input instructions.
 * @param input_dir Directory of the input file.
 * @return Error code (0 on success)
 */
int init_input(char* input[], int* input_length, const char* input_dir) {
    FILE* fp;
    int err;

    err = 0;                        // Initialize error to 0
    fp = fopen(input_dir, "r");     // Open the file specified by input_dir

    if (fp == NULL) {               // If the file failed to open
        printf("failed to open file\n");
        err = -1;                   // Set error to -1
        return err;
    }

    *input_length = 0;              // Initialize input length to 0

    char str[MAX_INPUT_LEN];        // Input buffer
    memset(str, 0, MAX_INPUT_LEN);  // Initialize input buffer

    while (fgets(str, MAX_INPUT_LEN, fp) != NULL) {
        input[*input_length] = calloc(strlen(str) + 1, sizeof(char));    // Allocate memory for the input string
        strcpy(input[*input_length], str);                               // Copy the input string
        memset(str, 0, MAX_INPUT_LEN);                                   // Initialize input buffer
        (*input_length)++;                                               // Increment the input length
    }

    fclose(fp);

    return err;
}


/**
 * @brief This function performs pass 1 of the assembler process.
 *
 * @param inst_table Array of instruction table
 * @param inst_table_length Length of the instruction table
 * @param input Array of input instructions
 * @param input_length Length of the input instructions
 * @param tokens Array of tokens to be filled
 * @param tokens_length Length of the tokens array
 * @param symbol_table Array of symbol table
 * @param symbol_table_length Length of the symbol table
 * @param literal_table Array of literal table
 * @param literal_table_length Length of the literal table
 * @return Error code (0 on success)
 *
 * @details
 * This function performs pass 1 of the assembler process. In pass 1,
 * it parses the input instructions into tokens and processes literals
 * and symbols. It also calls the token_parsing function to parse each
 * input line into tokens. Additionally, assem_pass2 function later uses
 * the information from symbol and literal tables created in pass 1.
 */
int assem_pass1(const inst* inst_table[], int inst_table_length,
    const char* input[], int input_length, token* tokens[],
    int* tokens_length, symbol* symbol_table[],
    int* symbol_table_length, literal* literal_table[],
    int* literal_table_length) {

    int memoryAddress; // LOCCTR (Location Counter)
    int inputIndex;    // Input index
    int symbolIndex;   // Symbol table index
    char thisArea[8];  // Control section or area

    memoryAddress = 0;
    inputIndex = 0;
    symbolIndex = 0;
    *literal_table_length = 0;

    allocMem(tokens, input_length, literal_table, symbol_table); // Allocate memory for arrays

    for (int i = 0; i < input_length + 50; i++) {
        token_parsing(input[inputIndex++], tokens[i], inst_table, inst_table_length); // Parse input into tokens
        addLiteral(tokens[i], literal_table, literal_table_length);                   // Add literals to LITTAB

        if (meetLTORG(tokens, literal_table, literal_table_length, &i)) {             // If meet LTORG, process literals
            *tokens_length = ++i;                                                     // If END is encountered, set tokens length
            break;                                                                    // Exit the loop after processing tokens
        }
    }

    for (int i = 0; i < *tokens_length; i++) {                                        // Process tokens for LOCCTR, LITTAB, SYMTAB
        location_counter(inst_table, inst_table_length, tokens[i], *tokens_length, &memoryAddress,
            symbol_table[symbolIndex], &symbolIndex, literal_table, *literal_table_length, thisArea);

        equalLocator(tokens[i], symbol_table, symbolIndex);                           // Process EQU operator
    }

    *symbol_table_length = symbolIndex;                                               // Set symbol table length

    return 0;
}

// Process EQU operator
void equalLocator(token* tok, symbol* symbol_table[], int symbolIndex) {
    symbol* sym;
    char* ptr;
    char tmp[20];
    int num1, num2;
    int whichPM;

    if (!strcmp(tok->operator, "EQU") && strncmp(tok->operand[0], "*", 1)) {
        if (whichPM = isTherePlusMinus(tok->operand[0])) {  // Check if operand contains + or -
            sym = symbol_table[symbolIndex - 1];
            strcpy(tmp, tok->operand[0]);

            ptr = strtok(tmp, "+-");                        // Tokenize operand by + or -
            for (int i = 0; i < symbolIndex - 1; i++) {
                if (!strcmp(symbol_table[i]->name, ptr)) {
                    num1 = symbol_table[i]->addr;           // Get address of the symbol
                    break;
                }
            }

            ptr = strtok(NULL, "+-");                       // Tokenize operand by + or -
            for (int i = 0; i < symbolIndex - 1; i++) {
                if (!strcmp(symbol_table[i]->name, ptr)) {
                    num2 = symbol_table[i]->addr;           // Get address of the symbol
                    break;
                }
            }

            if (whichPM == 1) {                             // If operand contains +
                sym->addr = num1 + num2;
            } else {                                        // If operand contains -
                sym->addr = num1 - num2;
            }
        }
    }
}



/**
 * @brief This function parses a single input instruction line into tokens.
 *
 * @param input Input line of assembly instruction
 * @param tok Structure to store parsed tokens
 * @param inst_table Array of instruction table
 * @param inst_table_length Length of the instruction table
 * @return Error code (0 on success)
 */
int token_parsing(const char* input, token* tok, const inst* inst_table[],
    int inst_table_length) {
    char* ptr;
    char str[100];
    int inst_index;
    char tmp[50];       // Temporary variable for storing intermediate strings

    strcpy(str, input);

    if (str[0] == '.') {                        // If the line starts with '.', it's a comment
        strcpy(tok->label, ".");                // Set the label to '.'

        ptr = strtok(str, "\t");
        ptr = strtok(NULL, "\t");
        if (ptr == NULL)
            strcpy(tok->operator, " ");
        else {
            RemoveEnd(ptr);
            strcpy(tok->operator, ptr);
        }

        strcpy(tok->operand[0], " ");
        strcpy(tok->operand[1], " ");
        strcpy(tok->operand[2], " ");

        strcpy(tok->comment, " ");

        return 0;                               // Return successfully for comment lines
    }

    if (str[0] == '\t') {                       // If the line starts with '\t', there is no label
        strcpy(tok->label, " ");                // Set the label to a space

        ptr = strtok(str, "\t");
    }
    else {                                      // If there is a label
        ptr = strtok(str, "\t");                // Extract the label
        strcpy(tok->label, ptr);

        ptr = strtok(NULL, "\t");
    }

    RemoveEnd(ptr);
    strcpy(tok->operator, ptr);
    // If the operator is an instruction in the instruction table
    if ((inst_index = search_opcode(tok->operator, inst_table, inst_table_length)) > -1) {
        tok->inst_index = inst_index;           // Set the OPCODE index
        if (inst_table[inst_index]->ops == 0) { // If there is no operand, set it to a space
            strcpy(tok->operand[0], " ");
        }
        else {                                  // If there are operands, extract them
            ptr = strtok(NULL, "\t");
            RemoveEnd(ptr);
            strcpy(tok->operand[0], ptr);
        }
    }
    else {                                      // If the operator is not an instruction
        tok->inst_index = -1;                   // Set the OPCODE index to -1
        ptr = strtok(NULL, "\t");               // Extract the operand

        if (ptr == NULL) {
            strcpy(tok->operand[0], " ");
        }
        else {
            RemoveEnd(ptr);
            strcpy(tok->operand[0], ptr);
        }
    }

    ptr = strtok(NULL, "\t");                   // Extract the comment

    if (ptr == NULL) {
        strcpy(tok->comment, " ");
    }
    else {
        RemoveEnd(ptr);
        strcpy(tok->comment, ptr);
    }

    strcpy(tok->operand[1], " ");
    strcpy(tok->operand[2], " ");

    strcpy(tmp, tok->operand[0]);

    if (isComma(tmp)) {
        ptr = strtok(tmp, ",");
        strcpy(tok->operand[0], ptr);

        ptr = strtok(NULL, ",");
        if (ptr != NULL) {
            strcpy(tok->operand[1], ptr);

            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                strcpy(tok->operand[2], ptr);
        }
    }

    return 0;
}


/**
 * @brief Searches for the given string in the instruction table and returns the index if found.
 *
 * @param str The string to search for
 * @param inst_table Array of instruction table
 * @param inst_table_length Length of the instruction table
 * @return The index of the instruction in the table (returns -1 if not found)
 *
 * @details
 * Searches for the given string in the instruction table and returns the index if found.
 * If the string starts with special characters like '+', '#', or '@', it removes them before searching.
 * For example, '+JSUB' is treated as 'JSUB' for the search.
 */
int search_opcode(const char* str, const inst* inst_table[],
    int inst_table_length) {
    if (str == NULL)
        return -1;

    char strChar[100];
    strcpy(strChar, str);   // Copy str to a char array

    if ((strChar[0] == '+') || (strChar[0] == '#') || (strChar[0] == '@')) {    // If the string starts with +, #, @
        RemoveFirst(strChar);
        for (int i = 0; i < inst_table_length; i++) {
            if (!strcmp(strChar, inst_table[i]->str)) {                         // Compare the operator with inst_table
                return i;
            }
        }
    } else {
        for (int i = 0; i < inst_table_length; i++) {                           // Compare the operator with inst_table
            if (!strcmp(strChar, inst_table[i]->str)) {
                return i;
            }
        }
    }
    return -1;
}

/**
 * @brief Generates the output file for the parsed OPCODE instructions. If `output_dir` is NULL,
 * the output is printed to stdout. The output is created for pass 1 of the assembler.
 *
 * @param output_dir Directory to save the output file, or NULL
 * @param tokens Array of parsed tokens
 * @param tokens_length Length of the tokens array
 * @param inst_table Array of instruction table
 * @param inst_table_length Length of the instruction table
 * @return Error code (0 on success)
 *
 * @details
 * Generates the output file for the parsed OPCODE instructions. If `output_dir` is NULL,
 * the output is printed to stdout. The output is for pass 1 of the assembler process.
 */
int make_opcode_output(const char* output_dir, const token* tokens[],
    int tokens_length, const inst* inst_table[],
    int inst_table_length) {
    FILE* fp = NULL;
    if (output_dir != NULL) {
        fp = fopen(output_dir, "w");
        if (fp == NULL) {
            printf("failed to open output file\n");
            return -1;
        }
    } else {
        fp = stdout;
    }

    for (int i = 0; i < tokens_length; i++) {
        const token* tok = tokens[i];
        if (tok->inst_index != -1) {
            fprintf(fp, "Instruction: %s, Opcode: %s\n", tok->label, inst_table[tok->inst_index]->opcode);
        } else {
            fprintf(fp, "Instruction: %s, Operator: %s\n", tok->label, tok->operator);
        }
    }

    if (output_dir != NULL) {
        fclose(fp);
    }

    return 0;
}

/**
 * @brief This function performs pass 2 of the assembler process.
 *
 * @param tokens Array of tokens
 * @param tokens_length Length of the tokens array
 * @param inst_table Array of instruction table
 * @param inst_table_length Length of the instruction table
 * @param symbol_table Array of symbol table
 * @param symbol_table_length Length of the symbol table
 * @param literal_table Array of literal table
 * @param literal_table_length Length of the literal table
 * @param obj_code Structure to store the generated object code
 * @return Error code (0 on success)
 *
 * @details
 * This function performs pass 2 of the assembler process. In pass 2,
 * it generates the object code based on the tokens parsed in pass 1.
 */
int assem_pass2(const token* tokens[], int tokens_length,
    const inst* inst_table[], int inst_table_length,
    const symbol* symbol_table[], int symbol_table_length,
    const literal* literal_table[], int literal_table_length,
    object_code* obj_code) {

    char op[3];             // Opcode from inst_table
    char nixbpe;            // nixbpe field from tokens[i]
    int inst_index;         // Index in the instruction table
    int opIndex;            // Opcode index

    opIndex = 0;

    for (int i = 0; i < tokens_length; i++) {                   // Iterate through the tokens to generate opcode
        inst_index = tokens[i]->inst_index;
        if (inst_index > -1) {                                  // If the operator exists in the instruction table
            if (tokens[i]->nixbpe) {                            // If nixbpe is set

                memset(op, 0, 3);
                strcpy(op, inst_table[inst_index]->opcode);     // Copy the opcode
                nixbpe = tokens[i]->nixbpe;                     // Copy the nixbpe field

                if (inst_table[inst_index]->format == 3 && tokens[i]->operator[0] != '+') {      // Format 3
                    if (inst_table[inst_index]->ops == 0) {            // If no operand, set object code with opcode and nixbpe
                        obj_code->objectCode[opIndex++] = (opcodeTransfer(inst_table[inst_index]->opcode[0]) << 20)
                            | (opcodeTransfer(inst_table[inst_index]->opcode[1]) << 16) | (tokens[i]->nixbpe << 12);
                    }
                    else if (inst_table[inst_index]->ops == 1) {       // If there is one operand
                        // Calculate pc Relative addressing
                        int relative = pcRelativeAddressing(tokens[i + 1]->location, tokens[i]->operand[0], tokens[i]->area,
                            symbol_table, symbol_table_length, literal_table, literal_table_length);
                        if (relative == -999) {                        // If pc relative error
                            printf("pc relative error\n");
                            obj_code->objectCode[opIndex++] = 0b11111111;

                            continue;
                        }
                        // Set object code with opcode, nixbpe and relative address
                        obj_code->objectCode[opIndex++] = (opcodeTransfer(inst_table[inst_index]->opcode[0]) << 20)
                            | (opcodeTransfer(inst_table[inst_index]->opcode[1]) << 16) | (tokens[i]->nixbpe << 12)
                            | relative;
                    }
                }
                else {  // Format 4
                    obj_code->objectCode[opIndex++] = (opcodeTransfer(inst_table[inst_index]->opcode[0]) << 28)
                        | (opcodeTransfer(inst_table[inst_index]->opcode[1]) << 24) | (tokens[i]->nixbpe << 20);
                }
            }
            else {                                                    // If nixbpe is not set
                if (inst_table[inst_index]->format == 2) {            // Format 2

                    char r1 = numberRegister(tokens[i]->operand[0]);

                    if (inst_table[inst_index]->ops == 1) {           // If one operand
                        obj_code->objectCode[opIndex++] = (opcodeTransfer(inst_table[inst_index]->opcode[0]) << 12)
                            | (opcodeTransfer(inst_table[inst_index]->opcode[1]) << 8) | (r1 << 4);
                    }
                    else if (inst_table[inst_index]->ops == 2) {      // If two operands
                        char r2 = numberRegister(tokens[i]->operand[1]);

                        obj_code->objectCode[opIndex++] = (opcodeTransfer(inst_table[inst_index]->opcode[0]) << 12)
                            | (opcodeTransfer(inst_table[inst_index]->opcode[1]) << 8) | (r1 << 4) | (r2);
                    }
                }
                else {                                               // Else, set object code to -1
                    obj_code->objectCode[opIndex++] = -1;
                }
            }
        }
        else {                                                      // If the operator does not exist in the instruction table
            // Handle cases for literals, BYTE, and WORD
            if (!strncmp(tokens[i]->label, "*", 1)) {               // Handle literal opcodes
                char* ptr;
                char tmp[10];

                strcpy(tmp, tokens[i]->operator);                   // Copy operator to temporary string

                if (isdigit(tmp[1])) {
                    ptr = tmp + 1;
                }
                else {
                    ptr = strtok(tmp, "'");
                    ptr = strtok(NULL, "'");
                }

                strcpy(tmp, ptr);
                if (!strncmp(tokens[i]->operator, "=X", 2)) {       // Handle =X'05'
                    char a, b;
                    int c;

                    a = (tmp[0] - 57) > 0 ? tmp[0] - 55 : tmp[0] - 48;  // Convert hex to integer
                    b = (tmp[1] - 57) > 0 ? tmp[1] - 55 : tmp[1] - 48;
                    c = a << 4 | b;

                    obj_code->objectCode[opIndex++] = c;            // Set object code
                }
                else if (!strncmp(tokens[i]->operator, "=C", 2)) {  // Handle =C'EOF'
                    char a, b, c;
                    int d;
                    int len = (int)strlen(tmp);

                    if (len == 1) {                                 // Handle single character
                        a = tmp[0];
                        d = a;
                    }
                    else if (len == 2) {                            // Handle two characters
                        a = tmp[0];
                        b = tmp[1];
                        d = a << 8 | b;
                    }
                    else if (len == 3) {                            // Handle three characters
                        a = tmp[0];
                        b = tmp[1];
                        c = tmp[2];
                        d = a << 16 | b << 8 | c;
                    }

                    obj_code->objectCode[opIndex++] = d;            // Set object code
                }
                else if (!strncmp(tokens[i]->operator, "=", 1)) {
                    int lit = (int)tmp[0] - 48;
                    obj_code->objectCode[opIndex++] = lit;
                }
            }
            else if (!strcmp(tokens[i]->operator, "BYTE")) {        // Handle BYTE opcode
                char* ptr;
                char tmp[10];

                strcpy(tmp, tokens[i]->operand[0]);                 // Copy operand to temporary string
                ptr = strtok(tmp, "'");
                ptr = strtok(NULL, "'");

                strcpy(tmp, ptr);

                char a, b;
                int c;

                a = (tmp[0] - 57) > 0 ? tmp[0] - 55 : tmp[0] - 48;
                b = (tmp[1] - 57) > 0 ? tmp[1] - 55 : tmp[1] - 48;
                c = a << 4 | b;

                obj_code->objectCode[opIndex++] = c;               // Set object code
            }
            else if (!strcmp(tokens[i]->operator, "WORD")) {
                // Check if the operand is a valid symbol, else set object code to 0
                int whichNum;
                if ((whichNum = isTherePlusMinus(tokens[i]->operand[0]))) { // Handle +, - in operand
                    char tmp[20], a[10], b[10];
                    char* ptr;
                    int aN, bN;

                    strcpy(tmp, tokens[i]->operand[0]);
                    ptr = strtok(tmp, "+-");
                    strcpy(a, ptr);
                    ptr = strtok(NULL, "+-");
                    strcpy(b, ptr);

                    if ((aN = addrSymbol(symbol_table, symbol_table_length, a, tokens[i]->area) > -1)
                        && (bN = addrSymbol(symbol_table, symbol_table_length, b, tokens[i]->area) > -1)) {
                        if (whichNum == 1)
                            obj_code->objectCode[opIndex++] = aN + bN;
                        else
                            obj_code->objectCode[opIndex++] = aN - bN;
                    }
                    else {
                        obj_code->objectCode[opIndex++] = 0;
                    }
                }
                else {                                                      // If WORD operand is a valid symbol
                    char tmp[20];
                    int aN;

                    strcpy(tmp, tokens[i]->operand[0]);

                    if ((aN = addrSymbol(symbol_table, symbol_table_length, tmp, tokens[i]->area)) > -1)
                        obj_code->objectCode[opIndex++] = aN;
                    else
                        obj_code->objectCode[opIndex++] = 0;
                }
            }
            else { // Else, set object code to -1
                obj_code->objectCode[opIndex++] = -1;
            }
        }
    }


    // Function to generate the object program by processing each opcode.
int object_index, JLT30;            // Indices for object code, check if text section exceeds 30 bytes
int text_index, modify_index;       // Indices for text and modify sections
int EndOfToken;                     // END token flag

object_index = 0;
JLT30 = 0;
text_index = 0;
modify_index = 0;
EndOfToken = 0;

for (int i = 0; i < opIndex; i++) { // Generate object code for each token and add it to the object program
    makingObject(tokens[i], obj_code->objectCode[i], obj_code, &object_index, symbol_table, symbol_table_length,
        &JLT30, inst_table, inst_table_length, &text_index, &modify_index, &i, &EndOfToken);
}

return 0;
}

// Function to find the address of a symbol in the symbol table
int addrSymbol(const symbol* symbol_table[], int symbol_table_length, char* ptrName, const char* ptrArea) {
    for (int i = 0; i < symbol_table_length; i++) {
        if (!strcmp(symbol_table[i]->name, ptrName) && !strcmp(symbol_table[i]->area, ptrArea))
            return symbol_table[i]->addr;
    }
    return -1;
}

// Function to create the object program and add it to the object code structure
void makingObject(const token* tok, int tok_object, object_code* obj, int* obj_index, const symbol* symbol_table[], int symbol_table_length,
    int* JLT30, const inst* inst_table[], int inst_table_length, int* text_index, int* modify_index, int* loopIndex, int* EndOfToken) {
    int T, M;
    control_section* sec;

    T = *text_index;    // Index for text section
    M = *modify_index;  // Index for modify section
    // Update section program length and start a new section if necessary
    sec = &(obj->sec[*obj_index]); // Current control section

    if (!strcmp(tok->operator, "START")) {                      // If operator is START, initialize header
        sec->header.type = 'H';
        strcpy(sec->header.programName, tok->label);
        sec->header.startLocation = atoi(tok->operand[0]);

        sec->end.type = 'E';
        sec->end.startlocation = atoi(tok->operand[0]);         // Initialize end start location
    } else if (!strcmp(tok->operator, "END")) {                 // If operator is END, finalize the section
        sec->header.programLength = tok->section_length;
        (*EndOfToken) = 1;
    } else if (!strcmp(tok->operator, "CSECT")) {               // If operator is CSECT, finalize the current section and start a new one
        sec->header.programLength = tok->section_length;        // Set the program length of the current section
        sec->text[T].textLength = *JLT30;                       // Set the text length

        (*text_index) = 0;                                      // Reset indices for the new section
        (*modify_index) = 0;
        (*JLT30) = 0;
        (*obj_index)++;

        sec = &(obj->sec[*obj_index]);                          // Initialize the new section
        sec->header.type = 'H';
        strcpy(sec->header.programName, tok->label);
        sec->header.startLocation = 0;

        sec->end.type = 'E';                                    // Initialize end type and start location
        sec->end.startlocation = -1;
    } else if (!strcmp(tok->operator, "EXTDEF")) {              // If operator is EXTDEF, add external definitions
        sec->define.type = 'D';                                 // Set define type and add definitions
        strcpy(sec->define.defName[0], tok->operand[0]);
        sec->define.defLocation[0] = returnLiteralAddress(tok->operand[0], symbol_table, symbol_table_length);

        if (tok->operand[1] != ' ') {                           // Add additional definitions if present
            strcpy(sec->define.defName[1], tok->operand[1]);
            sec->define.defLocation[1] = returnLiteralAddress(tok->operand[1], symbol_table, symbol_table_length);
        }

        if (tok->operand[2] != ' ') {                           // Add additional definitions if present
            strcpy(sec->define.defName[2], tok->operand[2]);
            sec->define.defLocation[2] = returnLiteralAddress(tok->operand[2], symbol_table, symbol_table_length);
        }
    } else if (!strcmp(tok->operator, "EXTREF")) {              // If operator is EXTREF, add external references
        sec->refer.type = 'R';
        strcpy(sec->refer.refName[0], tok->operand[0]);         // Add reference names

        if (strncmp(tok->operand[1], " ", 1)) {                 // Add additional references if present
            strcpy(sec->refer.refName[1], tok->operand[1]);
        }

        if (strncmp(tok->operand[2], " ", 1)) {                 // Add additional references if present
            strcpy(sec->refer.refName[2], tok->operand[2]);
        }
    } else if (!strcmp(tok->operator, "RESW") && *JLT30 == 0) { // If operator is RESW and text section is empty

    } else if (!strcmp(tok->operator, "RESW")) {                // If operator is RESW and text section is not empty
        sec->text[T].textLength = *JLT30;                       // Set the text length for the section
        *JLT30 = 0;
        (*text_index) += 1;
    } else {                                                    // Process the object code

        // Handle modification records
        if (!strncmp(tok->operator, "+", 1)) {                  // If operator starts with '+', handle modification
            char tmp[10];
            strcpy(tmp, "+");                                   // Prepare the modification record
            strcat(tmp, tok->operand[0]);
            strcpy(sec->modify[M].modifySymbol, tmp);

            sec->modify[M].type = 'M';                          // Set the modification record
            sec->modify[M].startlocation = tok->location + 1;
            sec->modify[M].modifyLength = 5;

            (*modify_index)++;                                  // Increment the modify index
        } else if (!strcmp(tok->operator, "WORD")) {            // If operator is WORD, handle modification
            char tmp[20];
            char partial[10];
            int whichNum;
            char* ptr;

            if (tok_object == 0) {                              // If object code is 000000, handle modification
                whichNum = isTherePlusMinus(tok->operand[0]);   // Check for +, -

                if (whichNum == 0) {                            // If no +, -, prepare modification record
                    strcpy(tmp, "+");
                    strcat(tmp, tok->operand[0]);
                    strcpy(sec->modify[M].modifySymbol, tmp);

                    sec->modify[M].type = 'M';
                    sec->modify[M].startlocation = tok->location;
                    sec->modify[M].modifyLength = 6;
                    (*modify_index)++;
                } else {                                        // If +, -, handle accordingly
                    strcpy(tmp, tok->operand[0]);
                    strcpy(partial, "+");

                    ptr = strtok(tmp, "+-");
                    strcat(partial, ptr);
                    strcpy(sec->modify[M].modifySymbol, partial);

                    sec->modify[M].type = 'M';
                    sec->modify[M].startlocation = tok->location;
                    sec->modify[M].modifyLength = 6;
                    (*modify_index)++;

                    memset(partial, 0, 10);

                    if (whichNum == 1)
                        strcpy(partial, "+");
                    else
                        strcpy(partial, "-");

                    ptr = strtok(NULL, "+-");
                    strcat(partial, ptr);
                    strcpy(sec->modify[M + 1].modifySymbol, partial);

                    sec->modify[M + 1].type = 'M';
                    sec->modify[M + 1].startlocation = tok->location;
                    sec->modify[M + 1].modifyLength = 6;
                    (*modify_index)++;
                }
            }
        }   // End of modification handling

        if (tok_object != -1) {                          // If object code is valid

            if (*JLT30 == 0) {                           // If text section is empty, initialize text record
                sec->text[T].type = 'T';
                sec->text[T].startlocation = tok->location;
            }

            if (isFourFormat(tok_object))                // Update text section length based on format
                *JLT30 += 4;
            else if (!strcmp(tok->operator, "BYTE"))
                *JLT30 += 1;
            else if (!strcmp(tok->operator, "WORD"))
                *JLT30 += 3;
            else if (!strncmp(tok->operator, "=X", 2))
                *JLT30 += 1;
            else if (!strncmp(tok->operator, "=C", 2)) { // Handle =C'EOF'
                char tmp[10];
                char* ptr;

                strcpy(tmp, tok->operator);
                ptr = strtok(tmp, "'");
                ptr = strtok(NULL, "'");
                *JLT30 += (int)strlen(ptr);
            }
            else if (!strncmp(tok->operator, "=", 1) && isdigit(tok->operator[1])) {
                *JLT30 += 3;
            }
            else
                *JLT30 += inst_table[tok->inst_index]->format;

            if (*JLT30 <= 30) {                         // If text section length is within limit
                for (int i = 0; i < 15; i++) {          // Find empty slot in text section
                    if (sec->text[T].text[i] == 0) {    // Add object code to text section
                        if (tok_object == 0)            // If opcode is 0, set text to -1
                            sec->text[T].text[i] = -1;
                        else
                            sec->text[T].text[i] = tok_object;  // Otherwise, add the object code
                        break;
                    }
                }
            } else {                                    // If text section length exceeds limit
                sec->text[T].textLength = *JLT30 - inst_table[tok->inst_index]->format; // Set text length and reset
                *JLT30 = 0;                             // Reset length, increment text index
                (*text_index)++;
                (*loopIndex)--;                         // Reprocess the current token
            }

            if ((*EndOfToken) == 1) {                   // If END token is encountered
                sec->header.programLength = tok->section_length;    // Set program length for the section
                sec->text[T].textLength = *JLT30;       // Set text length

                (*text_index) = 0;                      // Reset text and modify indices
                (*modify_index) = 0;
                (*JLT30) = 0;
                (*obj_index)++;                         // Move to the next section
            }
        }
    }
}


// 2�������� �˻��Ѵ�
int isTwoFormat(int a) {
    if (a >> 16 & 0b1111)
        return 0;

    return 1;
}

// 4�������� �˻��Ѵ�
int isFourFormat(int a) {
    if (a >> 28 & 0b1111)
        return 1;

    return 0;
}

// ���ڿ��� �´� ���ͷ����̺��� �ּҸ� ��ȯ�Ѵ�
int returnLiteralAddress(char* p, const symbol* symbol_table[], int symbol_table_length) {

    for (int i = 0; i < symbol_table_length; i++) {
        if (!strcmp(p, symbol_table[i]->name)) {
            return symbol_table[i]->addr;
        }
    }

    return 0xFF;
}
// �ƽ�Ű �ڵ尡 �����̸� 48�� ����, �����̸� 55�� ���� hex���� ��ȯ�Ѵ�
int opcodeTransfer(int a) {
    int k;

    if (a <= 57) {
        k = a - 48;
    }
    else {
        k = a - 55;
    }

    return k;
}

// pc relative addressing�� ����Ѵ�?
int pcRelativeAddressing(const int pcValue, const char* operand, const char* area, const symbol* symbol_table[], int symbol_table_length,
    const literal* literal_table[], int literal_table_length) {
    int result;

    result = -999;

    if (!strncmp(operand, "#", 1)) {        // ���� #�� ��ġ�ϸ�
        return *(operand + 1) - 48;
    }
    else if (!strncmp(operand, "@", 1)) {   // ���� @�� ��ġ�ϸ�
        operand += 1;
    }
    else if (!strncmp(operand, "=", 1)) {   // ���� =�� ��ġ�ϸ�
        for (int i = 0; i < literal_table_length; i++) {
            if (!strcmp(operand, literal_table[i]->literal)) {      // operand�� literal���� ��ġ�ϸ�
                if (literal_table[i]->addr >= pcValue) {            // ���� symbol table�� ���� pcValue������ ũ��
                    result = literal_table[i]->addr - pcValue;
                }
                else {                                              // �ƴҽ� ����ó���� ���ش�
                    result = 0x1000 - (pcValue - literal_table[i]->addr);
                }
            }
        }
    }

    for (int i = 0; i < symbol_table_length; i++) {

        if (!strncmp(symbol_table[i]->name, operand, strlen(operand)) && !strcmp(symbol_table[i]->area, area)) {
            if (symbol_table[i]->addr >= pcValue) {                     // ���� symbol table�� ���� pcValue������ ũ��
                result = symbol_table[i]->addr - pcValue;
            }
            else {
                result = 0x1000 - (pcValue - symbol_table[i]->addr);    // �ƴҽ� ����ó���� ���ش�
            }

            break;
        }
    }

    return result;
}

char numberRegister(char* operand) {        // ���������� ���� ���ڸ� ��ȯ�Ѵ�

    char num;

    if (!strncmp(operand, "A", 1)) {
        num = 0b0000;
    }
    else if (!strncmp(operand, "X", 1)) {
        num = 0b0001;
    }
    else if (!strncmp(operand, "L", 1)) {
        num = 0b0010;
    }
    else if (!strncmp(operand, "B", 1)) {
        num = 0b0011;
    }
    else if (!strncmp(operand, "S", 1)) {
        num = 0b0100;
    }
    else if (!strncmp(operand, "T", 1)) {
        num = 0b0101;
    }
    else if (!strncmp(operand, "F", 1)) {
        num = 0b0110;
    }

    return num;
}

/**
 * @brief Outputs the symbol table to a file. If `symbol_table_dir` is NULL, the output is printed to stdout.
 *
 * @param symbol_table_dir The directory to output the symbol table, or NULL for stdout.
 * @param symbol_table The symbol table array.
 * @param symbol_table_length The length of the symbol table array.
 * @return int (0 on success)
 *
 * @details
 * Outputs the symbol table to a file. If `symbol_table_dir` is NULL, the output is printed to stdout.
 * The function iterates over the symbol table and prints each entry.
 */
int make_symbol_table_output(const char* symbol_table_dir,
    const symbol* symbol_table[],
    int symbol_table_length) {
    FILE* fp;

    if (symbol_table_dir == NULL) {  // If directory is NULL, use stdout
        fp = stdout;
    } else {
        fp = fopen(symbol_table_dir, "w");  // Open file for writing
    }

    for (int i = 0; i < symbol_table_length; i++) {
        if (!strncmp(symbol_table[i]->area, " ", 1)) {  // If area is " ", print normally
            fprintf(fp, "%s\t\t%X\t%s\n", symbol_table[i]->name, symbol_table[i]->addr, symbol_table[i]->area);
        } else {  // Otherwise, add "+1 " before the area
            fprintf(fp, "%s\t\t%X\t%s%s\n", symbol_table[i]->name, symbol_table[i]->addr, "+1 ", symbol_table[i]->area);
        }
    }

    return 0;
}

/**
 * @brief Outputs the literal table to a file. If `literal_table_dir` is NULL, the output is printed to stdout.
 *
 * @param literal_table_dir The directory to output the literal table, or NULL for stdout.
 * @param literal_table The literal table array.
 * @param literal_table_length The length of the literal table array.
 * @return int (0 on success)
 *
 * @details
 * Outputs the literal table to a file. If `literal_table_dir` is NULL, the output is printed to stdout.
 * The function iterates over the literal table and prints each entry.
 */
int make_literal_table_output(const char* literal_table_dir,
    const literal* literal_table[],
    int literal_table_length) {
    FILE* fp;

    if (literal_table_dir == NULL) {  // If directory is NULL, use stdout
        fp = stdout;
    } else {
        fp = fopen(literal_table_dir, "w");  // Open file for writing
    }

    for (int i = 0; i < literal_table_length; i++) {
        fprintf(fp, "%s\t\t%X\n", literal_table[i]->literal, literal_table[i]->addr);
    }

    return 0;
}

/**
 * @brief Outputs the object code to a file. If `objectcode_dir` is NULL, the output is printed to stdout.
 *
 * @param objectcode_dir The directory to output the object code, or NULL for stdout.
 * @param obj_code The object code structure.
 * @return int (0 on success)
 *
 * @details
 * Outputs the object code to a file. If `objectcode_dir` is NULL, the output is printed to stdout.
 * The function iterates over the control sections and prints the header, definition, reference, text, modify, and end records.
 */
int make_objectcode_output(const char* objectcode_dir,
    const object_code* obj_code) {
    FILE* fp;

    if (objectcode_dir == NULL) {  // If directory is NULL, use stdout
        fp = stdout;
    } else {
        fp = fopen(objectcode_dir, "w");  // Open file for writing
    }

    control_section* sec;  // Control section pointer

    for (int i = 0; i < 3; i++) {
        sec = &(obj_code->sec[i]);

        // Print the header record
        fprintf(fp, "%c%-6s%06X%06X\n", sec->header.type, sec->header.programName, sec->header.startLocation, sec->header.programLength);

        // Print the define record if present
        if (sec->define.type == 'D') {
            fprintf(fp, "%c%s%06X%s%06X%s%06X\n", sec->define.type, sec->define.defName[0], sec->define.defLocation[0],
                sec->define.defName[1], sec->define.defLocation[1],
                sec->define.defName[2], sec->define.defLocation[2]);
        }

        // Print the reference record
        fprintf(fp, "%c", sec->refer.type);
        for (int j = 0; j < 5; j++) {
            if (sec->refer.refName[j][0] == 0)
                break;
            fprintf(fp, "%-6s", sec->refer.refName[j]);
        }
        fprintf(fp, "\n");

        // Print the text records
        for (int j = 0; j < 5; j++) {
            if (sec->text[j].type != 'T')
                break;

            fprintf(fp, "%c%06X%02X", sec->text[j].type, sec->text[j].startlocation, sec->text[j].textLength);

            for (int k = 0; k < 15; k++) {
                if (sec->text[j].text[k] != 0) {
                    if (isTwoFormat(sec->text[j].text[k])) {
                        fprintf(fp, "%06X", sec->text[j].text[k]);
                    } else if (sec->text[j].text[k] == -1) {
                        fprintf(fp, "%06d", 0);
                    } else {
                        fprintf(fp, "%06X", sec->text[j].text[k]);
                    }
                }
            }
            fprintf(fp, "\n");
        }

        // Print the modify records
        for (int j = 0; j < 5; j++) {
            if (sec->modify[j].type != 'M')
                break;

            fprintf(fp, "%c%06X%02X%s\n", sec->modify[j].type, sec->modify[j].startlocation,
                sec->modify[j].modifyLength, sec->modify[j].modifySymbol);
        }

        // Print the end record
        if (sec->end.startlocation)
            fprintf(fp, "%c%06X\n\n", sec->end.type, sec->end.startlocation);
        else
            fprintf(fp, "%c\n\n", sec->end.type);
    }

    return 0;
}

void allocMem(token** tokens, int input_length, literal* literal_table[], symbol* symbol_table[]) {   // tokens �迭�� �޸𸮸� �Ҵ��Ѵ�
    for (int i = 0; i < input_length + 50; i++) {
        tokens[i] = calloc(1, sizeof(token));                           // token �޸� �Ҵ�
        tokens[i]->label = calloc(8, sizeof(char));                     // token label �޸� �Ҵ�
        tokens[i]->operator = calloc(32, sizeof(char));                 // token operator �޸� �Ҵ�
        tokens[i]->operand[0] = calloc(32, sizeof(char));               // token operand �޸� �Ҵ�
        tokens[i]->operand[1] = calloc(32, sizeof(char));               // token operand �޸� �Ҵ�
        tokens[i]->operand[2] = calloc(32, sizeof(char));               // token operand �޸� �Ҵ�
        tokens[i]->comment = calloc(64, sizeof(char));                  // token comment �޸� �Ҵ�
    }

    for (int i = 0; i < 50; i++) {                                      // literal �޸� �Ҵ�
        literal_table[i] = calloc(1, sizeof(literal));
    }

    for (int i = 0; i < 50; i++) {
        symbol_table[i] = calloc(1, sizeof(symbol));                    // symbol_table �޸� �Ҵ�
    }
}

int isComma(char* buf) {            // ���ڿ��� �޸��� �ִ��� Ȯ���Ѵ�
    int i = 0;
    while (buf[i]) {                // buf[i]�� true�̸� �ݺ�
        if (buf[i] == ',')          // �޸������� return 1
            return 1;
        i++;
    }
    return 0;
}

void RemoveEnd(char* buf) {         // ���ڿ��� ������������ �����Ѵ�
    int i = 0;
    while (buf[i]) {                // buf[i]�� true�̸� �ݺ�
        i++;
    }
    if (buf[i - 1] == '\n')
        buf[i - 1] = '\0';          // i-1�� ������ ���� ��ġ�� �̸� �������ش�
}

void RemoveFirst(char* buf) {       // ���ڿ��� ù�κ��� �����Ѵ�
    int i = 0;
    for (i = 1; buf[i]; i++) {      // buf[i]�� ��(�ι��ڰ� �ƴ�)�̸� �ݺ�
        buf[i - 1] = buf[i];        // buf[i] ���ڸ� buf[i-1]�� �̵�
    }
    buf[i - 1] = '\0';              //���� i�� �ι��ڰ� �ִ� ��ġ, i-1�� ������ ���� ��ġ
}

// ��ū�迭�� ���鼭 �ּҰ��� �Ҵ��ϰ�, SYMTAB, LITTAB�� �߰��Ѵ�
void location_counter(const inst* inst_table[], int inst_table_length, token* token, int tokens_length, int* memoryAddress,
    symbol* symbol, int* sym_index, literal* literal_table[], int literal_table_length, char* area) {
    int inst_index;
    char bit_mask_ni, bit_mask_x, bit_mask_bp, bit_mask_e;

    bit_mask_ni = 0;                                        // bixbpe ��Ʈ������ �ϱ����� ������
    bit_mask_x = 0;
    bit_mask_bp = 0b0010;
    bit_mask_e = 0;
    // operator�� ��ġ�ϴ��� Ȯ���Ѵ�
    inst_index = search_opcode(token->operator, inst_table, inst_table_length);

    if (inst_index > -1) {                                  // ���� ��ġ�ϸ� ���Ŀ� ���� LOCCTR�� �÷��ش�
        token->location = *memoryAddress;
        if (!strncmp(token->operator, "+", 1)) {
            bit_mask_e = 0b0001;                            // 4������ ���? e��Ʈ�� üũ���ش�
            bit_mask_bp = 0b0000;

            token->location = *memoryAddress;               // �޸� �ּ� �������ֱ�
            *memoryAddress += 4;                            // �ּ� 4�߰�
        }
        else {
            token->location = *memoryAddress;               // �޸� �ּ� �������ֱ�
            *memoryAddress += inst_table[inst_index]->format;// �ּ� format��ŭ �߰�
        }

        if (inst_table[inst_index]->format >= 3) {
            if (!strcmp(token->operand[0], " ")) {          // ���� operand�� ������ bp��Ʈ�� 0���� �Ѵ�
                bit_mask_bp = 0b0000;
            }

            if (!strncmp(token->operand[0], "#", 1)) {      // ���� #���� �����ϸ� i��Ʈ üũ
                bit_mask_ni = 0b00010000;
                bit_mask_bp = 0b0000;
            }
            else if (!strncmp(token->operand[0], "@", 1)) { // ���� @���� �����ϸ� n��Ʈ üũ
                bit_mask_ni = 0b00100000;
            }
            else {
                bit_mask_ni = 0b00110000;
            }

            // bit_mask_x ó���ϴ� �ڵ�
            if (!strcmp(token->operand[1], "X")) {
                bit_mask_x = 0b1000;
            }
            // nixbpe�� �����ش�
            token->nixbpe = (bit_mask_ni | bit_mask_x | bit_mask_bp | bit_mask_e);
        }
        else { // 1, 2���� ������ nixbpe �ʿ����?
            token->nixbpe = 0b0000;
        }
    }
    else {
        token->nixbpe = 0b0000;                             // nixbpe�� ���Ͽ� 0���� ó��
        // ���� ��ġ���� ������
        if (!strcmp(token->operator, "START")) {            // ���� ���ڵ鿡 ���Ͽ� �ּҰ��� �������ָ鼭
            *memoryAddress = atoi(token->operand[0]);       // ���Ŀ� ���� �ּҰ��� �����ش�
            token->location = *memoryAddress;
            strcpy(area, token->label);
        }
        else if (!strncmp(token->label, ".", 1)) {
            token->location = -1;
        }
        else if (!strcmp(token->operator, "EXTDEF")) {
            token->location = -1;
        }
        else if (!strcmp(token->operator, "EXTREF")) {
            token->location = -1;
        }
        else if (!strcmp(token->operator, "LTORG")) {
            token->location = -1;
        }
        else if (!strcmp(token->operator, "RESW")) {
            token->location = *memoryAddress;
            *memoryAddress += atoi(token->operand[0]) * 3;
        }
        else if (!strcmp(token->operator, "RESW")) {
            token->location = *memoryAddress;
            *memoryAddress += atoi(token->operand[0]) * 3;
        }
        else if (!strcmp(token->operator, "RESB")) {
            token->location = *memoryAddress;
            *memoryAddress += atoi(token->operand[0]);
        }
        else if (!strcmp(token->operator, "EQU") && !strcmp(token->operand[0], "*")) {
            token->location = *memoryAddress;
        }
        else if (!strcmp(token->operator, "CSECT")) {
            token->section_length = *memoryAddress;
            *memoryAddress = 0;
            token->location = *memoryAddress;
            strcpy(area, token->label);
        }
        else if (!strcmp(token->operator, "BYTE")) {
            token->location = *memoryAddress;
            *memoryAddress += 1;
        }
        else if (!strcmp(token->operator, "WORD")) {
            token->location = *memoryAddress;
            *memoryAddress += 3;
        }
        else if (!strcmp(token->operator, "END")) {
            token->section_length = *memoryAddress;
            token->location = -1;
        }
        else if (!strncmp(token->label, "*", 1)) {         
            token->location = *memoryAddress;

            char tmp[10];
            strcpy(tmp, token->operator);

            char* p = strtok(tmp, "'");

            if (*(p + 1) == 'C') {                          // ex) =C'EOF'
                p = strtok(NULL, "'");
                *memoryAddress += (int)strlen(p);
            }
            else if (*(p + 1) == 'X') {                     // ex) =X'05'
                p = strtok(NULL, "'");
                *memoryAddress += (int)(strlen(p) + 1) / 2;
            }
            else if (isdigit(*(p + 1))) {
                *memoryAddress += 3;
            }

            token->section_length = *memoryAddress;

        }
    }
 
    strcpy(token->area, area);


    // 
    if ((*(token->label) != ' ') && (*(token->label) != '*') && (*(token->label) != '.')) {
        strcpy(symbol->name, token->label);
        symbol->addr = token->location;
        if (!strcmp(token->operator, "EQU") && (token->operand[0][0] != '*')) 
            strcpy(symbol->area, " ");
        else if (strcmp(area, symbol->name))                                 
            strcpy(symbol->area, area);
        else                                                                 
            strcpy(symbol->area, " ");

        (*sym_index)++;
    }

   
    if (*(token->label) == '*') {
        for (int i = 0; i < literal_table_length; i++) {   
            if (!strncmp(token->operator, literal_table[i]->literal, strlen(literal_table[i]->literal))) {
                literal_table[i]->addr = token->location;
                break;
            }
        }
    }
}



void addLiteral(token* token, literal* literal_table[], int* literal_table_length) {    
    if (!strncmp(token->operand[0], "=", 1)) {             
        for (int i = 0; i < *literal_table_length; i++) {  
            if (!strcmp(literal_table[i]->literal, token->operand[0])) {
                return;
            }
        }
        strcpy(literal_table[*literal_table_length]->literal, token->operand[0]);       
        literal_table[*literal_table_length]->check = 0;                               
        (*literal_table_length)++;                                                   
    }
}

int meetLTORG(token* tokens[], literal* literal_table[], int* literal_table_length, int* index) {  
    int isEOF;
    int tokenIndex;

    isEOF = 0;
    if (!strcmp(tokens[*index]->operator, "END")) {             
        isEOF = 1;
    }

    tokenIndex = *index + 1;

    if (!strcmp(tokens[*index]->operator, "LTORG") || isEOF) {  
        for (int i = 0; i < *literal_table_length; i++) {
            if (literal_table[i]->check == 0) {                
                strcpy(tokens[tokenIndex]->label, "*");
                strcpy(tokens[tokenIndex]->operator, literal_table[i]->literal);
                strcpy(tokens[tokenIndex]->operand[0], " ");
                strcpy(tokens[tokenIndex]->operand[1], " ");
                strcpy(tokens[tokenIndex]->operand[2], " ");
                strcpy(tokens[tokenIndex]->comment, " ");
                tokens[tokenIndex]->inst_index = -1;
                tokenIndex++;

                literal_table[i]->check = 1;
            }
        }
    }   *index = tokenIndex - 1;                                

    if (isEOF)                                                 
        return 1;

    return 0;
}


void binary_print(int a)
{
    int n = 6;	                    
    int mask = 1 << (n - 1);		


    for (int i = 1; i <= n; i++)
    {
        putchar((a & mask) ? '1' : '0');
       
        a <<= 1;                    // a = a << 1;
        if (i == 3 && i < n)
            putchar(' ');
    }
    putchar('\n');
}


int isTherePlusMinus(char* p) {
    int i = 0;

    while (p[i]) {
        if (p[i] == '+') {
            return 1;
        }
        else if (p[i] == '-') {
            return 2;
        }
        i++;
    }
    return 0;
}