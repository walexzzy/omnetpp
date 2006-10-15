//==========================================================================
//   CBOOLPAR.H  - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __CBOOLPAR_H
#define __CBOOLPAR_H

#include "cpar.h"

/**
 * FIXME revise docu in the whole class!!!!!!
 *
 * @ingroup SimCore
 */
class SIM_API cBoolPar : public cPar
{
  protected:
    // selector: flags & FL_ISEXPR
    union {
      cExpression *expr;
      bool val;
    };

  protected:
    bool evaluate() const;
    void deleteOld();

  public:
    /** @name Constructors, destructor, assignment. */
    //@{

    /**
     * Constructor.
     */
    explicit cBoolPar();

    /**
     * Copy constructor.
     */
    cBoolPar(const cBoolPar& other) {operator=(other);}

    /**
     * Destructor.
     */
    virtual ~cBoolPar();

    /**
     * Assignment operator.
     */
    cBoolPar& operator=(const cBoolPar& otherpar);
    //@}

    /** @name Redefined cPolymorphic functions */
    //@{

    /**
     * Creates and returns an exact copy of this object.
     */
    virtual cBoolPar *dup() const  {return new cBoolPar(*this);}

    /**
     * Produces a one-line description of object contents.
     */
    virtual std::string info() const;

    /**
     * Serializes the object into a buffer.
     */
    virtual void netPack(cCommBuffer *buffer);

    /**
     * Deserializes the object from a buffer.
     */
    virtual void netUnpack(cCommBuffer *buffer);
    //@}

    /** @name Redefined cPar setter functions. */
    //@{

    /**
     * Sets the value to the given constant.
     */
    virtual cBoolPar& setBoolValue(bool b);

    /**
     * Raises an error: cannot convert long to bool.
     */
    virtual cBoolPar& setLongValue(long l);

    /**
     * Raises an error: cannot convert double to bool.
     */
    virtual cBoolPar& setDoubleValue(double d);

    /**
     * Raises an error: cannot convert string to bool.
     */
    virtual cBoolPar& setStringValue(const char *s);

    /**
     * Raises an error: cannot convert XML to bool.
     */
    virtual cBoolPar& setXMLValue(cXMLElement *node);

    /**
     * Sets the value to the given expression. This object will
     * assume the responsibility to delete the expression object.
     */
    virtual cBoolPar& setExpression(cExpression *e);
    //@}

    /** @name Redefined cPar getter functions. */
    //@{

    /**
     * Returns the value of the parameter.
     */
    virtual bool boolValue() const;

    /**
     * Raises an error: cannot convert bool to long.
     */
    virtual long longValue() const;

    /**
     * Raises an error: cannot convert bool to double.
     */
    virtual double doubleValue() const;

    /**
     * Raises an error: cannot convert bool to string.
     */
    virtual std::string stringValue() const;

    /**
     * Raises an error: cannot convert bool to XML.
     */
    virtual cXMLElement *xmlValue() const;

    /**
     * Returns pointer to the expression stored by the object, or NULL.
     */
    virtual cExpression *expression() const;
    //@}

    /** @name Type, prompt text, input flag, change flag. */
    //@{

    /**
     * Returns 'B' (for "bool").
     */
    virtual char type() const;

    /**
     * Returns true.
     */
    virtual bool isNumeric() const;
    //@}

    /** @name Redefined cPar misc functions. */
    //@{

    /**
     * Replaces for non-const values, replaces the stored expression with its
     * evaluation.
     */
    virtual void convertToConst();

    /**
     * Returns the value in text form.
     */
    virtual std::string toString() const;

    /**
     * Converts from text.
     */
    virtual bool parse(const char *text);
    //@}
};

#endif


