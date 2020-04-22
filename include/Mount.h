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
	void stop_track();
	void move_axis ( Axis passed_axis, int passed_speed );
	void set_pier_side( PierSide passed_pier_side);
	void sync_mount(double passed_target_AR , double passed_target_DEC);
	
	//getter functions
	bool get_tracking() const;

	//setter functions
	
private:
	double right_ascension { 0 };
	double declination { 90 };
	
	bool abort { false }; 
	bool force_flip {false};
	bool tracking ; 
	
	PierSide pier_side { PierSide::east };
	PierSide old_pier_side { PierSide::east };
	mutex AR_mux;
	mutex DEC_mux;
	
	
};


#endif