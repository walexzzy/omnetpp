package org.omnetpp.experimental.seqchart.editors;


import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.HashMap;

import org.eclipse.draw2d.Figure;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.swt.graphics.Color;
import org.omnetpp.scave.engine.EventEntry;
import org.omnetpp.scave.engine.EventLog;
import org.omnetpp.scave.engine.MessageEntries;
import org.omnetpp.scave.engine.MessageEntry;

/**
 * This is a sequence chart as a single figure.
 *
 * @author andras
 */
//TODO implement "hand" to grab and move the chart
//TODO make events and message clickable (tooltip, go there in the log, etc)
//TODO draw "deliver" messages in a different color
//TODO draw arrowheads
//FIXME msg arrows that intersect the chart area but don't start or end there are not displayed (BUG!)
//FIXME performance: create a C++ interface where we can prevent instantiating a huge number of EventEntry/MessageEntry objects
//FIXME cache lines for the drawing (we need this to make the chart clickable as well)
public class SeqChartFigure extends Figure {
	
	protected double pixelsPerSec = 2;
	protected int tickScale = 2; // -1 means step=0.1
	protected EventLog eventLog;

	public double getPixelsPerSec() {
		return pixelsPerSec;	
	}
	
	/**
	 * Also adjusts tickScale which controls density of ticks
	 */
	public void setPixelsPerSec(double pps) {
		if (pps<=0)
			pps = 1e-12;
		pixelsPerSec = pps;
		int labelWidthNeeded = 50; // pixels
		tickScale = (int)Math.ceil(Math.log10(labelWidthNeeded/pps));

		System.out.println("pixels per sec: "+pixelsPerSec);
        
		recalculatePreferredSize();
		repaint();
	}

	/**
	 * Increases pixels per second. 
	 */
	public void zoomIn() {
		setPixelsPerSec(getPixelsPerSec() * 1.5);	
	}

	/**
	 * Decreases pixels per second. 
	 */
	public void zoomOut() {
		setPixelsPerSec(getPixelsPerSec() / 1.5);	
	}
	
	public EventLog getEventLog() {
		return eventLog;
	}

	public void setEventLog(EventLog eventLog) {
		this.eventLog = eventLog;
		recalculatePreferredSize();
		repaint();
	}

	private void recalculatePreferredSize() {
		EventEntry lastEvent = eventLog.getEvent(eventLog.getNumEvents()-1);
		int width = lastEvent==null ? 0 : (int)(lastEvent.getSimulationTime()*getPixelsPerSec());
		width = Math.max(width, 600); // at least half a screen
		int height = eventLog.getNumModules()*50;
		setPreferredSize(width, height);
	}

	/**
	 * Overridden org.eclipse.draw2d.Figure method.
	 */
	@Override
	protected void paintFigure(Graphics graphics) {
		if (eventLog!=null) {
			long startMillis = System.currentTimeMillis();
			
			// paint axes
			HashMap<Integer,Integer> moduleIdToAxisMap = new HashMap<Integer, Integer>();
			for (int i=0; i<eventLog.getNumModules(); i++) {
				moduleIdToAxisMap.put(eventLog.getModule(i).getModuleId(), i);
				drawLinearAxis(graphics, bounds.y+30+i*50, eventLog.getModule(i).getModuleFullName());
			}

			// paint events
			Rectangle rect = graphics.getClip(new Rectangle());
			double tleft = pixelToTime(rect.x);
			double tright = pixelToTime(rect.right());
			EventEntry startEvent = eventLog.getFirstEventAfter(tleft);
			EventEntry endEvent = eventLog.getFirstEventAfter(tright);
			int startEventIndex = (startEvent!=null) ? eventLog.findEvent(startEvent) : 0;
			int endEventIndex = (endEvent!=null) ? eventLog.findEvent(endEvent) : eventLog.getNumEvents(); 
			int startEventNumber = (startEvent!=null) ? startEvent.getEventNumber() : 0;
            
            graphics.setForegroundColor(new Color(null,255,0,0));
            graphics.setBackgroundColor(new Color(null,170,0,0));
            for (int i=startEventIndex; i<endEventIndex; i++) {
            	EventEntry event = eventLog.getEvent(i);
    			Point p = getEventCoords(event, moduleIdToAxisMap);
            	graphics.fillOval(p.x-2, p.y-2, 5, 5);
            	
            	// paint forward arrows for this event
            	MessageEntries consequences = event.getConsequences();
            	for (int j=0; j<consequences.size(); j++)
        			drawMessageArrow(consequences.get(j), moduleIdToAxisMap, graphics);
            	// paint backward arrows that wouldn't be painted otherwise
            	MessageEntries causes = event.getCauses();
            	for (int j=0; j<causes.size(); j++)
            		if (causes.get(j).getSource().getEventNumber() < startEventNumber)
            			drawMessageArrow(causes.get(j), moduleIdToAxisMap, graphics);
            }
			System.out.println("repaint(): "+(System.currentTimeMillis()-startMillis)+"ms");
			startMillis = System.currentTimeMillis();
			System.gc();
			System.out.println("     gc(): "+(System.currentTimeMillis()-startMillis)+"ms");
		}
	}

