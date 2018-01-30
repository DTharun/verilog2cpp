/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_VERILOG_PARSE_TAB_H_INCLUDED
# define YY_VERILOG_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int verilog_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DIDENTIFIER = 258,
    HIDENTIFIER = 259,
    IDENTIFIER = 260,
    PORTNAME = 261,
    SYSTEM_IDENTIFIER = 262,
    STRING = 263,
    NUMBER = 264,
    REALTIME = 265,
    K_ATCOMM = 266,
    K_LS = 267,
    K_RS = 268,
    K_ALS = 269,
    K_ARS = 270,
    K_POW = 271,
    K_LE = 272,
    K_GE = 273,
    K_EG = 274,
    K_SG = 275,
    K_EQ = 276,
    K_NE = 277,
    K_CEQ = 278,
    K_CNE = 279,
    K_LOR = 280,
    K_LAND = 281,
    K_NOR = 282,
    K_NXOR = 283,
    K_NAND = 284,
    K_TRIGGER = 285,
    K_AAA = 286,
    K_SIGNED = 287,
    K_UNSIGNED = 288,
    K_ATTRIBUTE = 289,
    K_PLUSRANGE = 290,
    K_MINUSRANGE = 291,
    K_always = 292,
    K_and = 293,
    K_assign = 294,
    K_automatic = 295,
    K_begin = 296,
    K_buf = 297,
    K_bufif0 = 298,
    K_bufif1 = 299,
    K_case = 300,
    K_casex = 301,
    K_casez = 302,
    K_cmos = 303,
    K_default = 304,
    K_deassign = 305,
    K_defparam = 306,
    K_disable = 307,
    K_edge = 308,
    K_else = 309,
    K_end = 310,
    K_endcase = 311,
    K_endconfig = 312,
    K_endfunction = 313,
    K_endgenerate = 314,
    K_endmodule = 315,
    K_endprimitive = 316,
    K_endspecify = 317,
    K_endtable = 318,
    K_endtask = 319,
    K_event = 320,
    K_for = 321,
    K_force = 322,
    K_forever = 323,
    K_fork = 324,
    K_function = 325,
    K_generate = 326,
    K_genvar = 327,
    K_highz0 = 328,
    K_highz1 = 329,
    K_if = 330,
    K_ifnone = 331,
    K_initial = 332,
    K_inout = 333,
    K_input = 334,
    K_integer = 335,
    K_join = 336,
    K_large = 337,
    K_library = 338,
    K_localparam = 339,
    K_macromodule = 340,
    K_medium = 341,
    K_module = 342,
    K_nand = 343,
    K_negedge = 344,
    K_nmos = 345,
    K_nor = 346,
    K_not = 347,
    K_notif0 = 348,
    K_notif1 = 349,
    K_or = 350,
    K_output = 351,
    K_parameter = 352,
    K_pmos = 353,
    K_posedge = 354,
    K_primitive = 355,
    K_pull0 = 356,
    K_pull1 = 357,
    K_pulldown = 358,
    K_pullup = 359,
    K_rcmos = 360,
    K_real = 361,
    K_realtime = 362,
    K_reg = 363,
    K_release = 364,
    K_repeat = 365,
    K_rnmos = 366,
    K_rpmos = 367,
    K_rtran = 368,
    K_rtranif0 = 369,
    K_rtranif1 = 370,
    K_scalared = 371,
    K_signed = 372,
    K_small = 373,
    K_specify = 374,
    K_specparam = 375,
    K_strong0 = 376,
    K_strong1 = 377,
    K_supply0 = 378,
    K_supply1 = 379,
    K_table = 380,
    K_task = 381,
    K_time = 382,
    K_tran = 383,
    K_tranif0 = 384,
    K_tranif1 = 385,
    K_tri = 386,
    K_tri0 = 387,
    K_tri1 = 388,
    K_triand = 389,
    K_trior = 390,
    K_trireg = 391,
    K_unsigned = 392,
    K_vectored = 393,
    K_wait = 394,
    K_wand = 395,
    K_weak0 = 396,
    K_weak1 = 397,
    K_while = 398,
    K_wire = 399,
    K_wor = 400,
    K_xnor = 401,
    K_xor = 402,
    K_design = 403,
    K_instance = 404,
    K_cell = 405,
    K_use = 406,
    K_liblist = 407,
    K_include = 408,
    K_incdir = 409,
    K_countdrivers = 410,
    K_getpattern = 411,
    K_incsave = 412,
    K_key = 413,
    K_list = 414,
    K_log = 415,
    K_nokey = 416,
    K_nolog = 417,
    K_reset = 418,
    K_reset_count = 419,
    K_reset_value = 420,
    K_restart = 421,
    K_save = 422,
    K_scale = 423,
    K_scope = 424,
    K_showscopes = 425,
    K_showvars = 426,
    K_sreadmemb = 427,
    K_sreadmemh = 428,
    K_D_attribute = 429,
    UNARY_PREC = 430,
    less_than_K_else = 431
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 49 "verilog.y" /* yacc.c:1909  */

  map<string,moe::Verilog::Expression*>* defparams;
  multimap<string,moe::Verilog::Expression*>* params;
  map<string,moe::Verilog::Net*>*    nets;
  moe::Verilog::Case::Item*          item;
  vector<moe::Verilog::Case::Item*>* items;
  moe::Verilog::Event*               evexpr;  
  vector<moe::Verilog::Event*>*      evexprs;
  moe::Verilog::EventStatement*      evstat;
  moe::Verilog::Expression*          expr;
  vector<moe::Verilog::Expression*>* exprs;
  moe::Verilog::Instance*            inst;
  vector<moe::Verilog::Instance*>*   insts;
  moe::Verilog::Net::nrm_*           regvar;
  list<moe::Verilog::Net::nrm_*>*    regvars;
  moe::Verilog::Statement*           stat;
  vector<moe::Verilog::Statement*>*  stats;
  list<char*>*                       texts;
  char*                              text;
  int                                type;

#line 253 "parse.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE verilog_lval;

int verilog_parse (void);

#endif /* !YY_VERILOG_PARSE_TAB_H_INCLUDED  */
