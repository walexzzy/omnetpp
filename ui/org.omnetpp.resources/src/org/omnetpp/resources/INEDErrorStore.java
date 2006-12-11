package org.omnetpp.resources;

import org.omnetpp.ned.model.NEDElement;

/**
 * Interface for issuing NED error messages
 * @author andras
 */
public interface INEDErrorStore {
	  public void add(NEDElement context, String message);
}
