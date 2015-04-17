package org.im4.mallo.ctrl;

import java.awt.Color;
import java.awt.Graphics;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import javax.swing.JPanel;

import com.illposed.osc.OSCPortIn;

public class MalloTracker extends JPanel {

	ArrayList<Double> time = new ArrayList<Double>();
	ArrayList<Double> h = new ArrayList<Double>();

	ArrayList<Double> prediction = new ArrayList<Double>();
	ArrayList<Double> predictionTime = new ArrayList<Double>();
	ArrayList<Double> hitTimes = new ArrayList<Double>();

	double trackTime = 0;
	double trackBackTime = 5;

	@Override
	public void paint(Graphics g) {
		super.paint(g);
		g.setColor(Color.BLUE);
		if (time.size() > 1) {
			// trackTime = time.get(time.size() - 1) + trackBackTime * 0.25;
			if (trackTime < time.get(time.size() - 1) + trackBackTime * 0.2) {
				trackTime = trackTime + trackBackTime * 0.5;
			}
			for (int a = prediction.size() - 1; a >= 0; --a) {
				int x1 = (int) ((1 - (trackTime - prediction.get(a))
						/ trackBackTime) * getWidth());
				int x2 = (int) ((1 - (trackTime - predictionTime.get(a))
						/ trackBackTime) * getWidth());
				g.setColor(Color.LIGHT_GRAY);
				g.fillRect(x2, 0, x1-x2, getHeight());
				g.setColor(Color.MAGENTA);
				g.drawLine(x1, 0, x1, getHeight());
				if (x1 < 10) {
					break;
				}
			}
			g.setColor(Color.BLUE);
			for (int a = time.size() - 1; a >= 0; --a) {
				int x = (int) ((1 - (trackTime - time.get(a)) / trackBackTime) * getWidth());
				int y = (int) ((1 - h.get(a)) * getHeight());
				g.drawRect(x, y, 1, 1);
				if (x < 10) {
					break;
				}
			}
			g.setColor(Color.RED);
			for (int a = hitTimes.size() - 1; a >= 0; --a) {
				int x = (int) ((1 - (trackTime - hitTimes.get(a)) / trackBackTime) * getWidth());
				g.drawLine(x, 0, x, getHeight());
				if (x < 10) {
					break;
				}
			}

		}
	}

	public void input(double t, double z) {
		time.add(t);
		h.add(z);
	}

	public void predict(double tPred, double t) {
		prediction.add(t);
		predictionTime.add(tPred);
	}
	
	public void inputHit(double t) {
		hitTimes.add(t);
	}
	
	public void saveResult(String fn) {
		try {
			FileWriter writer = new FileWriter(new File(fn));
			for (int a = 0; a < hitTimes.size(); ++a) {
				writer.write(" " + hitTimes.get(a));
			}
			writer.write("\n");
			for (int a = 0; a < prediction.size(); ++a) {
				writer.write(" " + prediction.get(a));
			}
			writer.write("\n");
			for (int a = 0; a < predictionTime.size(); ++a) {
				writer.write(" " + predictionTime.get(a));
			}
			writer.write("\n");
			for (int a = 0; a < time.size(); ++a) {
				writer.write(" " + time.get(a));
			}
			writer.write("\n");
			for (int a = 0; a < h.size(); ++a) {
				writer.write(" " + h.get(a));
			}
			writer.write("\n");
			writer.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

}
