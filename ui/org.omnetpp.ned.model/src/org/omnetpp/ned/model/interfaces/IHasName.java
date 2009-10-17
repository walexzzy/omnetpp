/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.
  
  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.ned.model.interfaces;

import org.omnetpp.ned.model.INEDElement;

/**
 * NED elements that have a name property
 *
 * @author rhornig
 */
public interface IHasName extends INEDElement {

	static String DEFAULT_TYPE_NAME = "Unnamed";

	/**
	 * Returns name attribute
	 */
	public String getName();

	/**
	 * Sets name attribute
	 */
	public void setName(String name);

}