//==========================================================================
//  SCENARIO.CC - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <assert.h>
#include <algorithm>
#include <sstream>
#include "scenario.h"
#include "valueiterator.h"
#include "cexception.h"
#include "stringutil.h"
#include "unitconversion.h"


/**
 * Resolves variables ($x, $y) and functions (sin, fabs, etc) in expressions.
 */
class Resolver : public Expression::Resolver
{
  private:
    Scenario *hostobject;
  public:
    Resolver(Scenario *object) {hostobject = object;}
    virtual ~Resolver() {};
    virtual Expression::Functor *resolveVariable(const char *varname);
    virtual Expression::Functor *resolveFunction(const char *funcname, int argcount);
};

Expression::Functor *Resolver::resolveVariable(const char *varname)
{
    return new Scenario::IterationVariable(hostobject, varname);
}

Expression::Functor *Resolver::resolveFunction(const char *funcname, int argcount)
{
    //FIXME check argcount!
    if (MathFunction::supports(funcname))
        return new MathFunction(funcname);
    else
        throw opp_runtime_error("Unrecognized function: %s()", funcname);
}

//----

Scenario::Scenario(const std::vector<IterationSpec>& iterationSpecs, const char *conditionText) :
iterspecs(iterationSpecs)
{
    // store the condition
    condition = NULL;
    if (conditionText)
    {
        condition = new Expression();
        Resolver resolver(this);
        try
        {
            condition->parse(conditionText, &resolver);
        }
        catch (std::exception& e)
        {
            throw cRuntimeError("Cannot parse condition expression `%s': %s", conditionText, e.what());
        }
    }

    // fill the variable tables
    itervars.resize(iterspecs.size());
    for (int i=0; i<iterspecs.size(); i++)
    {
        if (!iterspecs[i].value.empty())
            itervars[i].parse(iterspecs[i].value.c_str());
        if (!iterspecs[i].varname.empty() && !iterspecs[i].value.empty())
            namedvars[iterspecs[i].varname] = &itervars[i];
    }
}

Scenario::~Scenario()
{
}

Expression::StkValue Scenario::getIterationVariable(const char *varname)
{
    std::string value = getVar(varname);
    try
    {
        if (value[0]=='"')
            return opp_parsequotedstr(value.c_str());  // strips quotes
        else if (strcmp(value.c_str(), "true")==0)
            return true;
        else if (strcmp(value.c_str(), "false")==0)
            return false;
        else
            return UnitConversion::parseQuantity(value.c_str()); // converts to double
    }
    catch (std::exception& e)
    {
        throw cRuntimeError("Wrong value for iteration variable %s: %s", varname, e.what());
    }
}

int Scenario::getNumRuns()
{
    if (!restart())
        return 0;
    int count = 1;
    while (next())
        count++;  //XXX set some maximum!
    return count;
}

bool Scenario::restart()
{
    if (itervars.size()==0)
        return true;  // it is valid to have no iterations at all

    // reset all iterators. If all of them are immediately at end(), there's no
    // valid state and we must return false
    bool ok = false;
    for (int i=0; i<itervars.size(); i++)
    {
        itervars[i].restart();
        if (!itervars[i].end())
            ok = true;
    }
    if (ok==false)
        return false;

    // if there's a condition, make sure it holds
    while (condition && evaluateCondition()==false)
        if (!inc()) return false;
    return true;
}

bool Scenario::next()
{
    if (!inc()) return false;

    // if there's a condition, make sure it holds
    while (condition && evaluateCondition()==false)
        if (!inc()) return false;
    return true;
}

bool Scenario::inc()
{
    // try incrementing the last iteration variable first
    for (int k=itervars.size()-1; k>=0; k--)
    {
        itervars[k]++;
        if (!itervars[k].end())
            return true; // if incrementing was OK, we're done
        else
            itervars[k].restart(); // reset this counter, and go on incrementing the (k-1)th one
    }
    return false; // no variable could be incremented
}

bool Scenario::evaluateCondition()
{
    try
    {
        return condition->boolValue();
    }
    catch (std::exception& e)
    {
        throw cRuntimeError("Cannot evaluate condition expression: %s", e.what());
    }
}

std::vector<std::string> Scenario::generate(int runNumber)
{
    // spin the iteration variables to the given run number
    if (!restart())
        throw cRuntimeError("Iterators or condition too restrictive: not even one run can be generated");
    for (int i=0; i<runNumber; i++)
        if (!next())
            throw cRuntimeError("Run number %d is out of range", runNumber);

    // then collect and return the variables
    return get();
}

std::string Scenario::getVar(const char *varname) const
{
    std::map<std::string,ValueIterator*>::const_iterator it = namedvars.find(varname);
    if (it==namedvars.end())
        throw cRuntimeError("Unknown iteration variable: %s", varname);
    return it->second->get();
}

std::vector<std::string> Scenario::get() const
{
    std::vector<std::string> result(iterspecs.size());
    for (int i=0; i<iterspecs.size(); i++)
        result[i] = !iterspecs[i].value.empty() ? itervars[i].get() : !iterspecs[i].varname.empty() ? getVar(iterspecs[i].varname.c_str()) : "";
    return result;
}

std::string Scenario::str() const
{
    std::stringstream out;
    for (int i=0; i<itervars.size(); i++)
    {
        if (!itervars[i].get().empty()) {
            out << (i>0 ? ", " : "");
            if (iterspecs[i].varname.empty())
                out << "$" << i;
            else
                out << "$" << iterspecs[i].varname;
            out << "=" << itervars[i].get();
        }
    }
    return out.str();
}



