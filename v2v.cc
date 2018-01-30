/*
 * Copyright (c) 2002-2003 moe
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

#include "Verilog.hh"


int main(int argc,char* argv[])
{
  if( argc==1 )
    {
      cout << "v2v file-name.(v|v2k)\n";
      exit(1);
    }

  moe::Verilog ver;
  moe::Verilog::Module* mod=NULL;
  string modname;

  std::cerr << "load...\n";
  ver.parse( argv[1] );
  std::cerr << "link module...\n";
  ver.link();
  
  int i;
  for( i=2;i<argc;i++ )
    {
      if( strcmp(argv[i],"ungroup" )==0 )
	{
	  i++;
	  mod =ver.findModule(argv[i]);
	  std::cerr << "ungroup instance...\n";
	  mod->ungroup();	  
	  modname =argv[i];
	}
      else if( strcmp(argv[i],"tpd" )==0 )
	ver.setDecTPD(true);
    }

  if( mod==NULL )
    ver.toVerilog(std::cout);
  else
    mod->toVerilog(std::cout,modname);
}
