#define DEBUG

#include <wiringPi.h>
#include <chrono>
#include <mutex>
using std::unique_lock;

#include "Server.h"
#include "Parameters.h"

void test( int pin){
	digitalWrite(pin, HIGH);
	delay (200);
	digitalWrite(pin, LOW);
	delay (200);
	digitalWrite(pin, HIGH);
	delay (200);
	digitalWrite(pin, LOW);
	delay (800);
}

Mount::Mount(){ //default contructor

	wiringPiSetup(); //sets up GPIO pins

	pinMode ( AR_step_pin , OUTPUT ) ;
	pinMode ( AR_dir_pin , OUTPUT ) ;
	pinMode ( AR_enable_pin , OUTPUT ) ;
	pinMode ( AR_axis_error_pin , INPUT ) ;
	pinMode ( AR_error_enable_pin , OUTPUT ) ;

	pinMode ( DEC_step_pin , OUTPUT ) ;
	pinMode ( DEC_dir_pin , OUTPUT ) ;
	pinMode ( DEC_enable_pin , OUTPUT ) ;
	pinMode ( DEC_axis_error_pin , INPUT ) ;	
	pinMode ( DEC_error_enable_pin , OUTPUT ) ;
	
	#ifdef DEBUG
	test( AR_step_pin );
	test( AR_dir_pin );
	test( AR_enable_pin );
	test( DEC_step_pin );
	test( DEC_dir_pin );
	test( DEC_enable_pin );
	#endif
	
	//enable axis drivers
	digitalWrite( AR_enable_pin, HIGH );
	digitalWrite( DEC_enable_pin, HIGH );
	
}

int map_int ( int in, int in_from, int in_to, int out_from, int out_to ){
	return out_from + ( (in - in_from)*(out_to - out_from) / (in_to - in_from) );
}

int map_double ( double in, double in_from, double in_to, int out_from, int out_to ){
	return out_from + ( (in - in_from)*(out_to - out_from) / (in_to - in_from) );
}

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

void tracking_routine( bool *tracking, mutex *AR_mux ){	
	unique_lock <mutex> AR_lock ( *AR_mux );
	
	#ifdef DEBUG
	cout << "now tracking" << endl;
	#endif
	
	while ( *tracking ){
		std::this_thread::sleep_for ( std::chrono::microseconds ( AR_tracking_step_time ) );
		digitalWrite ( AR_step_pin , HIGH );
		std::this_thread::sleep_for ( std::chrono::microseconds ( AR_tracking_step_time ) );
		digitalWrite ( AR_step_pin , LOW );
	}
	
	#ifdef DEBUG
	cout << "end of tracking" << endl;
	#endif
}

void slew ( Mount *this_mount, Axis axis , int steps , bool direction, bool *abort, double *axis_coordinates, double target_coordinates, bool passed_relaunch_tracking ) {
	int step_pin, dir_pin;
	
	if( axis == 1 ){
		step_pin = AR_step_pin;
		dir_pin = AR_dir_pin;
	}
	else
	{
		step_pin = DEC_step_pin;
		dir_pin = DEC_dir_pin;
	}
	
	if ( direction )
		digitalWrite ( dir_pin , HIGH );
	else
		digitalWrite ( dir_pin , LOW );
		
	int i = 1;
	
	while ( *abort == false && i < ramp_steps ){
		delayMicroseconds( min_step_time*ramp_steps/i );
		digitalWrite ( step_pin , HIGH );
		delayMicroseconds ( min_step_time*ramp_steps/i );
		digitalWrite ( step_pin , LOW );
		i++;
	}
	
	int j=i;
	
	while ( *abort == false && i < steps-ramp_steps ){
		delayMicroseconds ( min_step_time );
		digitalWrite ( step_pin , HIGH );
		delayMicroseconds ( min_step_time );
		digitalWrite ( step_pin , LOW );
		i++;
	}
	
	#ifdef DEBUG
	if (*abort == true )
	cout << "slewing down due to abort" << endl;
	#endif
	
	for ( j ; j != 0 ; j-- ){
		delayMicroseconds ( min_step_time*ramp_steps/j );
		digitalWrite ( step_pin , HIGH );
		delayMicroseconds ( min_step_time*ramp_steps/j );
		digitalWrite ( step_pin , LOW );
		i++;
	}
	
	*axis_coordinates = target_coordinates;
		
	this_mount->start_track();
	
	#ifdef DEBUG
	cout << "end of slew on " << axis << " axis" << endl;
	#endif
}

bool Mount::get_tracking () const {
	return tracking;
}

