//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         texcept.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for Exception class - the field where all polygons lies
//
//----------------------------------------------------------------------------

#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <cstring.h>
#include <stdio.h>

class Exception {
	private:
   	string msg;
      string location;
   public:
   	Exception(const char* file, const int line, const char* message) {
      	char buf[20];
      	sprintf(buf,"%d",line);
      	location = string(file) + " line " + buf;
         msg = message;
      }
      const char* message() { return msg.c_str(); }
      const char* where() { return location.c_str(); }
};

#define Throw(x) throw Exception(__FILE__,__LINE__,x)

#endif


