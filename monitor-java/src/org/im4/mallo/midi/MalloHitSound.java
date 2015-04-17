package org.im4.mallo.midi;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;

public class MalloHitSound {

	static Clip clip1;
	static Clip clip2;
	static Clip clip3;
	static boolean inited = false;

	public static synchronized void playSound(int x) {
		if (inited == false) {
			new Thread(new Runnable() {
				// The wrapper thread is unnecessary, unless it blocks on the
				// Clip finishing; see comments.
				public void run() {
					try {
						clip1 = AudioSystem.getClip();
						InputStream strema = new FileInputStream(new File(
								"./chunky-snare.wav"));
						AudioInputStream inputStream = AudioSystem
								.getAudioInputStream(strema);
						clip1.open(inputStream);

						clip2 = AudioSystem.getClip();
						InputStream strema2 = new FileInputStream(new File(
								"./tom.wav"));
						AudioInputStream inputStream2 = AudioSystem
								.getAudioInputStream(strema2);
						clip2.open(inputStream2);

						clip3 = AudioSystem.getClip();
						InputStream strema3 = new FileInputStream(new File(
								"./hat.wav"));
						AudioInputStream inputStream3 = AudioSystem
								.getAudioInputStream(strema3);
						clip3.open(inputStream3);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}).start();
			inited = true;
		} else {
			if (x == 0) {
				clip3.stop();
				clip3.stop();
				System.out.println("play 3");
				clip3.start();
			} else if (x > 0) {
				clip1.stop();
				clip2.stop();
				System.out.println("play 1");
				clip1.start();
			} else {
				clip1.stop();
				clip2.stop();
				System.out.println("play 2");
				clip2.start();
			}
		}
	}

	public static void main(String[] test) {
		MalloHitSound.playSound(1);
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
			return;
		}
		for (int a = 0; a < 5; ++a) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
				return;
			}
			MalloHitSound.playSound(-1);
		}
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
			return;
		}
	}
}
