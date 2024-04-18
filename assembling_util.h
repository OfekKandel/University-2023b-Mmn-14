/* A helper file for the assembler's first-pass, containing the main functions needed to perform the
 * main first-pass logic, but does not edit any output files */
#pragma once

#include "binary_table.h"
#include "parser.h"
#include "symbol_table.h"

/* Processes a source line containing a command (e.g. prn #-5) adds its binary code to the
 * instruction table, as well as its label (if there is one) to the symbol table.
 * Input: The (already parsed) line, the instruction and symbol tables, and the line's log-context
 * Output: returns false on errors, else true, also edits the instruction and symbol tables
 * Errors: Any errors contained in the source line will be printed */
int add_command(CommandLine line, BinaryTable *instruction_table, SymbolTable *symbol_table,
                LogContext context);

/* Process a constant-declaration line (e.g. .define a = 5), adds the constant to the symbol table
 * Input: The (already processed) line, the symbol table, and the line's log-context
 * Output: Returns false on errors, else true. Also appends the symbol table
 * Errors: Any error contained in the source line will be printed */
int add_define_symbol(DefineLine line, SymbolTable *table, LogContext context);

/* Processes a dot-instruction (not including .define, i.e. .data, .string, .entry, .extern)
 * Input: The dot-instruction line, the data and symbol tables, and the line's log-context
 * Output: Returns false on errors, else true. Also edits the data and symbol tables
 * Errors: Any error contained in the source line will be printed */
int add_dot_instruction(DotInstructionLine line, BinaryTable *data_table, SymbolTable *symbol_table,
                        LogContext context);
