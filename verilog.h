#ifndef __VERILOG_H
#define __VERILOG_H

struct ltype
{
  unsigned first_line;
  unsigned first_column;
  unsigned last_line;
  unsigned last_column;
  const char*text;
};
#define YYLTYPE ltype
extern YYLTYPE yylloc;


#endif
