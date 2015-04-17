package org.im4.mallo.midi;

public interface MalloExtendedListener {

	void gotLatencies(double oneway, double roundtrip, double upper);
	void gotPrediction(double tPred, double t);
	void gotSample(double t, double z);
	void gotHit(double t, int x);
	void gotClockSync(double t, double tdif);
	void updateTime(double t);
}
