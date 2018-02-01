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

#ifndef _EMVER_HH
#define _EMVER_HH

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <cassert>
#include <cstdio>

namespace moe
{
  ////////////////////////////////////////////////////////////////////////
  template <uint32_t T>
  class UIntN
  {
    uint32_t* _v;
  public:
    uint32_t* v() const { return _v; }
    uint32_t  n() const { return T; }
    
    virtual ~UIntN(){}
    UIntN():
      _v(NULL)
    {}
    UIntN(uint32_t* a):
      _v(a)
    {}
    UIntN(uint32_t& a)
    {
      _v =&a;
    }
    UIntN(uint64_t& a)
    {
      _v =(uint32_t*)&a;
    }
    UIntN(UIntN& a)
    {
      _v =a.v();
    }
    
    UIntN& operator =(const UIntN& a)
    {
      memcpy(_v,a.v(),((T+31)/32)*4);
    }
    
    friend std::ostream& operator << (std::ostream& ostr,const UIntN& a)
    {
      int i;
      for( i=a.n()-1;i>=0;i-- )
	ostr << ((a._v[i/32]>>(i%32))&1);
      return ostr;
    }
  };
   /*** 
  template <uint32_t T,uint32_t A,uint32_t B>
  UIntN<T> operator +(const UIntN<A>& a,const UIntN<B>& b)
  {
    uint32_t tv[(T+31)/32];
    uint64_t t=0,tw=0;
    int i;
    for( i=0;i<(tw+31)/32;i++ )
      tv[i] =t =
	uint64_t( ((i*32)<aw)?av[i]:0 )+
	uint64_t( ((i*32)<bw)?bv[i]:0 )+(t>>32);
    if( (tw%32)!=0 )
      tv[(tw-1)/32] &=(~(0xffffffff<<(tw%32)));
    
    return UIntN<T>(tv);
  }



  friend UIntN operator -(const UIntN& a,const UIntN& b)
    {
      UIntN r;
      uint64_t t=0;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	r._v[i] =t =uint64_t(a._v[i])-uint64_t(b._v[i])-((t>>32)&1);
      return r;
    }
    friend UIntN operator &(const UIntN& a,const UIntN& b)
    {
      UIntN r;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	r._v[i] =a._v[i]&b._v[i];
      return r;
    }
    friend UIntN operator |(const UIntN& a,const UIntN& b)
    {
      UIntN r;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	r._v[i] =a._v[i]|b._v[i];
      return r;
    }
    friend UIntN operator ^(const UIntN& a,const UIntN& b)
    {
      UIntN r;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	r._v[i] =a._v[i]^b._v[i];
      return r;
    }
    friend UIntN operator ~(const UIntN& a)
    {
      UIntN r;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	r._v[i] =~a._v[i];
      return r;
    }
    friend UIntN operator >>(const UIntN& a,const uint32_t b)
    {
      UIntN r;
      uint32_t c=b>>5;
      uint32_t d=b&31;
      uint32_t e=32-d;
      unsigned int i;
      if( d==0 )
	for( i=0;i<(T+31)/32;i++ )
	  r._v[i]  =((i+c+0) < T) ? a._v[i+c] : 0;
      else
	for( i=0;i<(T+31)/32;i++ )
	  {
	    r._v[i]  =((i+c+0) < T) ? (a._v[i+c]>>d) : 0;
	    r._v[i] |=((i+c+1) < T) ? (a._v[i+c+1]<<e) : 0;
	  }
      return r;
    }
    friend UIntN operator <<(const UIntN& a,const uint32_t b)
    {
      UIntN r;
      uint32_t c=b>>5;
      uint32_t d=b&31;
      uint32_t e=32-d;
      int i;
      if( d==0 )
	for( i=0;i<(T+31)/32;i++ )
	  r._v[i]  =((i-c-0) >=0) ? a._v[i-c-0] : 0;
      else    
	for( i=0;i<(T+31)/32;i++ )
	  {
	    r._v[i]  =((i-c-0) >=0) ? (a._v[i-c-0]<<d) : 0;
	    r._v[i] |=((i-c-1) >=0) ? (a._v[i-c-1]>>e) : 0;
	  }
      return r;
    }
    friend bool operator ==(const UIntN& a,const UIntN& b)
    {
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	if( a._v[i]!=b._v[i] )
	  return false;
      return true;
    }
    friend bool operator !=(const UIntN& a,const UIntN& b)
    {
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	if( a._v[i]!=b._v[i] )
	  return true;
      return false;
    }
    friend bool operator <(const UIntN& a,const UIntN& b)
    {
      int i;
      for( i=((T+31)/32)-1;i>=0;i-- )
	if( a._v[i]<b._v[i] )
	  return true;
	else  if( a._v[i]>b._v[i] )
	  return false;
      return false;
    }
    friend bool operator >(const UIntN& a,const UIntN& b)
    {
      int i;
      for( i=((T+31)/32)-1;i>=0;i-- )
	if( a._v[i]>b._v[i] )
	  return true;
	else  if( a._v[i]<b._v[i] )
	  return false;
      return false;
    }
    friend bool operator <=(const UIntN& a,const UIntN& b)
    {
      int i;
      for( i=((T+31)/32)-1;i>=0;i-- )
	if( a._v[i]<b._v[i] )
	  return true;
	else  if( a._v[i]>b._v[i] )
	  return false;
      return false;
    }
    friend bool operator >=(const UIntN& a,const UIntN& b)
    {
      int i;
      for( i=((T+31)/32)-1;i>=0;i-- )
	if( a._v[i]>b._v[i] )
	  return true;
	else  if( a._v[i]<b._v[i] )
	  return false;
      return false;
    }


    friend bool case_equality(const UIntN& a,const UIntN& b,const UIntN& m)
    {
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	if( (a._v[i]&m._v[i])!=(b._v[i]&m._v[i]) )
	  return false;
      return true;
    }
    friend bool case_inequality(const UIntN& a,const UIntN& b,const UIntN& m)
    {
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	if( (a._v[i]&m._v[i])!=(b._v[i]&m._v[i]) )
	  return true;
      return false;
    }
    friend bool reduction_and(const UIntN& a)
    {
      uint32_t t=0xFFFFFFFF;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	t &=a.v[i];
      return (t==0xFFFFFFFF);
    }
    friend bool reduction_nand(const UIntN& a)
    {
      uint32_t t=0xFFFFFFFF;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	t &=a.v[i];
      return !(t==0xFFFFFFFF);
    }
    friend bool reduction_or(const UIntN& a)
    {
      uint32_t t=0;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	t |=a.v[i];
      return (t!=0);
    }
    friend bool reduction_nor(const UIntN& a)
    {
      uint32_t t=0;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	t |=a.v[i];
      return !(t!=0);
    }
    friend bool reduction_xor(const UIntN& a)
    {
      uint32_t t=0;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	t ^=a.v[i];
      t ^=(t>>16);
      t ^=(t>>8);
      t ^=(t>>4);
      t ^=(t>>2);
      t ^=(t>>1);    
      return (t&1);
    }
    friend bool reduction_nxor(const UIntN& a)
    {
      uint32_t t=0;
      unsigned int i;
      for( i=0;i<(T+31)/32;i++ )
	t ^=a.v[i];
      t ^=(t>>16);
      t ^=(t>>8);
      t ^=(t>>4);
      t ^=(t>>2);
      t ^=(t>>1);    
      return !(t&1);
    }
    bool index(uint32_t i)
    {
      uint32_t c=i>>5;
      uint32_t d=i&31;
      return (_v[c]>>d)&1;
    }
    friend UIntN Mask(unsigned int w)
    {
      UIntN r;
      unsigned int i;
      for( i=0;i<(w+31)/32;i++ )
        r._v[i] =0xFFFFFFFF;
      r._v[i-1] =~(0xFFFFFFFF<<(w&31));
      return r;
    }
    friend UIntN Mask(unsigned int msb,unsigned int lsb)
    {
      UIntN r;
      unsigned int i;

      return r;
    }


    UIntN operator ()(uint32_t m,uint32_t l) const //read
    {
    }
    _UIntN operator ()(uint32_t m,uint32_t l) //write
    {
    }
    UIntN part(uint32_t m,uint32_t l)
    {
      UIntN r;
      uint32_t lc =l>>5;
      uint32_t lb =l&31;
      uint32_t ld =32-lb;
      uint32_t mc =(m-l+1+31)>>5;
      uint32_t mm =31-(m-l+1+31)&31;
      
      unsigned int i;
      if( lb==0 )
	for( i=0;i<mc;i++ )
	  r._v[i]  =((i+lc+0) < T) ? _v[i+lc] : 0;
      else
	for( i=0;i<mc;i++ )
	  {
	    r._v[i]  =((i+lc+0) < T) ? (_v[i+lc+0]>>lb) : 0;
	    r._v[i] |=((i+lc+1) < T) ? (_v[i+lc+1]<<ld) : 0;
	  }
      r._v[mc-1] &=(0xFFFFFFFFUL>>mm);
      for( ;i<T;i++ )
	r._v[i] =0;
      return r;
    }
  };
  ***/  
  
