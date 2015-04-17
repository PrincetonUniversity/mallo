package org.im4.mallo.ctrl;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;

import java.awt.BorderLayout;

import javax.swing.JApplet;
import javax.swing.JTextField;
import javax.swing.JLabel;
import javax.swing.JToggleButton;
import javax.swing.JButton;

import java.awt.Component;
import java.awt.FileDialog;
import java.awt.FlowLayout;
import java.awt.LayoutManager;

import javax.swing.JScrollPane;

import java.awt.Dimension;

import javax.swing.JCheckBox;
import javax.swing.border.TitledBorder;
import javax.swing.JTextArea;

import org.im4.mallo.midi.MalloExtendedListener;
import org.im4.mallo.midi.MalloHitSound;
import org.im4.mallo.midi.MalloMidiView;

import java.awt.Color;
import java.awt.Font;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.File;

public class MalloWindow implements MalloExtendedListener {

	private JFrame frame;
	private JPanel mainPanel;
	private JTextField textField;
	private JTextField textField_1;
	private JTextField textField_2;
	private JTextField textField_3;

	private MalloStatusBar statusBar;
	private JPanel panel_4;
	private MalloMidiView panelMidiMonitor;
	private MalloTracker panelMallet;

	int port = 2121;

	public void setVisible(boolean visible) {
		this.frame.setVisible(visible);
	}

	/**
	 * Create the application.
	 */
	public MalloWindow() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		frame = new JFrame();
		frame.setBounds(100, 100, 682, 628);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		mainPanel = new JPanel();
		frame.getContentPane().add(mainPanel, BorderLayout.CENTER);
		mainPanel.setLayout(new BorderLayout(0, 0));

		JPanel panel = new JPanel();
		mainPanel.add(panel, BorderLayout.SOUTH);

		JLabel lblChat = new JLabel("Chat");
		panel.add(lblChat);

		textField_3 = new JTextField();
		panel.add(textField_3);
		textField_3.setColumns(24);

		JLabel lblControllerPort = new JLabel("Controller Port");
		panel.add(lblControllerPort);

		textField = new JTextField();
		textField.setText("7079");
		panel.add(textField);
		textField.setColumns(10);

		JPanel panel_1 = new JPanel();
		FlowLayout flowLayout = (FlowLayout) panel_1.getLayout();
		flowLayout.setAlignment(FlowLayout.LEFT);
		mainPanel.add(panel_1, BorderLayout.NORTH);

		JLabel lblRemoteIp = new JLabel("Remote IP");
		panel_1.add(lblRemoteIp);

		textField_1 = new JTextField();
		panel_1.add(textField_1);
		textField_1.setColumns(10);

		JLabel lblPort = new JLabel("Port");
		panel_1.add(lblPort);

		textField_2 = new JTextField();
		panel_1.add(textField_2);
		textField_2.setColumns(5);

		JButton btnJoin = new JButton("Join");
		panel_1.add(btnJoin);

		JLabel lblNotConnected = new JLabel("Not Connected");
		panel_1.add(lblNotConnected);

		JScrollPane scrollPane = new JScrollPane();
		scrollPane.setPreferredSize(new Dimension(180, 4));
		mainPanel.add(scrollPane, BorderLayout.WEST);

		JCheckBox chckbxChatOnly = new JCheckBox("Chat only");
		scrollPane.setColumnHeaderView(chckbxChatOnly);

		JTextArea textArea = new JTextArea();
		textArea.setFont(new Font("Lucida Grande", Font.PLAIN, 10));
		textArea.setWrapStyleWord(true);
		scrollPane.setViewportView(textArea);

		JPanel panel_2 = new JPanel();
		mainPanel.add(panel_2, BorderLayout.CENTER);
		panel_2.setLayout(new BorderLayout(0, 0));

		JPanel panel_3 = new JPanel();
		panel_3.setBorder(new TitledBorder(null, "Mallet Actions",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		panel_3.setPreferredSize(new Dimension(10, 120));
		panel_2.add(panel_3, BorderLayout.CENTER);
		panel_3.setLayout(new BorderLayout(0, 0));

		panelMallet = new MalloTracker();
		FlowLayout flowLayout_1 = (FlowLayout) panelMallet.getLayout();
		flowLayout_1.setAlignment(FlowLayout.LEFT);
		panelMallet.setBackground(Color.WHITE);
		panel_3.add(panelMallet, BorderLayout.CENTER);

		JButton btnSave = new JButton("save");
		btnSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				FileDialog dialog = new FileDialog(frame,
						"Save tracking result", FileDialog.SAVE);
				dialog.setVisible(true);
				try {
					String fn = dialog.getDirectory() + File.separator
							+ dialog.getFile();
					panelMallet.saveResult(fn);
				} catch (Exception e) {
					e.printStackTrace();
				}

			}
		});
		panelMallet.add(btnSave);

		JPanel statusContainer = new JPanel();
		statusContainer.setLayout(new BorderLayout());
		statusBar = new MalloStatusBar();
		FlowLayout flowLayout_2 = (FlowLayout) statusBar.getLayout();
		flowLayout_2.setAlignment(FlowLayout.RIGHT);
		statusBar.setBackground(new Color(50, 50, 20));
		statusBar.setPreferredSize(new Dimension(10, 180));
		statusContainer.setBorder(new TitledBorder(null, "Status",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		panel_2.add(statusContainer, BorderLayout.NORTH);
		statusContainer.add(statusBar);
		
		JButton btnSave_1 = new JButton("save");
		btnSave_1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				FileDialog dialog = new FileDialog(frame,
						"Save tracking result", FileDialog.SAVE);
				dialog.setVisible(true);
				try {
					String fn = dialog.getDirectory() + File.separator
							+ dialog.getFile();
					statusBar.saveResult(fn);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		statusBar.add(btnSave_1);

		panel_4 = new JPanel();
		panel_4.setBorder(new TitledBorder(null, "MIDI Monitor",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		panel_4.setPreferredSize(new Dimension(10, 160));
		panel_2.add(panel_4, BorderLayout.SOUTH);
		panel_4.setLayout(new BorderLayout(0, 0));

		panelMidiMonitor = new MalloMidiView(port);
		panelMidiMonitor.setBackground(new Color(47, 79, 79));
		panel_4.add(panelMidiMonitor, BorderLayout.CENTER);
		panelMidiMonitor.setListener(this);

	}

	@Override
	public void gotLatencies(double oneway, double roundtrip,
			double latencybound) {
		statusBar.addLatencyTick(oneway, roundtrip, latencybound);
	}

	@Override
	public void gotPrediction(double tPred, double t) {
		panelMallet.predict(tPred, t);
	}

	@Override
	public void gotSample(double t, double z) {
		panelMallet.input(t, z);
		panelMallet.repaint();
	}

	@Override
	public void gotClockSync(double t, double tdif) {

	}

	@Override
	public void updateTime(double t) {
		// TODO Auto-generated method stub

	}

	@Override
	public void gotHit(double t, int x) {
		MalloHitSound.playSound(x);
		panelMallet.inputHit(t);
	}

}
