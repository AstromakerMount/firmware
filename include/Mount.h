#ifndef MOUNT_H
#define MOUNT_H

#include <thread>
using std::thread;

#include <mutex>
using std::mutex;

#include "Parameters.h"

enum Axis{
	primary_axis,
	secondary_axis
};

enum PierSide{
	east,
	west
};


class Mount{

public:
	Mount();
	void abort_slew ();
	void slew_axis ( double passed_target_AR , double passed_target_DEC, double passed_sideral_time );
	void startup();
	void start_track();
	void stop_track_AR();
	void stop_track_DEC();
	void jog_axis ( Axis passed_axis, int passed_direction, int passed_speed );
	void sync ( double passed_target_AR_to_sync , double passed_target_DEC_to_sync, double sideral_time_to_sync );
	void home ();
	
	//getter functions
	bool get_tracking_AR() const;
	bool get_tracking_DEC() const;
	double get_right_ascension() const;
	double get_declination() const;
	float get_autoguide_speed () const;
	bool get_force_flip() const;

	//setter functions
	void set_force_flip( bool passed_force_flip);
	void set_right_ascension ( double passed_right_ascension );
	void set_declination ( double passed_declination );
	void set_autoguide_speed ( float passed_autoguide_speed );
	
private:
	double right_ascension { 0 };
	double declination { 90 };
	
	bool abort { false }; 
	bool force_flip {false};
	bool tracking_AR;
	bool tracking_DEC;
	
	float autoguide_speed {0};
	
	PierSide pier_side { PierSide::east };
	PierSide old_pier_side { PierSide::east };
	mutex AR_mux;
	mutex DEC_mux;
	
	
};


#endif