  ////////////////////////////////////////////////////////////////////////
  // helper functions
  class EmVer
  {
  public:
    EmVer(){}
    virtual ~EmVer(){}
    ////////////////////////////////////
    inline uint32_t* Bucket(uint32_t* tv,uint32_t tw,
			    uint32_t r)
    {
      tv[0] =r;
      int i;
      for( i=1;i<(tw+31)/32;i++ )
	tv[i] =0;
      return tv;
    }
    inline uint32_t* Bucket(uint32_t* tv,uint32_t tw,
			    uint64_t r)
    {
      tv[0] =r;
      tv[1] =(r>>32);
      int i;
      for( i=2;i<(tw+31)/32;i++ )
	tv[i] =0;
      return tv;
    }
    inline uint32_t* Bucket(uint32_t* tv,uint32_t tw,
			    uint32_t* rv,uint32_t rw)
    {
      int i;
      for( i=0;i<(tw+31)/32;i++ )
	tv[i] =(i<((rw+31)/32)) ? rv[i] : 0;
      return tv;
    }
    ////////////////////////////////////
    inline void Assign(uint32_t* av,uint32_t msb,uint32_t lsb,
		       uint32_t* bv,uint32_t bw)
    {
      int ai =lsb/32;
      int bi =0;
      if( (lsb%32)!=0 )
	av[ai] =(av[ai]&(~(0xffffffff<<(lsb%32))))|(bv[bi]<<(lsb%32));
      ai++;
      for( ;ai<(msb+1)/32;ai++,bi++ )
	{
	  av[ai]  =((bi-0) < ((bw+31)/32)) ? (bv[bi-0]<<(lsb%32)) : 0;
	  av[ai] |=((bi-1) < ((bw+31)/32)) ? (bv[bi-1]>>(32-(lsb%32))) : 0;
	}
      if( ((msb+1)%32)!=0 )
	{
	  av[ai] =(av[ai]&(0xffffffff<<((msb+1)%32))) |
	    ((~(0xffffffff<<((msb+1)%32)))&
	     (
	      (((bi-0) < ((bw+31)/32)) ? (bv[bi-0]<<(lsb%32)) : 0)|
	      (((bi-1) < ((bw+31)/32)) ? (bv[bi-1]>>(32-(lsb%32))) : 0)
	      ));
	}
	}
    inline void Assign(uint32_t* av,uint32_t aw,
		       uint32_t* bv,uint32_t bw)
    {
      int i;
      for( i=0;i<((aw+31)/32);i++ )
	av[i] =(i<((bw+31)/32)) ? bv[i] : 0;

      if( (aw%32)!=0 )
      av[(aw-1)/32] &=(~(0xffffffff<<(aw%32)));
    }
    inline void Assign(uint64_t& a,uint64_t mask,uint32_t lsb,uint64_t b)
    {
      a =(a&(~mask))|((b<<lsb)&mask);
    }
    inline void Assign(uint32_t& a,uint32_t mask,uint32_t lsb,uint32_t b)
    {
      a =(a&(~mask))|((b<<lsb)&mask);
    }
    inline void Assign(uint64_t& a,uint64_t mask,uint64_t b)
    {
      a =b&mask;
    }
    inline void Assign(uint32_t& a,uint32_t mask,uint32_t b)
    {
      a =b&mask;
    }
    inline bool Index(uint32_t a,int i)
    {
      return ((a>>i)&1);
    }
    inline uint32_t Part(uint32_t a,int l,uint32_t m)
    {
      return ((a>>l)&m);
    }
    inline uint32_t Concat(uint32_t l,uint32_t r,int rw)
    {
      return ((l<<rw)|r);
    }
    inline uint32_t Repeat(uint32_t repeat,uint32_t concat,int concat_width)
    {
      uint32_t r =concat;
      int i;
      for( i=1;i<repeat;i++ )
	r =(r<<concat_width)|r;
      return r;
    }
    ////////////////////////////////////////////////////////////////////////
    // unary expression
    inline bool ReductionAND(uint32_t r,uint32_t m)
    {
      return ((r&m)==m);
    }
    inline bool ReductionOR(uint32_t r,uint32_t m)
    {
      return ((r&m)!=0);
    }
    inline bool ReductionXOR(uint32_t r,uint32_t m)
    {
      r &=m;
      r ^=(r>>16);
      r ^=(r>>8);
      r ^=(r>>4);
      r ^=(r>>2);
      r ^=(r>>1);    
      return (r&1);
    }
    inline bool ReductionNAND(uint32_t r,uint32_t m)
    {
      return !((r&m)==m);
    }
    inline bool ReductionNOR(uint32_t r,uint32_t m)
    {
      return !((r&m)==0);
    }
    inline bool ReductionNXOR(uint32_t r,uint32_t m)
    {
      r &=m;
      r ^=(r>>16);
      r ^=(r>>8);
      r ^=(r>>4);
      r ^=(r>>2);
      r ^=(r>>1);    
      return !(r&1);
    }
    inline uint32_t SignExt(uint32_t a,uint32_t b)
    {
      if( (a&b)!=0 )
	return a|b;
      else
	return a;
    }
    inline uint64_t SignExt(uint64_t a,uint64_t b)
    {
      if( (a&b)!=0 )
	return a|b;
      else
	return a;
    }
    inline uint64_t SignExt(uint32_t a,uint64_t b)
    {
      if( (uint64_t(a)&b)!=0 )
	return uint64_t(a)|b;
      else
	return uint64_t(a);
    }

