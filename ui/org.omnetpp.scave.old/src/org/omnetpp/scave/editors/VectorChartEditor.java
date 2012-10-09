/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.scave.editors;


import org.omnetpp.scave.model.IDatasetStrategy;
import org.omnetpp.scave.model.VectorDatasetStrategy;


/**
 * Editor for scalar datasets.
 */
public class VectorChartEditor extends DatasetEditor {

    private static IDatasetStrategy strategy = new VectorDatasetStrategy();

    public VectorChartEditor() {
        super(strategy,
            new IDatasetEditorPage[] {
                new FilterPanelPage(strategy),
                new ChartPage(strategy)
            });
    }
}
