package org.im4.mallo.ctrl;

import java.awt.EventQueue;

import com.illposed.osc.OSCPortIn;
import com.illposed.osc.OSCPortOut;

public class MalloMonitor {
	
	MalloWindow mainView;
	
	OSCPortIn voxReciever;
	OSCPortOut voxCommander;
	
	public MalloMonitor() {
		mainView = new MalloWindow();
		mainView.setVisible(true);
	}
	
	public static void main(String[] args) {
		
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MalloMonitor controller = new MalloMonitor();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}
	
	

}