    inline uint32_t SignExt(uint32_t& a,long unsigned int b)
    {
      if( (a&b)!=0 )
	return a|b;
      else
	return a;
    }
    inline uint64_t SignExt(uint64_t& a,long long unsigned int b)
    {
      if( (a&b)!=0 )
	return a|b;
      else
	return a;
    }
    inline uint64_t SignExt(uint32_t& a,long long unsigned int b)
    {
      if( (uint64_t(a)&b)!=0 )
	return uint64_t(a)|b;
      else
	return uint64_t(a);
    }
      
    ////////////////////////////////////////////////////////////////////////
    // binary expression
    inline uint32_t ArithmeticAdd(uint32_t a,uint32_t b)
    {
      return (a+b);
    }
    inline uint32_t ArithmeticMinus(uint32_t a,uint32_t b)
    {
      return (a+b);
    }
    inline uint32_t ArithmeticMultiply(uint32_t a,uint32_t b)
    {
      return (a+b);
    }
    inline uint32_t ArithmeticDivide(uint32_t a,uint32_t b)
    {
      return (a+b);
    }
    inline uint32_t ArithmeticModulus(uint32_t a,uint32_t b)
    {
      return (a+b);
    }
    inline uint32_t BitwiseXOR(uint32_t a,uint32_t b)
    {
      return (a^b);
    }

