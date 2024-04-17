/* [DOCS NEEDED] */
#include "binary_table.h"
#include "parse_util.h"
#include "symbol_table.h"

/* [DOCS NEEDED] */
int add_command(CommandLine line, BinaryTable *instruction_table, SymbolTable *symbol_table,
                LogContext context);

/* [DOCS NEEDED] returns whether the operation was successful */
int add_define_symbol(DefineLine line, SymbolTable *table, LogContext context);

/* [DOCS NEEDED] */
int add_dot_instruction(DotInstructionLine line, BinaryTable *data_table, SymbolTable *symbol_table,
                        LogContext context);
