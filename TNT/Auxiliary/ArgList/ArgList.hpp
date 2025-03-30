///////////////////////////////////////////////////////////////////////////
//
//  ARGLIST.HPP
//
//  Each of the expand functions below will allocate and return an array
//  of arg_entry structures and the number allocated in NArgs.  
//
///////////////////////////////////////////////////////////////////////////
#ifndef ARGLIST_HPP
#define ARGLIST_HPP

#include "x_plus.hpp"

///////////////////////////////////////////////////////////////////////////

#define ARG_NULL        0
#define ARG_STRING      1
#define ARG_NUMBER      2
#define ARG_SWITCH      3

///////////////////////////////////////////////////////////////////////////

struct arg_entry
{
    s32  Type;             
    f32  Number;            // float version of a number
    char Str[X_MAX_PATH];   // string version of param or switch
};

///////////////////////////////////////////////////////////////////////////

arg_entry* ARG_ExpandCommandLine( s32 argc, char** argv,    s32& NArgs );
arg_entry* ARG_ExpandString     ( char* Str,                s32& NArgs );
arg_entry* ARG_ExpandFile       ( char* FileName,           s32& NArgs );

///////////////////////////////////////////////////////////////////////////
/*
class arg_list
{
public:
            enum type
            {
                ARG_NULL,
                ARG_STRING,
                ARG_NUMBER,
                ARG_SWITCH
            };

public:

        arg_list                ( void );
        ~arg_list               ();

void    Clear                   ( void );
void    ExpandCommandLine       ( s32 argc, char** argv );
void    ExpandString            ( char* Str );
void    ExpandFile              ( char* FileName );

s32     GetNumArgs              ( void );
type    GetType                 ( s32 ArgIndex );
f32     GetFloat                ( s32 ArgIndex );
s32     GetInt                  ( s32 ArgIndex );
char*   GetString               ( s32 ArgIndex );



private:

    s32         m_NArgsAllocated;
    s32         m_NArgs;
    arg_entry*  m_pArg;
};
*/
///////////////////////////////////////////////////////////////////////////


#endif