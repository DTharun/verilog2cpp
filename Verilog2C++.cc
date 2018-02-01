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

#include <string>
#include <cstdlib>
#include <cstdio>
#include <typeinfo>
#include <fstream>
#include <iostream>

#include "Verilog.hh"

namespace moe
{
  
  static void printProgress(ostream& ostr,double ratio)
  {
    if( ratio<0 )
      ostr << "....";
    else
      {
	ostr << "\b\b\b\b" << setw(3) << int(ratio) << '%';ostr.flush();
      }
  }


  static void printNet(ostream& ostr,int indent,const Verilog::Net* net)
  {
    if( net->sign() )
      {
	if( net->width()<=32 )
	  ostr << setw(indent) << "" << " int32_t   ";
	else if( net->width()<=64 )
	  ostr << setw(indent) << "" << " int64_t   ";
	else
	  ostr << setw(indent) << "" << " int32_t   ";
      }
    else
      {
	if( net->width()<=32 )
	  ostr << setw(indent) << "" << "uint32_t   ";
	else if( net->width()<=64 )
	  ostr << setw(indent) << "" << "uint64_t   ";
	else
	  ostr << setw(indent) << "" << "uint32_t   ";
      }

    //ostr << 'n' << (uint)net;
    ostr << 'n' << (uintptr_t)net;
    if( net->isArray() )
      ostr << '[' << net->depth() << ']';

    if( net->width()>64 )
      ostr << '[' << ((net->width()+31)/32) << ']';


    ostr << ";";

    /***/
    if( net->width()>64 )
      {
	std::cerr << "Sorry, a big bit vector is under development.\n";
	exit(1);
      }
    /***/
  }



  static void printTypedNet(ostream& ostr,int indent,char type,const Verilog::Net* net)
  {
    if( net->sign() )
      {
	if( net->width()<=32 )
	  ostr << setw(indent) << "" << " int32_t   ";
	else if( net->width()<=64 )
	  ostr << setw(indent) << "" << " int64_t   ";
	else
	  ostr << setw(indent) << "" << " int32_t   ";
      }
    else
      {
	if( net->width()<=32 )
	  ostr << setw(indent) << "" << "uint32_t   ";
	else if( net->width()<=64 )
	  ostr << setw(indent) << "" << "uint64_t   ";
	else
	  ostr << setw(indent) << "" << "uint32_t   ";
      }

    ostr << type << (uintptr_t)net;
    if( net->isArray() )
      ostr << '[' << net->depth() << ']';

    if( net->width()>64 )
      ostr << '[' << ((net->width()+31)/32) << ']';


    ostr << ";";
  }

  static void printPort(ostream& ostr,int indent,const Verilog::Net* net,const string& name)
  {
    if( net->width()<=32 )
      ostr << setw(indent) << "" << "uint32_t&  ";
    else if( net->width()<=64 )
      ostr << setw(indent) << "" << "uint64_t&  ";
    else
      ostr << setw(indent) << "" << "uint32_t*  ";
    
    ostr << name << "() { return ";
    //		  hhstr.form("n%08X",i->second);
    ostr << "n" << (uintptr_t)net;
    ostr << "; }";
  }


  static void printClass(ostream& ostr,unsigned int width,int indent=0)
  {
    if( width<=32 )
      ostr << setw(indent) << "" << "uint32_t  ";
    else if( width<=64 )
      ostr << setw(indent) << "" << "uint64_t  ";
    else
      ostr << setw(indent) << "" << "uint32_t* ";
  }

  static void printPort(ostream& ostr,unsigned int width,int indent=0)
  {
    if( width<=32 )
      ostr << setw(indent) << "" << "uint32_t&  ";
    else if( width<=64 )
      ostr << setw(indent) << "" << "uint64_t&  ";
    else
      ostr << setw(indent) << "" << "uint32_t*  ";
  }
  static void printCast(ostream& ostr,unsigned int width)
  {
    if( width<=32 )
      ostr << "uint32_t";
    else if( width<=64 )
      ostr << "uint64_t";
    else
      ostr << "UIntN<" << setw(3) << width << ">";
  }
  static void printRef(ostream& ostr,unsigned int width)
  {
    if( width<=32 )
      ostr << "uint32_t";
    else if( width<=64 )
      ostr << "uint64_t";
    else
      ostr << "uint32_t*";
  }

  static void printSignMask(ostream& ostr,unsigned int cast,unsigned int width)
  {
    if( cast<=32 )
      ostr << (0xFFFFFFFFUL<<(width-1)) << "UL";
    else if( cast<=64 )
      ostr << (0xFFFFFFFFFFFFFFFFULL<<(width-1)) << "ULL";
    else
      ostr << "SignMask(" << (width-1) << ')';
  }

  static void printMask(ostream& ostr,unsigned int width,
			unsigned int msb,unsigned int lsb)
  {
    if( width<=32 )
      ostr << ((0xFFFFFFFFUL>>(32-(msb-lsb+1)))<<lsb) << "UL";
    else if( width<=64 )
      ostr << ((0xFFFFFFFFFFFFFFFFULL>>(64-(msb-lsb+1)))<<lsb) << "ULL";
    else
      ostr << "Mask(" << msb << ',' << lsb << ')';
  }
  static void printMask(ostream& ostr,unsigned int width)
  {
    if( width<=32 )
      ostr << (0xFFFFFFFFUL>>(32-width)) << "UL";
    else if( width<=64 )
      ostr << (0xFFFFFFFFFFFFFFFFULL>>(64-width)) << "ULL";
    else
      ostr << "Mask(" << width << ')';
  }
  static uint64_t calcConstant(const string& num)
  {
    uint64_t ret =0;
    int i;
    for( i=0;i<num.size();i++ )
      {
	ret =ret<<1;
	if( num[i]=='1' )
	  ret |=1;
      }
    return ret;
  }
  
  
  ////////////////////////////////////////////////////////////////////////
  class Convert : public Verilog
  {
    
    
    ////////////////////////////////////////////////////////////////////////
    class RightExpression : public Callback
    {
      bool     comm_;
      ostream& ostr_;
      unsigned int cast_;
    public:
      RightExpression():
	comm_(true),
	ostr_(std::cout),
	cast_(32)
      {}
      RightExpression(bool comm,ostream& ostr,unsigned int cast):
	comm_(comm),
	ostr_(ostr),
	cast_(cast)
      {}
      ~RightExpression(){}
      