	private void drawMessageArrow(MessageEntry msg, HashMap<Integer, Integer> moduleIdToAxisMap, Graphics graphics) {
		Point p1 = getEventCoords(msg.getSource(), moduleIdToAxisMap);
		Point p2 = getEventCoords(msg.getTarget(), moduleIdToAxisMap);
		if (p1.y==p2.y) {
			drawArc(graphics, p1, p2);
		} else {
			graphics.drawLine(p1, p2);
		}
	}

	private void drawArc(Graphics graphics, Point p1, Point p2) {
		Rectangle.SINGLETON.setLocation(p1.x, p1.y-10);
		Rectangle.SINGLETON.setSize(p2.x-p1.x, 20);
		graphics.drawArc(Rectangle.SINGLETON, 0, 180);
	}

	private Point getEventCoords(EventEntry event, HashMap<Integer,Integer> moduleIdToAxisMap) {
		int x = timeToPixel(event.getSimulationTime());
		int axis = moduleIdToAxisMap.get(event.getModule().getModuleId());
		int y = bounds.y+30+axis*50;
		return new Point(x,y);
	}

	/**
	 * Draws the axis, according to the current pixelsPerSec and tickInterval
	 * settings.
	 */
	private void drawLinearAxis(Graphics graphics, int y, String label) {
		Rectangle rect = graphics.getClip(new Rectangle());
		Rectangle bounds = getBounds();
		rect.intersect(bounds); // although looks like Clip is already set up like this

		// draw axis label; may it should be "sticky" on the screen?
		graphics.drawText(label, bounds.x, y-20); //XXX refine

		// draw axis
		graphics.drawLine(rect.x, y, rect.right(), y);

		double tleft = pixelToTime(rect.x);
		double tright = pixelToTime(rect.right());

		// draw ticks and labels
		BigDecimal tickStart = new BigDecimal(tleft).setScale(-tickScale, RoundingMode.FLOOR);
		BigDecimal tickEnd = new BigDecimal(tright).setScale(-tickScale, RoundingMode.CEILING);
		BigDecimal tickIntvl = new BigDecimal(1).scaleByPowerOfTen(tickScale);
		//System.out.println(tickStart+" - "+tickEnd+ " step "+tickIntvl);

		for (BigDecimal t=tickStart; t.compareTo(tickEnd)<0; t = t.add(tickIntvl)) {
			int x = timeToPixel(t.doubleValue());
			graphics.drawLine(x, y-2, x, y+2);
			graphics.drawText(t.toPlainString()+"s", x, y+3);
		}
	}

	/**
	 * Translates from pixel x coordinate to seconds, using on pixelsPerSec.
	 */
	private double pixelToTime(int x) {
		return (x-getBounds().x) / pixelsPerSec;
	}

	/**
	 * Translates from seconds to pixel x coordinate, using on pixelsPerSec.
	 */
	private int timeToPixel(double t) {
		return (int)Math.round(t * pixelsPerSec) + getBounds().x;
	}

	/**
	 * Utility function, copied from org.eclipse.draw2d.Polyline.
	 */
	private boolean lineContainsPoint(int x1, int y1, int x2, int y2, int px, int py, int tolerance) {
		Rectangle.SINGLETON.setSize(0, 0);
		Rectangle.SINGLETON.setLocation(x1, y1);
		Rectangle.SINGLETON.union(x2, y2);
		Rectangle.SINGLETON.expand(tolerance, tolerance);
		if (!Rectangle.SINGLETON.contains(px, py))
			return false;

		int v1x, v1y, v2x, v2y;
		int numerator, denominator;
		int result = 0;

		// calculates the length squared of the cross product of two vectors, v1 & v2.
		if (x1 != x2 && y1 != y2) {
			v1x = x2 - x1;
			v1y = y2 - y1;
			v2x = px - x1;
			v2y = py - y1;

			numerator = v2x * v1y - v1x * v2y;

			denominator = v1x * v1x + v1y * v1y;

			result = (int)((long)numerator * numerator / denominator);
		}

		// if it is the same point, and it passes the bounding box test,
		// the result is always true.
		return result <= tolerance * tolerance;
	}
}