    inline bool LogicalEquality(uint32_t a,uint32_t b)
    {
      return (a==b);
    }
    inline bool CaseEquality(uint32_t a,uint32_t b,uint32_t m)
    {
      return ((a&m)==(b&m));
    }
    inline bool CaseInequality(uint32_t a,uint32_t b,uint32_t m)
    {
      return ((a&m)!=(b&m));
    }
    ////////////////////////////////////
    inline bool Index(uint64_t a,int i)
    {
      return ((a>>i)&1);
    }
    /*
    inline bool Index(const _UIntN& a,int i)
    {
      return ((*(a.v()+(i>>5))>>(i&31))&1);
    }
    */
    ////////////////////////////////////
    inline uint64_t Part(uint64_t a,int l,uint64_t m)
    {
      return ((a>>l)&m);
    }
    ////////////////////////////////////
    inline uint64_t Concat(uint64_t l,uint64_t r,int rw)
    {
      return ((l<<rw)|r);
    }
    ////////////////////////////////////
    inline uint64_t Concat(uint32_t repeat,uint64_t concat,int concat_width)
    {
      uint64_t r =concat;
      int i;
      for( i=1;i<repeat;i++ )
	r =(r<<concat_width)|r;
      return r;
    }
    ////////////////////////////////////
    inline uint64_t ArithmeticAdd(uint64_t a,uint64_t b)
    {
      return (a+b);
    }
    ////////////////////////////////////
    void vcd_dump(std::FILE* fp,uint32_t d,int w)
    {
      fputc('b',fp);
      int i;
      for( i=w-1;i>=0;i-- )
	if( ((d>>i)&1) )
	  fputc('1',fp);
	else
	  fputc('0',fp);
    }
    void vcd_dump(std::FILE* fp,uint64_t d,int w)
    {
      fputc('b',fp);
      int i;
      for( i=w-1;i>=0;i-- )
	if( ((d>>i)&1) )
	  fputc('1',fp);
	else
	  fputc('0',fp);
    }
    

  };


  inline int32_t SignExt(uint32_t a,uint32_t b)
  {
    if( (a&b)!=0 )
      return a|b;
    else
      return a;
  }
  inline int64_t SignExt(uint64_t a,uint64_t b)
  {
    if( (a&b)!=0 )
      return a|b;
    else
      return a;
  }

}

#endif








