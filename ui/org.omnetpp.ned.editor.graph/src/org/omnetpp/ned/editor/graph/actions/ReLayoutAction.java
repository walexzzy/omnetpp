package org.omnetpp.ned.editor.graph.actions;

import java.util.List;

import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.GroupRequest;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.ui.IWorkbenchPart;
import org.omnetpp.common.image.ImageFactory;
import org.omnetpp.ned.editor.graph.commands.ChangeLayoutSeedCommand;
import org.omnetpp.ned.editor.graph.edit.CompoundModuleEditPart;
import org.omnetpp.ned.model.ex.CompoundModuleNodeEx;

public class ReLayoutAction extends org.eclipse.gef.ui.actions.SelectionAction {

	public static final String ID = "Layout";
	public static final String MENUNAME = "Layout";
	public static final String TOOLTIP = "Creates a new compound module layout";
	public static final ImageDescriptor IMAGE = ImageFactory.getDescriptor(ImageFactory.TOOLBAR_IMAGE_LAYOUT);

	GroupRequest request;

	public ReLayoutAction(IWorkbenchPart part) {
		super(part);
		setText(MENUNAME);
		setId(ID);
		setToolTipText(TOOLTIP);
		setImageDescriptor(IMAGE);
		setHoverImageDescriptor(IMAGE);
	}

	/**
	 * This command can be executed ONLY on submodules who has fixed location
	 * @return
	 */
	protected boolean calculateEnabled() {
		// only support a single selected object
		if (getSelectedObjects().size() != 1)
			return false;
		if (getSelectedObjects().get(0) instanceof CompoundModuleEditPart)
			return true;
		return false;
	}

	/**
	 * get a command for the request from the containing compound module
	 * @return
	 */
	private Command getCommand() {
		List selEditParts = getSelectedObjects();
		
		if (selEditParts.size() != 1 || !(selEditParts.get(0) instanceof CompoundModuleEditPart))
			return null;
		
		// get the parent of the currently selected 
		CompoundModuleNodeEx compoundModuleNodeEx 
			= (CompoundModuleNodeEx)((CompoundModuleEditPart)selEditParts.get(0)).getModel();
		// create command that changes the compound modules layout seed
		return new ChangeLayoutSeedCommand(compoundModuleNodeEx); 
	}

	public void run() {
		execute(getCommand());
	}

}
