//=========================================================================
//
//  ERRMSG.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//   Contents:
//    emsg[]:  error message table
//
//  Author: Andras Varga
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/


#include "errmsg.h"


const char *cErrorMessages::get(ErrorCode e)
{
    switch (e)
    {
      case eOK:        return "No error -- everything's fine";
      case eBACKSCHED: return "Cannot schedule message (%s)%s to the past, t=%lg";
      case eBADCAST:   return "Cannot cast from type %s to %s";
      case eBADEXP:    return "Badly formed Reverse Polish expression";
      case eBADINIT:   return "setValue(): Type %s does not suit arg types";
      case eCANCEL:    return "Simulation cancelled";
      case eCANTCOPY:  return "Assignment, copying, dup() not supported by this class";
      case eCANTDUP:   return "This object cannot dup() itself";
      case eCANTPACK:  return "netPack()/netUnpack() not supported for this object";
      case eCUSTOM:    return "%s";
      case eENDEDOK:   return "No more events -- simulation ended";
      case eENDSIM:    return "Simulation stopped with endSimulation()";
      case eFINISH:    return "All finish() functions called, simulation ended";
      case eINFLOOP:   return "FSM: infinite loop of transient states (now in state %s)";
      case eLOCKED:    return "Object is locked against modifications, try unlockProperties() on parent object";
      case eMODFIN:    return "Message sent to already terminated module `%s'";
      case eMODINI:    return "Module initialization error";
      case eNEGTIME:   return "wait(): negative delay";
      case eNEGTOUT:   return "receive()/receiveNew(): negative timeout";
      case eNONET:     return "Network definition not found";
      case eNOPARSIM:  return "Simulation kernel was compiled without parallel simulation support (WITH_PARSIM=no)";
      case eNORECV:    return "Cannot use receive..() or wait() with handleMessage()";
      case eNUMARGS:   return "Bad number of arguments, %d expected";
      case eREALTIME:  return "CPU time limit reached -- simulation stopped";
      case eSIMTIME:   return "Simulation time limit reached -- simulation stopped";
      case eSTATECHG:  return "FSM: state changed during state entry code (now in state %s)";
      case eSTOPSIMRCVD: return "Simulation stopped by partition %d: %s";
      case eUSER:      return "User error: %s";
      default:         return "<missing error text>";
    }
}
