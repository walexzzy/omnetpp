package com.simulcraft.test.gui.core;

import org.eclipse.swt.SWT;
import org.omnetpp.common.util.ReflectionUtils;

public class PlatformUtils {
    public final static boolean isGtk = "gtk".equals(SWT.getPlatform());
    public final static boolean isWindows = "win32".equals(SWT.getPlatform());
    public final static boolean isCarbon = "carbon".equals(SWT.getPlatform());
    
    public final static Class<?> OS = classForName("org.eclipse.swt.internal." + SWT.getPlatform() + ".OS"); 
    
    public static boolean hasPendingUIEvents() {
        // some method like this is supposed to be part of SWT's Display class
        if (isWindows) {
            // return OS.PeekMessage(new MSG(), 0, 0, 0, OS.PM_NOREMOVE);
            return (Boolean)ReflectionUtils.invokeStaticMethod(OS, "PeekMessage", ReflectionUtils.newInstance("org.eclipse.swt.internal.win32.MSG"), 0, 0, 0, 0x0 /*OS.PM_NOREMOVE*/);
        }
        else if (isGtk) {
            // return OS._g_main_context_pending(0);
            return (Boolean)ReflectionUtils.invokeStaticMethod(OS, "_g_main_context_pending", 0);
        }
        else {
        	throw new RuntimeException("unsupported window system: " + SWT.getPlatform());
        }
    }
    
    private static Class<?> classForName(String className) {
        try {
            return Class.forName(className);
        }
        catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }
    
	public static int getModifiersFor(char ch) {
	    // see VkKeyScan documentation in the Windows API, and usage in Display.post()
        short vkKeyScan = OS.VkKeyScan ((short) wcsToMbcs (ch, 0)); //XXX platform specific (Windows only)
        byte vkModifier = (byte)(vkKeyScan >> 8);
        int modifier = 0;
        if ((vkModifier & 1) !=0) modifier |= SWT.SHIFT;
        if ((vkModifier & 2) !=0) modifier |= SWT.CONTROL;
        if ((vkModifier & 4) !=0) modifier |= SWT.ALT;
        return modifier;
	}
	
	private static int wcsToMbcs (char ch, int codePage) {
	    // from Display (win32)
	    if (OS.IsUnicode) return ch;
	    if (ch <= 0x7F) return ch;
	    TCHAR buffer = new TCHAR (codePage, ch, false);
	    return buffer.tcharAt (0);
	}

//GTK:
//	int keyCode = 0;
//	int /*long*/ keysym = untranslateKey (event.keyCode);
//	if (keysym != 0) keyCode = OS.XKeysymToKeycode (xDisplay, keysym);
//	if (keyCode == 0) {
//		char key = event.character;
//		switch (key) {
//			case SWT.BS: keysym = OS.GDK_BackSpace; break;
//			case SWT.CR: keysym = OS.GDK_Return; break;
//			case SWT.DEL: keysym = OS.GDK_Delete; break;
//			case SWT.ESC: keysym = OS.GDK_Escape; break;
//			case SWT.TAB: keysym = OS.GDK_Tab; break;
//			case SWT.LF: keysym = OS.GDK_Linefeed; break;
//			default:
//				keysym = wcsToMbcs (key);
//		}
//		keyCode = OS.XKeysymToKeycode (xDisplay, keysym);
//		if (keyCode == 0) return false;
//	}
//	OS.XTestFakeKeyEvent (xDisplay, keyCode, type == SWT.KeyDown, 0);
//	return true;

//	public static int getModifiersFor(char ch) {
//		// looks like it roughly goes like this:
//		// 1. determine the keysim to be produced (see in post(Event))
//		// 2. determine keycode from keysim using XKeysymToKeycode -- this is the "main" key only
//		// 3. look up what keysims that keycode can generate, using XKeycodeToKeysym(display, keycode, index),
//		//    and determine which index generates the desired keysim
//		// 4. then decode index as follows:
//		//      =0  None of Shift and Mode_switch are pressed
//		//      =1  Shift is pressed
//		//      =2  Mode_switch is pressed
//		//      =3  Both Shift and Mode_switch are pressed
//		// see http://homepage3.nifty.com/tsato/xvkbd/events.html,
//		// http://tronche.com/gui/x/xlib/utilities/keyboard/XKeycodeToKeysym.html,
//		// http://tronche.com/gui/x/xlib/input/keyboard-encoding.html#KeySym, etc.
//		// 
//		//TODO
//		return 0;
//	}
    

}
