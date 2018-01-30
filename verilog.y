%{
/*
 * Copyright (c) 1999-2003 moe
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <string>
#include <list>
#include <vector>
#include "verilog.h"
  
using namespace std;

void       verilog_error(char *str);
extern int verilog_lex();

extern unsigned int line;
extern string       file;

extern void lex_start_table();
extern void lex_end_table();

#include "Verilog.hh"
 
moe::Verilog::Module*   module_;
moe::Verilog::Function* function_;
moe::Verilog::Instance* instance_;

static void error(char* text)
  {
//    yyerror(text);
  }
%}

%union {
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
};

%token <text> DIDENTIFIER HIDENTIFIER IDENTIFIER PORTNAME SYSTEM_IDENTIFIER STRING
%token <text> NUMBER
%token <text> REALTIME

%token K_ATCOMM

%token K_LS
%token K_RS
%token K_ALS
%token K_ARS
%token K_POW
%token K_LE
%token K_GE
%token K_EG
%token K_SG
%token K_EQ
%token K_NE
%token K_CEQ
%token K_CNE
%token K_LOR
%token K_LAND
%token K_NOR
%token K_NXOR
%token K_NXOR
%token K_NAND
%token K_TRIGGER
%token K_AAA
%token K_SIGNED
%token K_UNSIGNED
%token K_ATTRIBUTE
%token K_PLUSRANGE
%token K_MINUSRANGE

%token K_always
%token K_and
%token K_assign
%token K_automatic
%token K_begin
%token K_buf
%token K_bufif0
%token K_bufif1
%token K_case
%token K_casex
%token K_casez
%token K_cmos
%token K_default
%token K_deassign
%token K_defparam
%token K_disable
%token K_edge
%token K_else
%token K_end
%token K_endcase
%token K_endconfig
%token K_endfunction
%token K_endgenerate
%token K_endmodule
%token K_endprimitive
%token K_endspecify
%token K_endtable
%token K_endtask
%token K_event
%token K_for
%token K_force
%token K_forever
%token K_fork
%token K_function
%token K_generate
%token K_genvar
%token K_highz0
%token K_highz1
%token K_if
%token K_ifnone
%token K_initial
%token K_inout
%token K_input
%token K_integer
%token K_join
%token K_large
%token K_library
%token K_localparam
%token K_macromodule
%token K_medium
%token K_module
%token K_nand
%token K_negedge
%token K_nmos
%token K_nor
%token K_not
%token K_notif0
%token K_notif1
%token K_or
%token K_output
%token K_parameter
%token K_pmos
%token K_posedge
%token K_primitive
%token K_pull0
%token K_pull1
%token K_pulldown
%token K_pullup
%token K_rcmos
%token K_real
%token K_realtime
%token K_reg
%token K_release
%token K_repeat
%token K_rnmos
%token K_rpmos
%token K_rtran
%token K_rtranif0
%token K_rtranif1
%token K_scalared
%token K_signed
%token K_small
%token K_specify
%token K_specparam
%token K_strong0
%token K_strong1
%token K_supply0
%token K_supply1
%token K_table
%token K_task
%token K_time
%token K_tran
%token K_tranif0
%token K_tranif1
%token K_tri
%token K_tri0
%token K_tri1
%token K_triand
%token K_trior
%token K_trireg
%token K_unsigned
%token K_vectored
%token K_wait
%token K_wand
%token K_weak0
%token K_weak1
%token K_while
%token K_wire
%token K_wor
%token K_xnor
%token K_xor

%token K_design
%token K_instance
%token K_cell
%token K_use
%token K_liblist


%token K_include
%token K_incdir

%token K_countdrivers
%token K_getpattern
%token K_incsave
%token K_input
%token K_key
%token K_list
%token K_log
%token K_nokey
%token K_nolog
%token K_reset
%token K_reset_count
%token K_reset_value
%token K_restart
%token K_save
%token K_scale
%token K_scope
%token K_showscopes
%token K_showvars
%token K_sreadmemb
%token K_sreadmemh


%token K_D_attribute


%type <text> attribute_instance_opt
%type <text> identifier
//%type <text> port_opt
%type <texts> list_of_variables
%type <text> net_decl_assign
%type <texts> net_decl_assigns
%type <regvar>  register_variable
%type <regvars> register_variable_list
%type <item>  case_item
%type <items> case_items
%type <inst>  module_instance
%type <insts> module_instance_list
%type <expr>  expression expr_primary
%type <expr>  lavalue lpvalue
%type <exprs> expression_list
%type <exprs> range range_opt
%type <type>  net_type
%type <type>  v2k_net_type
%type <type> gatetype
%type <type> port_type
%type <exprs> range_or_type_opt
%type <evexprs> event_expression_list
%type <evexpr> event_expression
%type <evstat> event_control
%type <stat>  statement statement_opt
%type <stats> statement_list

%type <nets>  block_item_decls_opt
%type <nets>  block_item_decls
%type <nets>  block_item_decl

%type <params> parameter_value_opt
%type <params> parameter_value_byname_list
%type <params> parameter_value_byname

%type <defparams> defparam_assign
%type <defparams> defparam_assign_list

////////////////////////////////////////////////////////////////////////

%right '?' ':'
%left K_LOR
%left K_LAND
%left '|'
%left '^' K_NXOR K_NOR
%left '&' K_NAND
%left K_EQ K_NE K_CEQ K_CNE
%left K_GE K_LE '<' '>'
%left K_LS K_RS
%left K_ALS K_ARS
%left '+' '-'
%left '*' '/' '%'
%left K_POW
%left UNARY_PREC

////////////////////////////////////////////////////////////////////////

%nonassoc less_than_K_else
%nonassoc K_else

////////////////////////////////////////////////////////////////////////

%%

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

main
: source_file
|
;

source_file
: description
| source_file description
;

description
: module
| udp_primitive
{
  error("not supported.");
}
| K_D_attribute '(' IDENTIFIER ',' STRING ',' STRING ')'
{
  error("not supported.");
  //  delete $3;
  //  delete $5;
  //  delete $7;
}
;

module
: K_module IDENTIFIER
{
  module_ =source_->addModule( $2 );
  //  delete $2;
}
list_of_ports_opt ';' module_item_list_opt K_endmodule
;

module_item_list_opt
: module_item_list
|
;

module_item_list
: module_item_list module_item
| module_item
;


port_item
: port_type range_opt list_of_variables
{
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      module_->newNet(*i,moe::Verilog::Net::IMPLICIT,msb,lsb,$1);
      // delete *i;
    }
  
  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;
}
| port_type v2k_net_type range_opt list_of_variables
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( $3!=NULL )
	{
	  msb =(*$3)[0]->clone();
	  lsb =(*$3)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      module_->newNet(*i,$2,msb,lsb,$1);
      // delete *i;
    }
  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  // delete $4;
}
| port_type K_signed range_opt list_of_variables
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( $3!=NULL )
	{
	  msb =(*$3)[0]->clone();
	  lsb =(*$3)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      module_->newNet(*i,moe::Verilog::Net::IMPLICIT,msb,lsb,$1,
		      NULL,NULL,true);
      // delete *i;
    }
  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  // delete $4;
}
| port_type K_signed v2k_net_type range_opt list_of_variables
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$5->begin();i!=$5->end();++i )
    {
      if( $4!=NULL )
	{
	  msb =(*$4)[0]->clone();
	  lsb =(*$4)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      module_->newNet(*i,$3,msb,lsb,$1,NULL,NULL,true);
      // delete *i;
    }
  if( $4!=NULL )
    {
      // delete (*$4)[0];
      // delete (*$4)[1];
      // delete $4;
    }
  // delete $5;
}
;


module_item
: port_item ';'
| net_type range_opt list_of_variables ';'
{
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}

      module_->newNet(*i,$1,msb,lsb,moe::Verilog::Net::PRIVATE);
      // delete *i;
    }

  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;
}
| net_type range_opt net_decl_assigns ';'
{
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      module_->newNet(*i,$1,msb,lsb,moe::Verilog::Net::PRIVATE);
      // delete *i;
    }

  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;
}
| K_reg range_opt register_variable_list ';'
{
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<moe::Verilog::Net::nrm_*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      
      module_->newNet((*i)->name,(*i)->type,msb,lsb,moe::Verilog::Net::PRIVATE,
		      (((*i)->start!=NULL)?(*i)->start->clone():NULL),
		      (((*i)->end!=NULL)?(*i)->end->clone():NULL));
      // delete *i;
    }
  
  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;
}
| net_type K_signed range_opt list_of_variables ';'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( $3!=NULL )
	{
	  msb =(*$3)[0]->clone();
	  lsb =(*$3)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}

      module_->newNet(*i,$1,msb,lsb,moe::Verilog::Net::PRIVATE,
		      NULL,NULL,true);
      // delete *i;
    }

  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  // delete $4;
}
| net_type K_signed range_opt net_decl_assigns ';'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( $3!=NULL )
	{
	  msb =(*$3)[0]->clone();
	  lsb =(*$3)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      module_->newNet(*i,$1,msb,lsb,moe::Verilog::Net::PRIVATE,
		      NULL,NULL,true);
      // delete *i;
    }

  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  // delete $4;
}
| K_reg K_signed range_opt register_variable_list ';'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<moe::Verilog::Net::nrm_*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( $3!=NULL )
	{
	  msb =(*$3)[0]->clone();
	  lsb =(*$3)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      
      module_->newNet((*i)->name,(*i)->type,msb,lsb,moe::Verilog::Net::PRIVATE,
		      (((*i)->start!=NULL)?(*i)->start->clone():NULL),
		      (((*i)->end!=NULL)?(*i)->end->clone():NULL),
		      true);
      // delete *i;
    }
  
  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  // delete $4;
}
| K_integer list_of_variables ';'
{
  list<char*>::iterator i;
  for( i=$2->begin();i!=$2->end();++i )
    {
      module_->newNet(*i,moe::Verilog::Net::INTEGER,
		      NULL,NULL,moe::Verilog::Net::PRIVATE);
      // delete *i;
    }
  // delete $2;
}
| K_parameter parameter_assign_list ';'
| K_assign drive_strength_opt delay3_opt assign_list ';'
| IDENTIFIER parameter_value_opt attribute_instance_opt module_instance_list ';'
{
  vector<moe::Verilog::Instance*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( *i!=NULL )
	{
	  (*i)->setType($1);

	  if( $2!=NULL )
	    (*i)->setParameters($2);
	}
    }
  // delete $1;
  // delete $4;
}
| K_always statement
{
  moe::Verilog::Process* proc =new moe::Verilog::Process(moe::Verilog::Process::ALWAYS,$2);
  module_->addProcess( proc );
}
| K_function range_or_type_opt IDENTIFIER ';'
{
  moe::Verilog::Expression* msb=NULL;
  moe::Verilog::Expression* lsb=NULL;
  if( $2!=NULL )
    {
      msb =(*$2)[0]->clone();
      lsb =(*$2)[1]->clone();
      
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }

  function_ =module_->newFunction($3);
  moe::Verilog::Net* net =new
    moe::Verilog::Net(moe::Verilog::Net::FUNCTION,
		      msb,lsb,moe::Verilog::Net::OUTPUT);
  function_->addNet($3,net);

  // delete $3;
}
func_body K_endfunction
////////////////////////////////////////////////////////////////////////
| K_defparam defparam_assign_list ';'
{
  module_->addDefparam( $2 );
}
| K_event list_of_variables ';'
{
  //  list<char*>::iterator i;
  //  for( i=$2->begin();i!=$2->end();++i )
    // delete *i;
  // delete $2;
}
| K_initial statement
{
  error("not supported.");
}
| K_task IDENTIFIER ';' task_body K_endtask
{
  error("not supported.");
  // delete $2;
}
| K_specify specify_item_list K_endspecify
{
  error("not supported.");
}
| K_D_attribute '(' IDENTIFIER ',' STRING ',' STRING ')' ';'
{
  error("not supported.");
  // delete $3;
  // delete $5;
  // delete $7;
}
| K_D_attribute '(' error ')' ';'
{
  error("not supported.");
}
| K_trireg charge_strength_opt range_opt delay3_opt list_of_variables ';'
{
  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  list<char*>::iterator i;
  //  for( i=$5->begin();i!=$5->end();++i )
    // delete *i;
  // delete $5;
}
| gatetype gate_instance_list ';'
{
}
| gatetype drive_strength gate_instance_list ';'
{
}
| gatetype delay3 gate_instance_list ';'
{
}
| gatetype drive_strength delay3 gate_instance_list ';'
{
}
| K_assign error '=' expression ';'
;

assign_list
: assign_list ',' assign
| assign
;

assign
: lavalue '=' expression
{
  module_->addAssign( $1,$3 );
}
;

statement_opt
: statement
| ';'
{
  $$ =NULL;
}
;

statement_list
: statement_list statement
{
  $1->push_back($2);
  $$ =$1;
}
| statement
{
  vector<moe::Verilog::Statement*>*tmp = new vector<moe::Verilog::Statement*>;
  tmp->push_back($1);
  $$ = tmp;
}
;

statement
: K_begin statement_list K_end
{
  moe::Verilog::Block* tmp =new moe::Verilog::Block(moe::Verilog::Block::SEQUENTIAL,*$2);
  $$ = tmp;
}
| K_begin K_end
{
  moe::Verilog::Block* tmp =new moe::Verilog::Block(moe::Verilog::Block::SEQUENTIAL);
  $$ = tmp;
}
| K_case '(' expression ')' case_items K_endcase
{
  moe::Verilog::Case* tmp =new moe::Verilog::Case(moe::Verilog::Case::CASE, $3,*$5);
  // delete $5;
  $$ = tmp;
}
| K_casex '(' expression ')' case_items K_endcase
{
  moe::Verilog::Case* tmp =new moe::Verilog::Case(moe::Verilog::Case::CASEX, $3,*$5);
  // delete $5;
  $$ = tmp;
}
| K_casez '(' expression ')' case_items K_endcase
{
  moe::Verilog::Case* tmp =new moe::Verilog::Case(moe::Verilog::Case::CASEZ, $3,*$5);
  // delete $5;
  $$ = tmp;
}
| K_if '(' expression ')' statement_opt %prec less_than_K_else
{
  moe::Verilog::Condition* tmp =new moe::Verilog::Condition($3,$5,NULL);
  $$ = tmp;
}
| K_if '(' expression ')' statement_opt K_else statement_opt
{
  moe::Verilog::Condition* tmp =new moe::Verilog::Condition($3,$5,$7);
  $$ = tmp;
}
| lpvalue '=' expression ';'
{
  moe::Verilog::Assign* tmp =new moe::Verilog::Assign(moe::Verilog::Assign::BLOCKING,$1,$3);
  $$ = tmp;
}
| lpvalue K_LE expression ';'
{
  moe::Verilog::Assign* tmp =new moe::Verilog::Assign(moe::Verilog::Assign::NONBLOCKING,$1,$3);
  $$ = tmp;
}
| lpvalue '=' delay1 expression ';'
{
  moe::Verilog::Assign* tmp =new moe::Verilog::Assign(moe::Verilog::Assign::BLOCKING,$1,$4);
  $$ = tmp;
}
| lpvalue K_LE delay1 expression ';'
{
  moe::Verilog::Assign* tmp =new moe::Verilog::Assign(moe::Verilog::Assign::NONBLOCKING,$1,$4);
  $$ =tmp;
}
| lpvalue '=' DIDENTIFIER expression ';'
{
  moe::Verilog::Assign* tmp =new moe::Verilog::Assign(moe::Verilog::Assign::BLOCKING,$1,$4);
  $$ = tmp;
}
| lpvalue K_LE DIDENTIFIER expression ';'
{
  moe::Verilog::Assign* tmp =new moe::Verilog::Assign(moe::Verilog::Assign::NONBLOCKING,$1,$4);
  $$ =tmp;
}
| event_control statement_opt
{
  $1->setStatement($2);
  $$ =$1;
}
////////////////////////////////////////////////////////////////////////
| K_begin ':' IDENTIFIER block_item_decls_opt statement_list K_end
{
  moe::Verilog::Block* tmp =new moe::Verilog::Block(moe::Verilog::Block::SEQUENTIAL,*$5,$3,module_);
  {
    map<string,moe::Verilog::Net*>::const_iterator i;
    for( i=(*$4).begin();i!=(*$4).end();++i )
      {
	string aname =string($3) + string(".") + i->first;
	module_->addNet( aname.c_str() ,i->second );
	i->second->setType(moe::Verilog::Net::NAMEDBLOCK_REG);
      }
  }
  // delete $4;
  $$ = tmp;
}
| K_begin ':' IDENTIFIER K_end
{
}
| K_assign lavalue '=' expression ';'
{
}
| K_deassign lavalue';'
{ 
}
| K_disable IDENTIFIER ';'
{
}
| K_force lavalue '=' expression ';'
{
}
| K_TRIGGER IDENTIFIER ';'
{
}
| K_forever statement
{
}
| K_fork statement_list K_join
{
}
| K_fork ':' IDENTIFIER block_item_decls_opt statement_list K_join
{
}
| K_fork K_join
{
}
| K_fork ':' IDENTIFIER K_join
{
}
| K_release lavalue ';'
{
}
| K_repeat '(' expression ')' statement
{
}
| K_for '(' lpvalue '=' expression ';' expression ';' lpvalue '=' expression ')' statement
{
  moe::Verilog::For* tmp =new moe::Verilog::For((moe::Verilog::Identifier*)$3,$5,
						$7,
						(moe::Verilog::Identifier*)$9,$11,
						$13);
  $$ = tmp;
}
| K_while '(' expression ')' statement
{
}
| delay1 statement_opt
{
}
| lpvalue '=' event_control expression ';'
{
}
| lpvalue '=' K_repeat '(' expression ')' event_control expression ';'
{
}
| lpvalue K_LE event_control expression ';'
{
}
| lpvalue K_LE K_repeat '(' expression ')' event_control expression ';'
{
}
| K_wait '(' expression ')' statement_opt
{
}
| SYSTEM_IDENTIFIER '(' expression_list ')' ';'
{
  moe::Verilog::CallTask* tmp =new moe::Verilog::CallTask($1,*$3);
  $$ = tmp;
}
| SYSTEM_IDENTIFIER '(' ')' ';'
{

}
| SYSTEM_IDENTIFIER ';'
{
  
}
| identifier '(' expression_list ')' ';'
{
}
| identifier '(' ')' ';'
{
}
| identifier ';'
{
}
| error ';'
{
}
| K_begin error K_end
{ yyerrok; }
| K_case '(' expression ')' error K_endcase
{ yyerrok; }
| K_casex '(' expression ')' error K_endcase
{ yyerrok; }
| K_casez '(' expression ')' error K_endcase
{ yyerrok; }
| K_if '(' error ')' statement_opt %prec less_than_K_else
{ yyerrok; }
| K_if '(' error ')' statement_opt K_else statement_opt
{ yyerrok; }
| K_for '(' error ')' statement
{ yyerrok; }
| K_while '(' error ')' statement
{ yyerrok; }
| K_for '(' lpvalue '=' expression ';' expression ';' error ')' statement
{ yyerrok; }
| K_for '(' lpvalue '=' expression ';' error ';' lpvalue '=' expression ')' statement
{ yyerrok; }
;

case_items
: case_items case_item
{
  $1->push_back($2);
  $$ =$1;
}
| case_item
{
  vector<moe::Verilog::Case::Item*>*tmp = new vector<moe::Verilog::Case::Item*>;
  tmp->push_back($1);
  $$ = tmp;
}
;

case_item
: expression_list ':' statement_opt
{
  moe::Verilog::Case::Item* tmp = new moe::Verilog::Case::Item(*$1,$3);
  // delete $1;
  $$ = tmp;
}
| K_default ':' statement_opt
{
  moe::Verilog::Case::Item*tmp = new moe::Verilog::Case::Item($3);
  $$ = tmp;
}
| K_default  statement_opt
{
  moe::Verilog::Case::Item*tmp = new moe::Verilog::Case::Item($2);
  $$ = tmp;
}
| error ':' statement_opt
{
  yyerrok;
}
;

event_expression_list
: event_expression
{
  vector<moe::Verilog::Event*>*tmp = new vector<moe::Verilog::Event*>;
  tmp->push_back($1);
  $$ = tmp;
}
| event_expression_list K_or event_expression
{
  $1->push_back($3);
  $$ =$1;
}
| event_expression_list ',' event_expression
{ // Verilog-2000 enhancements
  $1->push_back($3);
  $$ =$1;
}
;

event_expression
: K_posedge expression
{
  moe::Verilog::Event* tmp = new moe::Verilog::Event(moe::Verilog::Event::POSEDGE,$2);
  $$ = tmp;
}
| K_negedge expression
{
  moe::Verilog::Event* tmp = new moe::Verilog::Event(moe::Verilog::Event::NEGEDGE,$2);
  $$ = tmp;
}
| expression
{
  moe::Verilog::Event* tmp = new moe::Verilog::Event(moe::Verilog::Event::ANYEDGE,$1);
  $$ = tmp;
}
;

expression_list
: expression_list ',' expression
{
  $1->push_back($3);
  $$ =$1;
}
| expression
{
  vector<moe::Verilog::Expression*>*tmp = new vector<moe::Verilog::Expression*>;
  tmp->push_back($1);
  $$ = tmp;
}
| expression_list ','
{
  $$ = $1;
}
;

expr_primary
: NUMBER
{
  moe::Verilog::Number* tmp = new moe::Verilog::Number($1);
  // delete $1;
  $$ = tmp;
}
| REALTIME
{
  error("not supported.");
  // delete $1;
  $$ = 0;
}
| STRING
{
  moe::Verilog::String* tmp = new moe::Verilog::String($1);
  // delete $1;
  $$ = tmp;
}
| identifier
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1);
  // delete $1;
  $$ = tmp;
}
| SYSTEM_IDENTIFIER
{
  error("not supported.");
  // delete $1;
  $$ = 0;
}
| identifier '[' expression ']'
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,NULL,NULL,$3);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression ':' expression ']'
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,$3,$5);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression K_PLUSRANGE expression ']'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb =new moe::Verilog::Binary
    (
     moe::Verilog::Expression::ArithmeticMinus,
     (new moe::Verilog::Binary
      (
       moe::Verilog::Expression::ArithmeticAdd,$3,$5
       )
      ),
     (new moe::Verilog::Number("1"))
     );
  moe::Verilog::Expression* lsb =($3)->clone();

  moe::Verilog::Identifier* tmp =new moe::Verilog::Identifier($1,msb,lsb);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression K_MINUSRANGE expression ']'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* lsb =new moe::Verilog::Binary
    (
     moe::Verilog::Expression::ArithmeticAdd,
     new moe::Verilog::Binary
     (
      moe::Verilog::Expression::ArithmeticMinus,$3,$5
      ),
     new moe::Verilog::Number("1")
     );
  moe::Verilog::Expression* msb =($3)->clone();

  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,msb,lsb);
  // delete $1;
  $$ = tmp;
}
| identifier '(' expression_list ')'
{
  moe::Verilog::CallFunction* tmp = new moe::Verilog::CallFunction($1,*$3);
  $$ = tmp;
}
| SYSTEM_IDENTIFIER '(' expression_list ')'
{
  error("not supported.");
  // delete $1;
  $$ = 0;
}
| '(' expression ')'
{
  $$ = $2;
}
| '{' expression_list '}'
{
  moe::Verilog::Concat* tmp = new moe::Verilog::Concat(*$2);
  // delete $2;
  $$ = tmp;
}
| '{' expression '{' expression_list '}' '}'
{
  moe::Verilog::Concat* tmp = new moe::Verilog::Concat($2,*$4);
  // delete $4;
  $$ = tmp;
}
;

func_body
: function_item_list statement
{
  function_->setStatement($2);
}
| function_item_list
;

function_item_list
: function_item
| function_item_list function_item
;

function_item
: K_input range_opt list_of_variables ';'
{
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<char*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}

      function_->addNet(*i,new moe::Verilog::Net
			(moe::Verilog::Net::IMPLICIT,
			 msb,lsb,moe::Verilog::Net::INPUT) );
      // delete *i;
    }

  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;
}
| K_reg range_opt register_variable_list ';'
{
  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<moe::Verilog::Net::nrm_*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}

      function_->addNet((*i)->name,new moe::Verilog::Net
			((*i)->type,msb,lsb,moe::Verilog::Net::PRIVATE,
			 (((*i)->start!=NULL)?(*i)->start->clone():NULL),
			 (((*i)->end!=NULL)?(*i)->end->clone():NULL)));
      // delete *i;
    }

  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;
}
| K_integer list_of_variables ';'
{
  list<char*>::iterator i;
  for( i=$2->begin();i!=$2->end();++i )
    {
      function_->addNet(*i,new moe::Verilog::Net
			(moe::Verilog::Net::INTEGER,
			 NULL,NULL,moe::Verilog::Net::PRIVATE));
      // delete *i;
    }
  // delete $2;
}
;

module_instance_list
: module_instance_list ',' module_instance
{
  $1->push_back($3);
  $$ =$1;
}
| module_instance
{
  vector<moe::Verilog::Instance*>*tmp = new vector<moe::Verilog::Instance*>;
  tmp->push_back($1);
  $$ = tmp;
}
;

module_instance
: IDENTIFIER
{
  instance_ =module_->newInstance($1);
  // delete $1;
}
'(' port_name_list ')'
{
  $$ =instance_;
}
| '(' expression_list ')' // UDP_instance
{
  // delete $2;
  $$ =NULL;
}
;

port_name_list
: port_name_list ',' port_name
| port_name
;

port_name
: PORTNAME '(' expression ')'
{
  moe::Verilog::Instance::Port* port =new moe::Verilog::Instance::Port($1,$3);
  instance_->addPort( port );
}
| PORTNAME '(' ')'
{
  moe::Verilog::Instance::Port* port =new moe::Verilog::Instance::Port($1,NULL);
  instance_->addPort( port );
}
| PORTNAME '(' error ')'
{
  moe::Verilog::Instance::Port* port =new moe::Verilog::Instance::Port($1,NULL);
  instance_->addPort( port );
}
| expression
{
  moe::Verilog::Instance::Port* port =new moe::Verilog::Instance::Port("",$1);
  instance_->addPort( port );
}
|
{
}
;


identifier
: IDENTIFIER
{
  $$ =$1;
}
| HIDENTIFIER
{
  $$ =$1;
}
| DIDENTIFIER
{
  $$ =$1;
}
;

// Verilog-2000 enhancements
v2k_port_item
: port_type range_opt IDENTIFIER
{
  moe::Verilog::Expression* msb=NULL;
  moe::Verilog::Expression* lsb=NULL;
  if( $2!=NULL )
    {
      msb =(*$2)[0]->clone();
      lsb =(*$2)[1]->clone();
      
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }

  module_->newNet($3,moe::Verilog::Net::IMPLICIT,
		  msb,lsb,
		  $1 );
  module_->addPort( $3 );

  // delete $3;
}
| port_type v2k_net_type range_opt IDENTIFIER
{
  moe::Verilog::Expression* msb=NULL;
  moe::Verilog::Expression* lsb=NULL;
  if( $3!=NULL )
    {
      msb =(*$3)[0]->clone();
      lsb =(*$3)[1]->clone();
      
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }

  module_->newNet($4,$2,
		  msb,lsb,
		  $1);
  module_->addPort( $4 );

  // delete $4;
}
| port_type K_signed v2k_net_type range_opt IDENTIFIER
{ // Verilog-2001 enhancements
  moe::Verilog::Expression* msb=NULL;
  moe::Verilog::Expression* lsb=NULL;
  if( $4!=NULL )
    {
      msb =(*$4)[0]->clone();
      lsb =(*$4)[1]->clone();
      
      // delete (*$4)[0];
      // delete (*$4)[1];
      // delete $4;
    }

  module_->newNet($5,$3,
		  msb,lsb,
		  $1,
		  NULL,NULL,
		  true);
  module_->addPort( $5 );

  // delete $5;
}
;

port_opt
: IDENTIFIER
{
  module_->addPort( $1 );
  // delete $1;
}
| v2k_port_item
;

list_of_ports
: port_opt
| list_of_ports ',' port_opt
;

list_of_ports_opt
: '(' list_of_ports ')'
| '(' ')'
|
;

list_of_variables
: IDENTIFIER
{
  list<char*>* tmp =new list<char*>;
  tmp->push_back($1);
  $$ =tmp;
}
| list_of_variables ',' IDENTIFIER
{
  $1->push_back($3);
  $$ =$1;
}
;

lavalue
: identifier
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression ']'
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,NULL,NULL,$3);
  // delete $1;
  $$ = tmp;
}
| identifier range
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,(*$2)[0],(*$2)[1]);
  // delete $1;
  // delete $2;
  $$ = tmp;
}
| '{' expression_list '}'
{
  moe::Verilog::Concat* tmp = new moe::Verilog::Concat(*$2);
  // delete $2;
  $$ = tmp;
}
;

lpvalue
: identifier
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression ']'
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,NULL,NULL,$3);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression ':' expression ']'
{
  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,$3,$5);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression K_PLUSRANGE expression ']'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* msb =new moe::Verilog::Binary
    (
     moe::Verilog::Expression::ArithmeticMinus,
     (new moe::Verilog::Binary
      (
       moe::Verilog::Expression::ArithmeticAdd,$3,$5
       )
      ),
     (new moe::Verilog::Number("1"))
     );
  moe::Verilog::Expression* lsb =($3)->clone();

  moe::Verilog::Identifier* tmp =new moe::Verilog::Identifier($1,msb,lsb);
  // delete $1;
  $$ = tmp;
}
| identifier '[' expression K_MINUSRANGE expression ']'
{ // Verilog-2000 enhancements
  moe::Verilog::Expression* lsb =new moe::Verilog::Binary
    (
     moe::Verilog::Expression::ArithmeticAdd,
     new moe::Verilog::Binary
     (
      moe::Verilog::Expression::ArithmeticMinus,$3,$5
      ),
     new moe::Verilog::Number("1")
     );
  moe::Verilog::Expression* msb =($3)->clone();

  moe::Verilog::Identifier* tmp = new moe::Verilog::Identifier($1,msb,lsb);
  // delete $1;
  $$ = tmp;
}
| '{' expression_list '}'
{
  moe::Verilog::Concat* tmp = new moe::Verilog::Concat(*$2);
  // delete $2;
  $$ = tmp;
}
;

net_decl_assign
: IDENTIFIER '=' expression
{
  module_->addAssign( new moe::Verilog::Identifier($1),$3 );
  $$ =$1;
}
| delay1 IDENTIFIER '=' expression
{
  module_->addAssign( new moe::Verilog::Identifier($2),$4 );
  $$ =$2;
}
;

net_decl_assigns
: net_decl_assigns ',' net_decl_assign
{
  $1->push_back($3);
  $$ =$1;
}
| net_decl_assign
{
  list<char*>*tmp =new list<char*>;
  tmp->push_back($1);
  $$ =tmp;
}
;

parameter_assign_list
: parameter_assign
| parameter_assign_list ',' parameter_assign
;
parameter_assign
: range_opt IDENTIFIER '=' expression
{
  moe::Verilog::Expression* msb=NULL;
  moe::Verilog::Expression* lsb=NULL;
  if( $1!=NULL )
    {
      msb =(*$1)[0]->clone();
      lsb =(*$1)[1]->clone();
      
      // delete (*$1)[0];
      // delete (*$1)[1];
      // delete $1;
    }

  module_->newNet($2,moe::Verilog::Net::PARAMETER,msb,lsb,
		  moe::Verilog::Net::PRIVATE );
  module_->addParameter( new moe::Verilog::Identifier($2),$4 );

  // delete $2;
}
;

range
: '[' expression ':' expression ']'
{
  vector<moe::Verilog::Expression*>*tmp = new vector<moe::Verilog::Expression*>(2);
  (*tmp)[0] = $2;
  (*tmp)[1] = $4;
  $$ = tmp;
}
| '[' expression K_PLUSRANGE expression ']'
{ // Verilog-2000 enhancements
  vector<moe::Verilog::Expression*>*tmp = new vector<moe::Verilog::Expression*>(2);
  (*tmp)[0] = new moe::Verilog::Binary
    (
     moe::Verilog::Expression::ArithmeticMinus,
     new moe::Verilog::Binary
     (
      moe::Verilog::Expression::ArithmeticAdd, $2,$4 ),
     new moe::Verilog::Number("1") ) ;
  (*tmp)[1] = ($2)->clone();
  $$ = tmp;
}
| '[' expression K_MINUSRANGE expression ']'
{ // Verilog-2000 enhancements
  vector<moe::Verilog::Expression*>*tmp = new vector<moe::Verilog::Expression*>(2);
  (*tmp)[0] = ($2)->clone();
  (*tmp)[1] = new moe::Verilog::Binary
    (
     moe::Verilog::Expression::ArithmeticAdd,
     new moe::Verilog::Binary
     (
      moe::Verilog::Expression::ArithmeticMinus, $2,$4 ),
     new moe::Verilog::Number("1") ) ;
  $$ = tmp;
}
;

range_opt
: range
|
{
  $$ =0;
}
;

range_or_type_opt
: range 
{
  $$ =$1;
}
| K_integer
{
  $$ =NULL;
}
| K_real
{
  $$ =NULL;
}
|
{
  $$ =NULL;
}
;

register_variable_list
: register_variable
{
  list<moe::Verilog::Net::nrm_*>* tmp =new list<moe::Verilog::Net::nrm_*>;
  tmp->push_back($1);
  $$ =tmp;
 
}
| register_variable_list ',' register_variable
{
  $1->push_back($3);
  $$ =$1;
}
;
register_variable
: IDENTIFIER
{
  moe::Verilog::Net::nrm_* tmp =new moe::Verilog::Net::nrm_;
  tmp->name  =$1;
  tmp->start =NULL;
  tmp->end   =NULL;
  tmp->type  =moe::Verilog::Net::REG;
  $$ =tmp;
}
| IDENTIFIER '[' expression ':' expression ']'
{
  moe::Verilog::Net::nrm_* tmp =new moe::Verilog::Net::nrm_;
  tmp->name  =$1;
  tmp->start =$3;
  tmp->end   =$5;
  tmp->type  =moe::Verilog::Net::REG;
  $$ =tmp;
}
;

expression
: expr_primary
{
  $$ = $1;
}
| '+' expr_primary %prec UNARY_PREC
{
  $$ = $2;
}
| '-' expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ArithmeticMinus,$2);
  $$ = tmp;
}
| '~' expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::BitwiseNegation,$2);
  $$ = tmp;
}
| '&' expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ReductionAND,$2);
  $$ = tmp;
}
| '!' expr_primary %prec UNARY_PREC
{
    moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::LogicalNegation,$2);
  $$ = tmp;
}
| '|' expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ReductionOR,$2);
  $$ = tmp;
}
| '^' expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ReductionXOR,$2);
  $$ = tmp;
}
| K_NAND expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ReductionNAND,$2);
  $$ = tmp;
}
| K_NOR expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ReductionNOR,$2);
  $$ = tmp;
}
| K_NXOR expr_primary %prec UNARY_PREC
{
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::ReductionNXOR,$2);
  $$ = tmp;
}
| expression '^' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::BitwiseXOR,$1,$3);
  $$ = tmp;
}
| expression '*' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticMultiply,$1,$3);
  $$ = tmp;
}
| expression '/' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticDivide,$1,$3);
  $$ = tmp;
}
| expression '%' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticModulus,$1,$3);
  $$ = tmp;
}
| expression '+' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticAdd,$1,$3);
  $$ = tmp;
}
| expression '-' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticMinus,$1,$3);
  $$ = tmp;
}
| expression '&' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::BitwiseAND,$1,$3);
  $$ = tmp;
}
| expression '|' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::BitwiseOR,$1,$3);
  $$ = tmp;
}
| expression K_NOR expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::BitwiseNOR,$1,$3);
  $$ = tmp;
}
| expression K_NXOR expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::BitwiseNXOR,$1,$3);
  $$ = tmp;
}
| expression '<' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LessThan,$1,$3);
  $$ = tmp;
}
| expression '>' expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::GreaterThan,$1,$3);
  $$ = tmp;
}
| expression K_LS expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LeftShift,$1,$3);
  $$ = tmp;
}
| expression K_RS expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::RightShift,$1,$3);
  $$ = tmp;
}
| expression K_ALS expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticLeftShift,$1,$3);
  $$ = tmp;
}
| expression K_ARS expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticRightShift,$1,$3);
  $$ = tmp;
}
| expression K_POW expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::ArithmeticPower,$1,$3);
  $$ = tmp;
}
| expression K_EQ expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LogicalEquality,$1,$3);
  $$ = tmp;
}
| expression K_CEQ expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::CaseEquality,$1,$3);
  $$ = tmp;
}
| expression K_LE expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LessEqual,$1,$3);
  $$ = tmp;
}
| expression K_GE expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::GreaterEqual,$1,$3);
  $$ = tmp;
}
| expression K_NE expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LogicalInequality,$1,$3);
  $$ = tmp;
}
| expression K_CNE expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::CaseInequality,$1,$3);
  $$ = tmp;
}
| expression K_LOR expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LogicalOR,$1,$3);
  $$ = tmp;
}
| expression K_LAND expression
{
  moe::Verilog::Binary* tmp = new moe::Verilog::Binary( moe::Verilog::Expression::LogicalAND,$1,$3);
  $$ = tmp;
}
| expression '?' expression ':' expression
{
  moe::Verilog::Ternary* tmp = new moe::Verilog::Ternary($1,$3,$5);
  $$ = tmp;
}
| K_SIGNED '(' expression ')'
{ // Verilog-2000 enhancements
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::CastSigned,$3);
  $$ = tmp;
}
| K_UNSIGNED '(' expression ')'
{ // Verilog-2000 enhancements
  moe::Verilog::Unary* tmp = new moe::Verilog::Unary( moe::Verilog::Expression::CastUnsigned,$3);
  $$ = tmp;
}
;

gatetype
: K_and  
{
  $$ =moe::Verilog::Gate::AND;
}
| K_nand 
{
  $$ =moe::Verilog::Gate::NAND;
}
| K_or   
{
  $$ =moe::Verilog::Gate::OR;
}
| K_nor  
{
  $$ =moe::Verilog::Gate::NOR;
}
| K_xor  
{
  $$ =moe::Verilog::Gate::XOR;
}
| K_xnor 
{
  $$ =moe::Verilog::Gate::XNOR;
}
| K_buf  
{
  $$ =moe::Verilog::Gate::BUF;
}
| K_bufif0 
{
  $$ =moe::Verilog::Gate::BUFIF0;
}
| K_bufif1 
{
  $$ =moe::Verilog::Gate::BUFIF1;
}
| K_not    
{
  $$ =moe::Verilog::Gate::NOT;
}
| K_notif0 
{
  $$ =moe::Verilog::Gate::NOTIF0;
}
| K_notif1 
{
  $$ =moe::Verilog::Gate::NOTIF1;
}
| K_pulldown 
{
  $$ =moe::Verilog::Gate::PULLDOWN;
}
| K_pullup   
{
  $$ =moe::Verilog::Gate::PULLUP;
}
| K_nmos  
{
  $$ =moe::Verilog::Gate::NMOS;
}
| K_rnmos 
{
  $$ =moe::Verilog::Gate::RNMOS;
}
| K_pmos  
{
  $$ =moe::Verilog::Gate::PMOS;
}
| K_rpmos 
{
  $$ =moe::Verilog::Gate::RPMOS;
}
| K_cmos  
{
  $$ =moe::Verilog::Gate::CMOS;
}
| K_rcmos 
{
  $$ =moe::Verilog::Gate::RCMOS;
}
| K_tran  
{
  $$ =moe::Verilog::Gate::TRAN;
}
| K_rtran 
{
  $$ =moe::Verilog::Gate::RTRAN;
}
| K_tranif0 
{
  $$ =moe::Verilog::Gate::TRANIF0;
}
| K_tranif1 
{
  $$ =moe::Verilog::Gate::TRANIF1;
}
| K_rtranif0 
{
  $$ =moe::Verilog::Gate::RTRANIF0;
}
| K_rtranif1 
{
  $$ =moe::Verilog::Gate::RTRANIF1;
}
;

port_type
: K_input 
{
  $$ =moe::Verilog::Net::INPUT;
}
| K_output
{
  $$ =moe::Verilog::Net::OUTPUT;
}
| K_inout
{
  $$ =moe::Verilog::Net::INOUT;
}
;

net_type
: K_wire    
{
  $$ =moe::Verilog::Net::WIRE;
}
| K_tri     
{
  $$ =moe::Verilog::Net::TRI;
}
| K_tri1    
{
  $$ =moe::Verilog::Net::TRI1;
}
| K_supply0 
{
  $$ =moe::Verilog::Net::SUPPLY0;
}
| K_wand    
{
  $$ =moe::Verilog::Net::WAND;
}
| K_triand  
{
  $$ =moe::Verilog::Net::TRIAND;
}
| K_tri0    
{
  $$ =moe::Verilog::Net::TRI0;
}
| K_supply1 
{
  $$ =moe::Verilog::Net::SUPPLY1;
}
| K_wor     
{
  $$ =moe::Verilog::Net::WOR;
}
| K_trior   
{
  $$ =moe::Verilog::Net::TRIOR;
}
;



v2k_net_type
: K_reg
{
  $$ =moe::Verilog::Net::REG;
}
| K_wire    
{
  $$ =moe::Verilog::Net::WIRE;
}
| K_tri     
{
  $$ =moe::Verilog::Net::TRI;
}
| K_tri1    
{
  $$ =moe::Verilog::Net::TRI1;
}
| K_supply0 
{
  $$ =moe::Verilog::Net::SUPPLY0;
}
| K_wand    
{
  $$ =moe::Verilog::Net::WAND;
}
| K_triand  
{
  $$ =moe::Verilog::Net::TRIAND;
}
| K_tri0    
{
  $$ =moe::Verilog::Net::TRI0;
}
| K_supply1 
{
  $$ =moe::Verilog::Net::SUPPLY1;
}
| K_wor     
{
  $$ =moe::Verilog::Net::WOR;
}
| K_trior   
{
  $$ =moe::Verilog::Net::TRIOR;
}
;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

gate_instance_list
: gate_instance_list ',' gate_instance
| gate_instance
;
gate_instance
: IDENTIFIER '(' expression_list ')'
{
  // delete $1;
  // delete $3;
}
| '(' expression_list ')'
{
  // delete $2;
}
;


/******
port_opt
: port 
{
  $$ =$1;
}
|
{
  $$ =NULL;
}
;

port
: port_reference
{
  $$ =$1;
}
| PORTNAME '(' port_reference ')'
{
  error("not supported.");
  // delete $1;
  // delete $3;
  $$ =NULL; 
}
| '{' port_reference_list '}'
{
  error("not supported.");
  // delete $2;
  $$ =NULL; 
}
| PORTNAME '(' '{' port_reference_list '}' ')'
{
  error("not supported.");
  // delete $1;
  // delete $4;
  $$ =NULL; 
}
;

port_reference
: IDENTIFIER
{
  $$ =$1;
}
| IDENTIFIER '[' expression ':' expression ']'
{
  error("not supported.");
  // delete $1;
  // delete $3;
  // delete $5;
  $$ =NULL;
}
| IDENTIFIER '[' error ']'
{
  error("error");
}
;

port_reference_list
: port_reference
{
  $$ =$1;
}
| port_reference_list ',' port_reference
{
  error("not supported.");
  // delete $1;
  // delete $3;
  $$ =NULL;
}
;
******/

block_item_decls_opt
: block_item_decls
{
  $$ =$1;
}
|
{
  $$ =new map<string,moe::Verilog::Net*>;
}
;

block_item_decls
: block_item_decl
{
  map<string,moe::Verilog::Net*>*tmp =new map<string,moe::Verilog::Net*>;
  tmp->insert($1->begin(),$1->end());
  $$ =tmp;
}
| block_item_decls block_item_decl
{
  $1->insert($2->begin(),$2->end());
  $$ =$1;
}
;

block_item_decl
: K_reg range_opt register_variable_list ';'
{
  map<string,moe::Verilog::Net*>* tmp=new map<string,moe::Verilog::Net*>;

  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<moe::Verilog::Net::nrm_*>::iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    {
      if( $2!=NULL )
	{
	  msb =(*$2)[0]->clone();
	  lsb =(*$2)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      
      pair<map<string,moe::Verilog::Net*>::iterator,bool> ret =
	tmp->insert( pair<string,moe::Verilog::Net*>
		     ((*i)->name,new moe::Verilog::Net
		      ((*i)->type,msb,lsb,moe::Verilog::Net::PRIVATE,
		       (((*i)->start!=NULL)?(*i)->start->clone():NULL),
		       (((*i)->end!=NULL)?(*i)->end->clone():NULL))) );
      
      // delete *i;
    }
  
  if( $2!=NULL )
    {
      // delete (*$2)[0];
      // delete (*$2)[1];
      // delete $2;
    }
  // delete $3;

  $$ =tmp;
}
| K_reg K_signed range_opt register_variable_list ';'
{ // Verilog-2000 enhancements
  map<string,moe::Verilog::Net*>* tmp=new map<string,moe::Verilog::Net*>;

  moe::Verilog::Expression* msb;
  moe::Verilog::Expression* lsb;

  list<moe::Verilog::Net::nrm_*>::iterator i;
  for( i=$4->begin();i!=$4->end();++i )
    {
      if( $3!=NULL )
	{
	  msb =(*$3)[0]->clone();
	  lsb =(*$3)[1]->clone();
	}
      else
	{
	  msb =NULL;
	  lsb =NULL;
	}
      
      pair<map<string,moe::Verilog::Net*>::iterator,bool> ret =
	tmp->insert( pair<string,moe::Verilog::Net*>
		     ((*i)->name,new moe::Verilog::Net
		      ((*i)->type,msb,lsb,moe::Verilog::Net::PRIVATE,
		       (((*i)->start!=NULL)?(*i)->start->clone():NULL),
		       (((*i)->end!=NULL)?(*i)->end->clone():NULL),
		       true)) );
      // delete *i;
    }
  
  if( $3!=NULL )
    {
      // delete (*$3)[0];
      // delete (*$3)[1];
      // delete $3;
    }
  // delete $4;

  $$ =tmp;
}
| K_integer list_of_variables ';'
{
  map<string,moe::Verilog::Net*>* tmp=new map<string,moe::Verilog::Net*>;

  list<char*>::iterator i;
  for( i=$2->begin();i!=$2->end();++i )
    {
      pair<map<string,moe::Verilog::Net*>::iterator,bool> ret =
	tmp->insert( pair<string,moe::Verilog::Net*>
		     (*i,new moe::Verilog::Net
		      (moe::Verilog::Net::INTEGER,
		       NULL,NULL,moe::Verilog::Net::PRIVATE)) );
      // delete *i;
    }
  // delete $2;

  $$ =tmp;
}
;




specify_item
: K_specparam specparam_list ';'
| specify_simple_path '=' '(' expression_list ')' ';'
| K_if  '(' expression ')' specify_simple_path '=' '(' expression_list ')' ';'
| SYSTEM_IDENTIFIER '(' timing_check_event ',' timing_check_event ',' expression_list ')' ';'
{
}
;

timing_check_event
: event_expression K_AAA expression
{
}
| event_expression
{
}
;

specify_item_list
: specify_item
| specify_item_list specify_item
;

specify_simple_path
: '(' IDENTIFIER polarity_operator_opt K_EG IDENTIFIER ')'
| '(' IDENTIFIER polarity_operator_opt K_SG IDENTIFIER ')'
| '(' K_posedge IDENTIFIER K_SG '(' IDENTIFIER polarity_operator_opt ':' expression ')' ')'
| '(' K_negedge IDENTIFIER K_SG '(' IDENTIFIER polarity_operator_opt ':' expression ')' ')'
| '(' K_posedge IDENTIFIER K_EG '(' IDENTIFIER polarity_operator_opt ':' expression ')' ')'
| '(' K_negedge IDENTIFIER K_EG '(' IDENTIFIER polarity_operator_opt ':' expression ')' ')'
;

specparam
: IDENTIFIER '=' expression
{
}
;

specparam_list
: specparam
| specparam_list ',' specparam
;

polarity_operator_opt
: '+'
| '-'
|
;





defparam_assign
: identifier '=' expression
{
  map<string,moe::Verilog::Expression*>* params =new map<string,moe::Verilog::Expression*>;
  params->insert( pair<string,moe::Verilog::Expression*>
		  ($1,$3) );
  // delete $1;
  $$ =params;  
}
;

defparam_assign_list
: defparam_assign
{
  $$ =$1;
}
| range defparam_assign
{ // ? 
  $$ =$2;
}
| defparam_assign_list ',' defparam_assign
{
  if( $3!=NULL )
    $1->insert($3->begin(),$3->end());
  $$ =$1;
}
;

event_control
: '@' IDENTIFIER
{
  $$ =0;
}
| '@' '(' event_expression_list ')'
{
  moe::Verilog::EventStatement* tmp = new moe::Verilog::EventStatement(*$3);
  // delete $3;
  $$ = tmp;
}
| '@' '*'
{ // Verilog-2000 enhancements
  moe::Verilog::EventStatement* tmp = new moe::Verilog::EventStatement();
  $$ = tmp;
}
| '@' '(' error ')'
{
  $$ =0;
}
;

task_body
: task_item_list_opt statement_opt
{
}
;

task_item
: K_reg range register_variable_list ';'
{
}
| K_reg register_variable_list ';'
{
}
| K_integer list_of_variables ';'
{
}
| K_input range_opt list_of_variables ';'
{
}
| K_output range_opt list_of_variables ';'
{
}
| K_inout range_opt list_of_variables ';'
{
}
;

task_item_list_opt
: task_item_list
|
;

task_item_list
: task_item_list task_item
| task_item
;

////////////////////////////////////////////////////////////////////////

udp_body
: K_table
{
  lex_start_table();
}
udp_entry_list
K_endtable
{
  lex_end_table();
}
;

udp_entry_list
: udp_comb_entry_list
| udp_sequ_entry_list
;

udp_comb_entry
: udp_input_list ':' udp_output_sym ';'
;

udp_comb_entry_list
: udp_comb_entry
| udp_comb_entry_list udp_comb_entry
;

udp_sequ_entry_list
: udp_sequ_entry
| udp_sequ_entry_list udp_sequ_entry
;

udp_sequ_entry
: udp_input_list ':' udp_input_sym ':' udp_output_sym ';'
;

udp_initial
: K_initial IDENTIFIER '=' NUMBER ';'
{
  // delete $2;
  // delete $4;
}
;

udp_init_opt
: udp_initial
|
;

udp_input_list
: udp_input_sym
| udp_input_list udp_input_sym
;

udp_input_sym
: '0'
| '1'
| 'x'
| '?'
| 'b'
| '*'
| 'f'
| 'r'
| 'n'
| 'p'
| '_'
| '+'
| '%'
| 'P'
| 'N'
| 'F'
| 'R'
;

udp_output_sym
: '0'
| '1'
| 'x'
| '-'
;

udp_port_decl
: K_input list_of_variables ';'
{
  list<char*>::iterator i;
  //for( i=$2->begin();i!=$2->end();++i )
    // delete *i;
  // delete $2;
}
| K_output IDENTIFIER ';'
{
  // delete $2;
}
| K_reg IDENTIFIER ';'
{
  // delete $2;
}
;

udp_primitive
: K_primitive IDENTIFIER '(' udp_port_list ')' ';'
{
  // delete $2;
}
udp_port_decls
udp_init_opt
udp_body
K_endprimitive
;

udp_port_list
: IDENTIFIER
{
  // delete $1;
}
| udp_port_list ',' IDENTIFIER
{
  // delete $3;
}
;

udp_port_decls
: udp_port_decl
| udp_port_decls udp_port_decl
;


////////////////////////////////////////////////////////////////////////
attribute_instance_opt
: K_ATCOMM
{
  $$ =(char*)verilog_comment.c_str();
}
|
{
  $$ =NULL;  
}
;

parameter_value_opt
: '#' '(' expression_list ')'
{
  multimap<string,moe::Verilog::Expression*>* params =new multimap<string,moe::Verilog::Expression*>;
  vector<moe::Verilog::Expression*>::const_iterator i;
  for( i=$3->begin();i!=$3->end();++i )
    params->insert( pair<string,moe::Verilog::Expression*>
		    ("",(*i)) );
  $$ =params;
}
| '#' '(' parameter_value_byname_list ')'
{
  $$ =$3;
}
| '#' NUMBER
{
  multimap<string,moe::Verilog::Expression*>* params =new multimap<string,moe::Verilog::Expression*>;
  params->insert( pair<string,moe::Verilog::Expression*>
		  ("",new moe::Verilog::Number($2)) );
  // delete $2;
  $$ =params;
}
| '#' REALTIME
{
  // delete $2;
  $$ =NULL;
}
| '#' error
{
  $$ =NULL;
}
|
{
  $$ =NULL;
}
;

//  Verilog-2000 enhancements
parameter_value_byname_list
: parameter_value_byname
{
  $$ =$1;
}
| parameter_value_byname_list ',' parameter_value_byname
{
  if( $3!=NULL )
    $1->insert($3->begin(),$3->end());
  $$ =$1;
}
;

parameter_value_byname
: PORTNAME '(' expression ')'
{
  multimap<string,moe::Verilog::Expression*>* params =new multimap<string,moe::Verilog::Expression*>;
  params->insert( pair<string,moe::Verilog::Expression*>
		  ($1,$3) );
  // delete $1;
  $$ =params;  
}
| PORTNAME '(' ')'
{
  // delete $1;
  $$ =NULL;  
}
;

delay1
: '#' delay_value_simple
| '#' '(' delay_value ')'
;

delay3_opt
: delay3
|
;

delay3
: '#' delay_value_simple
| '#' '(' delay_value ')'
| '#' '(' delay_value ',' delay_value ')'
| '#' '(' delay_value ',' delay_value ',' delay_value ')'
;

delay_value
: expression
{
  // delete $1;
}
| expression ':' expression ':' expression
{
  // delete $1;
  // delete $3;
  // delete $5;
}
;

delay_value_simple
: NUMBER
{
  // delete $1;
}
| IDENTIFIER
{
  // delete $1;
}
;

drive_strength
: '(' dr_strength0 ',' dr_strength1 ')'
| '(' dr_strength1 ',' dr_strength0 ')'
| '(' dr_strength0 ',' K_highz1 ')'
| '(' dr_strength1 ',' K_highz0 ')'
| '(' K_highz1 ',' dr_strength0 ')'
| '(' K_highz0 ',' dr_strength1 ')'
;

drive_strength_opt
: drive_strength
|
;

dr_strength0
: K_supply0
| K_strong0
| K_pull0
| K_weak0
;

dr_strength1
: K_supply1
| K_strong1
| K_pull1
| K_weak1
;

charge_strength_opt
: charge_strength
|
;

charge_strength
: '(' K_small ')'
| '(' K_medium ')'
| '(' K_large ')'
;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

%%

void verilog_error(char *str)
{
  cerr << file << " : " << line << " : " << str << endl;
}

