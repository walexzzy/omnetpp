//==========================================================================
//  ERRMSG.H - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Error code constants
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __ERRMSG_H
#define __ERRMSG_H

#include "defs.h"

/**
 * Error codes
 */
enum ErrorCode
{
    eOK = 0,  // eOK must have zero numeric value
    eBACKSCHED,
    eBADCAST,
    eBADEXP,
    eBADINIT,
    eCANCEL,
    eCANTCOPY,
    eCANTDUP,
    eCANTPACK,
    eCUSTOM,
    eENDEDOK,
    eENDSIM,
    eESTKOFLOW,
    eESTKUFLOW,
    eEBADARGS,
    eECANTCAST,
    eENOPARENT,
    eFINISH,
    eINFLOOP,
    eLOCKED,
    eMODFIN,
    eMODINI,
    eNEGTIME,
    eNEGTOUT,
    eNOCHANDEF,
    eNONET,
    eNOPARSIM,
    eNOPARENTMOD,
    eNORECV,
    eNOSUCHNET,
    eNUMARGS,
    eREALTIME,
    eSIMTIME,
    eSTATECHG,
    eSTKLOW,
    eSTOPSIMRCVD,
    eUSER
};

/**
 * Provides error messages for error codes.
 *
 * @ingroup Internals
 */
class SIM_API cErrorMessages
{
  public:
    cErrorMessages() {}
    static const char *get(ErrorCode e);
};

#endif
