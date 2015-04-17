package org.im4.mallo.ctrl;

import java.awt.Color;
import java.awt.Graphics;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import javax.swing.JPanel;

public class MalloStatusBar extends JPanel {
	
	ArrayList<Double> onewayLatencies = new ArrayList<Double>();
	ArrayList<Double> roundTripLatencies = new ArrayList<Double>();
	
	int countLatTraceBack = 30;
	
	double latencybound = 0;
	
	class CommuteNode {
		CommuteNode nextHop;
		
	}

	@Override
	public void paint(Graphics g) {
		super.paint(g);
		
		g.setColor(Color.LIGHT_GRAY);
		g.drawLine(48, 58, getSize().width - 70 + 8, 58);
		
		g.setColor(Color.LIGHT_GRAY);
		g.fillOval(40, 50, 15, 15);
		
		g.fillOval(getSize().width - 70, 50, 15, 15);
		
		g.fillOval((int)(getSize().width / 2.5), 30, 20, 20);
		
		g.fillOval((int)(getSize().width / 2.5), 70, 20, 20);
		
		
		g.setColor(Color.WHITE);
		g.drawString("Local Node", 20, 75);
		g.drawString("Remote Node", getSize().width - 100, 75);
		g.drawString("(Oregon)", (int)(getSize().width / 2.5 + 25), 45);
		g.drawString("(LA)", (int)(getSize().width / 2.5 + 25), 85);
		
		g.setColor(Color.CYAN);
		g.drawRect(10, 10, 3, 3);
		g.drawString("one-way latency", 20, 15);
		
		g.setColor(Color.YELLOW);
		g.drawRect(145, 10, 3, 3);
		g.drawString("half RTT", 155, 15);
		
		
		
		
		int heightLat = getHeight() - 80;
		
		int xp = onewayLatencies.size() - countLatTraceBack;
		
		g.setColor(Color.LIGHT_GRAY);
		for (int lat = 0; lat <= 80; lat += 20) {
			int y = getHeight() - heightLat * lat / 120 - 10;
			g.drawLine(50, y, getWidth(), y);
			g.drawString("" + lat + "ms", 1, y);
		}
		
		g.setColor(Color.CYAN);
		for (int j = onewayLatencies.size() - 1; j >= 0; --j) {
			int x1 = (j-xp) * getWidth() / countLatTraceBack;
			int y1 = (int)(onewayLatencies.get(j) * 1000 * heightLat/ 120.0) + 10;
			int y12 = (int)(roundTripLatencies.get(j) * 1000 * heightLat/ 120.0 / 2) + 10; 
			g.setColor(Color.CYAN);
			g.drawRect(x1 - 1, getHeight() - y1 - 1, 3, 3);
			g.setColor(Color.YELLOW);
			g.drawRect(x1 - 1, getHeight() - y12 - 1, 3, 3);
			if (j > 0) {
				g.setColor(Color.CYAN);
				int y2 = (int)(onewayLatencies.get(j-1) * 1000 * heightLat/ 120.0) + 10;
				int x2 = (j-xp-1) * getWidth() / countLatTraceBack; 
				if (x2 < 50) {
					break;
				}
				g.drawLine(x1, getHeight() - y1, x2, getHeight() - y2);
				g.setColor(Color.YELLOW);
				y2 = (int)(roundTripLatencies.get(j-1) * 1000 * heightLat/ 120.0 / 2) + 10;
				g.drawLine(x1, getHeight() - y12, x2, getHeight() - y2);
			}
		}
		
		g.drawString("" + (int)(latencybound * 1000) + "ms", (int)(getSize().width / 1.4), 58);
	}
	
	void addLatencyTick(double owt, double rtt, double latencybound) {
		this.onewayLatencies.add(owt);
		this.roundTripLatencies.add(rtt);
		this.latencybound = latencybound;
		System.out.println("Lat(" + owt + "," + rtt + ")");
		repaint();
	}
	
	public void saveResult(String fn) {
		try {
			FileWriter writer = new FileWriter(new File(fn));
			for (int a = 0; a < onewayLatencies.size(); ++a) {
				writer.write(onewayLatencies.get(a) + " ");
			}
			writer.write("\n");
			for (int a = 0; a < onewayLatencies.size(); ++a) {
				writer.write(roundTripLatencies.get(a) + " ");
			}
			writer.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
}
