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

#include "parse.tab.h"
#include <cassert>

extern FILE*  verilog_input;
extern string verilog_file;
extern string verilog_comment;

unsigned int line;
string       file;

#define YY_USER_INIT reset_lexor();
#define yylval verilog_lval

static void reset_lexor();
static int comment_enter;

static void line_directive();

%}

%x ATTRIBUTE
%x CCOMMENT
%x LCOMMENT
%x CSTRING
%s UDPTABLE
%x PPTIMESCALE

W [ \t\b\f\r]+

%%

^"`line"[ ]+[0-9]+[ ]+\"[^\"]*\".* { line_directive(); }


[ \t\b\f\r] { ; }
\n { line += 1; }

"//" { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { verilog_comment.assign(yytext,strlen(yytext)-1);
                 line += 1; BEGIN(comment_enter); }

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { line += 1; yymore(); }
<CCOMMENT>"*/" { verilog_comment.assign(yytext,strlen(yytext)-2);
                 BEGIN(comment_enter); }

"(*" { comment_enter = YY_START; BEGIN(ATTRIBUTE); }
<ATTRIBUTE>.    { yymore(); }
<ATTRIBUTE>\n   { line += 1;yymore(); }
<ATTRIBUTE>"*)" { verilog_comment.assign(yytext,strlen(yytext)-2);
                   BEGIN(comment_enter);
                   return K_ATCOMM; }


"<<"         { return K_LS; }
">>"         { return K_RS; }
"<<<"        { return K_ALS; }
">>>"        { return K_ARS; }
"**"         { return K_POW; }
"<="         { return K_LE; }
">="         { return K_GE; }
"=>"         { return K_EG; }
"*>"         { return K_SG; }
"=="         { return K_EQ; }
"!="         { return K_NE; }
"==="        { return K_CEQ; }
"!=="        { return K_CNE; }
"||"         { return K_LOR; }
"&&"         { return K_LAND; }
"~|"         { return K_NOR; }
"~^"         { return K_NXOR; }
"^~"         { return K_NXOR; }
"~&"         { return K_NAND; }
"->"         { return K_TRIGGER; }
"&&&"        { return K_AAA; }
\$signed     { return K_SIGNED; }
\$unsigned   { return K_UNSIGNED; }
"+:"         { return K_PLUSRANGE; }
"-:"         { return K_MINUSRANGE; }


 /* ************************************************************************ */
 /* Annex B */
 /* B.1 All keywords */
always       { return K_always; }
and          { return K_and; }
assign       { return K_assign; }
automatic    { return K_automatic; }
begin        { return K_begin; }
buf          { return K_buf; }
bufif0       { return K_bufif0; }
bufif1       { return K_bufif1; }
case         { return K_case; }
casex        { return K_casex; }
casez        { return K_casez; }
cmos         { return K_cmos; }
deassign     { return K_deassign; }
default      { return K_default; }
defparam     { return K_defparam; }
disable      { return K_disable; }
edge         { return K_edge; }
else         { return K_else; }
end          { return K_end; }
endcase      { return K_endcase; }
endconfig    { return K_endconfig; }
endfunction  { return K_endfunction; }
endgenerate  { return K_endgenerate; }
endmodule    { return K_endmodule; }
endprimitive { return K_endprimitive; }
endspecify   { return K_endspecify; }
endtable     { return K_endtable; }
endtask      { return K_endtask; }
event        { return K_event; }
for          { return K_for; }
force        { return K_force; }
forever      { return K_forever; }
fork         { return K_fork; }
function     { return K_function; }
generate     { return K_generate; }
genvar       { return K_genvar; }
highz0       { return K_highz0; }
highz1       { return K_highz1; }
if           { return K_if; }
ifnone       { return K_ifnone; }
initial      { return K_initial; }
inout        { return K_inout; }
input        { return K_input; }
integer      { return K_integer; }
join         { return K_join; }
large        { return K_large; }
library      { return K_library; }
localparam   { return K_localparam; }
macromodule  { return K_macromodule; }
medium       { return K_medium; }
module       { return K_module; }
nand         { return K_nand; }
negedge      { return K_negedge; }
nmos         { return K_nmos; }
nor          { return K_nor; }
not          { return K_not; }
notif0       { return K_notif0; }
notif1       { return K_notif1; }
or           { return K_or; }
output       { return K_output; }
parameter    { return K_parameter; }
pmos         { return K_pmos; }
posedge      { return K_posedge; }
primitive    { return K_primitive; }
pull0        { return K_pull0; }
pull1        { return K_pull1; }
pulldown     { return K_pulldown; }
pullup       { return K_pullup; }
rcmos        { return K_rcmos; }
real         { return K_real; }
realtime     { return K_realtime; }
reg          { return K_reg; }
release      { return K_release; }
repeat       { return K_repeat; }
rnmos        { return K_rnmos; }
rpmos        { return K_rpmos; }
rtran        { return K_rtran; }
rtranif0     { return K_rtranif0; }
rtranif1     { return K_rtranif1; }
scalared     { return K_scalared; }
signed       { return K_signed; }
small        { return K_small; }
specify      { return K_specify; }
specparam    { return K_specparam; }
strong0      { return K_strong0; }
strong1      { return K_strong1; }
supply0      { return K_supply0; }
supply1      { return K_supply1; }
table        { return K_table; }
task         { return K_task; }
time         { return K_time; }
tran         { return K_tran; }
tranif0      { return K_tranif0; }
tranif1      { return K_tranif1; }
tri          { return K_tri; }
tri0         { return K_tri0; }
tri1         { return K_tri1; }
triand       { return K_triand; }
trior        { return K_trior; }
trireg       { return K_trireg; }
unsigned     { return K_unsigned; }
vectored     { return K_vectored; }
wait         { return K_wait; }
wand         { return K_wand; }
weak0        { return K_weak0; }
weak1        { return K_weak1; }
while        { return K_while; }
wire         { return K_wire; }
wor          { return K_wor; }
xnor         { return K_xnor; }
xor          { return K_xor; }

 /* B.2 Configuration */
design       { return K_design; }
instance     { return K_instance; }
cell         { return K_cell; }
use          { return K_use; }
liblist      { return K_liblist; }

 /* B.3 Library */
include      { return K_include; }
incdirt      { return K_incdir; }

 /* ************************************************************************ */
 /* Annex C */
 /* System tasks and functions */
\$countdrivers { return K_countdrivers; }
\$getpattern   { return K_getpattern; }
\$incsave      { return K_incsave; }
\$input        { return K_input; }
\$key          { return K_key; }
\$list         { return K_list; }
\$log          { return K_log; }
\$nokey        { return K_nokey; }
\$nolog        { return K_nolog; }
\$reset        { return K_reset; }
\$reset_count  { return K_reset_count; }
\$reset_value  { return K_reset_value; }
\$restart      { return K_restart; }
\$save         { return K_save; }
\$scale        { return K_scale; }
\$scope        { return K_scope; }
\$showscopes   { return K_showscopes; }
\$showvars     { return K_showvars; }
\$sreadmemb    { return K_sreadmemb; }
\$sreadmemh    { return K_sreadmemh; }

 /* ************************************************************************ */
 /* Annex D */
 /* Compiler directives */
`default_decay_time{W}?.*      { comment_enter = YY_START; BEGIN(LCOMMENT); }
`default_trireg_strength{W}?.* { comment_enter = YY_START; BEGIN(LCOMMENT); }
`delay_mode_distributed{W}?.*  { comment_enter = YY_START; BEGIN(LCOMMENT); }
`delay_mode_path{W}?.*         { comment_enter = YY_START; BEGIN(LCOMMENT); }
`delay_mode_unit{W}?.*         { comment_enter = YY_START; BEGIN(LCOMMENT); }
`delay_mode_zero{W}?.*         { comment_enter = YY_START; BEGIN(LCOMMENT); }








[}{;:\[\],()#=.@&!?<>%|^~+*/-] { return yytext[0]; }

\"            { BEGIN(CSTRING); }
<CSTRING>\\\" { yymore(); }
<CSTRING>\n   { BEGIN(0);
                yylval.text = strdup(yytext);
		cerr << file << " : " << line << ": Missing close quote of string.\n";
		line +=1;
		return STRING; }
<CSTRING>\"   { BEGIN(0);
                yylval.text = strdup(yytext);
		yylval.text[strlen(yytext)-1] = 0;
		return STRING; }
<CSTRING>.    { yymore(); }


<UDPTABLE>\(\?0\)        { return '_'; }
<UDPTABLE>\(\?1\)        { return '+'; }
<UDPTABLE>\(\?x\)        { return '%'; }
<UDPTABLE>\(\?\?\)       { return '*'; }
<UDPTABLE>\(01\)         { return 'r'; }
<UDPTABLE>\(0x\)         { return 'P'; }
<UDPTABLE>\(10\)         { return 'f'; }
<UDPTABLE>\(1x\)         { return 'N'; }
<UDPTABLE>\(x0\)         { return 'F'; }
<UDPTABLE>\(x1\)         { return 'R'; }
<UDPTABLE>[bB]           { return 'b'; }
<UDPTABLE>[fF]           { return 'f'; }
<UDPTABLE>[rR]           { return 'r'; }
<UDPTABLE>[xX]           { return 'x'; }
<UDPTABLE>[pPnN01\?\*\-] { return yytext[0]; }

\`[a-zA-Z_][a-zA-Z0-9$_]* {
  yylval.text =strdup(yytext);
  return DIDENTIFIER; }

[a-zA-Z_][a-zA-Z0-9$_]* {
  yylval.text =strdup(yytext);
  return IDENTIFIER; }

[a-zA-Z_][a-zA-Z0-9$_]*(\.[a-zA-Z_][a-zA-Z0-9$_]*)+ {
  yylval.text = strdup(yytext);
  return HIDENTIFIER; }

\\[^ \t\b\f\r]+ {
  yylval.text = strdup(yytext);
  return IDENTIFIER; }

\$([a-zA-Z0-9$_]+) {
  yylval.text = strdup(yytext);
  return SYSTEM_IDENTIFIER; }

\.{W}?[a-zA-Z_][a-zA-Z0-9$_]* {
  char*cp = yytext+1;
  while (! (isalpha(*cp) || (*cp == '_')))
    cp += 1;
  yylval.text = strdup(cp);
  return PORTNAME; }

[0-9][0-9_]*[ \t]*\'[sS]?[dD][ \t]*[0-9][0-9_]* {
  yylval.text = strdup(yytext);
  return NUMBER; }
[0-9][0-9_]*[ \t]*\'[sS]?[bB][ \t]*[0-1xzXZ_\?]+ {
  yylval.text = strdup(yytext);
  return NUMBER; }
[0-9][0-9_]*[ \t]*\'[sS]?[oO][ \t]*[0-7xzXZ_\?]+ {
  yylval.text = strdup(yytext);
  return NUMBER; }
[0-9][0-9_]*[ \t]*\'[sS]?[hH][ \t]*[0-9a-fA-FxzXZ_\?]+ {
  yylval.text = strdup(yytext);
  return NUMBER; }

\'[sS]?[dD][ \t]*[0-9][0-9_]* {
  yylval.text = strdup(yytext);
  return NUMBER; }
\'[sS]?[bB][ \t]*[0-1xzXZ_\?]+ {
  yylval.text = strdup(yytext);
  return NUMBER; }
\'[sS]?[oO][ \t]*[0-7xzXZ_\?]+ {
  yylval.text = strdup(yytext);
  return NUMBER; }
\'[sS]?[hH][ \t]*[0-9a-fA-FxzXZ_\?]+ {
  yylval.text = strdup(yytext);
  return NUMBER; }

[0-9][0-9_]* {
  yylval.text = strdup(yytext);
  return NUMBER; }

[0-9][0-9_]*\.[0-9][0-9_]*([Ee][+-]?[0-9][0-9_]*)? {
  yylval.text = strdup(yytext);
  return REALTIME; }

[0-9][0-9_]*[Ee][+-]?[0-9][0-9_]* {
  yylval.text = strdup(yytext);
  return REALTIME; }

\$attribute  { return K_ATTRIBUTE; }

`accelerate{W}?.*            { comment_enter = YY_START; BEGIN(LCOMMENT); }
`autoexpand_vectornets{W}?.* { comment_enter = YY_START; BEGIN(LCOMMENT); }
`celldefine{W}?.*            { comment_enter = YY_START; BEGIN(LCOMMENT); }
`default_nettype{W}?.*       { comment_enter = YY_START; BEGIN(LCOMMENT); }
`define{W}?.*                { comment_enter = YY_START; BEGIN(LCOMMENT); }
`else{W}?.*                  { comment_enter = YY_START; BEGIN(LCOMMENT); }
`endcelldefine{W}?.*         { comment_enter = YY_START; BEGIN(LCOMMENT); }
`endif{W}?.*                 { comment_enter = YY_START; BEGIN(LCOMMENT); }
`endprotect{W}?*             { comment_enter = YY_START; BEGIN(LCOMMENT); }
`endprotected{W}?*           { comment_enter = YY_START; BEGIN(LCOMMENT); }
`expand_vectornets{W}?.*     { comment_enter = YY_START; BEGIN(LCOMMENT); }
`ifdef{W}?.*                 { comment_enter = YY_START; BEGIN(LCOMMENT); }
`include{W}?.*               { comment_enter = YY_START; BEGIN(LCOMMENT); }
`noaccelerate{W}?.*          { comment_enter = YY_START; BEGIN(LCOMMENT); }
`noexpand_vectornets{W}?.*   { comment_enter = YY_START; BEGIN(LCOMMENT); }
`noremove_gatenames{W}?.*    { comment_enter = YY_START; BEGIN(LCOMMENT); }
`noremove_netnames{W}?.*     { comment_enter = YY_START; BEGIN(LCOMMENT); }
`nounconnected_drive{W}?.*   { comment_enter = YY_START; BEGIN(LCOMMENT); }
`protect{W}?*                { comment_enter = YY_START; BEGIN(LCOMMENT); }
`protected{W}?*              { comment_enter = YY_START; BEGIN(LCOMMENT); }
`remove_gatenames{W}?.*      { comment_enter = YY_START; BEGIN(LCOMMENT); }
`remove_netnames{W}?.*       { comment_enter = YY_START; BEGIN(LCOMMENT); }
`resetall{W}?.*              { comment_enter = YY_START; BEGIN(LCOMMENT); }
`timescale{W}?.*             { comment_enter = YY_START; BEGIN(LCOMMENT); }
`unconnected_drive{W}?.*     { comment_enter = YY_START; BEGIN(LCOMMENT); }
`undef{W}?.*                 { comment_enter = YY_START; BEGIN(LCOMMENT); }


. {
  cerr << file << " : " << line << ": unmatched character (";
  if (isgraph(yytext[0]))
    cerr << yytext[0];
  else
    cerr << (unsigned)yytext[0];
  cerr << ")" << endl; }

%%

void lex_start_table()
{
  BEGIN(UDPTABLE);
}
void lex_end_table()
{
  BEGIN(INITIAL);
}



static int yyerror()
{
  return 1;
}
int yywrap()
{
  return 1;
}
static void reset_lexor()
{
  yyrestart(verilog_input);
  line =1;
  file =verilog_file;
}
static void line_directive()
{
  assert(strncmp(yytext,"`line",5) == 0);
  char*cp = yytext + strlen("`line");
  cp += strspn(cp, " ");
  line = strtoul(cp,&cp,10);

  cp += strspn(cp, " ");
  if (*cp == 0) return;
  char*qt1 = strchr(yytext, '"');
  assert(qt1);
  qt1 += 1;
  char*qt2 = strchr(qt1, '"');
  assert(qt2);
  file.assign(qt1,(qt2-qt1));
}