      void trap(const String* self)
      {
	ostr_ << '"' << self->text() << '"';
      }
      void trap(const Number* self)
      {
	{
	  if( self->width()<=32 )
	    ostr_ << self->calcConstant() << "UL";
	  else if( self->width()<=64 )
	    ostr_ << self->calcConstant() << "ULL";
	  else
	    ostr_ << '\"' <<  self->value() << '\"';
	}
	if( self->isPartial() )
	  {
	    ostr_ << ',';
	    
	    if( self->width()<=32 )
	      ostr_ << calcConstant( self->mask() ) << "UL";
	    else if( self->width()<=64 )
	      ostr_ << calcConstant( self->mask() ) << "ULL";
	    else
	      ostr_ << "Constant(" <<  self->mask() << ')';
	  }
      }
      void trap(const Identifier* self)
      {
	if( self->net()->isArray() )
	  {
	    //	    ostr_.form("n%08X",self->net());
	    ostr_ << 'n' << (uintptr_t)self->net();
	    ostr_ << '[';
	    ostr_ << '(';
	    self->idx()->callback( *this );
	    ostr_ << '-' << self->net()->sa()->calcConstant();
	    ostr_ << ")%" << self->net()->depth();
	    ostr_ << ']';
	  }
	else
	  {
	    if( self->idx()!=NULL )
	      {
		ostr_ << "EmVer::Index(";
		//		ostr_.form("n%08X",self->net());
		ostr_ << 'n' << (uintptr_t)self->net();
		ostr_ << ',';
		ostr_ << self->idx()->calcConstant();
		ostr_ << '-' << self->net()->lsb()->calcConstant();
		ostr_ << ')';
	      }
	    else if( self->msb()!=NULL && self->lsb()!=NULL )
	      {
		ostr_ << "EmVer::Part(";
		//		ostr_.form("n%08X",self->net());
		ostr_ << 'n' << (uintptr_t)self->net();
		ostr_ << ',';
		
		if( self->net()->width() <=32 )
		  {
		    ostr_ << self->lsb()->calcConstant();
		    ostr_ << '-' << self->net()->lsb()->calcConstant();
		    ostr_ << ',';
		    //		    ostr_.form("0x%08XUL",0xFFFFFFFFUL>>
		    ostr_ << (0xFFFFFFFFUL>>
			      (31-(self->msb()->calcConstant()-self->lsb()->calcConstant()))) << "UL";
		  }
		else if( self->net()->width() <=64 )
		  {
		    ostr_ << self->lsb()->calcConstant();
		    ostr_ << '-' << self->net()->lsb()->calcConstant();
		    ostr_ << ',';
		    //		    ostr_.form("0x%016lXULL",0xFFFFFFFFFFFFFFFFULL>>
		    ostr_ << (0xFFFFFFFFFFFFFFFFULL>>
			      (63-(self->msb()->calcConstant()-self->lsb()->calcConstant()))) << "ULL";
		  }
		else
		  {
		    ostr_ << self->lsb()->calcConstant();
		    ostr_ << '-' << self->net()->lsb()->calcConstant();
		    ostr_ << ',';
		    
		    ostr_ << self->msb()->calcConstant();
		    ostr_ << '-' << self->lsb()->calcConstant();
		  }
		
		ostr_ << ')';
	      }
	    else
	      ostr_ << 'n' << (uintptr_t)self->net();
	  }
      }
      void trap(const Concat* self)
      {
	int cast=self->width();
	
	if( self->repeat()!=NULL )
	  {
	    ostr_ << "EmVer::Repeat(";
	    ostr_ << self->repeat()->calcConstant();
	    ostr_ << ',';
	    cast /=self->repeat()->calcConstant();
	  }
	
	if( self->list().size()==1 )
	  {
	    printCast(ostr_,cast);
	    ostr_ << '(';
	    self->list().front()->callback( *this );
	    ostr_ << ')';
	  }
	else
	  {
	    {
	      vector<Expression*>::const_iterator i;
	      for( i=self->list().begin();i!=self->list().end();++i )
		{
		  if( (*i)!=self->list().back() )
		    {
		      ostr_ << "EmVer::Concat(";
		      
		      printCast(ostr_,cast);
		      ostr_ << '(';
		      (*i)->callback( *this );
		      ostr_ << ')';
		      ostr_ << ',';
		      
		    }
		  else
		    {
		      printCast(ostr_,cast);
		      ostr_ << '(';
		      (*i)->callback( *this );
		      ostr_ << ')';
		    }
		}
	    }
	    
	    {
	      vector<Expression*>::const_reverse_iterator i;
	      int sum =0;
	      for( i=self->list().rbegin();i!=self->list().rend();++i )
		{
		  if( (*i)!=self->list().front() )
		    {
		      sum +=(*i)->width();
		      ostr_ << ',';
		      ostr_ << sum;
		      ostr_ << ')';
		    }
		}
	    }
	  }
	
	if( self->repeat()!=NULL )
	  {
	    ostr_ << ',';
	    ostr_ << cast;
	    ostr_ << ')';
	  }
      }
      void trap(const Event* self)
      {
	std::cerr << " a event expression in this statement is failure profit. \n";
      }
      ////////////////////////////////////
      void trap(const Unary* self)
      {
	switch( self->operation() )
	  {
	  case Expression::ArithmeticMinus:
	    ostr_ << "(-";
	    self->value()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::BitwiseNegation:
	    ostr_ << "((~";
	    self->value()->callback( *this );
	    ostr_ << ')';
	    ostr_ << "&";
	    printMask(ostr_,self->value()->width(),self->value()->width()-1,0);
	    ostr_ << ')';
	    break;
	    
	  case Expression::LogicalNegation:
	    ostr_ << "(!";
	    self->value()->callback( *this );
	    ostr_ << ')';
	    break;
	    
	  case Expression::ReductionAND:
	  case Expression::ReductionOR:
	  case Expression::ReductionXOR:
	  case Expression::ReductionNAND:
	  case Expression::ReductionNOR:
	  case Expression::ReductionNXOR:
	    ostr_ << "EmVer::" << self->opName();
	    ostr_ << '(';
	    self->value()->callback( *this );
	    ostr_ << ',';
	    printMask(ostr_,self->value()->width(),self->value()->width()-1,0);
	    ostr_ << ')';
	    break;

	  case Expression::CastSigned:
	    ostr_ << "EmVer::SignExt";
	    ostr_ << '(';
	    self->value()->callback( *this );
	    ostr_ << ',';
	    printSignMask(ostr_,cast_,self->value()->width());
	    ostr_ << ')';
	    break;
	  case Expression::CastUnsigned:

	    break;
	  }
      }
      ////////////////////////////////////
      void trap(const Binary* self)
      {
	switch( self->operation() )
	  {
	  case Expression::ArithmeticMultiply:
	  case Expression::ArithmeticDivide:
	  case Expression::ArithmeticModulus:
	  case Expression::ArithmeticAdd:
	  case Expression::ArithmeticMinus:
	    ostr_ << '(';
	    /*
	      if( self->width()!=self->left()->width() )
	      {
	      ostr_ << '(';
	      printCast(ostr_,self->width());
	      ostr_ << ')';
	      }
	    */
	    ostr_ << '(';
	    printCast(ostr_,cast_);
	    ostr_ << ')';
	    
	    self->left()->callback( *this );
	    switch( self->operation() )
	      {
	      case Expression::ArithmeticMultiply:
		ostr_ << '*';
		break;
	      case Expression::ArithmeticDivide:
		ostr_ << '/';
		break;
	      case Expression::ArithmeticModulus:
		ostr_ << '%';
		break;
	      case Expression::ArithmeticAdd:
		ostr_ << '+';
		break;
	      case Expression::ArithmeticMinus:
		ostr_ << '-';
		break;
	      }
	    /*
	      if( self->width()!=self->right()->width() )
	      {
	      ostr_ << '(';
	      printCast(ostr_,self->width());
	      ostr_ << ')';
	      }
	    */
	    ostr_ << '(';
	    printCast(ostr_,cast_);
	    ostr_ << ')';
	    
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	    
	    
	  case Expression::BitwiseXOR:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << '^';
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::BitwiseAND:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << '&';
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::BitwiseOR:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << '|';
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::BitwiseNOR:
	    ostr_ << "((~(";
	    self->left()->callback( *this );
	    ostr_ << '|';
	    self->right()->callback( *this );
	    ostr_ << "))";
	    ostr_ << "&";
	    printMask(ostr_,self->width(),self->width()-1,0);
	    ostr_ << ')';
	    break;
	  case Expression::BitwiseNXOR:
	    ostr_ << "((~(";
	    self->left()->callback( *this );
	    ostr_ << '^';
	    self->right()->callback( *this );
	    ostr_ << "))";
	    ostr_ << "&";
	    printMask(ostr_,self->width(),self->width()-1,0);
	    ostr_ << ')';
	    break;
	    
	    
	  case Expression::LeftShift:
	    ostr_ << "((";
	    self->left()->callback( *this );
	    ostr_ << "<<";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    //	    ostr_ << "&";
	    //	    printMask(ostr_,self->width(),self->width()-1,0);
	    ostr_ << ')';
	    break;
	  case Expression::RightShift:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << ">>";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	    
	    
	  case Expression::LogicalEquality:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << "==";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::LogicalInequality:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << "!=";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::LogicalOR:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << "||";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::LogicalAND:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << "&&";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::LessThan:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << "<";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::GreaterThan:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << ">";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::LessEqual:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << "<=";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	  case Expression::GreaterEqual:
	    ostr_ << '(';
	    self->left()->callback( *this );
	    ostr_ << ">=";
	    self->right()->callback( *this );
	    ostr_ << ')';
	    break;
	    
	  case Expression::CaseEquality:
	    if( (typeid( *(self->right()) )==typeid( Number ))&&
		((Number*)self->right())->isPartial() )
	      {
		ostr_ << "EmVer::CaseEquality";
		ostr_ << '(';
		self->left()->callback( *this );
		ostr_ << ',';
		
		((Number*)self->right())->mask();
		ostr_ << ',';
		((Number*)self->right())->value();
	      }
	    else
	      {
		ostr_ << '(';
		self->left()->callback( *this );
		ostr_ << "==";
		self->right()->callback( *this );
		ostr_ << ')';
	      }
	    break;
	  case Expression::CaseInequality:
	    if( (typeid( *(self->right()) )==typeid( Number ))&&
		((Number*)self->right())->isPartial() )
	      {
		ostr_ << "EmVer::CaseInequality";
		ostr_ << '(';
		self->left()->callback( *this );
		ostr_ << ',';
		
		((Number*)self->right())->mask();
		ostr_ << ',';
		((Number*)self->right())->value();
	      }
	    else
	      {
		ostr_ << '(';
		self->left()->callback( *this );
		ostr_ << "!=";
		self->right()->callback( *this );
		ostr_ << ')';
	      }
	    break;
	  }
      }
      ////////////////////////////////////
      void trap(const Ternary* self)
      {
	ostr_ << '(';
	self->condition()->callback( *this );
	ostr_ << " ? ";
	self->trueValue()->callback( *this );
	ostr_ << " : ";
	self->falseValue()->callback( *this );
	ostr_ << ')';
      }
      ////////////////////////////////////
      void trap(const CallFunction* self)
      {
	//	ostr_.form("f%08X(",self->net());
	ostr_ << 'f' << (uintptr_t)self->net() << '(';
	vector<Expression*>::const_iterator i;
	for( i=self->parameter().begin();i!=self->parameter().end();++i )
	  {
	    if( i!=self->parameter().begin() )
	      ostr_ << ",";
	    
	    (*i)->callback( *this );
	  }
	ostr_ << ")";
      }
    };
    
    
    ////////////////////////////////////////////////////////////////////////
    class LeftExpression : public Callback
    {
      bool         comm_;
      ostream&     ostr_;
      const set<const Net*>& flipflop_;
      unsigned int indent_;
      unsigned int cast_;
    public:
      LeftExpression(bool comm,ostream& ostr,const set<const Net*>& flipflop,unsigned int indent,unsigned int cast):
	comm_(comm),
	ostr_(ostr),
	flipflop_(flipflop),
	indent_(indent),
	cast_(cast)
      {}
      ~LeftExpression(){}
      
