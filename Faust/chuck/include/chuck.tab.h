/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     STRING_LIT = 259,
     CHAR_LIT = 260,
     NUM = 261,
     FLOAT = 262,
     POUND = 263,
     COMMA = 264,
     COLON = 265,
     SEMICOLON = 266,
     LPAREN = 267,
     RPAREN = 268,
     LBRACK = 269,
     RBRACK = 270,
     LBRACE = 271,
     RBRACE = 272,
     DOT = 273,
     PLUS = 274,
     MINUS = 275,
     TIMES = 276,
     DIVIDE = 277,
     PERCENT = 278,
     EQ = 279,
     NEQ = 280,
     LT = 281,
     LE = 282,
     GT = 283,
     GE = 284,
     AND = 285,
     OR = 286,
     ASSIGN = 287,
     IF = 288,
     THEN = 289,
     ELSE = 290,
     WHILE = 291,
     FOR = 292,
     DO = 293,
     LOOP = 294,
     BREAK = 295,
     CONTINUE = 296,
     NULL_TOK = 297,
     FUNCTION = 298,
     RETURN = 299,
     QUESTION = 300,
     EXCLAMATION = 301,
     S_OR = 302,
     S_AND = 303,
     S_XOR = 304,
     PLUSPLUS = 305,
     MINUSMINUS = 306,
     DOLLAR = 307,
     POUNDPAREN = 308,
     PERCENTPAREN = 309,
     SIMULT = 310,
     PATTERN = 311,
     CODE = 312,
     TRANSPORT = 313,
     HOST = 314,
     TIME = 315,
     WHENEVER = 316,
     NEXT = 317,
     UNTIL = 318,
     EVERY = 319,
     BEFORE = 320,
     AFTER = 321,
     AT = 322,
     AT_SYM = 323,
     ATAT_SYM = 324,
     NEW = 325,
     SIZEOF = 326,
     TYPEOF = 327,
     SAME = 328,
     PLUS_CHUCK = 329,
     MINUS_CHUCK = 330,
     TIMES_CHUCK = 331,
     DIVIDE_CHUCK = 332,
     S_AND_CHUCK = 333,
     S_OR_CHUCK = 334,
     S_XOR_CHUCK = 335,
     SHIFT_RIGHT_CHUCK = 336,
     SHIFT_LEFT_CHUCK = 337,
     PERCENT_CHUCK = 338,
     SHIFT_RIGHT = 339,
     SHIFT_LEFT = 340,
     TILDA = 341,
     CHUCK = 342,
     COLONCOLON = 343,
     S_CHUCK = 344,
     AT_CHUCK = 345,
     LEFT_S_CHUCK = 346,
     UNCHUCK = 347,
     UPCHUCK = 348,
     CLASS = 349,
     INTERFACE = 350,
     EXTENDS = 351,
     IMPLEMENTS = 352,
     PUBLIC = 353,
     PROTECTED = 354,
     PRIVATE = 355,
     STATIC = 356,
     ABSTRACT = 357,
     CONST = 358,
     SPORK = 359,
     ARROW_RIGHT = 360,
     ARROW_LEFT = 361,
     L_HACK = 362,
     R_HACK = 363
   };
#endif
/* Tokens.  */
#define ID 258
#define STRING_LIT 259
#define CHAR_LIT 260
#define NUM 261
#define FLOAT 262
#define POUND 263
#define COMMA 264
#define COLON 265
#define SEMICOLON 266
#define LPAREN 267
#define RPAREN 268
#define LBRACK 269
#define RBRACK 270
#define LBRACE 271
#define RBRACE 272
#define DOT 273
#define PLUS 274
#define MINUS 275
#define TIMES 276
#define DIVIDE 277
#define PERCENT 278
#define EQ 279
#define NEQ 280
#define LT 281
#define LE 282
#define GT 283
#define GE 284
#define AND 285
#define OR 286
#define ASSIGN 287
#define IF 288
#define THEN 289
#define ELSE 290
#define WHILE 291
#define FOR 292
#define DO 293
#define LOOP 294
#define BREAK 295
#define CONTINUE 296
#define NULL_TOK 297
#define FUNCTION 298
#define RETURN 299
#define QUESTION 300
#define EXCLAMATION 301
#define S_OR 302
#define S_AND 303
#define S_XOR 304
#define PLUSPLUS 305
#define MINUSMINUS 306
#define DOLLAR 307
#define POUNDPAREN 308
#define PERCENTPAREN 309
#define SIMULT 310
#define PATTERN 311
#define CODE 312
#define TRANSPORT 313
#define HOST 314
#define TIME 315
#define WHENEVER 316
#define NEXT 317
#define UNTIL 318
#define EVERY 319
#define BEFORE 320
#define AFTER 321
#define AT 322
#define AT_SYM 323
#define ATAT_SYM 324
#define NEW 325
#define SIZEOF 326
#define TYPEOF 327
#define SAME 328
#define PLUS_CHUCK 329
#define MINUS_CHUCK 330
#define TIMES_CHUCK 331
#define DIVIDE_CHUCK 332
#define S_AND_CHUCK 333
#define S_OR_CHUCK 334
#define S_XOR_CHUCK 335
#define SHIFT_RIGHT_CHUCK 336
#define SHIFT_LEFT_CHUCK 337
#define PERCENT_CHUCK 338
#define SHIFT_RIGHT 339
#define SHIFT_LEFT 340
#define TILDA 341
#define CHUCK 342
#define COLONCOLON 343
#define S_CHUCK 344
#define AT_CHUCK 345
#define LEFT_S_CHUCK 346
#define UNCHUCK 347
#define UPCHUCK 348
#define CLASS 349
#define INTERFACE 350
#define EXTENDS 351
#define IMPLEMENTS 352
#define PUBLIC 353
#define PROTECTED 354
#define PRIVATE 355
#define STATIC 356
#define ABSTRACT 357
#define CONST 358
#define SPORK 359
#define ARROW_RIGHT 360
#define ARROW_LEFT 361
#define L_HACK 362
#define R_HACK 363




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 59 "chuck.y"
{
    int pos;
    int ival;
    double fval;
    c_str sval;
    
    a_Program program;
    a_Section program_section;
    a_Stmt_List stmt_list;
    a_Class_Def class_def;
    a_Class_Ext class_ext;
    a_Class_Body class_body;
    a_Stmt stmt;
    a_Exp exp;
    a_Func_Def func_def;
    a_Var_Decl_List var_decl_list;
    a_Var_Decl var_decl;
    a_Type_Decl type_decl;
    a_Arg_List arg_list;
    a_Id_List id_list;
    a_Array_Sub array_sub;
    a_Complex complex_exp;
    a_Polar polar_exp;
}
/* Line 1529 of yacc.c.  */
#line 290 "chuck.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

