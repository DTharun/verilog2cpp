/*
 * Copyright (c) 2000-2003 moe
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


#ifndef __VERILOG_HH
#define __VERILOG_HH

#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

extern int verilog_parse();
extern FILE*  verilog_input;
extern string verilog_file;
extern string verilog_comment;

//int verilog_parse();
//FILE*  verilog_input;
//string verilog_file;
//string verilog_comment;

namespace moe
{
  class Verilog;
}
extern moe::Verilog* source_;

namespace moe
{
  ////////////////////////////////////////////////////////////////////////
  class Verilog
  {
    public:
    class LineInfo
    {
      string       file_;
      unsigned int line_;
    public:
      LineInfo():
	line_(0)
      {}
      
      void setFile(const string& name){ file_=name; }
      void setLine(unsigned int line) { line_=line; }

      const string& file() const { return file_; }
      unsigned int  line() const { return line_; }
      
      string getInfo() const
      {
	char buf[16];
	snprintf(buf,sizeof(buf),"%u",line_);
	return (file_ + " : " + buf);
      }
    };

    class Callback;
    ////////////////////////////////////////////////////////////////////////
    class Module;
    class Net;
    ////////////////////////////////////////////////////////////////////////
    class Expression
    {
    public:
      virtual ~Expression(){}
      virtual bool isConstant() const { return false; }
      virtual signed calcConstant() const { return 0; }
      virtual unsigned int width() const { return 0; }
      virtual void link(const map<string,Net*>& net,Module* mod,const string& scope) {}
      virtual const Net* net() const { return NULL; }
      
      virtual Expression* clone(const string& hname) const { return NULL; }
      virtual Expression* clone() const { return NULL; }

      virtual void chain(set<const Net*>& ev) const {}
      virtual void chain(set<const Expression*>& ev) const { ev.insert((Expression*)this); }

      virtual void toXML(std::ostream& ostr) const {}
      virtual void toVerilog(std::ostream& ostr) const {}

      enum
      {
	ArithmeticAdd,
	ArithmeticMinus,
	ArithmeticMultiply,
	ArithmeticDivide,
	ArithmeticModulus,
	ArithmeticLeftShift,
	ArithmeticRightShift,
	ArithmeticPower,

	LeftShift,
	RightShift,
	
	LessThan,
	GreaterThan,
	LessEqual,
	GreaterEqual,
	CaseEquality,
	CaseInequality,
	
	LogicalNegation,
	LogicalAND,
	LogicalOR,
	LogicalEquality,
	LogicalInequality,
	
	BitwiseAND,
	BitwiseOR,
	BitwiseNOR,
	BitwiseNXOR,
	BitwiseXOR,
	BitwiseNegation,
	
	ReductionAND,
	ReductionNAND,
	ReductionNOR,
	ReductionNXOR,
	ReductionXOR,
	ReductionOR,

	CastUnsigned,
	CastSigned
      };

      virtual void callback(Callback& cb) const{}
    };
    ////////////////////////////////////////////////////////////////////////
    class String : public Expression
    {
      string text_;
    public:
      String()
      {}
      String(const char* text);
      ~String(){}

      const string& text() const { return text_; }

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const { return new String(*this); }
      Expression* clone() const { return new String(*this); }

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Number : public Expression
    {
      string text_;
      string bitset_;

      string value_;
      string mask_;

      unsigned int width_;
    public:
      Number():
      width_(0)
      {}
      Number(const char* text);
      ~Number(){}

      const string& text() const { return text_; }
      const string& bitset() const { return bitset_; }
      unsigned int width() const { return width_; }

      bool isConstant() const { return true; }
      bool isPartial() const;

      const string& value() const { return value_; }
      const string& mask() const { return mask_; }

      signed calcConstant() const;

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const { return new Number(*this); }
      Expression* clone() const { return new Number(*this); }

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Identifier : public Expression
    {
      string      name_;
      Expression* msb_;
      Expression* lsb_;
      Expression* idx_;
      Net*        net_;
      //////////////////
    public:
      Identifier(const char* name,
		 Expression* msb=NULL,Expression* lsb=NULL,
		 Expression* idx=NULL):
	name_(name),
	msb_(msb),lsb_(lsb),
	idx_(idx),
	net_(NULL)
      {}
      Identifier():
	msb_(NULL),lsb_(NULL),
	idx_(NULL),
	net_(NULL)
      {}
      ~Identifier();
      const string& name() const { return name_; }
      const Expression* msb() const { return msb_; }
      const Expression* lsb() const { return lsb_; }
      const Expression* idx() const { return idx_; }

      bool isPartial() const;

      signed calcConstant() const
      {
	if( net_!=NULL )
	  return net_->calcConstant();
	else
	  return 0;
      }

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      unsigned int width() const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      const Net* net() const { return net_; }
      void setNet(Net* net) { net_=net; }
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Concat : public Expression
    {
      Expression*         repeat_;
      vector<Expression*> list_;
    public:
      Concat(const vector<Expression*>& l):
	repeat_(NULL),
	list_(l)
      {}
      Concat(Expression* r,const vector<Expression*>& l):
	repeat_(r),
	list_(l)
      {}
      Concat():
	repeat_(NULL)
      {}
      ~Concat();
      const Expression* repeat() const { return repeat_; }
      const vector<Expression*>& list() const { return list_; }

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      unsigned int width() const;
      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Event : public Expression
    {
      int         type_;
      Expression* expr_;
    public:
      enum
      {
	ANYEDGE,
	POSEDGE,
	NEGEDGE,
	POSITIVE
      };
      Event(int t,Expression* e):
	type_(t),
	expr_(e)
      {}
      Event():
	expr_(NULL)
      {}
      ~Event();
      int         type() const { return type_; }
      Expression* expression() const { return expr_; }

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      unsigned int width() const { return 0; }
      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Unary : public Expression
    {
      /*
	ArithmeticMinus
	BitwiseNegation
	LogicalNegation
	ReductionAND
	ReductionOR
	ReductionXOR
	ReductionNAND
	ReductionNOR
	ReductionNXOR
	CastSigned
	CastUnsigned
       */
      int         op_;
      Expression* expr_;
    public:
      Unary(int o,Expression* e):
	op_(o),
	expr_(e)
      {}
      Unary():
	expr_(NULL)
      {}
      ~Unary();

      int operation() const { return op_; }
      const Expression* value() const { return expr_; }

      unsigned int width() const;
      bool isConstant() const { return expr_->isConstant(); }
      signed calcConstant() const;

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      const char* opToken() const;
      const char* opName() const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Binary : public Expression
    {
      int         op_;
      Expression* left_;
      Expression* right_;
    public:
      Binary(int o,Expression* l,Expression* r):
	op_(o),
	left_(l),
	right_(r)
      {}
      Binary():
	left_(NULL),
	right_(NULL)
      {}
      ~Binary();

      int operation() const { return op_; }
      const Expression* left() const { return left_; }
      const Expression* right() const { return right_; }

      unsigned int width() const;
      bool isConstant() const { return (left_->isConstant()&&right_->isConstant()); }
      signed calcConstant() const;

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      const char* opToken() const;
      const char* opName() const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Ternary : public Expression
    {
      Expression* expr_;
      Expression* true_;
      Expression* false_;
    public:
      Ternary(Expression* e,Expression* t,Expression* f):
	expr_(e),
	true_(t),
	false_(f)
      {}
      Ternary():
	expr_(NULL),
	true_(NULL),
	false_(NULL)
      {}
      ~Ternary();

      const Expression* condition() const { return expr_; }
      const Expression* trueValue() const { return true_; }
      const Expression* falseValue() const { return false_; }
      
      unsigned int width() const { return (false_!=NULL) ? max( true_->width(),false_->width() ) : true_->width() ; }
      bool isConstant() const{}

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    class Function;
    ////////////////////////////////////////////////////////////////////////
    class CallFunction : public Expression
    {
      string              name_;
      vector<Expression*> parms_;
      Function*           func_;
      Net*                net_;
    public:
      CallFunction(const char* n,const vector<Expression*> &p):
	name_(n),
	parms_(p),
	func_(NULL),
	net_(NULL)
      {}
      CallFunction():
	func_(NULL),
	net_(NULL)
      {}
      ~CallFunction();

      const string name() const { return name_; }
      const vector<Expression*>& parameter() const { return parms_; }
      const Function* function() const { return func_; }
      const Net* net() const { return net_; }

      void toXML(std::ostream& ostr) const;
      void toVerilog(std::ostream& ostr) const;

      unsigned int width() const { return net_->width(); }
      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Expression* clone(const string& hname) const;
      Expression* clone() const;

      void chain(set<const Net*>& ev) const;
      void chain(set<const Expression*>& ev) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Net
    {
    public:
      class nrm_
      {
      public:
	char* name;
	Expression* start;
	Expression* end;
	int   type;
	nrm_():
	  name(NULL)
	{}
	~nrm_()
	{
	  delete name;
	  delete start;
	  delete end;
	}
      };
      int    type_;
      int    interface_;
      bool   sign_;
      
      Expression* msb_;
      Expression* lsb_;
      Expression* sa_;
      Expression* ea_;
      bool constant_;
      string name_;

      const Expression* rval_;
    public:
      enum
      {
	IMPLICIT,
	WIRE,
	TRI,
	TRI1,
	SUPPLY0,
	WAND,
	TRIAND,
	TRI0,
	SUPPLY1,
	WOR, 
	TRIOR,
	REG,
	INTEGER,
	REAL,
	PARAMETER,
	FUNCTION,
	DEFINE,
	CONSTANT,

	NAMEDBLOCK_REG
      };
      enum
      {
	PRIVATE,
	INPUT,
	OUTPUT,
	INOUT
      };
      enum
      {
	UNSIGNED,
	SIGNED
      };

      Net(int type,Expression* msb=NULL,Expression* lsb=NULL,
	  int inter=PRIVATE,Expression* sa=NULL,Expression* ea=NULL,
	  bool sign=false):
	type_(type),
	msb_(msb),
	lsb_(lsb),
	interface_(inter),
	sa_(sa),
	ea_(ea),
	sign_(sign),
	rval_(NULL)
      {}
      Net(){}
      ~Net(){}
      void setInterface(int p) { interface_=p; }
      int interface() const { return interface_; }
      void setType(int t) { type_=t; }
      int type() const { return type_; }
      bool sign() const { return sign_; }

      const string& name() const { return name_; }

      const Expression* msb() const { return msb_; }
      const Expression* lsb() const { return lsb_; }
      const Expression* sa() const { return sa_; }
      const Expression* ea() const { return ea_; }

      const Expression* rightValue() const { return rval_; }
      
      bool isArray() const
      {
	if( (sa_!=NULL)&&(ea_!=NULL) )
	  return true;
	else
	  return false;
      }

      unsigned int depth() const
      {
	if( (sa_!=NULL)&&(ea_!=NULL) )
	  return abs(ea_->calcConstant()-sa_->calcConstant())+1;
	else
	  return 0;
      }
      unsigned int width() const
      {
	if( (msb_!=NULL)&&(lsb_!=NULL) )
	  return abs(msb_->calcConstant()-lsb_->calcConstant())+1;
	else
	  if( (type_==INTEGER)||(type_==PARAMETER) )
	    return 32;
	  else
	    return 1;
      }
      
      signed calcConstant() const
      {
	if( rval_!=NULL )
	  return rval_->calcConstant();
	else
	  return 0;
      }


      void toXML(std::ostream& ostr,const string& name,int indent=0) const;
      void toVerilog(std::ostream& ostr,const string& name,
		     int indent=0,bool namedbblock=false) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Net* clone(const string& hname) const;
      Net* clone() const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Statement
    {
    public:
      Statement(){}
      virtual ~Statement(){}

      virtual void toXML(std::ostream& ostr,int indent=0) const {}
      virtual void toVerilog(std::ostream& ostr,int indent=0) const {}

      virtual void link(const map<string,Net*>& net,Module* mod,const string& scope) {}
      virtual Statement* clone(const string& hname) const { return NULL; }

      virtual void chain(set<const Statement*>& ss) const {}

      virtual void callback(Callback& cb) const{}
    };
    ////////////////////////////////////////////////////////////////////////
    class Block : public Statement
    {
      string             name_;
      int                type_;
      vector<Statement*> list_;
      const Module*      module_;
    public:
      enum
      {
	SEQUENTIAL,
	PARALLEL
      };
      Block(int type,const vector<Statement*>& list):
	module_(NULL),
	type_(type),
	list_(list)
      {}
      Block(int type,const vector<Statement*>& list,
	    const char* name,const Module* mod):
	type_(type),
	list_(list),
	name_(name),
	module_(mod)
      {}
      Block(int type):
	type_(type)
      {}
      Block(){}
      ~Block();
      
      const vector<Statement*>& list() const { return list_; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;

      void chain(set<const Statement*>& ss) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Case : public Statement
    {
    public:
      ////////////////////////////////////////////////////////////////////////
      class Item : public Statement
      {
	vector<Expression*> expr_;
	Statement*          stat_;
      public:
	Item(vector<Expression*> expr,Statement* stat):
	  expr_(expr),
	  stat_(stat)
	{}
	Item(Statement* stat):
	  stat_(stat)
	{}
	Item():
	  stat_(NULL)
	{}
	~Item();

	const vector<Expression*>& expression() const { return expr_; }
	const Statement* statement() const { return stat_; }

	void toXML(std::ostream& ostr,int indent=0) const;
	void toVerilog(std::ostream& ostr,int indent=0) const;

	void link(const map<string,Net*>& net,Module* mod,const string& scope);
	Item* clone(const string& hname) const;

	void chain(set<const Statement*>& ss) const;

	void callback(Callback& cb) const;
      };
    private:
      int           type_;
      Expression*   expr_;
      vector<Item*> items_;
    public:
      enum
      {
	CASE,
	CASEX,
	CASEZ
      };

      Case(int type,Expression* ex,const vector<Item*>& it):
	type_(type),
	expr_(ex),
	items_(it)
      {}
      Case():
	expr_(NULL)
      {}
      ~Case();
      int type() const { return type_; }
      Expression* expression() const { return expr_; }
      const vector<Item*>& items() const { return items_; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;

      void chain(set<const Statement*>& ss) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Condition : public Statement
    {
      Expression* expr_;
      Statement*  true_;
      Statement*  false_;
    public:
      Condition(Expression* ex,Statement* t,Statement* f=NULL):
	expr_(ex),
	true_(t),
	false_(f)
      {}
      Condition():
	expr_(NULL),
	true_(NULL),
	false_(NULL)
      {}
      ~Condition();
      const Expression* expression() const { return expr_; }
      const Statement*  trueStatement() const { return true_; }
      const Statement*  falseStatement() const { return false_; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;

      void chain(set<const Statement*>& ss) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class EventStatement : public Statement
    {
      vector<Event*> event_;
      Statement*     stat_;
    public:
      EventStatement(const vector<Event*>& ee):
	event_(ee),
	stat_(NULL)
      {}
      EventStatement(Event*ee):
	event_(1),
	stat_(NULL)
      {
	event_[0] = ee;
      }
      EventStatement():
	stat_(NULL)
      {}
      ~EventStatement();
      const vector<Event*>& event() const { return event_; }
      const Statement* statement() const { return stat_; }
      void setStatement(Statement* stat){ stat_ =stat; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;

      void chain(set<const Statement*>& ss) const;

      bool isEdge() const;
      bool isLevel() const;
      bool isStorage() const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Assign : public Statement
    {
      int         type_;
      Expression* lval_;
      Expression* rval_;
    public:
      enum
      {
	BLOCKING,
	NONBLOCKING
      };
      Assign(int type,Expression* lval,Expression* rval):
	type_(type),
	lval_(lval),
	rval_(rval)
      {}
      Assign():
	lval_(NULL),
	rval_(NULL)
      {}
      ~Assign();
      int type() const { return type_; }
      const Expression* leftValue()  const { return lval_; }
      const Expression* rightValue() const { return rval_; }

      void setLeftValue(Expression* e) { lval_=e; }
      void setRightValue(Expression* e) { rval_=e; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;
      bool isSimple() const;
      bool isSimpleLeft() const;
      bool isSimpleRight() const;

      void chain(set<const Statement*>& ss) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class For : public Statement
    {
      Identifier* ita_;
      Expression* begin_;
      Expression* cond_;
      Expression* reach_;
      Statement*  stat_;
    public:
      For(Identifier* i1,Expression* e1,
	  Expression* e2,
	  Identifier* i2,Expression* e3,
	  Statement* s);
      For():
	ita_(NULL),
	begin_(NULL),
	cond_(NULL),
	reach_(NULL),
	stat_(NULL)
      {}
      ~For();
      const Identifier* iterat() const { return ita_; }
      const Expression* begin() const { return begin_; }
      const Expression* condition() const { return cond_; }
      const Expression* reach() const { return reach_; }
      const Statement*  statement() const { return stat_; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;

      void chain(set<const Statement*>& ss) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class CallTask : public Statement
    {
      string              name_;
      vector<Expression*> args_;
    public:
      CallTask(const char* name,const vector<Expression*> &args):
	name_(name),
	args_(args)
      {}
      CallTask()
      {}
      ~CallTask(){}
      const string name() const { return name_; }
      const vector<Expression*>& arguments() const { return args_; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(const map<string,Net*>& net,Module* mod,const string& scope);
      Statement* clone(const string& hname) const;

      void chain(set<const Statement*>& ss) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Function
    {
      vector<string>   port_;
      map<string,Net*> net_;
      Statement*       stat_;
    public:
      Function():
	stat_(NULL)
      {}
      ~Function();
      const vector<string>&   port()      const { return port_; }
      const map<string,Net*>& net()       const { return net_; }
      const Statement*        statement() const { return stat_; }

      void addNet(const char* name,Net* net);
      void setStatement(Statement* stat) { stat_=stat; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(Module* mod);
      Function* clone(const string& hname) const;

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Process
    {
      int        type_;
      Statement* stat_;
      string     name_;

      set<const Net*> eventChain_;
      set<const Net*> leftChain_;
      set<const Net*> rightChain_;

      set<const Net*> nbLeftChain_;
      set<const Net*> nbRightChain_;

      set<const Net*> bLeftChain_;
      set<const Net*> bRightChain_;

      set<const Statement*> statChain_;
    public:
      enum
      {
	INITIAL,
	TASK,
	ALWAYS,
	ASSIGN,
	PARAMETER
      };
      Process(int type,Statement* stat=NULL):
	type_(type),
	stat_(stat)
      {}
      Process():
	stat_(NULL)
      {}
      ~Process();
      int type() const { return type_; }
      const Statement* statement() const { return stat_; }

      void toXML(std::ostream& ostr,int indent=0) const;
      void toVerilog(std::ostream& ostr,int indent=0) const;

      void link(Module* mod);
      Process* clone(const string& hname) const;

      bool isEdge() const;
      bool isLevel() const;
      bool isStorage() const;
      const Statement* queryStatement(int type,const Net* src) const;

      const set<const Net*>& eventChain() const { return eventChain_; }
      const set<const Net*>& leftChain()  const { return leftChain_; }
      const set<const Net*>& rightChain() const { return rightChain_; }

      const set<const Net*>& nbLeftChain()  const { return nbLeftChain_; }
      const set<const Net*>& nbRightChain() const { return nbRightChain_; }
      const set<const Net*>& bLeftChain()  const { return bLeftChain_; }
      const set<const Net*>& bRightChain() const { return bRightChain_; }

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////    
    class Gate
    {
    public:
      enum
      {
	AND,
	NAND,
	OR,
	NOR,
	XOR,
	XNOR,
	BUF,
	BUFIF0,
	BUFIF1,
	NOT,
	NOTIF0,
	NOTIF1,
	PULLDOWN,
	PULLUP,
	NMOS,
	RNMOS,
	PMOS,
	RPMOS,
	CMOS,
	RCMOS,
	TRAN,
	RTRAN, 
	TRANIF0,
	TRANIF1,
	RTRANIF0,
	RTRANIF1
      };

      int                 type_;
      vector<Expression*> pin_;
    public:
      Gate(int t,const vector<Expression*>& pin):
	type_(t),
	pin_(pin)
      {}
      Gate(){}
      ~Gate();

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////    
    class Instance
    {
      ////////////////////////////////////////////////////////////////////////    
    public:
      class Port
      {
	string      ref_;
	Expression* con_;
	Net*        net_;
      public:
	Port(const char* ref,Expression* con):
	  ref_(ref),
	  con_(con),
	  net_(NULL)
	{}
	Port():
	  con_(NULL),
	  net_(NULL)
	{}
	~Port();
	const string&     reference() const { return ref_; }
	const Expression* connect() const { return con_; }

	void toXML( std::ostream& ostr,int indent=0 ) const;
	void toVerilog( std::ostream& ostr,int indent=0 ) const;

	void link(const map<string,Net*>& net,Module* mod,const string& scope,Module* rmod,int idx);
	const Net* net() const { return net_; }
	Port* clone(const string& hname) const;
	void ungroup(Module* mod,const string& cname,const string& sname);

	void callback(Callback& cb) const;
      };

      string        type_;
      vector<Port*> port_;
      Module*       module_;

      multimap<string,Expression*> params_;
    public:
      Instance(const char* t):
	type_(t),
	module_(NULL)
      {}
      Instance():
	module_(NULL)
      {}
      ~Instance();
      const string&        type() const { return type_; }
      const vector<Port*>& port() const { return port_; }
      const Module* module() const { return module_; }

      void setParameters(const multimap<string,Expression*>* p)
      {
	params_.insert(p->begin(),p->end());
      }

      void setType(const char* type){ type_ =type; }
      void addPort(Port* p);

      void toXML( std::ostream& ostr,const string& name,int indent=0 ) const;
      void toVerilog( std::ostream& ostr,const string& name,int indent=0 ) const;

      void link(Verilog* veri,const map<string,Net*>& net,
		const string& scope,Module* mod);
      Instance* clone(const string& hname) const;
      void ungroup(Module* mod,const string& cname,const string& sname);

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    class Module
    {
    private:
      //      Verilog&              source_;
      
      string                name_;

      vector<string>        port_;
      map<string,Net*>      net_;
      map<string,Function*> function_;
      map<string,Instance*> instance_;
      vector<Process*>      process_;

      map<string,Expression*> defparams_;
    public:
      Module(){}
      ~Module();
      //      const Verilog&               source()   const { return source_; }
      const vector<string>&        port()     const { return port_; }
      const map<string,Net*>&      net()      const { return net_; }
      const map<string,Function*>& function() const { return function_; }
      const map<string,Instance*>& instance() const { return instance_; }
      const vector<Process*>&      process()  const { return process_; }
      const string&                name()     const { return name_; }

      const map<string,Expression*>& defparam() const { return defparams_; }


      map<string,Instance*>& instance() { return instance_; }
      Instance* newInstance(const char* name);
      Function* newFunction(const char* name);
      Net*      newNet(const char* name,
		       int type,
		       Expression* msb=NULL,Expression* lsb=NULL,
		       int inter=Net::PRIVATE,
		       Expression* sa=NULL,Expression* ea=NULL,
		       bool sign=false);

      void addPort(const char* name);
      void addNet(const char* name,Net* net);
      void addAssign(Expression* l,Expression* r);
      void addParameter(Expression* l,Expression* r);
      void addProcess(Process* proc);
      void addFunction(const char* name,Function* func);
      void addInstance(const char* name,Instance* inst);

      void addDefparam(map<string,Expression*>* p)
      {
	defparams_.insert(p->begin(),p->end());
      }
      void addDefparam(string n,Expression* e)
      {
	defparams_.insert( pair<string,Expression*>(n,e) );
      }
      

      void toXML( std::ostream& ostr,const string& name,int indent=0 ) const;
      void toVerilog( std::ostream& ostr,const string& name,int indent=0 ) const;

      void link(Verilog* veri);
      void ungroup(Module* mod,const string& hname,const multimap<string,Expression*>& param);

      void link();
      void ungroup();

      const Net* findNet(const char* name) const { map<string,Net*>::const_iterator i;i=net_.find(string(name));if( i!=net_.end() ) return i->second; else return NULL; }

      //
      const char* findName(const Net* net) const
      {
	map<string,Net*>::const_iterator i;
	for( i=net_.begin();i!=net_.end();++i )
	  if( i->second==net )
	    return i->first.c_str();
	return NULL;
      }
      //

      void callback(Callback& cb) const;
    };
    ////////////////////////////////////////////////////////////////////////
    map<string,Module*> module_;
    bool                debug_;
    //    map<string,Number*> constant_;
    
    bool                dec_tpd_;
  public:
    Verilog(bool debug=false):
      debug_(debug),
      dec_tpd_(false)
    {}
    virtual ~Verilog();

    bool decTPD() const { return dec_tpd_; }
    void setDecTPD(bool flag) { dec_tpd_ =flag; }

    bool debug() const { return debug_; }

    int parse(const char* filename);
    int parse(FILE* fp);

    const map<string,Module*>& module() const { return module_; }
    Module* addModule(const char* name);

    void toXML(std::ostream& ostr,int indent=0) const;
    virtual void toVerilog(std::ostream& ostr,int indent=0) const;

    void link();
    void ungroup(Module* top);
    Module* findModule(const char* name){ map<string,Module*>::const_iterator i;i=module_.find(string(name));if( i!=module_.end() ) return i->second; else return NULL; }
    //
    const char* findName(const Module* mod) const
    {
      map<string,Module*>::const_iterator i;
      for( i=module_.begin();i!=module_.end();++i )
	if( i->second==mod )
	  return i->first.c_str();
      return NULL;
    }
    //

    void callback(Callback& cb) const;

    ////////////////////////////////////////////////////////////////////////
    class Callback
    {
    public:
      Callback(){}
      virtual ~Callback(){}

      virtual void trap(const Case::Item* self)
      {
	if( self==NULL )
	  return;

	vector<Expression*>::const_iterator i;
	for( i=self->expression().begin();i!=self->expression().end();++i )
	  (*i)->callback( *this );
	self->statement()->callback( *this );
      }
      virtual void trap(const Instance::Port* self)
      {
	if( self==NULL )
	  return;

	self->callback( *this );
      }
      virtual void trap(const Assign* self)
      {
	if( self==NULL )
	  return;

	self->rightValue()->callback( *this );
	self->leftValue()->callback( *this );
      }
      virtual void trap(const Binary* self)
      {
	if( self==NULL )
	  return;

	self->left()->callback( *this );
	self->right()->callback( *this );
      }
      virtual void trap(const Block* self)
      {
	if( self==NULL )
	  return;

	vector<Statement*>::const_iterator i;
	for( i=self->list().begin();i!=self->list().end();++i )
	  (*i)->callback( *this );
      }
      virtual void trap(const CallFunction* self)
      {
	if( self==NULL )
	  return;

	vector<Expression*>::const_iterator i;
	for( i=self->parameter().begin();i!=self->parameter().end();++i )
	  (*i)->callback( *this );
      }
      virtual void trap(const Case* self)
      {
	if( self==NULL )
	  return;

	vector<Case::Item*>::const_iterator i;
	for( i=self->items().begin();i!=self->items().end();++i )
	  {
	    if( !(*i)->expression().empty() )
	      {
		vector<Expression*>::const_iterator ii;
		for( ii=(*i)->expression().begin();ii!=(*i)->expression().end();++ii )
		  (*ii)->callback( *this );
	      }
	    (*i)->statement()->callback( *this );
	  }
      }
      virtual void trap(const Concat* self)
      {
	if( self==NULL )
	  return;

	vector<Expression*>::const_reverse_iterator i;
	for( i=self->list().rbegin();i!=self->list().rend();++i )
	  (*i)->callback( *this );
      }
      virtual void trap(const Condition* self)
      {
	if( self==NULL )
	  return;

	self->expression()->callback( *this );
	self->trueStatement()->callback( *this );
	if( self->falseStatement()!=NULL )
	  self->falseStatement()->callback( *this );
      }
      virtual void trap(const Event* self)
      {
	if( self==NULL )
	  return;

	self->expression()->callback( *this );
      }
      virtual void trap(const EventStatement* self)
      {
	if( self==NULL )
	  return;

	vector<Event*>::const_reverse_iterator i;
	for( i=self->event().rbegin();i!=self->event().rend();++i )
	  (*i)->callback( *this );
	self->statement()->callback( *this );
      }
      virtual void trap(const For* self)
      {
	if( self==NULL )
	  return;

	//	self->iterat()->callback( *this );
	//	self->begin()->callback( *this );
	//	self->condition()->callback( *this );
	//	self->reach()->callback( *this );

	self->statement()->callback( *this );
      }
      virtual void trap(const CallTask* self)
      {
	if( self==NULL )
	  return;

      }
      virtual void trap(const Function* self)
      {
	if( self==NULL )
	  return;

	self->statement()->callback( *this );
      }
      virtual void trap(const Gate* self){}
      virtual void trap(const Identifier* self)
      {
	if( self==NULL )
	  return;

	if( self->msb()!=NULL )
	  self->msb()->callback( *this );
	if( self->lsb()!=NULL )
	  self->lsb()->callback( *this );
	if( self->idx()!=NULL )
	  self->idx()->callback( *this );
	if( self->net()!=NULL )
	  self->net()->callback( *this );
      }
      virtual void trap(const Instance* self)
      {
	if( self==NULL )
	  return;

	vector<Instance::Port*>::const_iterator i;
	for( i=self->port().begin();i!=self->port().end();++i )
	  (*i)->callback( *this );
      }
      virtual void trap(const Module* self)
      {
	if( self==NULL )
	  return;
	{
	  map<string,Net*>::const_iterator i;
	  for( i=self->net().begin();i!=self->net().end();++i )
	    i->second->callback( *this );
	}
	{
	  map<string,Function*>::const_iterator i;
	  for( i=self->function().begin();i!=self->function().end();++i )
	    i->second->callback( *this );
	}
	{
	  map<string,Instance*>::const_iterator i;
	  for( i=self->instance().begin();i!=self->instance().end();++i )
	    i->second->callback( *this );
	}
	{
	  vector<Process*>::const_iterator i;
	  for( i=self->process().begin();i!=self->process().end();++i )
	    (*i)->callback( *this );
	}
      }
      virtual void trap(const Net* self)
      {
	if( self==NULL )
	  return;
	
	if( self->msb()!=NULL )
	  self->msb()->callback( *this );
	if( self->lsb()!=NULL )
	  self->lsb()->callback( *this );
	if( self->sa()!=NULL )
	  self->sa()->callback( *this );
	if( self->ea()!=NULL )
	  self->ea()->callback( *this );
      }
      virtual void trap(const String* self){}
      virtual void trap(const Number* self){}
      virtual void trap(const Process* self)
      {
	if( self==NULL )
	  return;

	self->statement()->callback( *this );
	{
	  set<const Net*>::const_iterator i;
	  for( i=self->eventChain().begin();i!=self->eventChain().end();++i )
	    (*i)->callback( *this );
	}	
      }
      virtual void trap(const Ternary* self)
      {
	if( self==NULL )
	  return;

	if( self->condition()!=NULL )
	  self->condition()->callback( *this );
	if( self->trueValue()!=NULL )
	  self->trueValue()->callback( *this );
	if( self->falseValue()!=NULL )
	  self->falseValue()->callback( *this );
      }
      virtual void trap(const Unary* self)
      {
	if( self==NULL )
	  return;

	if( self->value()!=NULL )
	  self->value()->callback( *this );
      }
      virtual void trap(const Verilog* self)
      {
	if( self==NULL )
	  return;

	map<string,Module*>::const_iterator i;
	for( i=self->module().begin();i!=self->module().end();++i )
	  i->second->callback( *this );	
      }
    };
    ////////////////////////////////////////////////////////////////////////
    class LeftNetChainCB : public Callback
    {
      set<const Net*>& chain_;
    public:
      LeftNetChainCB(set<const Net*>& chain):
	chain_(chain)
      {}
      ~LeftNetChainCB(){}

      void trap(const Case::Item* self);
      void trap(const Assign* self);
      void trap(const Block* self);
      void trap(const Case* self);
      void trap(const Concat* self);
      void trap(const Condition* self);
      void trap(const EventStatement* self);
      void trap(const Identifier* self);
      void trap(const Net* self);
      void trap(const Process* self);
    };
    ////////////////////////////////////////////////////////////////////////
    class RightNetChainCB : public Callback
    {
      set<const Net*>& chain_;
      bool             left_;
    public:
      RightNetChainCB(set<const Net*>& chain):
	chain_(chain),
	left_(false)
      {}
      ~RightNetChainCB(){}

      void trap(const Case::Item* self);
      void trap(const Assign* self);
      void trap(const Block* self);
      void trap(const Case* self);
      void trap(const Condition* self);
      void trap(const EventStatement* self);
      void trap(const Net* self);
      void trap(const Process* self);

      void trap(const Binary* self);
      void trap(const CallFunction* self);
      void trap(const Concat* self);
      void trap(const Ternary* self);
      void trap(const Unary* self);
      void trap(const Identifier* self);      
    };
    ////////////////////////////////////////////////////////////////////////
    class EventNetChainCB : public Callback
    {
      set<const Net*>& chain_;
    public:
      EventNetChainCB(set<const Net*>& chain):
	chain_(chain)
      {}
      ~EventNetChainCB(){}

      void trap(const Process* self);
      void trap(const EventStatement* self);
      void trap(const Event* self);
      void trap(const Identifier* self);
      void trap(const Net* self);
    };
    ////////////////////////////////////////////////////////////////////////
    class NetChainCB : public Callback
    {
      set<const Net*>& nbLeftChain_;
      set<const Net*>& nbRightChain_;
      set<const Net*>& bLeftChain_;
      set<const Net*>& bRightChain_;

      bool             left_;
      bool             blocking_;

    public:
      NetChainCB(set<const Net*>& nbLeftChain,
		 set<const Net*>& nbRightChain,
		 set<const Net*>& bLeftChain,
		 set<const Net*>& bRightChain):
	nbLeftChain_(nbLeftChain),
	nbRightChain_(nbRightChain),
	bLeftChain_(bLeftChain),
	bRightChain_(bRightChain),
	left_(false),
	blocking_(false)
      {}
      ~NetChainCB(){}

      void trap(const Case::Item* self);
      void trap(const Assign* self);
      void trap(const Block* self);
      void trap(const Case* self);
      void trap(const Concat* self);
      void trap(const Condition* self);
      void trap(const EventStatement* self);
      void trap(const Identifier* self);
      void trap(const Net* self);
      void trap(const Process* self);

      // only right net 
      void trap(const Binary* self);
      void trap(const CallFunction* self);
      void trap(const Ternary* self);
      void trap(const Unary* self);
    };
    ////////////////////////////////////////////////////////////////////////
  };
}

#endif