      void trap(const Number* self)
      {
	if( self->isPartial() )
	  std::cerr << " a partial number in this left value is failure profit. \n";
	else if( self->width()>32 )
	  std::cerr << " a large number in this left number is failure profit. \n";
	else
	  ostr_ << self->calcConstant();
      }
      void trap(const Identifier* self)
      {
	RightExpression re(comm_,ostr_,32);

	if( flipflop_.find( self->net() )!=flipflop_.end() )
	  {
	    if( self->net()->isArray() )
	      {
		ostr_ << setw(indent_) << "";
		//		ostr_.form("d%08X =preproduct",self->net());
		ostr_ << 'd' << (uintptr_t)self->net() << " =preproduct";
		if( cast_!=self->net()->width() )
		  {
		    ostr_<< '&';
		    printMask( ostr_,self->net()->width() );
		  }
		ostr_ << ";\n";
		
		ostr_ << setw(indent_) << "";
		//		ostr_.form("a%08X",self->net());
		ostr_ << 'a' << (uintptr_t)self->net();
		ostr_ << " =(";
		self->idx()->callback( re );
		ostr_ << '-' << self->net()->sa()->calcConstant();
		ostr_ << ")%" << self->net()->depth() << ";\n";

		ostr_ << setw(indent_) << "";
		//		ostr_.form("u%08X",self->net());
		ostr_ << 'u' << (uintptr_t)self->net();
		ostr_ << " =true;\n";
	      }
	    else
	      {
		if( self->idx()!=NULL )
		  {
		    ostr_ << setw(indent_) << "";
		    ostr_ << "Assign(";
		    //		    ostr_.form("d%08X",self->net());
		    ostr_ << 'd' << (uintptr_t)self->net();
		    ostr_ << ',';
		    printMask( ostr_,self->net()->width(),
			       self->idx()->calcConstant(),
			       self->idx()->calcConstant() );
		    ostr_ << ',';
		    ostr_ << self->idx()->calcConstant();
		    ostr_ << ",preproduct);\n";
		    
		    ostr_ << setw(indent_) << "";
		    //		    ostr_.form("u%08X",self->net());
		    ostr_ << 'u' << (uintptr_t)self->net();
		    ostr_ << " =true;\n";
		  }
		else if( self->msb()!=NULL && self->lsb()!=NULL )
		  {
		    ostr_ << setw(indent_) << "";
		    ostr_ << "Assign(";
		    //		    ostr_.form("d%08X",self->net());
		    ostr_ << 'd' << (uintptr_t)self->net();
		    ostr_ << ',';
		    printMask( ostr_,self->net()->width(),
			       self->msb()->calcConstant(),
			       self->lsb()->calcConstant() );
		    ostr_ << ',';
		    ostr_ << self->lsb()->calcConstant();
		    ostr_ << ",preproduct);\n";
		    
		    ostr_ << setw(indent_) << "";
		    //		    ostr_.form("u%08X",self->net());
		    ostr_ << 'u' << (uintptr_t)self->net();
		    ostr_ << " =true;\n";
		  }
		else
		  {
		    if( self->net()->width()<=64 )
		      {
			ostr_ << setw(indent_) << "";
			ostr_ << 'd' << (uintptr_t)self->net() << " =preproduct";
			if( cast_!=self->net()->width() )
			  {
			    ostr_<< '&';
			    printMask( ostr_,self->net()->width() );
			  }
			ostr_ << ";\n";
		      }
		    else
		      {
			ostr_ << setw(indent_) << "";
			ostr_ << "Assign(";
			ostr_ << 'd' << (uintptr_t)self->net() << ',';
			ostr_ << self->net()->width() << ',';
			ostr_ << "preproduct,";
			ostr_ << cast_;
			ostr_ << ");\n";
		      }

		    ostr_ << setw(indent_) << "";
		    ostr_ << 'u' << (uintptr_t)self->net();
		    ostr_ << " =true;\n";
		  }
	      }
	  }
	else if( self->net()->type()==Net::FUNCTION )
	  {
	    ostr_ << setw(indent_) << "";
	    //	    ostr_.form("return preproduct",self->net());
	    ostr_ << "return preproduct";
	    if( cast_!=self->net()->width() )
	      {
		ostr_<< '&';
		printMask( ostr_,self->net()->width() );
	      }
	    ostr_ << ";\n";
	    
	  }
	else
	  {
	    if( self->net()->isArray() )
	      {
		ostr_ << setw(indent_) << "";
		//		ostr_.form("n%08X",self->net());
		ostr_ << 'n' << (uintptr_t)self->net();
		ostr_ << '[';
		ostr_ << '(';
		self->idx()->callback( re );
		ostr_ << '-' << self->net()->sa()->calcConstant();
		ostr_ << ")%" << self->net()->depth();
		ostr_ << ']';
		
		ostr_ << " =preproduct";
		if( cast_!=self->net()->width() )
		  {
		    ostr_<< '&';
		    printMask( ostr_,self->net()->width() );
		  }
		ostr_ << ";\n";
	      }
	    else
	      {
		if( self->idx()!=NULL )
		  {
		    ostr_ << setw(indent_) << "";
		    ostr_ << "Assign(";
		    //		    ostr_.form("n%08X",self->net());
		    ostr_ << 'n' << (uintptr_t)self->net();
		    ostr_ << ',';
		    printMask( ostr_,self->net()->width(),
			       self->idx()->calcConstant(),
			       self->idx()->calcConstant() );
		    ostr_ << ',';
		    ostr_ << self->idx()->calcConstant();
		    ostr_ << ",preproduct);\n";
		  }
		else if( self->msb()!=NULL && self->lsb()!=NULL )
		  {
		    ostr_ << setw(indent_) << "";
		    ostr_ << "Assign(";
		    //		    ostr_.form("n%08X",self->net());
		    ostr_ << 'n' << (uintptr_t)self->net();
		    ostr_ << ',';
		    printMask( ostr_,self->net()->width(),
			       self->msb()->calcConstant(),
			       self->lsb()->calcConstant() );
		    ostr_ << ',';
		    ostr_ << self->lsb()->calcConstant();
		    ostr_ << ",preproduct);\n";
		  }
		else
		  {
		    {
		      ostr_ << setw(indent_) << "";
		      ostr_ << 'n' << (uintptr_t)self->net() << " =preproduct";
		      if( cast_!=self->net()->width() )
			{
			  ostr_<< '&';
			  printMask( ostr_,self->net()->width() );
			}
		      ostr_ << ";\n";
		    }



		  }
	      }
	  }
	
      }
      void trap(const Concat* self)
      {
	if( self->repeat()!=NULL )
	  std::cerr << " a repeat expression in this left value is failure profit. \n";
	
	{
	  vector<Expression*>::const_reverse_iterator i;
	  for( i=self->list().rbegin();i!=self->list().rend();++i )
	    {
	      (*i)->callback( *this );
	      if( (*i)!=self->list().front() )
		ostr_ << setw(indent_) << "" << "preproduct >>=" << (*i)->width() << ";\n";
	    }
	}
	
      }
      void trap(const Event* self)
      {
	std::cerr << " a event expression in this left value is failure profit. \n";
      }
      void trap(const Unary* self)
      {
	std::cerr << " a unary expression in this left value is failure profit. \n";
      }
      void trap(const Binary* self)
      {
	std::cerr << " a binary expression in this left value is failure profit. \n";
      }
      void trap(const Ternary* self)
      {
	std::cerr << " a ternary expression in this left value is failure profit. \n";
      }
      void trap(const CallFunction* self)
      {
	std::cerr << " a call function expression in this left value is failure profit. \n";
      }
    };
    
    
    
    ////////////////////////////////////////////////////////////////////////
    class StatementSplice : public Callback
    {
      bool     comm_;
      ostream& ostr_;
      const set<const Net*>& flipflop_;
      unsigned int indent_;
    public:
      StatementSplice(bool comm,ostream& ostr,const set<const Net*>& flipflop,unsigned int indent):
	comm_(comm),
	ostr_(ostr),
	flipflop_(flipflop),
	indent_(indent)
      {}
      ~StatementSplice(){}
      
