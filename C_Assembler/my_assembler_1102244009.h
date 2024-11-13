/**
 * @file my_assembler_00000000.h
 * @date 2024-04-09
 * @version 0.1.0
 *
 * @brief Macro and structure declarations for my_assembler_00000000.c
 */

#ifndef __MY_ASSEMBLER_H__
#define __MY_ASSEMBLER_H__

#define MAX_INST_TABLE_LENGTH 256
#define MAX_INPUT_LINES 5000
#define MAX_TABLE_LENGTH 5000
#define MAX_OPERAND_PER_INST 3
#define MAX_OBJECT_CODE_STRING 74
#define MAX_OBJECT_CODE_LENGTH 5000
#define MAX_CONTROL_SECTION_NUM 10

#define MAX_INPUT_LEN 1024 // Maximum length of the assembly code to be input

 /**
  * @brief Structure to store a single SIC/XE instruction
  *
  * @details
  * A structure that stores a single SIC/XE instruction as specified in the
  * machine instruction list file (inst_table.txt). The instruction list file
  * stores one instruction per line, and this structure is created by
  * extracting information from it.
  */
typedef struct _inst {
    char str[10];     /** Instruction name */
    unsigned char op; /** Instruction opcode */
    int format;       /** Instruction format */
    int ops;          /** Number of operators the instruction has */
    unsigned char opcode[2]; /** Instruction opcode */
} inst;

/**
 * @brief Structure to decompose and store a line of source code
 *
 * @details
 * To facilitate assembly, a line of source code is parsed into label,
 * operator, operand, and comment, which are stored in this structure.
 * The `operator` field allows renaming.
 */
typedef struct _token {
    char* label;   /** Pointer to the label */
    char* operator; /** Pointer to the operator */
    char* operand[MAX_OPERAND_PER_INST]; /** Array of pointers to the operands */
    char* comment; /** Pointer to the comment */
    char nixbpe;   /** Special bit information */
    int location;   // Memory address value, set to -1 if none
    int inst_index; // Index of the instruction in the opcode table
    char area[8];   // Used to check if in the same area, functions like a symbol area
    int section_length; // Indicates the length of the area
} token;

/**
 * @brief Structure to store information about a single symbol
 *
 * @details
 * A structure that stores a symbol obtained from SIC/XE source code. The
 * predefined fields `name` and `addr` must be used. If more fields are
 * needed, they can be added to the structure.
 */
typedef struct _symbol {
    char name[10]; /** Symbol name */
    int addr;      /** Symbol address */
    char area[8];  // Used to check if in the same area
    /* add fields if needed */
} symbol;

/**
 * @brief Structure to store information about a single literal
 *
 * @details
 * A structure that stores a literal obtained from SIC/XE source code. The
 * predefined fields `literal` and `addr` must be used. If more fields are
 * needed, they can be added to the structure. Note that the `addr` field
 * stores the address of the literal, not its value.
 */
typedef struct _literal {
    char literal[20]; /** Literal expression */
    int addr;         /** Literal address */
    int check;        // Check if it is in the literal table
    /* add fields if needed */
} literal;

/**
 * @brief Structure that contains information about the entire object code
 *
 * @details
 * A structure that contains information about the entire object code.
 * It should include information about the Header Record, Define Record,
 * Modification Record, etc. This structure should be able to define the
 * entire object code.
 */
typedef struct _section_header {
    char type;              // Defines the record type such as H, D, R, T, M
    char programName[8];    // Program name
    int startLocation;      // Program start address
    int programLength;      // Program length
} section_header;

typedef struct _section_define {
    char type;              // Defines the record type such as H, D, R, T, M
    char defName[8][8];     // Names of defined symbols
    int defLocation[8];     // Addresses of defined symbols
} section_define;

typedef struct _section_refer {
    char type;              // Defines the record type such as H, D, R, T, M
    char refName[8][8];     // Names of referenced symbols
} section_refer;

typedef struct _section_text {
    char type;              // Defines the record type such as H, D, R, T, M
    int startlocation;      // Start address of the object code
    int textLength;         // Length of a line
    int text[15];           // Text of a line
} section_text;

typedef struct _section_modify {
    char type;              // Defines the record type such as H, D, R, T, M
    int startlocation;      // Start address of the object code
    int modifyLength;       // Length to be modified
    char modifySymbol[8];   // Symbol to be added
} section_modify;

typedef struct _section_end {
    char type;              // Defines the record type such as H, D, R, T, M
    int startlocation;      // Start address of the object code
} section_end;

typedef struct _control_section {
    section_header header;
    section_define define;
    section_refer refer;
    section_text text[5];
    section_modify modify[5];
    section_end end;
} control_section;

typedef struct _object_code {
    /* add fields */
    int objectCode[100];        // Stores the value of tokens converted to object code
    control_section sec[10];    // Declares 10 control_section structures
} object_code;


int init_inst_table(inst* inst_table[], int* inst_table_length,
    const char* inst_table_dir);
int init_input(char* input[], int* input_length, const char* input_dir);
int assem_pass1(const inst* inst_table[], int inst_table_length,
    const char* input[], int input_length, token* tokens[],
    int* tokens_length, symbol* symbol_table[],
    int* symbol_table_length, literal* literal_table[],
    int* literal_table_length);
int token_parsing(const char* input, token* tok, const inst* inst_table[],
    int inst_table_length);
int search_opcode(const char* str, const inst* inst_table[],
    int inst_table_length);
int make_opcode_output(const char* output_dir, const token* tokens[],
    int tokens_length, const inst* inst_table[],
    int inst_table_length);
int assem_pass2(const token* tokens[], int tokens_length,
    const inst* inst_table[], int inst_table_length,
    const symbol* symbol_table[], int symbol_table_length,
    const literal* literal_table[], int literal_table_length,
    object_code* obj_code);
int make_symbol_table_output(const char* symbol_table_dir,
    const symbol* symbol_table[],
    int symbol_table_length);
int make_literal_table_output(const char* literal_table_dir,
    const literal* literal_table[],
    int literal_table_length);
int make_objectcode_output(const char* objectcode_dir,
    const object_code* obj_code);

#endif
