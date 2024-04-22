/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
//
// Description: This file contains global variables used in OpenSees files.
// if you change some of the variables, you must recompile ALL the code.
//
// Written: fmk
// Created: 11/99
//
#ifndef _OPS_Globals_h
#define _OPS_Globals_h

#define _USING_OpenSees_STREAMS
#include <OPS_Stream.h>
extern OPS_Stream *opserrPtr;
#define opserr (*opserrPtr)
#define endln "\n"

#include <string.h>
#define OPS_STATIC

#define TCL_Char const char

class Domain;
class Element;

extern double   ops_Dt;                // current delta T for current domain doing an update
extern int ops_Creep;
extern Domain  *ops_TheActiveDomain;   // current domain undergoing an update
extern Element *ops_TheActiveElement;  // current element undergoing an update

// global variable for initial state analysis
// added: Chris McGann, University of Washington
extern bool  ops_InitialStateAnalysis;

#define OPS_PRINT_CURRENTSTATE 0
#define OPS_PRINT_PRINTMODEL_SECTION  1
#define OPS_PRINT_PRINTMODEL_MATERIAL 2
#define OPS_PRINT_PRINTMODEL_JSON   25000

#define OPS_PRINT_JSON_ELEM_INDENT "      "
#define OPS_PRINT_JSON_NODE_INDENT "      "

#endif