      void trap(const EventStatement* self)
      {
	std::cerr << "a event statement in this handle is failure profit. \n";
      }
      void trap(const Block* self)
      {
	ostr_ << setw(indent_) << "" << "{\n";indent_+=2;
	
	vector<Statement*>::const_iterator i;
	for( i=self->list().begin();i!=self->list().end();++i )
	  (*i)->callback( *this );
	
	indent_-=2;ostr_ << setw(indent_) << "" << "}\n";
      }
      void trap(const Condition* self)
      {
	RightExpression re(comm_,ostr_,32);
	
	ostr_ << setw(indent_) << "" << "if( ";
	self->expression()->callback( re );
	ostr_ << " )\n";
	
	indent_+=2;
	self->trueStatement()->callback( *this );
	indent_-=2;
	
	if( self->falseStatement()!=NULL )
	  {
	    ostr_ << setw(indent_) << "" << "else\n";
	    
	    indent_+=2;
	    self->falseStatement()->callback( *this );
	    indent_-=2;
	  }
      }
      void trap(const Case* self)
      {
	RightExpression re(comm_,ostr_,32);
	
	ostr_ << setw(indent_) << "" << "{\n";indent_+=2;
	
	ostr_ << setw(indent_) << "";
	printClass( ostr_,self->expression()->width() );
	ostr_ << "preproduct =";
	self->expression()->callback( re );// case pre-product
	ostr_ << ";\n";
	
	vector<Case::Item*>::const_iterator i;
	for( i=self->items().begin();i!=self->items().end();++i )
	  {
	    if( i!=self->items().begin() )
	      {
		ostr_ << setw(indent_) << "" << "else ";
	      }
	    else
	      {
		ostr_ << setw(indent_) << "";
	      }
	    
	    if( !(*i)->expression().empty() )
	      {
		ostr_ << "if( ";
		vector<Expression*>::const_iterator ii;
		for( ii=(*i)->expression().begin();ii!=(*i)->expression().end();++ii )
		  {
		    RightExpression re(comm_,ostr_,self->expression()->width());
		    
		    if( ii!=(*i)->expression().begin() )
		      {
			ostr_ << "||" << std::endl;
			ostr_ << setw(indent_) << "";
			
			if( i!=self->items().begin() )
			  ostr_ << "         ";
			else
			  ostr_ << "    ";
		      }
		    
		    {
		      if( typeid( *(*ii) )==typeid( Number ) )
			{
			  if( ((Number*)(*ii))->isPartial() )
			    {
			      ostr_ << "EmVer::CaseEquality(preproduct,";
			      (*ii)->callback( re );
			      ostr_ << ')';
			    }
			  else
			    {
			      ostr_ << "(preproduct==";
			      (*ii)->callback( re );
			      ostr_ << ')';
			    }
			}
		      else
			{
			  ostr_ << "(preproduct==";
			  (*ii)->callback( re );
			  ostr_ << ')';
			}
		    }
		    
		    
		  }
		ostr_ << " )\n";
	      }
	    else
	      ostr_ << "\n";
	    
	    indent_+=2;
	    ostr_ << setw(indent_) << "" << "{\n";
	    indent_+=2;
	    (*i)->statement()->callback( *this );
	    indent_-=2;
	    ostr_ << setw(indent_) << "" << "}\n";
	    indent_-=2;
	  }
	
	indent_-=2;ostr_ << setw(indent_) << "" << "}\n";
      }
      
      
      void trap(const Case::Item* self)
      {
	std::cerr << "a case-item statement in this callback is failure profit. \n";
      }
      
      
      void trap(const Assign* self)
      {
	ostr_ << setw(indent_) << "" << "{\n";indent_+=2;
	
	RightExpression re(comm_,ostr_,self->leftValue()->width() );
	LeftExpression le(comm_,ostr_,flipflop_,indent_,
			  32*((self->leftValue()->width() +31)/32) );
	{
	  if( self->leftValue()->width()>64 )
	    {
	      ostr_ << setw(indent_) << "";
	      ostr_ << "uint32_t bucket";
	      ostr_ << '[';
	      ostr_ << (self->leftValue()->width()+31)/32;
	      ostr_ << ']';
	      ostr_ << ";\n";
	    }

	  ostr_ << setw(indent_) << "";
	  printClass( ostr_,self->leftValue()->width() );
	  ostr_ << "preproduct =";
	  
	  if( self->leftValue()->width()>64 )
	    {
	      ostr_ << "Bucket(";
	      ostr_ << "bucket";
	      ostr_ << ',';
	      ostr_ << self->leftValue()->width();
	      ostr_ << ',';
	    }
	  self->rightValue()->callback( re );
	  if( self->leftValue()->width()>64 )
	    {
	      ostr_ << ")";
	    }
	  ostr_ << ";\n";
	  
	  self->leftValue()->callback( le );
	}
	
	indent_-=2;ostr_ << setw(indent_) << "" << "}\n";
      }

      void trap(const CallTask* self)
      {
	ostr_ << setw(indent_) << "" << "{\n";indent_+=2;
	
	//	ostr_ << setw(indent_) << "" << self->name() << "(";
	ostr_ << setw(indent_) << "" << (&self->name().c_str()[1]) << "(";

	vector<Expression*>::const_iterator i;
	for( i=self->arguments().begin();i!=self->arguments().end();++i )
	  {
	    if( i!=self->arguments().begin() )
	      ostr_ << ',';

	    {
	      RightExpression re( false,ostr_,(*i)->width() );
	      (*i)->callback( re );
	    }
	  }
	ostr_ << ");\n";

	indent_-=2;ostr_ << setw(indent_) << "" << "}\n";
      }
    };
    
    
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    class Handle
    {
      const Statement* stat_;
      set<const Net*>  left_;
      set<const Net*>  right_;
      

    public:
      Handle():
	stat_(NULL)
      {}
      ~Handle(){}
      Handle(const Statement* stat):
	stat_(stat)
      {}
      void setLeft(const set<const Net*>& l) { left_=l; }
      void setRight(const set<const Net*>& r) { right_=r; }
      const Statement* statement()  const { return stat_; }
      const set<const Net*>& left()  const { return left_; }
      const set<const Net*>& right() const { return right_; }
    };
    ////////////////////////////////////////////////////////////////////////
    class Synchronous
    {
      set<const Net*>   syncnet_;
      const Net*   net_;
      int          type_;
      list<Handle> handle_;
    public:
      void dump(ostream &ostr)
      {
	list<Handle>::const_iterator i;
	for( i=handle_.begin();i!=handle_.end();++i )
	  (*i).statement()->toVerilog( ostr ,0 );
      }
      
      ~Synchronous(){}
      Synchronous():
	net_(NULL)
      {}
      Synchronous(const Net* net,int type):
	net_(net),
	type_(type)
      {}
      const Net* net() const { return net_; }
      int type() const { return type_; }
      list<Handle>& handle() { return handle_; }
      const list<Handle>& handle() const { return handle_; }

      void setSyncNet(const Net* net) { syncnet_.insert(net); }
      const set<const Net*>& syncNet() const { return syncnet_; }
    };
    
    
    ////////////////////////////////////////////////////////////////////////
    Module* top_;
    
    vector<Synchronous*> sync_;
    Synchronous*         anysync_;
    
    set<const Net*> syncsrc_;
    set<const Net*> latch_;
    set<const Net*> flipflop_;
    
    multimap<const Net*,const Statement*> map_;
    
    //    map<const Net*,const Assign*>    constant_;
    