void Mount::abort_slew(){
	abort=true;
	cout << "Aborting slew" << endl;
}

void Mount::slew_axis ( double passed_target_AR , double passed_target_DEC, double passed_sideral_time ){
	bool relaunch_tracking = {tracking};
	tracking = false; //stops tracking if enabled; waits for tracking routine to destroy AR_mux unique_lock before taking control of axis
	abort = false;
	
	int AR_steps;
	int DEC_steps;
	double AR_difference;
	double DEC_difference;
	bool AR_direction;
	bool DEC_direction;
	
	//evaluate nominal steps to reach target from current position
	
	//if trajectory does not pass through 0h 0m 0s
	if (    (right_ascension < 12 && ( passed_target_AR > right_ascension && passed_target_AR < right_ascension+12 ) ) ||
		(right_ascension > 12 && !( passed_target_AR < right_ascension-12 || passed_target_AR > right_ascension ) ) ) {
		
		AR_difference = passed_target_AR - right_ascension;
		}
	else 
		AR_difference = 24 - passed_target_AR + right_ascension;
	
	//if trajectory requires passage near the north pole 
	if (    ( right_ascension > 6 && right_ascension < 18 && ( passed_target_AR < right_ascension-6 || passed_target_AR > right_ascension+6 ) ) || 
		( right_ascension <= 6 && ( passed_target_AR > right_ascension+6 || passed_target_AR < right_ascension+18 ) ) || 
		( right_ascension >= 18 && ( passed_target_AR < right_ascension-6 || passed_target_AR < right_ascension-18 ) ) ) {
		
		DEC_difference = 180 - declination - passed_target_DEC;
		}
	else
		DEC_difference = passed_target_DEC - declination;
		
	//if moving to object that requires inversion of pier side because westwards wrt local meridian
	if (   	( passed_sideral_time < 12 && passed_target_AR > passed_sideral_time && passed_target_AR < passed_sideral_time+12 ) ||
		( passed_sideral_time > 12 && !(passed_target_AR < passed_sideral_time-12 && passed_target_AR > passed_sideral_time ) ) ){
		
			pier_side = east;
		}
	else 
		pier_side = west;
	
	//if force flip is requested inverts found pier side
	if( force_flip ){
		if ( pier_side == east )
		pier_side = west;
		else
		pier_side = east;
	}
			
	//correction of nominal steps evaluating direction of move
	if ( pier_side == old_pier_side ){
		if( pier_side == east ){ //telescope normally watching westwards
			AR_steps = -AR_steps;
		}
		else{
		
		}
	}
	
	else{
		if( pier_side == east ){ //telescope normally watching westwards
			AR_steps = -AR_steps;
		}
		else{
		
		}
		
	}
	
	AR_steps = map_double ( AR_difference, 0, 24, 0, AR_step_per_rev);
	DEC_steps = map_double ( DEC_difference, 0, 360, 0, DEC_step_per_rev);
	
	#ifdef DEBUG
	cout << "AR nominal steps: " << abs(AR_steps) << " in direction: " << sign(AR_steps) 
	<< " \nDEC nominal steps: " << abs(DEC_steps) << " in direction: " << sign(DEC_steps) << endl; 
	#endif	
	
	unique_lock <mutex> AR_lock ( AR_mux );
	unique_lock <mutex> DEC_lock ( DEC_mux );
	
	#ifdef DEBUG
	cout << "slewing to AR: " << passed_target_AR <<" DEC: " << passed_target_DEC << endl; 
	#endif
	
	thread AR_slew ( slew , this, Axis::primary_axis, abs(AR_steps), AR_direction, &abort , &right_ascension, passed_target_AR, relaunch_tracking );
	thread DEC_slew ( slew , this, Axis::secondary_axis, abs(DEC_steps), DEC_direction, &abort, &declination, passed_target_DEC, relaunch_tracking );
	
	AR_slew.detach();
	DEC_slew.detach();
}

void Mount::start_track(){
	tracking = true;
	thread tracking_thread ( tracking_routine, &tracking , &AR_mux);
	tracking_thread.detach();
}

void Mount::stop_track(){
	tracking = false;
}

void Mount::move_axis ( Axis passed_axis, int passed_speed ){

}

void Mount::set_pier_side( PierSide passed_pier_side){
	
}

void Mount::sync_mount(double passed_target_AR , double passed_target_DEC){
	right_ascension = passed_target_AR;
	declination = passed_target_DEC;
	old_pier_side = pier_side;
	
	#ifdef DEBUG
	cout << "sincked mount to AR: " << right_ascension << " DEC: " << declination << endl;
	#endif
}