package org.im4.mallo.midi;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

import javax.swing.JPanel;
import javax.swing.Timer;

import org.im4.mallo.ctrl.MalloEvent;

import com.illposed.osc.OSCListener;
import com.illposed.osc.OSCMessage;
import com.illposed.osc.OSCPortIn;
import com.illposed.osc.OSCPortOut;

public class MalloMidiView extends JPanel implements ActionListener,
		OSCListener {

	long timeback = 20;

	double baseTime = 0;
	double timeDiff = 0;

	long sampleTime = 0;

	Timer timer = new Timer(5, this);
	HashMap<String, Integer> midimap = new HashMap<String, Integer>();
	String[] namemap = new String[8];

	MalloExtendedListener listener = null;

	OSCPortIn portIn;
	OSCPortOut portOut;

	public MalloMidiView(int port) {
		super();
		timer.start();
		envokeEvent(1, 12, "Welcome");
		envokeEvent(5, 12, "None");
		try {
			portIn = new OSCPortIn(port);
			portIn.addListener("*", this);
			portOut = new OSCPortOut(InetAddress.getLocalHost(), 9191);
		} catch (SocketException e) {
			e.printStackTrace();
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		if (portIn != null) {
			portIn.startListening();
			System.out.println("Port active: " + portIn.isListening());
		}
		for (int a = 0; a < namemap.length; ++a) {
			namemap[a] = "";
		}
		map("connected", 1);
		map("clocksync", 2);
		map("hit", 3);
		map("pred", 4);
		map("latt", 5);
		map("chat", 6);
		map("error", 7);
		sampleTime = System.currentTimeMillis();
	}

	double getTime() {
		return (System.currentTimeMillis() - sampleTime) / 1000.0 + baseTime;
	}

	ArrayList<MalloEvent> events = new ArrayList<MalloEvent>();

	@Override
	public void paint(Graphics g) {
		super.paint(g);
		long tt = System.currentTimeMillis();
		for (int a = events.size() - 1; a >= 0; a--) {
			int x = (int) ((events.get(a).time - (tt - timeback * 1000))
					* getWidth() / (timeback * 1000));
			int y = (int) (getHeight() * ((events.get(a).cmd % 8 + 0.5) / 8));
			if (x < 80) {
				break;
			}
			float degree = events.get(a).arg / 127.0f;
			if (degree > 1)
				degree = 1;
			if (degree < 0)
				degree = 0;
			g.setColor(new Color(degree, 1 - degree, 1 - degree));
			g.fillRect(x - 5, y - getHeight() / 20, 5, getHeight() / 10);
			g.setColor(Color.WHITE);
			g.drawString(events.get(a).label, x, y + 4);
		}
		g.setColor(Color.LIGHT_GRAY);
		for (int a = 0; a < 8; ++a) {
			int y = (int) (getHeight() * ((a % 8 + 0.5) / 8));
			g.drawLine(70, y, getWidth(), y);
			g.drawString(namemap[a], 1, y + 4);
		}
		g.drawLine(70, 0, 70, getHeight());
	}

	public void map(String msg, int row) {
		if (msg.charAt(0) != '/') {
			msg = "/" + msg;
		}
		midimap.put(msg, row);
		namemap[row] = msg;
	}

	public void envokeEvent(int cmd, int code, String label) {
		long tt = System.currentTimeMillis();
		MalloEvent event = new MalloEvent();
		event.time = tt;
		event.arg = code;
		event.cmd = cmd;
		event.label = label;
		events.add(event);
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		repaint();
		if (listener != null) {
			listener.updateTime(getTime());
		}
	}

	@Override
	public void acceptMessage(Date time, OSCMessage message) {
		String address = message.getAddress();
		if (address.equals("/clocksync")) {
			if (listener != null) {
				System.out.println("sync: "
						+ (Double) message.getArguments().get(0));
				double tbase = (Double) message.getArguments().get(0);
				double tdiff = (Double) message.getArguments().get(1);
				this.baseTime = tbase;
				this.timeDiff = tdiff;
				sampleTime = System.currentTimeMillis();
				listener.gotClockSync(tbase, tdiff);
			}
		}
		if (address.equals("/latt")) {
			if (listener != null) {
				// System.out.println("lat: " + (Double)
				// message.getArguments().get(0));
				listener.gotLatencies((Double) message.getArguments().get(0),
						(Double) message.getArguments().get(1),
						(Double) message.getArguments().get(2));
			}
		}
		if (address.equals("/smp")) {
			listener.gotSample((Double) message.getArguments().get(0),
					(Double) message.getArguments().get(1));
			return;
		}
		if (address.equals("/pred")) {
			listener.gotPrediction((Double) message.getArguments().get(0),
					(Double) message.getArguments().get(1));
			return;
		}
		if (address.equals("/hit")) {
			System.out.println("HIT: " + (Integer) message.getArguments().get(0));
			listener.gotHit(0, (Integer) message.getArguments().get(0));
			return;
		}
		Integer r = midimap.get(address);
		if (r != null) {
			if (message.getArguments().get(0) instanceof Double) {
				envokeEvent(r,
						(int) ((Double) message.getArguments().get(0))
								.doubleValue(), "");
			} else {
				envokeEvent(r, (Integer) message.getArguments().get(0), "");
			}
		} else {
			System.out.println("unkonwn: " + address);
		}
	}

	public void setListener(MalloExtendedListener listener) {
		this.listener = listener;
	}
}