    ////////////////////////////////////////////////////////////////////////
    void _trace(const Net* net,int type,Synchronous* sync,int& curr,int& total)
    {
      total +=top_->process().size();
      if( total==0 )
	printProgress(std::cerr,100.0);
      else
	printProgress(std::cerr,(100.0*curr)/total);

      sync->setSyncNet( net );

      vector<Process*>::const_iterator i;
      for( i=top_->process().begin();i!=top_->process().end();++i )
	{
	  curr ++;
	  
	  if( typeid( *((*i)->statement()) )==typeid( Assign ) )
	    {
	      Assign* ass=(Assign*)(*i)->statement();
	      
	      if( ass->isSimple() )
		if( typeid( *(ass->rightValue()) )==typeid( Identifier )&&
		    ((Identifier*)ass->rightValue())->net()==net )
		  _trace(((Identifier*)ass->leftValue())->net(),type,sync,curr,total);
	    }
	  else
	    {
	      const Statement* stat;
	      stat =(*i)->queryStatement(type,net);
	      if( stat!=NULL )
		{
		  set<const Net*> right;
		  RightNetChainCB cb( right );
		  stat->callback( cb );

		  Handle handle(stat);
		  handle.setRight( right );
		  sync->handle().push_back( handle );
		}
	    }
	}

    }
    bool trace(const Net* net,int type,Synchronous* sync)
    {
      printProgress(std::cerr,-1);
      int curr=0;
      int total=0;

      sync->setSyncNet( net );

      vector<Process*>::const_iterator i;
      for( i=top_->process().begin();i!=top_->process().end();++i )
	{
	  curr ++;
	  
	  if( typeid( *((*i)->statement()) )==typeid( Assign ) )
	    {
	      Assign* ass=(Assign*)(*i)->statement();
	      
	      if( ass->isSimple() )
		if( typeid( *(ass->rightValue()) )==typeid( Identifier )&&
		    ((Identifier*)ass->rightValue())->net()==net )
		  _trace(((Identifier*)ass->leftValue())->net(),type,sync,curr,total);
	    }
	  else
	    {
	      const Statement* stat;
	      stat =(*i)->queryStatement(type,net);
	      if( stat!=NULL )
		{
		  set<const Net*> right;
		  RightNetChainCB cb( right );
		  stat->callback( cb );
 
		  Handle handle(stat);
		  handle.setRight( right );
		  sync->handle().push_back( handle );
		}
	    }
	}
      
      printProgress(std::cerr,100.0);
      return true;
    }
    ////////////////////////////////////////////////////////////////////////
    void _relate(const Net* net,
		 set<const Net*>& actnet,
		 set<const Statement*>& actstat,
		 list<Handle>& chain)
    {
      if( actnet.find(net)==actnet.end() )
	{
	  actnet.insert( net );
	  
	  multimap<const Net*,const Statement*>::const_iterator i;
	  set<const Net*>::const_iterator ii;
	  i =map_.find( net );
	  if( i!=map_.end() )
	    {
	      do
		{
		  if( actstat.find( i->second )==actstat.end() )
		    {
		      actstat.insert( i->second );
		      
		      set<const Net*> left;
		      set<const Net*> right;
		      LeftNetChainCB  lcb( left );
		      RightNetChainCB rcb( right );
		      i->second->callback( lcb );
		      i->second->callback( rcb );
		
		      Handle handle( i->second );
		      handle.setRight( right );
		      handle.setLeft( left );
		      chain.push_front( handle );
		      
		      for( ii=right.begin();ii!=right.end();++ii )
			_relate(*ii,actnet,actstat,chain);
		    }
		  i++;
		}
	      while( i!=map_.upper_bound( net ) );
	    }
	}
    }
    ////////////////////////////////////
    bool relate(list<Handle>& chain)
    {
      set<const Net*> actnet;
      
      set<const Statement*> actstat;
      list<Handle>::const_iterator i;
      set<const Net*>::const_iterator ii;
      
      double per =0.0;
      double step =100.0/chain.size();
      
      printProgress(std::cerr,-1);
      for( i=chain.begin();i!=chain.end();++i )
	{
	  printProgress(std::cerr,per);
	  per +=step;
	  
	  for( ii=(*i).right().begin();ii!=(*i).right().end();++ii )
	    _relate(*ii,actnet,actstat,chain);
	}
      printProgress(std::cerr,100.0);
      
      return true;
    }
    ////////////////////////////////////////////////////////////////////////
    bool _inclusion(const set<const Net*>& l,const set<const Net*>& r)
    {
      set<const Net*>::const_iterator i;
      for( i=l.begin();i!=l.end();++i )
	if( r.find( *i )!=r.end() )
	  return true;
      return false; 
    }
    ////////////////////////////////////
    bool inspect(list<Handle>& chain)
    {
      list<Handle>::iterator i;
      list<Handle>::iterator ii;
      
      double per =0.0;
      double step =100.0/chain.size();
      printProgress(std::cerr,-1);
      for( i=chain.begin();i!=chain.end();++i )
	{
	  printProgress(std::cerr,per);
	  per +=step;
	  
	  for( ii=i;ii!=chain.end();++ii )
	    {
	      if( i!=ii )
		if( _inclusion( (*i).right(),(*ii).left() ) )
		  {
		    chain.insert( i,(*ii) );
		    chain.erase( ii );
		    return true;
		  }
	    }
	}
      printProgress(std::cerr,100.0);
      
      return false;
    }
    ////////////////////////////////////
    bool sort(list<Handle>& chain)
    {
      bool swap;
      set<const Handle*> loop;
      list<Handle>::iterator i;
      list<Handle>::iterator ii;
      
      double per =0.0;
      double step =100.0/chain.size();
      
      printProgress(std::cerr,-1);
      for( i=chain.begin();i!=chain.end();++i )
	{
	  printProgress(std::cerr,per);
	  per +=step;
	  
	  do
	    {
	      for( ii=i;ii!=chain.end();++ii )
		{
		  swap=false;
		  if( i!=ii )
		    if( _inclusion( (*i).right(),(*ii).left() ) )
		      {
			if( loop.find( &(*ii) )!=loop.end() )
			  {
			    std::cerr << "\nlacing error !\n";
			    (*ii).statement()->toVerilog(cerr,0);
			    
			    return false;
			  }
			
			i =chain.insert( i,(*ii) );
			chain.erase( ii );
			
			loop.insert( &(*ii)  );
			
			swap=true;
			break;
		      }
		  if( swap )
		    break;
		}
	    }
	  while( swap );
	  loop.clear();
	}
      
      printProgress(std::cerr,100.0);
      
      return true;
    }
    ////////////////////////////////////////////////////////////////////////
    
    
  public:
    Convert(bool debug=false):
      Verilog(debug),
      top_(NULL)
    {
      anysync_ = new Synchronous(NULL,Event::ANYEDGE);
    }
    ~Convert()
    {
      delete anysync_;
    }
    
    const vector<Synchronous*>& synchronous() const { return sync_; }
    const set<const Net*>&  latch() const { return latch_; }
    const set<const Net*>&  flipflop() const { return flipflop_; }
    //    const set<const Net*>&  constant() const { return constant_; }
    //    const set<const Net*>&  input() const { return input_; }
    //    const set<const Net*>&  output() const { return output_; }
    const multimap<const Net*,const Statement*>& handle() const { return map_; }
    
