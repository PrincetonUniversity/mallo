package org.im4.mallo.midi;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import java.awt.BorderLayout;

import javax.swing.border.TitledBorder;

import java.awt.Color;

import javax.swing.border.EtchedBorder;
import javax.swing.border.BevelBorder;
import javax.swing.JLabel;
import javax.xml.soap.Text;

import java.awt.Font;
import javax.swing.SwingConstants;
import java.awt.Component;

public class MalloReceiver implements MalloExtendedListener {

	private JFrame frame;
	private JPanel panelTime;
	private JLabel lblTime;
	private JPanel panel_1;
	private JLabel lblPort;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MalloReceiver window = new MalloReceiver();
					window.frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the application.
	 */
	public MalloReceiver() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		frame = new JFrame();
		frame.setBounds(100, 100, 450, 300);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.getContentPane().setLayout(new BorderLayout(0, 0));

		JPanel panel = new JPanel();
		panel.setBorder(new TitledBorder(null, "Message View",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		frame.getContentPane().add(panel, BorderLayout.CENTER);
		panel.setLayout(new BorderLayout(0, 0));

		panelTime = new JPanel();
		panel.add(panelTime, BorderLayout.NORTH);
		panelTime.setLayout(new BorderLayout(0, 0));

		lblTime = new JLabel("TIME");
		lblTime.setHorizontalAlignment(SwingConstants.LEFT);
		lblTime.setFont(new Font("Lucida Grande", Font.PLAIN, 21));
		panelTime.add(lblTime);

		panel_1 = new JPanel();
		panel.add(panel_1, BorderLayout.SOUTH);
		String s = "1919";
		s = (String) JOptionPane.showInputDialog(frame,
				"Port to be monitored\n", "Port?", JOptionPane.PLAIN_MESSAGE,
				null, null, "1919");

		// If a string was returned, say so.
		if ((s != null) && (s.length() > 0)) {
		}

		lblPort = new JLabel("port = " + s);
		panel_1.add(lblPort);

		MalloMidiView panelMessages = new MalloMidiView(Integer.parseInt(s));
		panelMessages.setBorder(new BevelBorder(BevelBorder.LOWERED, null,
				null, null, null));
		panelMessages.setBackground(Color.GRAY);
		panel.add(panelMessages);
		panelMessages.setListener(this);
	}

	@Override
	public void gotLatencies(double oneway, double roundtrip, double upper) {
		// TODO Auto-generated method stub

	}

	@Override
	public void gotPrediction(double tPred, double t) {
		// TODO Auto-generated method stub

	}

	@Override
	public void gotSample(double t, double z) {
		// TODO Auto-generated method stub

	}

	@Override
	public void gotClockSync(double t, double tdif) {

	}

	String timeText = "";
	int tfollow = 0;

	@Override
	public void updateTime(double t) {
		timeText = timeText + String.format("%.3f ", t);
		if (timeText.length() > 20) {
			timeText = String.format("%.3f ", t);
		}
		lblTime.setText(timeText);
		if (tfollow < t) {
			MalloHitSound.playSound(1);
			tfollow = (int)t + 1;
		}
	}

	public void gotHit(double t, int x) {
		// TODO Auto-generated method stub
		
	}

}
