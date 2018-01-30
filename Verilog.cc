/*
 * Copyright (C) 2000-2003 moe
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

#include "Verilog.hh"

FILE*  verilog_input;
string verilog_file;
string verilog_comment;

moe::Verilog* source_;

namespace moe
{
  static char* opName_[] =
  {
    "ArithmeticAdd",
    "ArithmeticMinus",
    "ArithmeticMultiply",
    "ArithmeticDivide",
    "ArithmeticModulus",
    "ArithmeticLeftShift",
    "ArithmeticRightShift",
    "ArithmeticPower",

    "LeftShift",
    "RightShift",
    
    "LessThan",
    "GreaterThan",
    "LessEqual",
    "GreaterEqual",
    "CaseEquality",
    "CaseInequality",
    
    "LogicalNegation",
    "LogicalAND",
    "LogicalOR",
    "LogicalEquality",
    "LogicalInequality",
    
    "BitwiseAND",
    "BitwiseOR",
    "BitwiseNOR",
    "BitwiseNXOR",
    "BitwiseXOR",
    "BitwiseNegation",
    
    "ReductionAND",
    "ReductionNAND",
    "ReductionNOR",
    "ReductionNXOR",
    "ReductionXOR",
    "ReductionOR",

    "CastUnsigned",
    "CastSigned"
  };
  static char* opToken_[] =
  {
    "+",   // ArithmeticAdd,
    "-",   // ArithmeticMinus,
    "*",   // ArithmeticMultiply,
    "/",   // ArithmeticDivide,
    "%%",  // ArithmeticModulus,
    "<<<", // ArithmeticLeftShift,
    ">>>", // ArithmeticRightShift,
    "**",  // ArithmeticPower,

    "<<",  // LeftShift,
    ">>",  // RightShift,
    
    "<",   // LessThan,
    ">",   // GreaterThan,
    "<=",  // LessEqual,
    ">=",  // GreaterEqual,
    "===", // CaseEquality,
    "!==", // CaseInequality,
    
    "!",   // LogicalNegation,
    "&&",  // LogicalAND,
    "||",  // LogicalOR,
    "==",  // LogicalEquality,
    "!=",  // LogicalInequality,
    
    "&",   // BitwiseAND,
    "|",   // BitwiseOR,
    "~|",  // BitwiseNOR,
    "~^",  // BitwiseNXOR,
    "^",   // BitwiseXOR,
    "~",   // BitwiseNegation,
    
    "&",   // ReductionAND,
    "~&",  // ReductionNAND,
    "~|",  // ReductionNOR,
    "~^",  // ReductionNXOR,
    "^",   // ReductionXOR,
    "|",   // ReductionOR,

    "$unsigned", // CastUnsigned,
    "$signed",   // CastSigned
  };
  static void printName(ostream& ostr,const string& name)
    {
      ostr << name;
      if( name.c_str()[0]=='\\' )
	ostr << ' ';
    }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::String
  ////////////////////////////////////
  Verilog::String::String(const char* text):
    text_(text)
  {
  }
  void Verilog::String::toXML(std::ostream& ostr) const
  {
    ostr << '"' << text_ << '"';
  }
  void Verilog::String::toVerilog(std::ostream& ostr) const
  {
    ostr << '"' << text_ << '"';
  }
  void Verilog::String::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
  }
  void Verilog::String::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Number
  ////////////////////////////////////
  Verilog::Number::Number(const char* text):
    text_(text)
  {
    static  char* BIN_NUM ="01XZ";
    static  char* OCT_NUM ="01234567XZ";
    static  char* HEX_NUM ="0123456789ABCDEFXZ";
    
    vector<char> bits;
    const char* ptr;
    int base;
    unsigned int width;
    
    ptr =strchr(text,'\'');
    if( ptr!=NULL )
      {
	ptr =text;
	width =0;
	for( ;*ptr!='\'';ptr++ )
	  if( isdigit(*ptr) )
	    {
	      width *=10;
	      width +=int(*ptr-'0');
	    }
	if( width==0 )
	  width =32;

	ptr++;
	if( *ptr!=0 )
	  {
	    if( *ptr=='b' || *ptr=='B' )
	      base =2;
	    else if( *ptr=='o' || *ptr=='O' )
	      base =8;
	    else if( *ptr=='h' || *ptr=='H' )
	      base =16;
	    else
	      base =10;
	  }
	else
	  base =10;
      }
    else
      {
	width =32;
	base   =10;
	ptr    =text;
      }
    
    int i;	  
    unsigned long val;

    
    if( base==10 )
      {
	val =0;
	for( ;*ptr!=0;ptr++ )
	  if( isdigit(*ptr) )
	    {
	      val *=10;
	      val +=int(*ptr-'0');
	    }
	
	for(i=0;i<width;i++)
	  bits.push_back( ((val>>i)&1) ? '1' : '0' );
      }
    else
      {
	i =0;
	char* idx;
	ptr =text+strlen(text)-1;
	for( ;*ptr!='\'';ptr-- )
	  {
	    switch( base )
	      {
	      case 2:
		//idx = index(BIN_NUM,toupper(*ptr)); IITH
		idx = strchr(BIN_NUM,toupper(*ptr));
		if( idx!=NULL )
		  if( bits.size()<width )
		    bits.push_back( *idx );
		break;
	      case 8:
		//idx = index(OCT_NUM,toupper(*ptr)); IITH
		idx = strchr(OCT_NUM,toupper(*ptr));
		if( idx!=NULL )
		  {
		    val =int(idx-OCT_NUM);
		    if( val < 8 )
		      for( i=0;i<3;i++ )
			if( bits.size()<width )
			  bits.push_back( ((val>>i)&1) ? '1' : '0' );
			else
			  for( i=0;i<3;i++ )
			    if( bits.size()<width )
			      bits.push_back( *idx );
		  }
		break;
	      case 16:
		//idx =index(HEX_NUM,toupper(*ptr)); IITH
		idx = strchr(HEX_NUM,toupper(*ptr));
		if( idx!=NULL )
		  {
		    val =int(idx-HEX_NUM);
		    if( val < 16 )
		      for( i=0;i<4;i++ )
			if( bits.size()<width )
			  bits.push_back( ((val>>i)&1) ? '1' : '0' );
			else
			  for( i=0;i<4;i++ )
			    if( bits.size()<width )
			      bits.push_back( *idx );
		  }
		break;
	      }
	    
	    if( ptr==text )
	      break;
	  }
      }
    
    for( i=bits.size();i<width;i++ )
      bits.push_back( '0' );
    
    for( i=width-1;i>=0;i-- )
      bitset_ +=bits[i];
    
    if( isPartial() )
      {
	string::iterator i;
	string tmp =bitset_;
	
	for( i=tmp.begin();i!=tmp.end();++i )
	  if( ((*i)=='X') || ((*i)=='Z') || ((*i)=='?') )
	    (*i) ='0';
	value_ =tmp;
	
	tmp =bitset_;
	for( i=tmp.begin();i!=tmp.end();++i )
	  if( ((*i)=='1') || ((*i)=='0') )
	    (*i) ='1';
	  else
	    (*i) ='0';
	mask_ =tmp;
      }
    else
      {
	value_ =bitset_;
      }

    width_ =bitset_.size();

  }
  bool Verilog::Number::isPartial() const
  {
    if( bitset_.find('X')!=string::npos )
      return( true );
    if( bitset_.find('?')!=string::npos )
      return( true );
    if( bitset_.find('Z')!=string::npos )
      return( true );
    
    return( false );
  }
  signed Verilog::Number::calcConstant() const
  {
    signed ret =0;
    int i;
    for( i=0;i<bitset_.size();i++ )
      {
	ret =ret<<1;
	if( bitset_[i]=='1' )
	  ret |=1;
      }
    return ret;
  }
  void Verilog::Number::toXML(std::ostream& ostr) const
  {
    ostr << bitset_;
  }
  void Verilog::Number::toVerilog(std::ostream& ostr) const
  {
    ostr << text_;
  }
  void Verilog::Number::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    /**
       string tmp =bitset_;
       string::iterator i;
       
       {
       for( i=tmp.begin();i!=tmp.end();++i )
       if( ((*i)=='X') ||((*i)=='Z') )
       (*i)='0';
       value_ =mod->newNet(tmp.c_str(),Net::CONSTANT,width()-1,0,Net::PRIVATE,0,0);
       }
       
       if( isPartial() )
       {
       tmp =bitset_;
       for( i=tmp.begin();i!=tmp.end();++i )
       if( ((*i)=='1') ||((*i)=='0') )
       (*i)='1';
       else if( ((*i)=='X') ||((*i)=='Z') )
       (*i)='0';
       mask_ =mod->newNet(tmp.c_str(),Net::MASK,width()-1,0,Net::PRIVATE,0,0);
       }
    **/
  }
  void Verilog::Number::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Identifier
  ////////////////////////////////////
  Verilog::Identifier::~Identifier()
  {
    delete msb_;
    delete lsb_;
    delete idx_;
  }
  void Verilog::Identifier::toXML(std::ostream& ostr) const
  {
    ostr << name_;
    
    if( idx_!=NULL )
      {
	ostr << '[';
	idx_->toXML(ostr);
	ostr << ']';
      }
    else if( msb_!=NULL && lsb_!=NULL )
      {
	ostr << '[';
	msb_->toXML(ostr);
	ostr << ':';
	lsb_->toXML(ostr);
	ostr << ']';
      }
  }
  void Verilog::Identifier::toVerilog(std::ostream& ostr) const
  {
    printName( ostr,name_ );

    if( idx_!=NULL )
      {
	ostr << '[';
	idx_->toVerilog(ostr);
	ostr << ']';
      }
    else if( msb_!=NULL && lsb_!=NULL )
      {
	ostr << '[';
	msb_->toVerilog(ostr);
	ostr << ':';
	lsb_->toVerilog(ostr);
	ostr << ']';
      }
  }
  bool Verilog::Identifier::isPartial() const
  {
    if( idx_!=NULL )
      {
	return true;
      }
    else if( msb_!=NULL && lsb_!=NULL )
      {
	if( net_!=NULL )
	  {
	    if( net_->msb()!=NULL && net_->lsb()!=NULL )
	      {
		if( (msb_->calcConstant()==net_->msb()->calcConstant()) &&
		    (lsb_->calcConstant()==net_->lsb()->calcConstant()) )
		  return false;
		else
		  return true;
	      }
	    else
	      {
		return true;
	      }
	  }
	else
	  {
	    std::cerr << " can't estimate. " << name_ << " is not linked.\n";
	    return true;
	  }
      }
    else
      return false;
  }
  unsigned int Verilog::Identifier::width() const
  {
    if( idx_!=NULL )
      {
	if( net_->isArray() )
	  return net_->width();
	else
	  return 1;
      }
    else if( msb_!=NULL && lsb_!=NULL )
      {
	return (msb_->calcConstant()-lsb_->calcConstant()+1);
      }
    else
      {
	return net_->width();
      }
  }
  void Verilog::Identifier::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    if( msb_!=NULL )
      msb_->link(net,mod,scope);
    if( lsb_!=NULL )
      lsb_->link(net,mod,scope);
    if( idx_!=NULL )
      idx_->link(net,mod,scope);
    
    // HIDENTIFIER 
    // DIDENTIFIER 
    bool hie=false;
    if( (name_.c_str()[0]!='\\')&&
	(name_.c_str()[0]!='`') )
      if( name_.find(".")!=string::npos )
	{
	  hie =true;
	  name_ ='\\' + name_;
	}

    {
      int i;
      while( (i=name_.find(".\\"))!=string::npos )
	name_.replace(i,2,".");
    }
    
    map<string,Net*>::const_iterator i;

    if( name_.c_str()[0]=='\\' )
      {
	i =net.find( scope + '.' + name_.substr(name_.rfind(".")+1) );
	if( i!=net.end() )
	  {
	    net_ =i->second;
	    return;
	  }
      }

    i =net.find( scope + '.' + name_ );
    if( i!=net.end() )
      {
	net_ =i->second;
	return;
      }

    i =net.find(name_);
    if( i!=net.end() )
      {
	net_ =i->second;
	return;
      }

    if( name_.c_str()[0]=='`' )
      {
	std::cerr << "not defined identifier : " << name_ << std::endl;
	return;
      }

    if( !hie )
      {
//	cout << scope << ' ' << name_ << ' '
//	       << string(scope + '.' + name_.substr(name_.rfind(".")+1))
//	       << endl;
	std::cerr << "implicit identifier : " << name_ << std::endl;

		  
	net_ =mod->newNet( name_.c_str(),Verilog::Net::IMPLICIT );
	return;
      }

  }


  Verilog::Expression* Verilog::Identifier::clone(const string& hname) const
  {
    Verilog::Identifier* ret =new Identifier();
    if( name_.c_str()[0]=='`' ) // global defined name
      ret->name_ =name_;
    else
      ret->name_ =hname + name_;

    ret->msb_ =(msb_!=NULL) ? msb_->clone(hname) : NULL;
    ret->lsb_ =(lsb_!=NULL) ? lsb_->clone(hname) : NULL;
    ret->idx_ =(idx_!=NULL) ? idx_->clone(hname) : NULL;

    return ret;
  }
  Verilog::Expression* Verilog::Identifier::clone() const
  {
    Verilog::Identifier* ret =new Identifier();
    ret->name_ =name_;
    ret->msb_ =(msb_!=NULL) ? msb_->clone() : NULL;
    ret->lsb_ =(lsb_!=NULL) ? lsb_->clone() : NULL;
    ret->idx_ =(idx_!=NULL) ? idx_->clone() : NULL;
    return ret;
  }
  void Verilog::Identifier::chain(set<const Net*>& ev) const
  {
    if( net_!=NULL )
      ev.insert(net_);
    if( msb_!=NULL )
      msb_->chain(ev);
    if( lsb_!=NULL )
      lsb_->chain(ev);
    if( idx_!=NULL )
      idx_->chain(ev);
  }
  void Verilog::Identifier::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    if( msb_!=NULL )
      msb_->chain(ev);
    if( lsb_!=NULL )
      lsb_->chain(ev);
    if( idx_!=NULL )
      idx_->chain(ev);
  }
  void Verilog::Identifier::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Concat
  ////////////////////////////////////
  Verilog::Concat::~Concat()
  {
    vector<Expression*>::iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::Concat::toXML(std::ostream& ostr) const
  {
    if( repeat_!=NULL )
      {
	ostr << '{';
	repeat_->toXML(ostr);
      }
    
    ostr << '{';
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      {
	if( i!=list_.begin() )
	  ostr << ',';
	(*i)->toXML(ostr);
      }
    
    ostr << '}';
    if( repeat_!=NULL )
      ostr << '}';
  }
  void Verilog::Concat::toVerilog(std::ostream& ostr) const
  {
    if( repeat_!=NULL )
      {
	ostr << '{';
	repeat_->toVerilog(ostr);
      }
    
    ostr << '{';
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      {
	if( i!=list_.begin() )
	  ostr << ',';
	(*i)->toVerilog(ostr);
      }
    
    ostr << '}';
    if( repeat_!=NULL )
      ostr << '}';
  }
  void Verilog::Concat::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    if( repeat_!=NULL )
      repeat_->link(net,mod,scope);
    vector<Expression*>::iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->link(net,mod,scope);
  }
  unsigned int Verilog::Concat::width() const
  {
    unsigned int ret=0;
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      ret +=(*i)->width();
    
    if( repeat_!=NULL )
      ret *=repeat_->calcConstant();
    
    return ret;
  }
  Verilog::Expression* Verilog::Concat::clone(const string& hname) const
  {
    Verilog::Concat* ret =new Verilog::Concat;
    if( repeat_!=NULL )
      ret->repeat_ =repeat_->clone(hname);
    
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      ret->list_.push_back( (*i)->clone(hname) );
    
    return ret;
  }
  Verilog::Expression* Verilog::Concat::clone() const
  {
    Verilog::Concat* ret =new Verilog::Concat;
    if( repeat_!=NULL )
      ret->repeat_ =repeat_->clone();
    
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      ret->list_.push_back( (*i)->clone() );
    
    return ret;
  }
  void Verilog::Concat::chain(set<const Net*>& ev) const
  {
    if( repeat_!=NULL )
      repeat_->chain(ev);
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->chain(ev);
  }
  void Verilog::Concat::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    if( repeat_!=NULL )
      repeat_->chain(ev);
    vector<Expression*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->chain(ev);
  }
  void Verilog::Concat::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Event
  ////////////////////////////////////
  Verilog::Event::~Event()
  {
    delete expr_;
  }
  void Verilog::Event::toXML(std::ostream& ostr) const
  {
    switch( type_ )
      {
      case POSEDGE:
	ostr << "posedge ";
	break;
      case NEGEDGE:
	ostr << "negedge ";
	break;
      }
    expr_->toXML(ostr);
  }
  void Verilog::Event::toVerilog(std::ostream& ostr) const
  {
    switch( type_ )
      {
      case POSEDGE:
	ostr << "posedge ";
	break;
      case NEGEDGE:
	ostr << "negedge ";
	break;
      }
    expr_->toVerilog(ostr);
  }
  void Verilog::Event::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    expr_->link(net,mod,scope);
  }
  Verilog::Expression* Verilog::Event::clone(const string& hname) const
  {
    return new Verilog::Event(type_,
			      (expr_!=NULL) ? expr_->clone(hname) : NULL );
  }
  Verilog::Expression* Verilog::Event::clone() const
  {
    return new Verilog::Event(type_,
			      (expr_!=NULL) ? expr_->clone() : NULL );
  }
  void Verilog::Event::chain(set<const Net*>& ev) const
  {
    if( expr_!=NULL )
      expr_->chain(ev);
  }
  void Verilog::Event::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    if( expr_!=NULL )
      expr_->chain(ev);
  }
  void Verilog::Event::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Unary
  ////////////////////////////////////
  Verilog::Unary::~Unary()
  {
    delete expr_;
  }
  unsigned int Verilog::Unary::width() const
  {
    unsigned int w =expr_->width();

    switch( op_ )
      {
      case Expression::ArithmeticMinus:
	w =expr_->width();
	break;
      case Expression::BitwiseNegation:
	w =expr_->width();
	break;
      case Expression::LogicalNegation:
      case Expression::ReductionAND:
      case Expression::ReductionOR:
      case Expression::ReductionXOR:
      case Expression::ReductionNAND:
      case Expression::ReductionNOR:
      case Expression::ReductionNXOR:
	w =1;
	break;

      case Expression::CastSigned:
      case Expression::CastUnsigned:
	w =expr_->width();
	break;
	
      default:
	w =1;
	break;
      }

    return w;
  }
  signed Verilog::Unary::calcConstant() const
  {
    int ret;
    switch( op_ )
      {
      case ArithmeticMinus:
	ret =-expr_->calcConstant();
	break;
      default:
	ret =expr_->calcConstant();
	break;
      }
    return ret;
  }
  void Verilog::Unary::toXML(std::ostream& ostr) const
  {
    ostr << opToken_[op_];
    ostr << '(';
    expr_->toXML(ostr);
    ostr << ')';
  }
  void Verilog::Unary::toVerilog(std::ostream& ostr) const
  {
    switch( op_ )
      {
      case Expression::CastSigned:
      case Expression::CastUnsigned:
	ostr << opToken_[op_];
	ostr << '(';
	break;
	
      default:
	ostr << '(';
	ostr << opToken_[op_];
	break;
      }
    
    expr_->toVerilog(ostr);
    ostr << ')';
  }
  const char* Verilog::Unary::opToken() const
  {
    return opToken_[op_];
  }
  const char* Verilog::Unary::opName() const
  {
    return opName_[op_];
  }
  void Verilog::Unary::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    expr_->link(net,mod,scope);
  }
  Verilog::Expression* Verilog::Unary::clone(const string& hname) const
  {
    return new Verilog::Unary(op_,
			      (expr_!=NULL) ? expr_->clone(hname) : NULL );
  }
  Verilog::Expression* Verilog::Unary::clone() const
  {
    return new Verilog::Unary(op_,
			      (expr_!=NULL) ? expr_->clone() : NULL );
  }
  void Verilog::Unary::chain(set<const Net*>& ev) const
  {
    if( expr_!=NULL )
      expr_->chain(ev);
  }
  void Verilog::Unary::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    if( expr_!=NULL )
      expr_->chain(ev);
  }
  void Verilog::Unary::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Binary
  ////////////////////////////////////
  Verilog::Binary::~Binary()
  {
    delete left_;
    delete right_;
  }
  unsigned int Verilog::Binary::width() const
  {
    unsigned int w;

    switch( op_ )
      {
      case ArithmeticMultiply:
	w =left_->width()+right_->width();
	break;
      case ArithmeticDivide:
	w =max( left_->width(),right_->width() );
	break;
      case ArithmeticModulus:
	w =max( left_->width(),right_->width() );
	break;
      case ArithmeticAdd:
	w =max( left_->width(),right_->width() )+1;
	break;
      case ArithmeticMinus:
	//	w =max( left_->width(),right_->width() ); ?? 
	w =max( left_->width(),right_->width() )+1;
	break;

      case ArithmeticLeftShift:
	w =max( left_->width(),right_->width() );
	break;
      case ArithmeticRightShift:
	w =max( left_->width(),right_->width() );
	break;
      case ArithmeticPower:
	w =left_->width()*right_->width();
	break;

      case BitwiseAND:
      case BitwiseOR:
      case BitwiseNOR:
      case BitwiseNXOR:
      case BitwiseXOR:
	w =max( left_->width(),right_->width() );
	break;

      case LeftShift:
      case RightShift:
	w =left_->width();
	break;

      case LogicalEquality:
      case CaseEquality:
      case LessEqual:
      case GreaterEqual:
      case LogicalInequality:
      case CaseInequality:
      case LogicalOR:
      case LogicalAND:
      case LessThan:
      case GreaterThan:
	w =1;
	break;

      default:
	w =0;
	break;
      }

    return w;
  }
  signed Verilog::Binary::calcConstant() const
  {
    int ret;
    switch( op_ )
      {
      case BitwiseXOR:
	ret =left_->calcConstant() ^ right_->calcConstant();
	break;
      case ArithmeticMultiply:
	ret =left_->calcConstant() * right_->calcConstant();
	break;
      case ArithmeticDivide:
	ret =left_->calcConstant() / right_->calcConstant();
	break;
      case ArithmeticModulus:
	ret =left_->calcConstant() % right_->calcConstant();
	break;
      case ArithmeticAdd:
	ret =left_->calcConstant() + right_->calcConstant();
	break;
      case ArithmeticMinus:
	ret =left_->calcConstant() - right_->calcConstant();
	break;

      case ArithmeticLeftShift:
	ret =left_->calcConstant() << right_->calcConstant();
	break;
      case ArithmeticRightShift:
	ret =left_->calcConstant() >> right_->calcConstant();
	break;
      case ArithmeticPower:
	ret =(int)pow((double)left_->calcConstant(),(double)right_->calcConstant());
	break;

      case BitwiseAND:
	ret =left_->calcConstant() & right_->calcConstant();
	break;
      case BitwiseOR:
	ret =left_->calcConstant() | right_->calcConstant();
	break;
      case BitwiseNOR:
	ret =~(left_->calcConstant() | right_->calcConstant());
	break;
      case BitwiseNXOR:
	ret =~(left_->calcConstant() ^ right_->calcConstant());
	break;
      case LessThan:
	ret =left_->calcConstant() < right_->calcConstant();
	break;
      case GreaterThan:
	ret =left_->calcConstant() > right_->calcConstant();
	break;
      case LeftShift:
	ret =left_->calcConstant() << right_->calcConstant();
	break;
      case RightShift:
	ret =left_->calcConstant() >> right_->calcConstant();
	break;
      case LogicalEquality:
	ret =left_->calcConstant() == right_->calcConstant();
	break;
      case CaseEquality:
	ret =left_->calcConstant() == right_->calcConstant();
	break;
      case LessEqual:
	ret =left_->calcConstant() <= right_->calcConstant();
	break;
      case GreaterEqual:
	ret =left_->calcConstant() >= right_->calcConstant();
	break;
      case LogicalInequality:
	ret =left_->calcConstant() != right_->calcConstant();
	break;
      case CaseInequality:
	ret =left_->calcConstant() != right_->calcConstant();
	break;
      case LogicalOR:
	ret =left_->calcConstant() || right_->calcConstant();
	break;
      case LogicalAND:
	ret =left_->calcConstant() && right_->calcConstant();
	break;
      default:
	ret =0;
	break;
      }
    return ret;
  }
  void Verilog::Binary::toXML(std::ostream& ostr) const
  {
    ostr << '(';
    left_->toXML(ostr);
    ostr << opToken_[op_];
    right_->toXML(ostr);
    ostr << ')';
  }
  void Verilog::Binary::toVerilog(std::ostream& ostr) const
  {
    ostr << '(';
    left_->toVerilog(ostr);
    ostr << opToken_[op_];
    right_->toVerilog(ostr);
    ostr << ')';
  }
  const char* Verilog::Binary::opToken() const
  {
    return opToken_[op_];
  }
  const char* Verilog::Binary::opName() const
  {
    return opName_[op_];
  }
  void Verilog::Binary::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    left_->link(net,mod,scope);
    right_->link(net,mod,scope);
  }
  Verilog::Expression* Verilog::Binary::clone(const string& hname) const
  {
    return new Verilog::Binary( op_,
				(left_!=NULL) ? left_->clone(hname) : NULL,
				(right_!=NULL) ? right_->clone(hname) : NULL );
  }
  Verilog::Expression* Verilog::Binary::clone() const
  {
    return new Verilog::Binary( op_,
				(left_!=NULL) ? left_->clone() : NULL,
				(right_!=NULL) ? right_->clone() : NULL );
  }
  void Verilog::Binary::chain(set<const Net*>& ev) const
  {
    if( left_!=NULL )
      left_->chain(ev);
    if( right_!=NULL )
      right_->chain(ev);
  }
  void Verilog::Binary::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    if( left_!=NULL )
      left_->chain(ev);
    if( right_!=NULL )
      right_->chain(ev);
  }
  void Verilog::Binary::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Ternary
  ////////////////////////////////////
  Verilog::Ternary::~Ternary()
  {
    delete expr_;
    delete true_;
    delete false_;
  }
  void Verilog::Ternary::toXML(std::ostream& ostr) const
  {
    expr_->toXML(ostr);
    ostr << " ? (";
    true_->toXML(ostr);
    ostr << ") : (";
    false_->toXML(ostr);
    ostr << ')';
  }
  void Verilog::Ternary::toVerilog(std::ostream& ostr) const
  {
    ostr << '(';
    expr_->toVerilog(ostr);
    ostr << " ? ";
    true_->toVerilog(ostr);
    ostr << " : ";
    false_->toVerilog(ostr);
    ostr << ')';
  }
  void Verilog::Ternary::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    expr_->link(net,mod,scope);
    true_->link(net,mod,scope);
    if( false_!=NULL )
      false_->link(net,mod,scope);
  }
  Verilog::Expression* Verilog::Ternary::clone(const string& hname) const
  {
    return new Verilog::Ternary( expr_->clone(hname),
				 (true_!=NULL) ? true_->clone(hname) : NULL,
				 (false_!=NULL) ? false_->clone(hname) : NULL );
  }
  Verilog::Expression* Verilog::Ternary::clone() const
  {
    return new Verilog::Ternary( expr_->clone(),
				 (true_!=NULL) ? true_->clone() : NULL,
				 (false_!=NULL) ? false_->clone() : NULL );
  }
  void Verilog::Ternary::chain(set<const Net*>& ev) const
  {
    if( expr_!=NULL )
      expr_->chain(ev);
    if( true_!=NULL )
      true_->chain(ev);
    if( false_!=NULL )
      false_->chain(ev);
  }
  void Verilog::Ternary::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    if( expr_!=NULL )
      expr_->chain(ev);
    if( true_!=NULL )
      true_->chain(ev);
    if( false_!=NULL )
      false_->chain(ev);
  }
  void Verilog::Ternary::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::CallFunction
  ////////////////////////////////////
  Verilog::CallFunction::~CallFunction()
  {
    vector<Expression*>::iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      {
	delete (*i);
      }
  }
  void Verilog::CallFunction::toXML(std::ostream& ostr) const
  {
    ostr << name_ << '(';
    vector<Expression*>::const_iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      {
	if( i!=parms_.begin() )
	  ostr << ',';
	(*i)->toXML(ostr);
      }
    ostr << ')';
  }
  void Verilog::CallFunction::toVerilog(std::ostream& ostr) const
  {
    printName( ostr,name_ );

    ostr << '(';
    vector<Expression*>::const_iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      {
	if( i!=parms_.begin() )
	  ostr << ',';
	(*i)->toVerilog(ostr);
      }
    ostr << ')';
  }
  void Verilog::CallFunction::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    {
      vector<Expression*>::iterator i;
      for( i=parms_.begin();i!=parms_.end();++i )
	(*i)->link(net,mod,scope);
    }
    {
      map<string,Function*>::const_iterator i =mod->function().find(name_);
      if( i!=mod->function().end() )
	{
	  func_ =i->second;
	  map<string,Net*>::const_iterator ii =func_->net().find(name_);
	  if( ii!=func_->net().end() )
	    net_ =ii->second;
	  else
	    std::cerr << "can't link port of function : " << name_ << std::endl;
	}
      else
	std::cerr << "can't link function : " << name_ << std::endl;
    }
  }
  Verilog::Expression* Verilog::CallFunction::clone(const string& hname) const
  {
    Verilog::CallFunction* ret =new Verilog::CallFunction();
    ret->name_ =hname + name_;
    vector<Expression*>::const_iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      ret->parms_.push_back( (*i)->clone(hname) );
    return ret;
  }
  Verilog::Expression* Verilog::CallFunction::clone() const
  {
    Verilog::CallFunction* ret =new Verilog::CallFunction();
    ret->name_ =name_;
    vector<Expression*>::const_iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      ret->parms_.push_back( (*i)->clone() );
    return ret;
  }
  void Verilog::CallFunction::chain(set<const Net*>& ev) const
  {
    vector<Expression*>::const_iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      (*i)->chain(ev);
  }
  void Verilog::CallFunction::chain(set<const Expression*>& ev) const
  {
    ev.insert((Expression*)this);
    vector<Expression*>::const_iterator i;
    for( i=parms_.begin();i!=parms_.end();++i )
      (*i)->chain(ev);
  }
  void Verilog::CallFunction::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Net
  ////////////////////////////////////
  void Verilog::Net::toXML(std::ostream& ostr,const string& name,int indent) const
  {
    ostr << std::setw(indent) << "" << "<net name=\"" << name << "\" ";
    
    if( (msb_!=NULL)&&(lsb_!=NULL) )
      {
	ostr << "bitrange=\"";
	msb_->toXML(ostr);
	ostr << ' ';
	lsb_->toXML(ostr);
	ostr << "\" ";
      }
    
    if( (sa_!=NULL)&&(ea_!=NULL) )
      {
	ostr << "addressrange=\"";
	sa_->toXML(ostr);
	ostr << ' ';
	ea_->toXML(ostr);
	ostr << "\" ";
      }

    ostr << "type=\"";
    switch( type_ )
      {
      case IMPLICIT:
	ostr << "implicit";break;
      case WIRE:
	ostr << "wire";break;
      case TRI:
	ostr << "tri";break;
      case TRI1:
	ostr << "tri1";break;
      case SUPPLY0:
	ostr << "supply0";break;
      case WAND:
	ostr << "wand";break;
      case TRIAND:
	ostr << "triand";break;
      case TRI0:
	ostr << "tri0";break;
      case SUPPLY1:
	ostr << "supply1";break;
      case WOR: 
	ostr << "wor";break;
      case TRIOR:
	ostr << "trior";break;
      case REG:
	ostr << "reg";break;
      case INTEGER:
	ostr << "integer";break;
      case REAL:
	ostr << "real";break;
	//      case PARAMETER:
	//	ostr << "parameter";break;
      }
    ostr << "\" ";
    
    ostr << "interface=\"";
    switch( interface_ )
      {
      case PRIVATE:
	ostr << "private";break;
      case INPUT:
	ostr << "input";break;
      case OUTPUT:
	ostr << "output";break;
      case INOUT:
	ostr << "inout";break;
      }
    ostr << "\" ";
    
    ostr << "/>\n";
  }
  void Verilog::Net::toVerilog(std::ostream& ostr,const string& name,
			       int indent,bool namedblock) const
  {
    if( interface_!=PRIVATE )
      {
	ostr << std::setw(indent) << "";
	switch( interface_ )
	  {
	  case INPUT:
	    ostr << "input";break;
	  case OUTPUT:
	    {
	      if( type_==FUNCTION )
		ostr << "function";
	      else
		ostr << "output";
	    }
	    break;
	  case INOUT:
	    ostr << "inout";break;
	  }
	
	if( (msb_!=NULL)&&(lsb_!=NULL) )
	  {
	    ostr << ' ' << '[';
	    msb_->toVerilog(ostr);
	    ostr << ':';
	    lsb_->toVerilog(ostr);
	    ostr << ']';
	  }
	ostr << ' ';
	printName( ostr,name );
	ostr << ";\n";
      }
    
    switch( type_ )
      {
      case NAMEDBLOCK_REG:
	if( namedblock )
	  {
	    ostr << std::setw(indent) << "";
	    ostr << "reg";
	    
	    if( sign_ )
	      ostr << " signed";
	    
	    if( (msb_!=NULL)&&(lsb_!=NULL) )
	      {
		ostr << ' ' << '[';
		msb_->toVerilog(ostr);
		ostr << ':';
		lsb_->toVerilog(ostr);
		ostr << ']';
	      }
	    
	    ostr << ' ';
	    printName( ostr,name );
	    
	    if( (sa_!=NULL)&&(ea_!=NULL) )
	      {
		ostr << '[';
		sa_->toVerilog(ostr);
		ostr << ':';
		ea_->toVerilog(ostr);
		ostr << ']';
	      }
	    ostr << ";\n";
	  }
	break;

      case IMPLICIT:
	if( interface_!=PRIVATE )
	  break;
      case WIRE:
      case PARAMETER:
      case TRI:
      case TRI1:
      case SUPPLY0:
      case WAND:
      case TRIAND:
      case TRI0:
      case SUPPLY1:
      case WOR: 
      case TRIOR:
      case REG:
      case INTEGER:
	ostr << std::setw(indent) << "";
	switch( type_ )
	  {
	  case IMPLICIT:
	    ostr << "wire";break;
	  case WIRE:
	    ostr << "wire";break;
	  case PARAMETER:
	    ostr << "parameter";break;
	  case TRI:
	    ostr << "tri";break;
	  case TRI1:
	    ostr << "tri1";break;
	  case SUPPLY0:
	    ostr << "supply0";break;
	  case WAND:
	    ostr << "wand";break;
	  case TRIAND:
	    ostr << "triand";break;
	  case TRI0:
	    ostr << "tri0";break;
	  case SUPPLY1:
	    ostr << "supply1";break;
	  case WOR: 
	    ostr << "wor";break;
	  case TRIOR:
	    ostr << "trior";break;
	  case REG:
	    ostr << "reg";break;
	  case INTEGER:
	    ostr << "integer";break;
	  }

	if( sign_ )
	  ostr << " signed";
	
	if( (msb_!=NULL)&&(lsb_!=NULL) )
	  {
	    ostr << ' ' << '[';
	    msb_->toVerilog(ostr);
	    ostr << ':';
	    lsb_->toVerilog(ostr);
	    ostr << ']';
	  }

	ostr << ' ';
	printName( ostr,name );
	
	if( (sa_!=NULL)&&(ea_!=NULL) )
	  {
	    ostr << '[';
	    sa_->toVerilog(ostr);
	    ostr << ':';
	    ea_->toVerilog(ostr);
	    ostr << ']';
	  }
	ostr << ";\n";
	break;
      }
  }
  void Verilog::Net::link(const map<string,Net*>& net,Module* mod,const string& scope)
    {
      if( msb_!=NULL )
	msb_->link(net,mod,scope);
      if( lsb_!=NULL )
	lsb_->link(net,mod,scope);

      if( sa_!=NULL )
	sa_->link(net,mod,scope);
      if( ea_!=NULL )
	ea_->link(net,mod,scope);

      //////////////////
      if( type_==PARAMETER )
	{
	  vector<Process*>::const_iterator i;
          for( i=mod->process().begin();i!=mod->process().end();++i )
            if( (*i)->type()==Process::PARAMETER )
              if( typeid(*(*i)->statement())==typeid(Verilog::Assign) )
                {
                  Assign* param =(Verilog::Assign*)(*i)->statement();
                  if( typeid(*(param->leftValue()))==typeid(Verilog::Identifier) )
                    {
                      Identifier* id =(Verilog::Identifier*)param->leftValue();
                      if( id->net()==this )
                        {
                          rval_ =param->rightValue();
                          return;
                        }
                    }
                }
	}
    }
  Verilog::Net* Verilog::Net::clone(const string& hname) const
    {
      Verilog::Net* ret =new Net
	(
	 type_,
	 ((msb_!=NULL) ? msb_->clone(hname) : NULL),
	 ((lsb_!=NULL) ? lsb_->clone(hname) : NULL),
	 interface_,
	 ((sa_!=NULL) ? sa_->clone(hname) : NULL),
	 ((ea_!=NULL) ? ea_->clone(hname) : NULL),
	 sign_
	 );

      return ret;
    }
  Verilog::Net* Verilog::Net::clone() const
    {
      Verilog::Net* ret =new Net
	(
	 type_,
	 ((msb_!=NULL) ? msb_->clone() : NULL),
	 ((lsb_!=NULL) ? lsb_->clone() : NULL),
	 interface_,
	 ((sa_!=NULL) ? sa_->clone() : NULL),
	 ((ea_!=NULL) ? ea_->clone() : NULL),
	 sign_
	 );
      return ret;
    }
  void Verilog::Net::callback(Callback& cb) const
    {
      cb.trap( this );
    }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Block
  ////////////////////////////////////
  Verilog::Block::~Block()
  {
    vector<Statement*>::iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::Block::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<block>\n";
    vector<Statement*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->toXML(ostr,indent);
    ostr << std::setw(--indent) << "" << "</block>\n";
  }
  void Verilog::Block::toVerilog(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "begin";
    if( name_.empty() )
      ostr << '\n';
    else // named block
      {
	ostr << " : " << name_ << '\n';
	if( module_!=NULL )
	  {
	    map<string,Net*>::const_iterator i;
	    for( i=module_->net().begin();i!=module_->net().end();++i )
	      {
		if( i->first.find(name_)==0 )
		  {
		    i->second->toVerilog(ostr,
					 i->first.substr(i->first.rfind('.')+1),
					 indent,true);
		  }
	      }
	  }
      }

    vector<Statement*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->toVerilog(ostr,indent);
    ostr << std::setw(--indent) << "" << "end\n";
  }
  void Verilog::Block::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    //    cout << scope << ' ' << name_ << endl;
    vector<Statement*>::iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->link(net,mod,((name_.length()<scope.length()) ? scope : name_) );
  }
  Verilog::Statement* Verilog::Block::clone(const string& hname) const
  {
    Verilog::Block* ret =new Verilog::Block(type_);
    ret->name_ =hname + name_;
    {
      vector<Statement*>::const_iterator i;
      for( i=list_.begin();i!=list_.end();++i )
	ret->list_.push_back( (*i)->clone(hname) );
    }
    return ret;
  }
  void Verilog::Block::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
    vector<Statement*>::const_iterator i;
    for( i=list_.begin();i!=list_.end();++i )
      (*i)->chain(ss);
  }
  void Verilog::Block::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Case::Item
  ////////////////////////////////////
  Verilog::Case::Item::~Item()
  {
    vector<Expression*>::iterator i;
    for( i=expr_.begin();i!=expr_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::Case::Item::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<item ";
    ostr << "condition=\"";
    vector<Expression*>::const_iterator i;
    for( i=expr_.begin();i!=expr_.end();++i )
      {
	if( i!=expr_.begin() )
	  ostr << ',';
	(*i)->toXML(ostr);
      }
    ostr << "\"";
    ostr << ">\n";
    stat_->toXML(ostr,indent);
    ostr << std::setw(--indent) << "" << "</item>\n";
  }
  void Verilog::Case::Item::toVerilog(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "";
    if( expr_.empty() )
      {
	ostr << "default";
      }
    else
      {
	vector<Expression*>::const_iterator i;
	for( i=expr_.begin();i!=expr_.end();++i )
	  {
	    if( i!=expr_.begin() )
	      {
		ostr << ",\n";
		ostr << std::setw(indent) << "";
	      }
	    (*i)->toVerilog(ostr);
	  }
      }
    ostr << " :";


    if( typeid(*stat_)==typeid(Verilog::Assign) )
      indent =1;
    else
      {
	ostr << '\n';
	indent++;
      }

    stat_->toVerilog(ostr,indent);
  }
  void Verilog::Case::Item::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    vector<Expression*>::iterator i;
    for( i=expr_.begin();i!=expr_.end();++i )
      (*i)->link(net,mod,scope);
    stat_->link(net,mod,scope);
  }
  Verilog::Case::Item* Verilog::Case::Item::clone(const string& hname) const
  {
    Verilog::Case::Item* ret =new Verilog::Case::Item();
    vector<Expression*>::const_iterator i;
    for( i=expr_.begin();i!=expr_.end();++i )
      ret->expr_.push_back( (*i)->clone(hname) );
    ret->stat_ =(stat_!=NULL) ? stat_->clone(hname) : NULL;
    return ret;
  }
  void Verilog::Case::Item::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
    if( stat_!=NULL )
      stat_->chain(ss);
  }
  void Verilog::Case::Item::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Case
  ////////////////////////////////////
  Verilog::Case::~Case()
  {
    delete expr_;
    vector<Item*>::iterator i;
    for( i=items_.begin();i!=items_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::Case::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<case type=\"";
    switch( type_ )
      {
      case CASE:
	ostr << "case";
	break;
      case CASEX:
	ostr << "casex";
	break;
      case CASEZ:
	ostr << "casez";
	break;
      }
    ostr << "\">\n";
    
    vector<Item*>::const_iterator i;
    for( i=items_.begin();i!=items_.end();++i )
      (*i)->toXML(ostr,indent);
    
    ostr << std::setw(--indent) << "" << "</case>\n";
  }
  void Verilog::Case::toVerilog(std::ostream& ostr,int indent) const
  {
    switch( type_ )
      {
      case CASE:
	ostr << std::setw(indent) << "" << "case(";
	break;
      case CASEX:
	ostr << std::setw(indent) << "" << "casex(";
	break;
      case CASEZ:
	ostr << std::setw(indent) << "" << "casez(";
	break;
      }
    expr_->toVerilog(ostr);
    ostr << ")\n";
    indent++;
    
    vector<Item*>::const_iterator i;
    for( i=items_.begin();i!=items_.end();++i )
      (*i)->toVerilog(ostr,indent);
    ostr << std::setw(--indent) << "" << "endcase\n";
  }
  void Verilog::Case::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    expr_->link(net,mod,scope);
    vector<Item*>::iterator i;
    for( i=items_.begin();i!=items_.end();++i )
      (*i)->link(net,mod,scope);
  }
  Verilog::Statement* Verilog::Case::clone(const string& hname) const
  {
    Verilog::Case* ret =new Verilog::Case();
    ret->type_ =type_;
    ret->expr_ =(expr_!=NULL) ? expr_->clone(hname) : NULL;
    vector<Item*>::const_iterator i;
    for( i=items_.begin();i!=items_.end();++i )
      ret->items_.push_back( (*i)->clone(hname) );
    return ret;
  }
  void Verilog::Case::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
    vector<Item*>::const_iterator i;
    for( i=items_.begin();i!=items_.end();++i )
      (*i)->chain(ss);
  }
  void Verilog::Case::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Condition
  ////////////////////////////////////
  Verilog::Condition::~Condition()
  {
    delete expr_;
    delete true_;
    delete false_;
  }
  void Verilog::Condition::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<condition when=\"";
    expr_->toXML(ostr);
    ostr << "\">\n";
    ostr << std::setw(indent++) << "" << "<true>\n";
    true_->toXML(ostr,indent);
    ostr << std::setw(--indent) << "" << "</true>\n";
    if( false_!=NULL )
      {
	ostr << std::setw(indent++) << "" << "<false>\n";
	false_->toXML(ostr,indent);
	ostr << std::setw(--indent) << "" << "</false>\n";
      }
    ostr << std::setw(--indent) << "" << "</condition>\n";
  }
  void Verilog::Condition::toVerilog(std::ostream& ostr,int indent) const
  {
    if( indent<0 )
      indent =-indent;
    else
      ostr << std::setw(indent) << "";
    ostr << "if(";

    expr_->toVerilog(ostr);
    ostr << ")\n";
    true_->toVerilog(ostr,indent+1);
    if( false_!=NULL )
      {
	ostr << std::setw(indent) << "" << "else";
	if( typeid(*false_)==typeid(Verilog::Condition) )
	  {
	    ostr << ' ';
	    false_->toVerilog(ostr,-indent);
	  }
	else
	  {
	    ostr << '\n';
	    false_->toVerilog(ostr,indent+1);
	  }
      }
  }
  void Verilog::Condition::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    expr_->link(net,mod,scope);
    true_->link(net,mod,scope);
    if( false_!=NULL )
      false_->link(net,mod,scope);
  }
  Verilog::Statement* Verilog::Condition::clone(const string& hname) const
  {
    Verilog::Condition* ret =new Verilog::Condition();
    ret->expr_ =(expr_!=NULL) ? expr_->clone(hname) : NULL;
    ret->true_ =(true_!=NULL) ? true_->clone(hname) : NULL;
    ret->false_ =(false_!=NULL) ? false_->clone(hname) : NULL;
    return ret;
  }
  void Verilog::Condition::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
    true_->chain(ss);
    if( false_!=NULL )
      false_->chain(ss);
  }
  void Verilog::Condition::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::EventStatement
  ////////////////////////////////////
  Verilog::EventStatement::~EventStatement()
  {
    delete stat_;
    vector<Event*>::iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::EventStatement::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<event when=\"";
    vector<Event*>::const_iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      {
	if( i!=event_.begin() )
	  ostr << " or ";
	(*i)->toXML(ostr);
      }
    ostr << "\">\n";
    if( stat_!=NULL )
      stat_->toXML(ostr,indent);
    ostr << std::setw(--indent) << "" << "</event>\n";
  }
  void Verilog::EventStatement::toVerilog(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "" << "@(";
    vector<Event*>::const_iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      {
	if( i!=event_.begin() )
	  ostr << " or ";
	(*i)->toVerilog(ostr);
      }
    ostr << ")\n";
    if( stat_!=NULL )
      stat_->toVerilog(ostr,indent+1);
  }
  void Verilog::EventStatement::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    vector<Event*>::iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      (*i)->link(net,mod,scope);
    stat_->link(net,mod,scope);

    { // Verilog-2000 enhancements
      if( event_.empty() )
	{
	  set<const Net*> chain;
	  RightNetChainCB rnccb( chain );
	  stat_->callback( rnccb );

	  set<const Net*> lchain;
	  LeftNetChainCB lnccb( lchain );
	  stat_->callback( lnccb );

	  set<const Net*>::const_iterator i;
	  for( i=chain.begin();i!=chain.end();++i )
	    {
	      if( lchain.find(*i)==lchain.end() )
		{
		  event_.push_back( new Event(Event::ANYEDGE,
					      new Identifier(mod->findName(*i))) );
		  event_.back()->link(net,mod,scope);	
		}      
	    }
	}
    }
  }
  Verilog::Statement* Verilog::EventStatement::clone(const string& hname) const
  {
    Verilog::EventStatement* ret =new Verilog::EventStatement();
    vector<Event*>::const_iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      ret->event_.push_back( (Event*)(*i)->clone(hname) );
    ret->stat_ =stat_->clone(hname);
    return ret;
  }
  void Verilog::EventStatement::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
    if( stat_!=NULL )
      stat_->chain(ss);
  }
  bool Verilog::EventStatement::isEdge() const
  {
    vector<Event*>::const_iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      if( ( (*i)->type()!=Event::POSEDGE )&&
	  ( (*i)->type()!=Event::NEGEDGE ) )
	return false;
    return true;
  }
  bool Verilog::EventStatement::isLevel() const
  {
    vector<Event*>::const_iterator i;
    for( i=event_.begin();i!=event_.end();++i )
      if( (*i)->type()!=Event::ANYEDGE )
	return false;
    return true;
  }
  void Verilog::EventStatement::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Assign
  ////////////////////////////////////
  Verilog::Assign::~Assign()
  {
    delete lval_;
    delete rval_;
  }
  void Verilog::Assign::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<assign type=\"";
    switch( type_ )
      {
      case BLOCKING:    ostr << "blocking\">\n"; break;
      case NONBLOCKING: ostr << "nonblocking\">\n"; break;
      }
    set<const Net*> lc;
    lval_->chain(lc);
    ostr << std::setw(indent+1) << "" << "<left chain=\"" << lc.size() << "\">";
    lval_->toXML(ostr);
    ostr << "</left>\n";
    set<const Net*> rc;
    rval_->chain(rc);
    ostr << std::setw(indent+1) << "" << "<right chain=\"" << rc.size() << "\">";
    rval_->toXML(ostr);
    ostr << "</right>\n";
    ostr << std::setw(--indent) << "" << "</assign>\n";
  }
  void Verilog::Assign::toVerilog(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "";
    lval_->toVerilog(ostr);
    switch( type_ )
      {
      case BLOCKING: ostr << " ="; break;
      case NONBLOCKING: ostr << " <=";
	if( source_->dec_tpd_ )
	  ostr << "`TPD ";
	break;
      }
    rval_->toVerilog(ostr);
    ostr << ";\n";
  }
  void Verilog::Assign::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    lval_->link(net,mod,scope);
    rval_->link(net,mod,scope);
  }
  Verilog::Statement* Verilog::Assign::clone(const string& hname) const
  {
    return new Verilog::Assign( type_,
				(lval_!=NULL) ? lval_->clone(hname) : NULL,
				(rval_!=NULL) ? rval_->clone(hname) : NULL );
  }
  bool Verilog::Assign::isSimpleLeft() const
  {
    if( (typeid(*lval_)==typeid(Verilog::Identifier)) )
      {
	Verilog::Identifier* l=(Verilog::Identifier*)lval_;
	if( (l->msb()==NULL)&&
	    (l->lsb()==NULL)&&
	    (l->idx()==NULL) )
	  return true;
      }
    return false;
  }
  bool Verilog::Assign::isSimpleRight() const
  {
    if( (typeid(*rval_)==typeid(Verilog::Identifier)) )
      {
	Verilog::Identifier* r=(Verilog::Identifier*)rval_;
	
	if( (r->msb()==NULL)&&
	    (r->lsb()==NULL)&&
	    (r->idx()==NULL) )
	  return true;
      }
    return false;
  }
  bool Verilog::Assign::isSimple() const
  {
    return (isSimpleLeft()&&isSimpleRight());
  }
  void Verilog::Assign::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
  }
  void Verilog::Assign::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::For
  ////////////////////////////////////
  Verilog::For::For(Identifier* i1,Expression* e1,
		    Expression* e2,
		    Identifier* i2,Expression* e3,
		    Statement* s)
  {
      {
	ita_ =i1;
	begin_ =e1;
	cond_ =e2;
	reach_ =e3;
	stat_ =s;
      }
      //std::cerr << "can't support a complexed expression in for-loop\n";
  }
  Verilog::For::~For()
  {
    delete ita_;
    delete begin_;
    delete cond_;
    delete reach_;
    delete stat_;
  }
  void Verilog::For::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<for ";
    ostr << "iterator=\"";ita_->toXML(ostr);ostr << "\" ";
    ostr << "begin=\"";begin_->toXML(ostr);ostr << "\" ";
    ostr << "condition=\"";  cond_->toXML(ostr);ostr << "\" ";
    ostr << "reach=\"";reach_->toXML(ostr);ostr << "\" ";
    ostr << ">\n";
    stat_->toXML(ostr,indent);
    ostr << std::setw(--indent) << "" << "</for>\n";
  }
  void Verilog::For::toVerilog(std::ostream& ostr,int indent) const
  {
    if( indent<0 )
      indent =-indent;
    else
      ostr << std::setw(indent) << "";

    ostr << "for(";
    ita_->toVerilog(ostr);ostr << "=";begin_->toVerilog(ostr);ostr << ";";
    cond_->toVerilog(ostr);ostr << ";";
    ita_->toVerilog(ostr);ostr << "=";reach_->toVerilog(ostr);
    ostr << ")\n";
    stat_->toVerilog(ostr,indent+1);
  }
  void Verilog::For::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    ita_->link(net,mod,scope);
    begin_->link(net,mod,scope);
    cond_->link(net,mod,scope);
    reach_->link(net,mod,scope);
    stat_->link(net,mod,scope);
  }
  Verilog::Statement* Verilog::For::clone(const string& hname) const
  {
    Verilog::For* ret =new Verilog::For();
    ret->ita_   =(Verilog::Identifier*)ita_->clone(hname);
    ret->begin_ =begin_->clone(hname);
    ret->cond_  =cond_->clone(hname);
    ret->reach_ =reach_->clone(hname);
    ret->stat_  =stat_->clone(hname);
    return ret;
  }
  void Verilog::For::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
    //
    stat_->chain(ss);
    //
  }
  void Verilog::For::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::CallTask
  ////////////////////////////////////
  void Verilog::CallTask::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<callTask ";
    ostr << "name=\"" << name_ << "\" ";
    ostr << ">\n";
    ostr << std::setw(--indent) << "" << "</callTask>\n";
  }
  void Verilog::CallTask::toVerilog(std::ostream& ostr,int indent) const
  {
    if( indent<0 )
      indent =-indent;
    else
      ostr << std::setw(indent) << "";

    ostr << name_ << "(";
    vector<Expression*>::const_iterator i;
    for( i=args_.begin();i!=args_.end();++i )
      {
	if( i!=args_.begin() )
	  ostr << ',';
	(*i)->toVerilog(ostr);
      }
    ostr << ");\n";
  }
  void Verilog::CallTask::link(const map<string,Net*>& net,Module* mod,const string& scope)
  {
    vector<Expression*>::iterator i;
    for( i=args_.begin();i!=args_.end();++i )
      (*i)->link(net,mod,scope);
  }
  Verilog::Statement* Verilog::CallTask::clone(const string& hname) const
  {
    Verilog::CallTask* ret =new Verilog::CallTask();
    ret->name_   =name_; // only system task ?

    vector<Expression*>::const_iterator i;
    for( i=args_.begin();i!=args_.end();++i )
      ret->args_.push_back( (*i)->clone(hname) );

    return ret;
  }
  void Verilog::CallTask::chain(set<const Statement*>& ss) const
  {
    ss.insert((Statement*)this);
  }
  void Verilog::CallTask::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Function
  ////////////////////////////////////
  Verilog::Function::~Function()
  {
    map<string,Net*>::iterator i;
    for( i=net_.begin();i!=net_.end();++i )
      {
	delete i->second;
      }
    delete stat_;
  }
  void Verilog::Function::addNet(const char* name,Verilog::Net* net)
  {
    port_.push_back(name);
    pair<map<string,Net*>::iterator,bool> ret =net_.insert( pair<string,Net*>(name,net) );
    if( !ret.second )
      {
	if( ret.first->second->interface()==Net::PRIVATE )
	  ret.first->second->setInterface( net->interface() );
	if( ret.first->second->type()==Net::IMPLICIT )
	  ret.first->second->setType( net->type() );
      }
  }
  void Verilog::Function::toXML(std::ostream& ostr,int indent) const
  {
  }
  void Verilog::Function::toVerilog(std::ostream& ostr,int indent) const
  {
    vector<string>::const_iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      {
	net_.find(*i)->second->toVerilog(ostr,*i,indent);
	if( i==port_.begin() )
	  indent++;
      }

    stat_->toVerilog(ostr,indent--);
    ostr << std::setw(indent) << "" << "endfunction\n";
  }
  void Verilog::Function::link(Module* mod)
  {
    stat_->link(net_,mod,string(""));
  }
  Verilog::Function* Verilog::Function::clone(const string& hname) const
  {
    string tmp;
    Verilog::Function* ret =new Verilog::Function();
    map<string,Net*>::const_iterator i;
    for( i=net_.begin();i!=net_.end();++i )
      {
	tmp =hname + i->first;
	ret->port_.push_back(tmp);
	ret->net_[tmp] =i->second->clone(hname);
      }
    ret->stat_ =(stat_!=NULL) ? stat_->clone(hname) : NULL;
    return ret;
  }
  void Verilog::Function::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  //Verilog::Process
  ////////////////////////////////////
  Verilog::Process::~Process()
  {
    delete stat_;
  }
  void Verilog::Process::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "" << "<process type=\"";
    switch( type_ )
      {
      case INITIAL:
	ostr << "initial";
	break;
      case ALWAYS:
	ostr << "always";
	break;
      case TASK:
	ostr << "task\" name=\""<< name_;
	break;
      case ASSIGN:
	ostr << "assign";
	break;
      }
    ostr << "\">\n";
    stat_->toXML(ostr,indent+1);  
    ostr << std::setw(indent) << "" << "</process>\n";
  }
  void Verilog::Process::toVerilog(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "";
    switch( type_ )
      {
      case INITIAL:
	ostr << "initial\n";
	stat_->toVerilog(ostr,indent+1);  
	break;
      case ALWAYS:
	ostr << "always\n";
	stat_->toVerilog(ostr,indent+1);  
	break;
      case TASK:
	ostr << "task ";
	printName( ostr,name_ );
	ostr << ";\n";
	stat_->toVerilog(ostr,indent+1);
	ostr << std::setw(indent) << "endtask\n";
	break;
      case ASSIGN:
	ostr << "assign ";
	stat_->toVerilog(ostr,0);
	break;
      case PARAMETER:
	ostr << "parameter ";
	stat_->toVerilog(ostr,0);
	break;
      }
  }
  void Verilog::Process::link(Module* mod)
  {
    stat_->link(mod->net(),mod,string(""));
    stat_->chain(statChain_);
    {
      EventNetChainCB cb(eventChain_);
      stat_->callback( cb );
    }
    {
      LeftNetChainCB cb(leftChain_);
      stat_->callback( cb );
    }
    {
      RightNetChainCB cb(rightChain_);
      stat_->callback( cb );
    }
    //
    {
      NetChainCB cb(nbLeftChain_,
		    nbRightChain_,
		    bLeftChain_,
		    bRightChain_);
      stat_->callback( cb );
    }
  }
  Verilog::Process* Verilog::Process::clone(const string& hname) const
  {
    Verilog::Process* ret =new Verilog::Process();
    ret->type_ =type_;
    ret->stat_ =(stat_!=NULL) ? stat_->clone(hname) : NULL;
    ret->name_ =hname + name_;
    return ret;
  }

  bool Verilog::Process::isEdge() const
  {
    if( typeid( *stat_ )==typeid( Assign ) )
      return false;
    else if( typeid( *stat_ )==typeid( EventStatement ) )
      return ((EventStatement*)(stat_))->isEdge();
    return true;
  }
  bool Verilog::Process::isLevel() const
  {
    if( typeid( *stat_ )==typeid( Assign ) )
      return true;
    else if( typeid( *stat_ )==typeid( EventStatement ) )
      return ((EventStatement*)(stat_))->isLevel();
    return false;
  }
  bool Verilog::Process::isStorage() const
  {
    if( typeid( *stat_ )==typeid( Assign ) )
      return false;
    else if( typeid( *stat_ )==typeid( EventStatement ) )
      {
	set<const Net*>::const_iterator i;
	for( i=rightChain_.begin();i!=rightChain_.end();++i )
	  if( eventChain_.find(*i)==eventChain_.end() )
	    return true;
      }
    return false;
  }
  const Verilog::Statement* Verilog::Process::queryStatement(int type,const Net* src) const
  {
    if( (typeid(*stat_)==typeid(EventStatement)) )
      {
	EventStatement* es =(EventStatement*)stat_;
	vector<Event*>::const_iterator iii;
	for( iii=es->event().begin();iii!=es->event().end();++iii )
	  {
	    if( (*iii)->type()==type )
	      if( (typeid(*(*iii)->expression())==typeid(Identifier)) )
		if( ((Identifier*)((*iii)->expression()))->net()==src )
		  {
		    if( es->event().size()==1 )
		      {
			if( eventChain_.find((Net*)src)!=eventChain_.end() )
			  return es->statement();
		      }
		    else if( es->event().size()>1 )
		      {
			Condition* c;
			set<const Statement*>::const_iterator i;
			set<const Net*>::const_iterator ii;
			for( i=statChain_.begin();i!=statChain_.end();++i )
			  if( typeid(*(*i))==typeid(Condition) )
			    {
			      c =((Condition*)(*i));
			      set<const Net*> n;
			      c->expression()->chain(n);
			      ii =search( eventChain_.begin(),eventChain_.end(),n.begin(),n.end() );
			      if( ii!=eventChain_.end() )
				{
				  if( (*ii)==src )
				    return c->trueStatement();
				  else
				    return c->falseStatement();
				}
			      
			    }
		      }
		  }
	  }
      }
    
    return NULL;
  }
  void Verilog::Process::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////    
  // Verilog::Gate
  ////////////////////////////////////
  Verilog::Gate::~Gate()
  {
    vector<Expression*>::iterator i;
    for( i=pin_.begin();i!=pin_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::Gate::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////    
  // Verilog::Instance::Port
  ////////////////////////////////////
  Verilog::Instance::Port::~Port()
  {
    //    delete con_;
  }
  void Verilog::Instance::Port::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "" << "<port ";
    if( ref_!="" )
      ostr << "reference=\"" << ref_ << "\" ";
    if( con_!=NULL )
      {
	ostr << "connect=\"";con_->toXML(ostr); ostr << "\"";
      }
    ostr << "/>\n";
  }
  void Verilog::Instance::Port::toVerilog(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent) << "";
    if( ref_!="" )
      ostr << '.' << ref_ << '(';
    if( con_!=NULL )
      con_->toVerilog(ostr);
    if( ref_!="" )
      ostr << ')';
  }
  void Verilog::Instance::Port::link(const map<string,Net*>& net,Module* mod,const string& scope,Module* rmod,int idx)
  {
    if( con_!=NULL )
      con_->link(net,mod,string(""));
    
    if( rmod!=NULL )
      {
	if( ref_.empty() )
	  {
	    if( idx<rmod->port().size() )
	      ref_ =rmod->port()[idx];
	  }
	
	map<string,Net*>::const_iterator i =rmod->net().find(ref_);
	if( i!=rmod->net().end() )
	  net_ =i->second;
	else
	  std::cerr << "can't link port of instance : " << ref_ << std::endl;
      }
  }
  void Verilog::Instance::Port::ungroup(Verilog::Module* mod,
					const string& cname,const string& sname)
  {
    if( ref_!="" && con_!=NULL )
      {
	string tmp =sname + ref_;
	if( net_->interface()==Verilog::Net::INPUT )
	  {
	    mod->addAssign(new Verilog::Identifier(tmp.c_str()),con_->clone(cname));
	  }
	else if( net_->interface()==Verilog::Net::OUTPUT )
	  {
	    mod->addAssign(con_->clone(cname),new Verilog::Identifier(tmp.c_str()));
	  }
	else
	  {
	    std::cerr << "can't connect type : " << "INOUT" << std::endl;
	  }
      }
  }
  Verilog::Instance::Port* Verilog::Instance::Port::clone(const string& hname) const
  {
    return new Verilog::Instance::Port(ref_.c_str(),con_->clone(hname));
  }
  void Verilog::Instance::Port::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Instance
  ////////////////////////////////////
  Verilog::Instance::~Instance()
  {
    vector<Port*>::iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      {
	delete *i;
      }
  }
  void Verilog::Instance::addPort(Port* p)
  {
    port_.push_back(p);
  }
  void Verilog::Instance::toXML(std::ostream& ostr,const string& name,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<instance name=\"" << name << "\" type=\"" << type_ << "\">\n";
    vector<Port*>::const_iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      (*i)->toXML(ostr,indent);
    ostr << std::setw(--indent) << "" << "</instance>\n";
  }
  void Verilog::Instance::toVerilog(std::ostream& ostr,const string& name,int indent) const
  {
    ostr << std::setw(indent++) << "" << type_ << ' ';

    if( params_.size()!=0 )
      {
	ostr << "#(";
	multimap<string,Expression*>::const_iterator i;
	for( i=params_.begin();i!=params_.end();++i )
	  {
	    if( i!=params_.begin() )
	      ostr << ',';

	    if( (i->first).length()!=0 )
	      ostr << '.' << i->first << '(';

	    i->second->toVerilog(ostr);

	    if( (i->first).length()!=0 )
	      ostr << ')';
	  }
	ostr << ") ";
      }

    printName( ostr,name );
    ostr << '\n';

    ostr << std::setw(indent++) << "" << "(\n";
    vector<Port*>::const_iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      {
	if( i!=port_.begin() )
	  ostr << ',' << std::endl;
	(*i)->toVerilog(ostr,indent);
      }
    ostr << std::endl;
    ostr << std::setw(--indent) << "" << ");\n";
  }
  void Verilog::Instance::link(Verilog* veri,const map<string,Net*>& net,
			       const string& scope,Module* mod)
  {
    {
      map<string,Module*>::const_iterator i =veri->module().find(type_);
      if( i!=veri->module().end() )
	module_ =i->second;
      else
	std::cerr << "can't link module : " << type_ << std::endl;
    }
    {
      int idx=0;
      vector<Port*>::iterator i;
      for( i=port_.begin();i!=port_.end();++i )
	(*i)->link(net,mod,scope,module_,idx++);
    }
  }
  Verilog::Instance* Verilog::Instance::clone(const string& hname) const
  {
    Verilog::Instance* ret =new Verilog::Instance();
    ret->type_ =type_;
    vector<Port*>::const_iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      ret->addPort( (*i)->clone(hname) );
    return ret;
  }
  void Verilog::Instance::ungroup(Verilog::Module* mod,const string& cname,const string& sname)
  {
    module_->ungroup(mod,sname,params_);
    vector<Port*>::iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      (*i)->ungroup(mod,cname,sname);
  }
  void Verilog::Instance::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::Module
  ////////////////////////////////////
  Verilog::Module::~Module()
  { 
    {
      map<string,Net*>::iterator i;
      for( i=net_.begin();i!=net_.end();++i )
	{
	  delete i->second;
	}
    }
    {
      vector<Process*>::iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	{
	  delete *i;
	}
    }
    {
      map<string,Function*>::iterator i;
      for( i=function_.begin();i!=function_.end();++i )
	{
	  delete i->second;
	}
    }
    {
      map<string,Instance*>::iterator i;
      for( i=instance_.begin();i!=instance_.end();++i )
	{
	  delete i->second;
	}
    }
  }
  void Verilog::Module::addPort(const char* name)
  {
    port_.push_back(name);
  }
  void Verilog::Module::addNet(const char* name,Verilog::Net* net)
  {
    pair<map<string,Net*>::iterator,bool> ret =net_.insert( pair<string,Net*>(name,net));
  }
  Verilog::Net* Verilog::Module::newNet(const char* name,
					int type,
					Expression* msb,Expression* lsb,
					int inter,
					Expression* sa,Expression* ea,
					bool sign)
    {
      Verilog::Net* ret =NULL;
      map<string,Net*>::iterator i =net_.find(name);
      
    if( i==net_.end() )
      {
	ret =new Verilog::Net(type,msb,lsb,inter,sa,ea,sign);
	net_.insert( pair<string,Net*>(name,ret));
      }
    else
      {
	ret =i->second;
	if( ret->interface()==Net::PRIVATE )
	  ret->setInterface( inter );
	if( ret->type()==Net::IMPLICIT )
	  ret->setType( type );
	
      }
    return ret;
  }
  void Verilog::Module::addAssign(Expression* l,Expression* r)
  {
    moe::Verilog::Statement* stat =new moe::Verilog::Assign(moe::Verilog::Assign::BLOCKING,l,r);
    moe::Verilog::Process* proc =new moe::Verilog::Process(moe::Verilog::Process::ASSIGN,stat);
    
    process_.push_back(proc);
  }
  void Verilog::Module::addParameter(Expression* l,Expression* r)
  {
    moe::Verilog::Statement* stat =new moe::Verilog::Assign(moe::Verilog::Assign::BLOCKING,l,r);

    moe::Verilog::Process* proc =new moe::Verilog::Process(moe::Verilog::Process::PARAMETER,stat);
    
    process_.push_back(proc);
  }
  Verilog::Instance* Verilog::Module::newInstance(const char* name)
  {
    moe::Verilog::Instance* inst =new moe::Verilog::Instance;
    pair<map<string,Instance*>::iterator,bool> ret =instance_.insert( pair<string,Instance*>(name,inst));
    if( !ret.second )
      {
	std::cerr << "instance name repetition error : " << name << "\n";
	delete inst;
      }
    return ret.first->second;
  }
  void Verilog::Module::addProcess(Process* proc)
  {
    process_.push_back(proc);
  }
  Verilog::Function* Verilog::Module::newFunction(const char* name)
  {
    Verilog::Function* func =new Function;
    pair<map<string,Function*>::iterator,bool> ret =function_.insert( pair<string,Function*>(name,func) );
    if( !ret.second )
      {
	std::cerr << "function name repetition error : " << name << "\n";
	delete func;
      }
    return ret.first->second;
  }
  void Verilog::Module::addFunction(const char* name,Verilog::Function* func)
  {
    pair<map<string,Function*>::iterator,bool> ret =function_.insert( pair<string,Function*>(name,func) );
    if( !ret.second )
      {
	std::cerr << "function name repetition error : " << name << "\n";
	delete func;
      }
  }
  void Verilog::Module::addInstance(const char* name,moe::Verilog::Instance* inst)
  {
    pair<map<string,Instance*>::iterator,bool> ret =instance_.insert( pair<string,Instance*>(name,inst));
    if( !ret.second )
      {
	std::cerr << "instance name repetition error : " << name << "\n";
	delete inst;
      }
  }
  void Verilog::Module::toXML( std::ostream& ostr,const string& name,int indent ) const
  {
    ostr << std::setw(indent++) << "" << "<module name=\"" << name << "\">\n";
    {
      ostr << std::setw(indent++) << "" << "<port_order>\n";
      vector<string>::const_iterator i;
      for( i=port_.begin();i!=port_.end();++i )
	ostr <<  std::setw(indent) << "" << *i << std::endl;
      ostr <<  std::setw(--indent) << "" << "</port_order>\n";
    }
    {
      map<string,Net*>::const_iterator i;
      for( i=net_.begin();i!=net_.end();++i )
	i->second->toXML(ostr,i->first,indent);
    }
    {
      map<string,Function*>::const_iterator i;
      for( i=function_.begin();i!=function_.end();++i )
	i->second->toXML(ostr,indent);
    }
    {
      map<string,Instance*>::const_iterator i;
      for( i=instance_.begin();i!=instance_.end();++i )
	i->second->toXML(ostr,i->first,indent);
    }
    {
      vector<Process*>::const_iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	(*i)->toXML(ostr,indent);
    }
    ostr << std::setw(--indent) << "" << "</module>\n";
  }
  void Verilog::Module::toVerilog( std::ostream& ostr,const string& name,int indent ) const
  {
    ostr << std::setw(indent++) << "" << "module ";
    printName( ostr,name );
    ostr << std::endl;

    ostr <<  std::setw(indent++) << "" << "(\n";
    vector<string>::const_iterator i;
    for( i=port_.begin();i!=port_.end();++i )
      {
	if( i!=port_.begin() )
	  ostr << ",\n";
	ostr << std::setw(indent) << "" << *i;
      }
    ostr << std::endl;
    ostr << std::setw(--indent) << "" << ");\n";
    
    // parameter
    {
      vector<Process*>::const_iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	if( (*i)->type()==Process::PARAMETER )
	  (*i)->toVerilog(ostr,indent);
    }
    // net::public
    {
      vector<string>::const_iterator i;
      map<string,Net*>::const_iterator ii;
      for( i=port_.begin();i!=port_.end();++i )
	{
	  ii =net_.find(*i);
	  if( ii!=net_.end() )
	    ii->second->toVerilog(ostr,ii->first,indent);
	}
    }
    // net::private
    {
      map<string,Net*>::const_iterator i;
      for( i=net_.begin();i!=net_.end();++i )
	{
	  if( i->second->interface()==Net::PRIVATE )
	    if( i->second->type()!=Net::PARAMETER )
	      i->second->toVerilog(ostr,i->first,indent);
	}
    }
    // function
    {
      map<string,Function*>::const_iterator i;
      for( i=function_.begin();i!=function_.end();++i )
	i->second->toVerilog(ostr,indent);
    }
    // instance
    {
      map<string,Instance*>::const_iterator i;
      for( i=instance_.begin();i!=instance_.end();++i )
	i->second->toVerilog(ostr,i->first,indent);
    }
    // process
    {
      vector<Process*>::const_iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	if( (*i)->type()!=Process::PARAMETER )
	  (*i)->toVerilog(ostr,indent);
    }
    // defparam
    {
      multimap<string,Expression*>::const_iterator i;
      for( i=defparams_.begin();i!=defparams_.end();++i )
	{
	  ostr << std::setw(indent) << "" << "defparam ";	  
	  ostr << i->first << " =";
	  i->second->toVerilog(ostr);
	  ostr << ";\n";
	}
    }
    ostr <<  std::setw(--indent) << "" << "endmodule\n";
  }
  void Verilog::Module::link(Verilog* veri)
  {
    if( source_->debug() )
      std::cerr << "link...\n";

    name_ =std::string(veri->findName(this));
    {
      map<string,Function*>::iterator i;
      for( i=function_.begin();i!=function_.end();++i )
	{
	  if( source_->debug() )
	    std::cerr << " function : " << i->first << std::endl;
	  
	  i->second->link(this);
	}
    }
    {
      map<string,Instance*>::iterator i;
      for( i=instance_.begin();i!=instance_.end();++i )
	{
	  if( source_->debug() )
	    std::cerr << " instance : " << i->first << std::endl;
	  i->second->link(veri,net_,string(""),this);
	}
    }
    {
      vector<Process*>::iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	{
	  if( source_->debug() )
	    {
	      std::cerr << " process : " ;
	      (*i)->toVerilog(std::cerr,0);
	    }

	  (*i)->link(this);
	}
    }

    //

    {
      map<string,Net*>::iterator i;
      for( i=net_.begin();i!=net_.end();++i )
	{
	  if( source_->debug() )
	    std::cerr << " net : " << i->first << std::endl;
	  i->second->link(net_,this,string(""));
	}
    }

  }
  void Verilog::Module::ungroup(Verilog::Module* mod,const string& name,const multimap<string,Expression*>& params)
  {
    if( source_->debug() )
      std::cerr << "ungroup..."<< name << std::endl;
    
    string hname;
    {
      Verilog::Net* net;
      map<string,Net*>::iterator i;
      for( i=net_.begin();i!=net_.end();++i )
	{
	  hname =name + i->first;
	  net =i->second->clone(name);
	  //	  if( i->second->type()==Net::PARAMETER )
	  //	    net->setInterface(Verilog::Net::INPUT);
	  //	  else
	  net->setInterface(Verilog::Net::PRIVATE);
	  net->setType( i->second->type() );
	  mod->addNet(hname.c_str(),net);
	}
    }
    {
      Verilog::Function* func;
      map<string,Function*>::iterator i;
      for( i=function_.begin();i!=function_.end();++i )
	{
	  hname =name + i->first;
	  mod->addFunction(hname.c_str(),i->second->clone(name));
	}
    }
    {
      map<string,Expression*>::const_iterator i;
      for( i=defparams_.begin();i!=defparams_.end();++i )
	{
	  mod->addDefparam(name+i->first,i->second->clone(name));
	}
    }

    {
      map<string,Instance*>::iterator i;
      for( i=instance_.begin();i!=instance_.end();++i )
	{
	  if( i->second->module()!=NULL )
	    {
	      hname =name + i->first + '.';
	      i->second->ungroup(mod,name,hname);
	    }
	  else
	    {
	      hname =name + i->first;
	      mod->addInstance( hname.c_str(),i->second->clone(name) );
	    }
	}
    }
    {
      multimap<string,Expression*>::const_iterator p;
      p=params.begin();
      //
      vector<Process*>::iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	{
	  Process* proc =(*i)->clone(name.c_str());
	  
	  if( (*i)->type()==Process::PARAMETER )
	    {
	      if( params.size()!=0 )
		{
		  if( (params.begin()->first).length()==0 ) // parameter_list
		    {
		      for( ;p!=params.end();++p )
			{
			  if( typeid(*(*i)->statement())==typeid(Verilog::Assign) )
			    {
			      Assign* param =(Verilog::Assign*)proc->statement();
			      param->setRightValue(p->second);
			      ++p;
			      break;
			    }
			}
		    }
		  else // parameter_byname_list
		    {
		      if( typeid(*(*i)->statement())==typeid(Verilog::Assign) )
			{
			  Assign* param =(Verilog::Assign*)(*i)->statement();
			  if( typeid(*(param->leftValue()))==typeid(Verilog::Identifier) )
			    {
			      Identifier* id =(Verilog::Identifier*)param->leftValue();
			      
			      multimap<string,Expression*>::const_iterator p;
			      p =params.find(id->name());
			      if( p!=params.end() )
				{
				  Assign* clonedparam =(Verilog::Assign*)proc->statement();
				  clonedparam->setRightValue(p->second);
				}
			    }  
			}
		    }
		}
	    }
	  
	  mod->addProcess(proc);
	}
    }
  }
  void Verilog::Module::ungroup()
  {
    {
      string hname;
      map<string,Instance*>::iterator i;
      for( i=instance_.begin();i!=instance_.end();++i )
	{
	  if( i->second->module()!=NULL )
	    {
	      hname ='\\' + i->first + '.';
	      i->second->ungroup(this,string(""),hname);
	      delete i->second;
	      instance_.erase(i->first);
	    }
	}
    }
    {
      map<string,Expression*>::const_iterator i;
      for( i=defparams_.begin();i!=defparams_.end();++i )
	{
	  if( i->first.c_str()[0]!='\\' )
	    if( i->first.find(".")!=string::npos )
	      {
		string hname ='\\' + i->first;
		Expression* e=i->second->clone();
		defparams_.erase(i->first);
		addDefparam(hname,e);
	      }
	}
    }

  }
  void Verilog::Module::link()
  {
    {
      map<string,Function*>::iterator i;
      for( i=function_.begin();i!=function_.end();++i )
	i->second->link(this);
    }
    {
      vector<Process*>::iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	(*i)->link(this);
    }

    {
      vector<Process*>::const_iterator i;
      for( i=process_.begin();i!=process_.end();++i )
	if( (*i)->type()==Process::PARAMETER )
	  if( typeid(*(*i)->statement())==typeid(Verilog::Assign) )
	    {
	      Assign* param =(Verilog::Assign*)(*i)->statement();
	      if( typeid(*(param->leftValue()))==typeid(Verilog::Identifier) )
		{
		  Identifier* id =(Verilog::Identifier*)param->leftValue();
		  
		  {
		    map<string,Expression*>::const_iterator defp;
		    defp =defparams_.find(id->name());
		    if( defp!=defparams_.end() )
		      {
			param->setRightValue(defp->second);
		      }
		  }
		}
	    }
    }

    {
      map<string,Net*>::iterator i;
      for( i=net_.begin();i!=net_.end();++i )
	i->second->link(net_,this,string(""));
    }
  }
  void Verilog::Module::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog
  ////////////////////////////////////
  Verilog::~Verilog()
  {
    map<string,Module*>::iterator i;
    for( i=module_.begin();i!=module_.end();++i )
      {
	delete i->second;
      }
  }
  int Verilog::parse(const char* filename)
  {
    source_ =this;
    verilog_input =::fopen( filename,"r" );
    verilog_file  =filename;

    int ret =verilog_parse();
    ::fclose( verilog_input );
    return ret;
  }
  int Verilog::parse(FILE* fp)
  {
    source_ =this;
    verilog_input =fp;
    return ::verilog_parse();
  }
  Verilog::Module* Verilog::addModule(const char* name)
  {
    Verilog::Module* mod =new Verilog::Module;
    pair<map<string,Module*>::iterator,bool> ret =module_.insert( pair<string,Module*>(name,mod) );
    if( !ret.second )
      {
	std::cerr << "module name repetition error : " << name << "\n";
	delete mod;
      }
    return ret.first->second;
  }
  void Verilog::toXML(std::ostream& ostr,int indent) const
  {
    ostr << std::setw(indent++) << "" << "<verilog>\n";
    {
      map<string,Module*>::const_iterator i;
      for( i=module_.begin();i!=module_.end();++i )
	i->second->toXML( ostr,i->first,indent );
    }
    ostr << std::setw(--indent) << "" << "</verilog>\n";
  }
  void Verilog::toVerilog(std::ostream& ostr,int indent) const
  {
    map<string,Module*>::const_iterator i;
    for( i=module_.begin();i!=module_.end();++i )
      i->second->toVerilog( ostr,i->first,indent );
  }
  void Verilog::link()
  {
    map<string,Module*>::iterator i;
    for( i=module_.begin();i!=module_.end();++i )
      {
	std::cerr << "link..." << i->first << std::endl;
	i->second->link( this );
      }
  }
  void Verilog::ungroup(Verilog::Module* top)
  {
    string hname;
    map<string,Instance*>::const_iterator i;
    for( i=top->instance().begin();i!=top->instance().end();++i )
      {
	if( i->second->module()!=NULL )
	  {
	    hname ='\\' + i->first + '.';
	    i->second->ungroup(top,string(""),hname);
	  }
	delete i->second;
      }
    top->instance().clear();
  }
  void Verilog::callback(Callback& cb) const
  {
    cb.trap( this );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::LeftNetChainCB
  ////////////////////////////////////
  void Verilog::LeftNetChainCB::trap(const Process* self)
  {
    self->statement()->callback( *this );
  }
  //////////////////
  void Verilog::LeftNetChainCB::trap(const Block* self)
  {
    vector<Statement*>::const_iterator i;
    for( i=self->list().begin();i!=self->list().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::LeftNetChainCB::trap(const Case* self)
  {
    vector<Case::Item*>::const_iterator i;
    for( i=self->items().begin();i!=self->items().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::LeftNetChainCB::trap(const Case::Item* self)
  {
    self->statement()->callback( *this );
  }
  void Verilog::LeftNetChainCB::trap(const Condition* self)
  {
    self->trueStatement()->callback( *this );
    if( self->falseStatement()!=NULL )
      self->falseStatement()->callback( *this );
  }
  void Verilog::LeftNetChainCB::trap(const EventStatement* self)
  {
    self->statement()->callback( *this );
  }
  void Verilog::LeftNetChainCB::trap(const Assign* self)
  {
    self->leftValue()->callback( *this );
  }  
  //////////////////
  void Verilog::LeftNetChainCB::trap(const Identifier* self)
  {
    if( self->net()!=NULL )
      self->net()->callback( *this );
  }
  void Verilog::LeftNetChainCB::trap(const Concat* self)
  {
    vector<Expression*>::const_iterator i;
    for( i=self->list().begin();i!=self->list().end();++i )
      (*i)->callback( *this );
  }
  //////////////////
  void Verilog::LeftNetChainCB::trap(const Net* self)
  {
    chain_.insert( self );
  }
  ////////////////////////////////////////////////////////////////////////
  // Verilog::RightNetChainCB
  ////////////////////////////////////
  void Verilog::RightNetChainCB::trap(const Process* self)
  {
    self->statement()->callback( *this );
  }
  //////////////////
  void Verilog::RightNetChainCB::trap(const Block* self)
  {
    vector<Statement*>::const_iterator i;
    for( i=self->list().begin();i!=self->list().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Case* self)
  {
    self->expression()->callback( *this );
    vector<Case::Item*>::const_iterator i;
    for( i=self->items().begin();i!=self->items().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Case::Item* self)
  {
    vector<Expression*>::const_iterator i;
    for( i=self->expression().begin();i!=self->expression().end();++i )
      (*i)->callback( *this );
    self->statement()->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Condition* self)
  {
    self->expression()->callback( *this );
    self->trueStatement()->callback( *this );
    if( self->falseStatement()!=NULL )
      self->falseStatement()->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const EventStatement* self)
  {
    self->statement()->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Assign* self)
  {
    left_=true;
    self->leftValue()->callback( *this );
    left_=false;
    self->rightValue()->callback( *this );
  }  
  //////////////////
  void Verilog::RightNetChainCB::trap(const Identifier* self)
  {
    if( left_ )
      {
	if( self->msb()!=NULL )
	  self->msb()->callback( *this );
	if( self->lsb()!=NULL )
	  self->lsb()->callback( *this );
	if( self->idx()!=NULL )
	  self->idx()->callback( *this );
      }
    else
      {
	if( self->net()!=NULL )
	  self->net()->callback( *this );

	{
	  if( self->msb()!=NULL )
	    self->msb()->callback( *this );
	  if( self->lsb()!=NULL )
	    self->lsb()->callback( *this );
	  if( self->idx()!=NULL )
	    self->idx()->callback( *this );
	}
      }
  }
  void Verilog::RightNetChainCB::trap(const Concat* self)
  {
    vector<Expression*>::const_iterator i;
    for( i=self->list().begin();i!=self->list().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Binary* self)
  {
    self->left()->callback( *this );
    self->right()->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const CallFunction* self)
  {
    vector<Expression*>::const_iterator i;
    for( i=self->parameter().begin();i!=self->parameter().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Ternary* self)
  {
    self->condition()->callback( *this );
    self->trueValue()->callback( *this );
    self->falseValue()->callback( *this );
  }
  void Verilog::RightNetChainCB::trap(const Unary* self)
  {
    self->value()->callback( *this );
  }
  //////////////////
  void Verilog::RightNetChainCB::trap(const Net* self)
  {
    chain_.insert( self );
  }
  ////////////////////////////////////////////////////////////////////////
  void Verilog::EventNetChainCB::trap(const Process* self)
  {
    self->statement()->callback( *this );
  }
  void Verilog::EventNetChainCB::trap(const EventStatement* self)
  {
    vector<Event*>::const_iterator i;
    for( i=self->event().begin();i!=self->event().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::EventNetChainCB::trap(const Event* self)
  {
    self->expression()->callback( *this );
  }  
  void Verilog::EventNetChainCB::trap(const Identifier* self)
  {
    self->net()->callback( *this );
  }
  void Verilog::EventNetChainCB::trap(const Net* self)
  {
    chain_.insert( self );
  }

  ////////////////////////////////////////////////////////////////////////
  // Verilog::NetChainCB
  ////////////////////////////////////
  void Verilog::NetChainCB::trap(const Process* self)
  {
    self->statement()->callback( *this );
  }
  //////////////////
  void Verilog::NetChainCB::trap(const Block* self)
  {
    vector<Statement*>::const_iterator i;
    for( i=self->list().begin();i!=self->list().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Case* self)
  {
    left_ =false;
    self->expression()->callback( *this );

    vector<Case::Item*>::const_iterator i;
    for( i=self->items().begin();i!=self->items().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Case::Item* self)
  {
    left_ =false;
    vector<Expression*>::const_iterator i;
    for( i=self->expression().begin();i!=self->expression().end();++i )
      (*i)->callback( *this );

    self->statement()->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Condition* self)
  {
    left_ =false;
    self->expression()->callback( *this );

    self->trueStatement()->callback( *this );
    if( self->falseStatement()!=NULL )
      self->falseStatement()->callback( *this );
  }
  void Verilog::NetChainCB::trap(const EventStatement* self)
  {
    self->statement()->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Assign* self)
  {
    if( self->type()==Verilog::Assign::BLOCKING )
      blocking_ =true;
    else
      blocking_ =false;
    //
    left_=true;
    self->leftValue()->callback( *this );

    left_=false;
    self->rightValue()->callback( *this );

  }  
  //////////////////
  void Verilog::NetChainCB::trap(const Identifier* self)
  {
    if( left_ )
      {
	left_ =false;
	if( self->msb()!=NULL )
	  self->msb()->callback( *this );
	if( self->lsb()!=NULL )
	  self->lsb()->callback( *this );
	if( self->idx()!=NULL )
	  self->idx()->callback( *this );

	left_ =true;
	if( self->net()!=NULL )
	  self->net()->callback( *this );
      }
    else
      {
	if( self->net()!=NULL )
	  self->net()->callback( *this );

	{
	  if( self->msb()!=NULL )
	    self->msb()->callback( *this );
	  if( self->lsb()!=NULL )
	    self->lsb()->callback( *this );
	  if( self->idx()!=NULL )
	    self->idx()->callback( *this );
	}
      }
  }
  void Verilog::NetChainCB::trap(const Concat* self)
  {
    vector<Expression*>::const_iterator i;
    for( i=self->list().begin();i!=self->list().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Binary* self)
  {
    self->left()->callback( *this );
    self->right()->callback( *this );
  }
  void Verilog::NetChainCB::trap(const CallFunction* self)
  {
    vector<Expression*>::const_iterator i;
    for( i=self->parameter().begin();i!=self->parameter().end();++i )
      (*i)->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Ternary* self)
  {
    self->condition()->callback( *this );
    self->trueValue()->callback( *this );
    self->falseValue()->callback( *this );
  }
  void Verilog::NetChainCB::trap(const Unary* self)
  {
    self->value()->callback( *this );
  }
  //////////////////
  void Verilog::NetChainCB::trap(const Net* self)
  {
    if( blocking_ )
      if( left_ )
	bLeftChain_.insert( self );
      else
	bRightChain_.insert( self );
    else
      if( left_ )
	nbLeftChain_.insert( self );
      else
	nbRightChain_.insert( self );
  }
  ////////////////////////////////////////////////////////////////////////

}