    const Module* top() const { return top_; }
    ////////////////////////////////////////////////////////////////////////
    bool setTop(const char* name)
    {
      top_=findModule(name);
      
      if( top_!=NULL )
	{
	  cerr << "ungroup instance...\n";
	  
	  top_->ungroup();
	  
	  top_->link();

	  {
	    vector<Process*>::const_iterator i;
	    for( i=top_->process().begin();i!=top_->process().end();++i )
	      {
		if( (*i)->isStorage()&&(*i)->isEdge() )
		  {
		    set<const Net*>::const_iterator ii;
		    //for( ii=(*i)->leftChain().begin();ii!=(*i)->leftChain().end();++ii )
		    for( ii=(*i)->nbLeftChain().begin();ii!=(*i)->nbLeftChain().end();++ii )
		      {
			flipflop_.insert( (*ii) );
		      }
		  }
		else
		  {
		    if( (*i)->isStorage()&&(*i)->isLevel() )
		      latch_.insert( (*i)->leftChain().begin(),(*i)->leftChain().end() );
		    
		    switch( (*i)->type() )
		      {
			
		      case Process::ASSIGN:
			{
			  
		    
			  set<const Net*>::const_iterator ii;
			  for( ii=(*i)->leftChain().begin();ii!=(*i)->leftChain().end();++ii )
			    map_.insert( pair<const Net*,const Statement*>
					 ((*ii),(*i)->statement()) );			    
			}
			break;
			
		      case Process::ALWAYS:
			{
			  if( typeid( *(*i)->statement() )==typeid( EventStatement ) )
			    {
			      
			      if( typeid( *( ((EventStatement*)(*i)->statement())->statement() ) )==
				  typeid( Block ) )
				{
				  /**
				  vector<Statement*>::const_iterator ii;
				  for( ii=((Block*)(((EventStatement*)(*i)->statement())->statement()))->list().begin();
				       ii!=((Block*)(((EventStatement*)(*i)->statement())->statement()))->list().end();
				       ++ii )
				    {
				      set<const Net*> chain;
				      LeftNetChainCB cb(chain);
				      (*ii)->callback(cb);
				      
				      set<const Net*>::const_iterator iii;
				      for( iii=chain.begin();iii!=chain.end();++iii )
					map_.insert( pair<const Net*,const Statement*>
						     ((*iii),(*ii)) );
				    }
				  **/
				  set<const Net*>::const_iterator ii;
				  for( ii=(*i)->leftChain().begin();ii!=(*i)->leftChain().end();++ii )
				    map_.insert( pair<const Net*,const Statement*>
						 ((*ii),
						  ((EventStatement*)(*i)->statement())->statement()) );
				  
				}
			      else
				{
				  set<const Net*>::const_iterator ii;
				  for( ii=(*i)->leftChain().begin();ii!=(*i)->leftChain().end();++ii )
				    map_.insert( pair<const Net*,const Statement*>
						 ((*ii),
						  ((EventStatement*)(*i)->statement())->statement()) );
				}
			    }
			}
			break;

		      }
		  }
	      }
	  }

 	  return true;
	}
      else
	return false;
    }
    ////////////////////////////////////////////////////////////////////////
    bool setSynchronous(const char* name,int type)
    {
      if( top_!=NULL )
	{
	  bool ret;
	  const Net* net =top_->findNet(name);
	  
	  if( net!=NULL )
	    {
	      syncsrc_.insert(net);
	      
	      std::cerr << "synchronous source : ";
	      
	      std::cerr << name;
	      if( type==Event::POSEDGE )
		std::cerr << ".posedge";
	      else if( type==Event::NEGEDGE )
		std::cerr << ".negedge";
	      else
		std::cerr << ".anyedge";	    
	      std::cerr << endl;
	      
	      Synchronous* sync =new Synchronous(net,type);
	      sync_.push_back( sync );

	      std::cerr << "trace... ";
	      trace(net,type,sync);
	      std::cerr << '\n';
	      
	      std::cerr << "relate... ";
	      ret =relate(sync->handle());
	      std::cerr << '\n';
	      
	      std::cerr << "sort... ";
	      sort(sync->handle());
	      std::cerr << '\n';
	      
	      std::cerr << "inspect... ";
	      inspect(sync->handle());
	      std::cerr << '\n';

	      return ret;
	    }
	}
      return false;
    }
    bool setAnything()
    {
      if( top_!=NULL )
	{
	  bool ret;
	  
	  std::cerr << "anything source... \n";
	  
	  {
	    map<string,Net*>::const_iterator i;
	    for( i=top_->net().begin();i!=top_->net().end();++i )
	      {
		if( ((i->second->interface()==Net::OUTPUT)||
		     (i->second->interface()==Net::INOUT))&&
		    (flipflop_.find(i->second)==flipflop_.end()) )
		  {
		    set<const Net*> right;
		    right.insert( i->second );
		    Handle handle(NULL);
		    handle.setRight( right );
		    anysync_->handle().push_back( handle );
		  }
	      }
	  }
	  
	  std::cerr << "relate... ";
	  ret =relate(anysync_->handle());
	  std::cerr << '\n';
	  
	  std::cerr << "sort... ";
	  sort(anysync_->handle());
	  std::cerr << '\n';
	  
	  std::cerr << "inspect... ";
	  inspect(anysync_->handle());
	  std::cerr << '\n';
	  
	  return ret;
	}
      else
	return false;
    }
    ////////////////////////////////////////////////////////////////////////
    void toEmVer(const char* path,bool comm=false,bool vcd=false)
    {
      string fname;
      int indent;
      set<const Net*> vcdNet;
      
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      
      fname =string(path) + findName( top_ ) + ".hh";
      std::ofstream hhstr(fname.c_str());
      indent=0;
      
      ////////////////////////////////////////////////////////
      // create header file
      if( debug() )
	std::cerr << "create a header file\n";

      hhstr << setw(indent) << "" << "#include \"EmVer.hh\"" << endl;
      
      hhstr << setw(indent) << "" << "namespace moe" << endl;
      hhstr << setw(indent) << "" << '{' << endl;
      indent+=2;
      
      hhstr << setw(indent) << "" << "class " << findName( top_ ) << ": public EmVer" << endl;
      hhstr << setw(indent) << "" << "{" << endl;indent+=2;
      
      ////////////////////////////////////
      // vcd
      if( vcd )
	{
	  hhstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	  hhstr << setw(indent) << "" << "std::FILE* vcd_;\n";
	  hhstr << setw(indent) << "" << "long       time_;\n";
	  hhstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	}
      


      /***
      ////////////////////////////////////
      // constant
      {
	set<string>::const_iterator i;
	for( i=pool(Number()).begin();i!=pool(Number()).end();++i )
	  {
	    if( (*i).size()>64 )
	      {
		hhstr << setw(indent) << "" << "UIntN<" << setw(3) << (*i).size() << "> ";
		hhstr.form("c%08X;",i);
		if( comm )
		  hhstr << " // " << (*i) << ".constant";
		hhstr << endl;
	      }
	  }
      }
      ***/
      

      ////////////////////////////////////
      // regist
      if( debug() )
	std::cerr << "generate registers\n";
      {
	map<string,Net*>::const_iterator i;
	for( i=top_->net().begin();i!=top_->net().end();++i )
	  {
	    if( syncsrc_.find( i->second )==syncsrc_.end() )
	      {
		if( (i->second->interface()!=Net::PRIVATE)||
		    (flipflop_.find(i->second)!=flipflop_.end())||
		    (latch_.find(i->second)!=latch_.end()) )	       
		  {
		    printNet(hhstr,indent,i->second);

		    /**
		    printClass(hhstr,i->second->width(),indent);
		    //		    hhstr.form("n%08X",i->second);
		    hhstr << 'n' << (uint)i->second;
		    if( i->second->isArray() )
		      hhstr << '[' << i->second->depth() << ']';
		    hhstr << ";";
		    **/

		    if( comm )
		      {
			hhstr << " // " << i->first;
			if( flipflop_.find(i->second)!=flipflop_.end() )
			  hhstr << ":flipflop";
			if( latch_.find(i->second)!=latch_.end() )
			  hhstr << ":latch";
			if( i->second->interface()!=Net::PRIVATE )
			  hhstr << ":port";
			//hhstr << " "<< i->second->width();
		      }
		    hhstr << endl;
		    
		    
		    vcdNet.insert( i->second );

		    ////////////////////////////////////
		    /**
		    if( flipflop_.find(i->second)!=flipflop_.end() )
		      {
			hhstr << setw(indent) << "" << "bool       ";
			//			hhstr.form("b%08X",i->second);
			hhstr << 'b' << (uint)i->second;
			hhstr << ";";
			if( comm )
			  {
			    hhstr << " // " << i->first;
			    if( flipflop_.find(i->second)!=flipflop_.end() )
			      hhstr << ":booth";
			  }
			hhstr << endl;
		      }
		    **/
		  }
	      }
	  }
      }


      ////////////////////////////////////
      // function
      if( debug() )
	std::cerr << "generate functions\n";
      {
	hhstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";

	map<string,Function*>::const_iterator i;
	for( i=top_->function().begin();i!=top_->function().end();++i )
	  {
	    Function* func =i->second;
	    vector<string>::const_iterator ii;
	    bool second=false;
	    for( ii=func->port().begin();ii!=func->port().end();++ii )
	      {
		Net* net =func->net().find(*ii)->second;
		if( net->type()==Net::FUNCTION )
		  {
		    hhstr << setw(indent) << "";
		    printClass(hhstr,net->width());
		    //		    hhstr.form("f%08X",net);
		    hhstr << 'f' << (uintptr_t)net;
		    
		    hhstr << '(';
		  }
	      }
	    for( ii=func->port().begin();ii!=func->port().end();++ii )
	      {
		Net* net =func->net().find(*ii)->second;
		if( net->interface()==Net::INPUT )
		  {
		    if( second )
		      hhstr << ',';
		    
		    printRef(hhstr,net->width());
		    hhstr << ' ';
		    //		    hhstr.form("n%08X",net);
		    hhstr << 'n' << (uintptr_t)net;
		    
		    second=true;
		  }
	      }
	    hhstr << ");" << endl;
	  }
      }


      ////////////////////////////////////
      hhstr << setw(indent-2) << "" << "public:" << endl;
      hhstr << setw(indent) << "" << findName( top_ ) << "()";
      if( vcd )
	{
	  hhstr << ":\n";
	  hhstr << setw(indent+2) << "" << "vcd_(NULL),\n";
	  hhstr << setw(indent+2) << "" << "time_(0)\n";
	}
      else
	{
	  hhstr << "\n";
	}
      hhstr << setw(indent) << "" << "{}" << endl;
      hhstr << setw(indent) << "" << '~' << findName( top_ ) << "(){}" << endl;


      ////////////////////////////////////
      // vcd
      if( vcd )
	{
	  hhstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	  hhstr << setw(indent) << "" << "void vcd_open(const char* fname);\n";
	  hhstr << setw(indent) << "" << "void vcd_close();\n";
	}

      ////////////////////////////////////
      // port
      if( debug() )
	std::cerr << "generete ports\n";
      {
	hhstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	map<string,Net*>::const_iterator i;
	for( i=top_->net().begin();i!=top_->net().end();++i )
	  {
	    if( syncsrc_.find( i->second )==syncsrc_.end() )
	      if( (i->second->interface()!=Net::PRIVATE) )
		{
		  printPort(hhstr,indent,i->second,i->first);
		  /*
		  printPort(hhstr,i->second->width(),indent);
		  hhstr << i->first << "() { return ";
		  //		  hhstr.form("n%08X",i->second);
		  hhstr << 'n' << (uint)i->second;
		  hhstr << "; }\n";
		  */
		  hhstr << "\n";

		  vcdNet.insert( i->second );
		}
	  }
      }
      

      ////////////////////////////////////
      // temporal event signals
      if( debug() )
	std::cerr << "generate event methods\n";
      {
	vector<Synchronous*>::const_iterator i;
	for( i=sync_.begin();i!=sync_.end();++i )
	  {
	    hhstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	    
	    hhstr << setw(indent) << "" << "void ";
	    hhstr << top_->findName( (*i)->net() );
	    
	    if( (*i)->type()==Event::POSEDGE )
	      hhstr << "_posedge";
	    else if( (*i)->type()==Event::NEGEDGE )
	      hhstr << "_negedge";
	    else
	      hhstr << "_anyedge";	    
	    
	    hhstr << "();" << endl;
	  }
      }
      
      
      indent-=2;hhstr << setw(--indent) << "" << "};" << endl;
      indent-=2;hhstr << setw(indent) << "" << '}' << endl;
      
      
      
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      
      fname =string(path) + findName( top_ ) + ".cc";
      std::ofstream ccstr(fname.c_str());
      indent=0;
      

      ////////////////////////////////////////////////////////
      // create source file
      if( debug() )
	std::cerr << "create a source file\n";
      {
	ccstr << setw(indent) << "" << "#include \"" << findName( top_ ) <<  ".hh\"" << endl;
	
	ccstr << setw(indent) << "" << "namespace moe" << endl;
	ccstr << setw(indent) << "" << '{' << endl;
	indent+=2;
      }
      
      ////////////////////////////////////
      // function
      if( debug() )
	std::cerr << "generate functions\n";
      {
	ccstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	
        map<string,Function*>::const_iterator i;
        for( i=top_->function().begin();i!=top_->function().end();++i )
          {
	    Function* func =i->second;
	    vector<string>::const_iterator ii;
	    bool second=false;
	    for( ii=func->port().begin();ii!=func->port().end();++ii )
	      {
		Net* net =func->net().find(*ii)->second;
		if( net->type()==Net::FUNCTION )
		  {
		    ccstr << setw(indent) << "";
		    printClass(ccstr,net->width());
		    ccstr << findName( top_ ) << "::";
		    //		    ccstr.form("f%08X",net);
		    ccstr << 'f' << (uintptr_t)net;
		    ccstr << '(';
		  }
	      }

	    for( ii=func->port().begin();ii!=func->port().end();++ii )
	      {
		Net* net =func->net().find(*ii)->second;
		if( net->interface()==Net::INPUT )
		  {
		    if( second )
		      ccstr << ',';
		    
		    printRef(ccstr,net->width());
		    ccstr << ' ';
		    
		    //		    ccstr.form("n%08X",net);
		    ccstr << 'n' << (uintptr_t)net;
		    
		    second=true;
		  }
	      }
	    ccstr << ')' << endl;
	    ccstr << setw(indent) << "" << '{' << endl;indent+=2;
	    
	    for( ii=func->port().begin();ii!=func->port().end();++ii )
	      {
		Net* net =func->net().find(*ii)->second;
		
		if( net->interface()==Net::PRIVATE )
		  if( net->type()==Net::REG )
		      {
			ccstr << setw(indent) << "";
			printRef(ccstr,net->width());
			ccstr << ' ';
			//		    ccstr.form("n%08X",net);
			ccstr << 'n' << (uintptr_t)net;
			ccstr << ";\n";
		      }
	      }

	    {
	      StatementSplice cpp(comm,ccstr,flipflop_,indent);
	      {
		if( comm )
		  {
		    ccstr << setw(indent) << "" << '/' << "************************************" << endl;
		    i->second->statement()->toVerilog(ccstr,indent+2);
		    ccstr << setw(indent) << "" << '*' << '/' << endl;
		  }
		i->second->statement()->callback( cpp );
	      }
	    }

	    indent-=2;ccstr << setw(indent) << "" << '}' << endl;


	  }
      }


      /***
      ////////////////////////////////////
      // do anything signals
      {
	ccstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";

	ccstr << setw(indent) << "" << "void ";
	ccstr << findName( top_ ) << "::" << "_anything()\n";

	ccstr << setw(indent) << "" << '{' << endl;indent+=2;
	{
	  StatementSplice cpp(comm,ccstr,flipflop_,indent);
	  list<Handle>::const_iterator ii;
	  for( ii=anysync_->handle().begin();ii!=anysync_->handle().end();++ii )
	    if( (*ii).statement()!=NULL )
	      {
		if( comm )
		  {
		    ccstr << setw(indent) << "" << '/' << "************************************" << endl;
		    (*ii).statement()->toVerilog(ccstr,indent+2);
		    ccstr << setw(indent) << "" << '*' << '/' << endl;
		  }
		
		(*ii).statement()->callback( cpp );
	      }
	}
	indent-=2;ccstr << setw(indent) << "" << '}' << endl;
      }
      ***/


      ////////////////////////////////////
      // temporal event signals
      if( debug() )
	std::cerr << "generate event methods\n";
      {
	vector<Synchronous*>::const_iterator i;
	for( i=sync_.begin();i!=sync_.end();++i )
	  {
	    vector<Synchronous*>::const_iterator sync =i;
	    
	    ccstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";

	    ccstr << setw(indent) << "" << "void ";
	    ccstr << findName( top_ ) << "::";
	    ccstr << top_->findName( (*i)->net() );

	    if( (*i)->type()==Event::POSEDGE )
	      ccstr << "_posedge";
	    else if( (*i)->type()==Event::NEGEDGE )
	      ccstr << "_negedge";
	    else
	      ccstr << "_anyedge";	    

	    ccstr << "()" << endl;

	    ////////////////////////////////////
	    ccstr << setw(indent) << "" << '{' << endl;indent+=2;
	    {
	      map<string,Net*>::const_iterator i;
	      for( i=top_->net().begin();i!=top_->net().end();++i )
		{
		  if( flipflop_.find(i->second)==flipflop_.end() )
		    {
		      if( (i->second->interface()==Net::PRIVATE) )
			{

			  printNet(ccstr,indent,i->second);
			  /*
			    ccstr << setw(indent) << "";
			    printClass(ccstr,i->second->width());
			    //ccstr.form("n%08X;",i->second);
			    ccstr << 'n' << (uint)i->second;
			  */

			  if( i->second->type()==Net::PARAMETER )
			    {
			      if( i->second->rightValue()!=NULL )
				{
				  ccstr << 'n' << (uintptr_t)i->second;
				  ccstr << " =";
				  RightExpression re(false,ccstr,32);
				  i->second->rightValue()->callback( re );
				  ccstr << ';';
				}
			    }


			  if( comm )
			    {
			      if( i->second->type()==Net::PARAMETER )
				ccstr << " // " << i->first << ":parameter";
			      else
				ccstr << " // " << i->first << ":private";
			    }
			  ccstr << endl;

			  vcdNet.insert( i->second );
			}
		    }
		  else
		    {
		      if( i->second->isArray() )
			{
			  ccstr << setw(indent) << "";
			  printClass(ccstr,i->second->width());

			  ccstr << 'd' << (uintptr_t)i->second << ';';
			  if( comm )
			    ccstr << " // " << i->first << ":data";
			  ccstr << endl;

			  ccstr << setw(indent) << "" << "uint32_t   ";
			  //			  ccstr.form("a%08X;",i->second);
			  ccstr << 'a' << (uintptr_t)i->second << ';';
			  if( comm )
			    ccstr << " // " << i->first << ":address";
			  ccstr << endl;
			  
			  ccstr << setw(indent) << "" << "bool       ";
			  //			  ccstr.form("u%08X",i->second);
			  ccstr << 'u' << (uintptr_t)i->second;
			  ccstr << " =false;";
			  if( comm )
			    ccstr << " // " << i->first << ":update";
			  ccstr << endl;
			}
		      else
			{
			  // ccstr << setw(indent) << "";
			  //printClass(ccstr,i->second->width());
			  {
			    printTypedNet(ccstr,indent,'d',i->second);
			    if( i->second->width()<=64 )
			      ccstr << 'd' << (uintptr_t)i->second << " =n" << (uintptr_t)i->second << ';';
			    else
			      ccstr << "memcpy("
				    << 'd' << (uintptr_t)i->second << ','
				    << 'n' << (uintptr_t)i->second << ','
				    << (((i->second->width()+31)/32)*4) << ");";
			  }

			  if( comm )
			    ccstr << " // " << i->first << ":data";
			  ccstr << endl;

			  ccstr << setw(indent) << "" << "bool       ";

			  ccstr << 'u' << (uintptr_t)i->second;
			  ccstr << " =false;";
			  if( comm )
			    ccstr << " // " << i->first << ":update";
			  ccstr << endl;
			}
		    }
		}
	    }

	    {
	      StatementSplice cpp(comm,ccstr,flipflop_,indent);
	      list<Handle>::const_iterator ii;
	      for( ii=(*i)->handle().begin();ii!=(*i)->handle().end();++ii )
		{
		  if( (*ii).statement()!=NULL )
		    {
		      if( comm )
			{
			  ccstr << setw(indent) << "" << '/' << "************************************" << endl;
			  
			  (*ii).statement()->toVerilog(ccstr,indent+2);
			  ccstr << setw(indent) << "" << '*' << '/' << endl;
			}
		      (*ii).statement()->callback( cpp );
		    }
		}
	    }
	    ////////////////////////////////////

	    {
	      map<string,Net*>::const_iterator i;
	      for( i=top_->net().begin();i!=top_->net().end();++i )
		{
		  if( flipflop_.find(i->second)!=flipflop_.end() )
		    {
		      if( i->second->isArray() )
			{
			  ccstr << setw(indent) << "";
			  //			  ccstr.form("if( u%08X )\n",i->second);
			  ccstr << "if( u" << (uintptr_t)i->second << " )\n";
			  indent+=2;
		      
			  ccstr << setw(indent) << "";
			  if( i->second->width()<=64 )
			    ccstr << 'n' << (uintptr_t)i->second << "[a" << (uintptr_t)i->second << "] =d" << (uintptr_t)i->second << ";\n";
			  else
			    ccstr << "memcpy("
				  << 'n' << (uintptr_t)i->second << "[a" << (uintptr_t)i->second << ']' <<  ','
				  << 'd' << (uintptr_t)i->second << ','
			      
				  << (((i->second->width()+31)/32)*4) << ");\n";
			  indent-=2;
			}
		      else
			{
			  ccstr << setw(indent) << "";
			  if( i->second->width()<=64 )
			    ccstr << 'n' << (uintptr_t)i->second << " =d" << (uintptr_t)i->second << ";\n";
			  else
			    ccstr << "memcpy("
				  << 'n' << (uintptr_t)i->second << ','
				  << 'd' << (uintptr_t)i->second << ','
				  << (((i->second->width()+31)/32)*4) << ");\n";
			}

		      ////////////////////////////////////
		      /**
			 ccstr << setw(indent) << "";
			 // ccstr.form("b%08X =u%08X;\n",i->second,i->second);
		      ccstr << 'b' << (uint)i->second << " =u" << (uint)i->second << ";\n";
		      **/
		    }
		}
	    }
	    ////////////////////////////////////
	    //	    ccstr << setw(indent) << "" << "_anything();\n";

	    {
	      StatementSplice cpp(comm,ccstr,flipflop_,indent);
	      list<Handle>::const_iterator ii;
	      for( ii=anysync_->handle().begin();ii!=anysync_->handle().end();++ii )
		if( (*ii).statement()!=NULL )
		  {
		    if( comm )
		      {
			ccstr << setw(indent) << "" << '/' << "************************************" << endl;
			(*ii).statement()->toVerilog(ccstr,indent+2);
			ccstr << setw(indent) << "" << '*' << '/' << endl;
		      }
		    
		    (*ii).statement()->callback( cpp );
		  }
	    }

	    

	    ////////////////////////////////////
	    // vcd
	    if( vcd )
	    {
	      
	      ccstr << setw(indent) << "" << "if( vcd_!=NULL )\n";
	      ccstr << setw(indent) << "" << "{\n";
	      indent+=2;

	      {
		ccstr << setw(indent) << "" << "fprintf(vcd_,\"#%d\\n\",time_);\n";
		{
		  set<const Net*>::const_iterator ii;
		  for( ii=(*sync)->syncNet().begin();ii!=(*sync)->syncNet().end();++ii )
		    {
		      ccstr << setw(indent) << "" << "fputs(\"b";
		      if( (*sync)->type()==Event::POSEDGE )
			ccstr << "1";
		      else if( (*sync)->type()==Event::NEGEDGE )
			ccstr << "0";
		      else
			ccstr << "0";
		      ccstr << "\",vcd_);";
		      ccstr << "fputs(\" ";
		      ccstr << 'n' << (uintptr_t)((*ii));
		      ccstr << "\\n\",vcd_);\n";
		    }
		}
	      }
	      
	      ccstr << setw(indent) << "" << "fprintf(vcd_,\"#%d\\n\",time_+3);\n";

	      {
		vector<Synchronous*>::const_iterator i;
		for( i=sync_.begin();i!=sync_.end();++i )
		  {
		    set<const Net*>::const_iterator ii;
		    for( ii=(*i)->syncNet().begin();ii!=(*i)->syncNet().end();++ii )
		      {
			set<const Net*>::const_iterator iii;
			for( iii=vcdNet.begin();iii!=vcdNet.end();++iii )
			  if( (*iii)==(*ii) )
			    vcdNet.erase( iii );
		      }
		  }
	      }

	      const char* cptr;
	      set<const Net*>::const_iterator i;
	      for( i=vcdNet.begin();i!=vcdNet.end();++i )
		if( !((*i)->isArray()) )
		  {
		    ccstr << setw(indent) << "" << "vcd_dump(vcd_,";
		    ccstr << 'n' << (uintptr_t)(*i);
		    ccstr << ',';
		    ccstr << (*i)->width() << ");";
		    ccstr << "fputs(\" ";
		    ccstr << 'n' << (uintptr_t)(*i);
		    ccstr << "\\n\",vcd_);\n";
		  }



	      {
		ccstr << setw(indent) << "" << "fprintf(vcd_,\"#%d\\n\",time_+5);\n";
		{
		  set<const Net*>::const_iterator ii;
		  for( ii=(*sync)->syncNet().begin();ii!=(*sync)->syncNet().end();++ii )
		    {
		      ccstr << setw(indent) << "" << "fputs(\"b";
		      if( (*sync)->type()==Event::POSEDGE )
			ccstr << "0";
		      else if( (*sync)->type()==Event::NEGEDGE )
			ccstr << "1";
		      else
			ccstr << "0";
		      ccstr << "\",vcd_);";
		      ccstr << "fputs(\" ";
		      ccstr << 'n' << (uintptr_t)((*ii));
		      ccstr << "\\n\",vcd_);\n";
		    }
		}
	      }

	      indent-=2;
	      ccstr << setw(indent) << "" << "}\n";
	      ccstr << setw(indent) << "" << "time_ +=10;\n";
	    }



	    ////////////////////////////////////
	    indent-=2;ccstr << setw(indent) << "" << '}' << endl;

	  }
      }

      ////////////////////////////////////
      // vcd support
      if( vcd )
	{
	  ccstr << setw(indent) << "" << "////////////////////////////////////////////////////////////////////////\n";
	  
	  
	  ccstr << setw(indent) << "" << "void ";
	  ccstr << findName( top_ ) << "::";
	  ccstr << "vcd_open(const char* fname)\n";
	  ccstr << setw(indent) << "" << '{' << endl;indent+=2;
	  {
	    ccstr << setw(indent) << "" << "vcd_ =std::fopen(fname,\"w\");\n";
	    ccstr << setw(indent) << "" << "time_ =0;\n";
	    
	    ccstr << setw(indent) << "" << "fputs(\"$version\\n\",vcd_);\n";
	    ccstr << setw(indent) << "" << "fputs(\" moe verilog simulation\\n\",vcd_);\n";
	    ccstr << setw(indent) << "" << "fputs(\"$end\\n\",vcd_);\n";
	    

	    {
	      vector<Synchronous*>::const_iterator ii;
	      for( ii=sync_.begin();ii!=sync_.end();++ii )
		{
		  set<const Net*>::const_iterator i;
		  for( i=(*ii)->syncNet().begin();i!=(*ii)->syncNet().end();++i )
		    {
		      const char* cptr;
		      ccstr << setw(indent) << "" << "fputs(\"$var reg ";
		      ccstr << (*i)->width() << ' ';

		      ccstr << 'n' << (uintptr_t)(*i);
		      ccstr << ' ';
		      cptr =top_->findName( *i );
		      if( *cptr=='\\' )
			cptr++;
		      ccstr << ' ' << cptr;
		      ccstr << " $end\\n\",vcd_);\n";
		    }
		}
	    }
	    

	    {
	      const char* cptr;
	      set<const Net*>::const_iterator i;
	      for( i=vcdNet.begin();i!=vcdNet.end();++i )
		if( !((*i)->isArray()) )
		  {
		    ccstr << setw(indent) << "" << "fputs(\"$var reg ";
		    ccstr << (*i)->width() << ' ';
		    //		    ccstr.form("n%08X",(*i));
		    ccstr << 'n' << (uintptr_t)(*i);
		    ccstr << ' ';
		    cptr =top_->findName( *i );
		    if( *cptr=='\\' )
		      cptr++;
		    ccstr << ' ' << cptr;
		    if( (*i)->width() > 1 )
		      {
			ccstr << " [ ";
			ccstr << (*i)->msb()->calcConstant();
			ccstr << " : ";
			ccstr << (*i)->lsb()->calcConstant();
			ccstr << " ]";
		      }
		    ccstr << " $end\\n\",vcd_);\n";
		  }
	    }
	    ccstr << setw(indent) << "" << "fputs(\"$enddefinitions $end\\n\",vcd_);\n";
	    ccstr << setw(indent) << "" << "fputs(\"$dumpvars\\n\",vcd_);\n";
	  }

	  indent-=2;ccstr << setw(indent) << "" << '}' << endl;
	  
	  
	  ccstr << setw(indent) << "" << "void ";
	  ccstr << findName( top_ ) << "::";
	  ccstr << "vcd_close()\n";
	  ccstr << setw(indent) << "" << '{' << endl;indent+=2;
	  {
	    ccstr << setw(indent) << "" << "fputs(\"$end\\n\",vcd_);\n";
	    ccstr << setw(indent) << "" << "fclose(vcd_);\n";
	  }
	  indent-=2;ccstr << setw(indent) << "" << '}' << endl;
	}

      ////////////////////////////////////
      indent-=2;ccstr << setw(indent) << "" << '}' << endl;


    if( debug() )
      std::cerr << "finish\n";
    }
    ////////////////////////////////////////////////////////////////////////
  };
  
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int main(int argc,char* argv[])
{
  if( argc==1 )
    {
      //      cout << "Verilog2C++ file-name.(v|v2k) module-name ( (posedge|negedge) port-name )+ comments? dumpvars? debug? \n";
      std::cerr << "Verilog2C++ file-name.(v|v2k) module-name ( (posedge|negedge) port-name )+ comments? dumpvars?\n";
      exit(1);
    }
  bool comments =false;
  bool dumpvars =false;
  bool debug    =false;
  int i;
  for( i=3;i<argc;i++ )
    {
      if( strcmp(argv[i],"comments" )==0 )
	comments=true;
      else if( strcmp(argv[i],"dumpvars" )==0 )
	dumpvars=true;
      else if( strcmp(argv[i],"debug" )==0 )
	debug=true;
    }

  moe::Convert conv(debug);
  
  cerr << "load...\n";
  conv.parse( argv[1] );

  cerr << "link module...\n";
  conv.link();

  conv.setTop( argv[2] );
  
  for( i=3;i<argc;i+=2 )
    {
      if( strcmp(argv[i],"posedge" )==0 )
	conv.setSynchronous( argv[i+1],moe::Verilog::Event::POSEDGE);
      else if( strcmp(argv[i],"negedge" )==0 )
	conv.setSynchronous( argv[i+1],moe::Verilog::Event::NEGEDGE);
    }

  conv.setAnything();

  conv.toEmVer("./",comments,dumpvars);
}







