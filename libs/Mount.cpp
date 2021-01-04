#define DEBUG

#include <wiringPi.h>
#include <chrono>
#include <mutex>
using std::unique_lock;

#include "Server.h"
#include "Parameters.h"

void test( int pin){
	digitalWrite(pin, HIGH);
	delay (100);
	digitalWrite(pin, LOW);
	delay (100);
	digitalWrite(pin, HIGH);
	delay (100);
	digitalWrite(pin, LOW);
	delay (100);
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

	pinMode ( autoguide_AR_plus , INPUT ) ;
	pinMode ( autoguide_AR_minus , INPUT ) ;
	pinMode ( autoguide_DEC_plus , INPUT ) ;
	pinMode ( autoguide_DEC_minus , INPUT ) ;
	pullUpDnControl ( autoguide_AR_plus , PUD_DOWN );
	pullUpDnControl ( autoguide_AR_minus , PUD_DOWN );
	pullUpDnControl ( autoguide_DEC_plus , PUD_DOWN );
	pullUpDnControl ( autoguide_DEC_minus , PUD_DOWN );
	
	pinMode ( homing_sensor_AR , INPUT ) ;
	pinMode ( homing_sensor_DEC , INPUT ) ;
	pullUpDnControl ( homing_sensor_AR , PUD_DOWN );
	pullUpDnControl ( homing_sensor_DEC , PUD_DOWN );
	
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
	
	this-> start_track();
	
}

int map_int ( int in, int in_from, int in_to, int out_from, int out_to ){
	return out_from + ( (in - in_from)*(out_to - out_from) / (in_to - in_from) );
}

int map_double ( double in, double in_from, double in_to, int out_from, int out_to ){
	return out_from + ( (in - in_from)*(out_to - out_from) / (in_to - in_from) );
}

double map_int ( int in, int in_from, int in_to, double out_from, double out_to ){
	return out_from + ( (in - in_from)*(out_to - out_from) / (in_to - in_from) );
}

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

float ST4_read ( Axis axis , float autoguide_speed){
	
		if ( axis == primary_axis ) {
				if ( digitalRead(autoguide_AR_plus) )
				return 1+autoguide_speed;
				
				else if ( digitalRead(autoguide_AR_minus) )
				return 1-autoguide_speed;
				
				else 
				return 1;
		} 
		
		else {
				if ( digitalRead(autoguide_DEC_plus) )
				return autoguide_speed;
				
				else if ( digitalRead(autoguide_DEC_minus) )
				return -autoguide_speed;
				
				else 
				return 0;
		}
}

void tracking_routine( Mount *mount, Axis axis, mutex *mux ){	
	unique_lock <mutex> axis_mutex_lock ( *mux );
	
	#ifdef DEBUG
	cout << "now tracking along " << axis << " axis" << endl;
	#endif
	
	int step_pin, dir_pin, tracking_step_time, step_time;
	float tracking_speed;
	
	if ( axis == primary_axis ){
			step_pin = AR_step_pin;
			dir_pin = AR_dir_pin;
			tracking_step_time = AR_tracking_step_time;
			
			while ( mount->get_tracking_AR() ){
				tracking_speed = ST4_read( axis , mount->get_autoguide_speed() );
				
				if ( tracking_speed != 0 ){
					if ( sign( tracking_speed ) ==1 )
						digitalWrite ( dir_pin , HIGH );
					else
						digitalWrite ( dir_pin , LOW );
						
					step_time = int(tracking_step_time/abs(tracking_speed) );
					
					std::this_thread::sleep_for ( std::chrono::microseconds ( step_time ) );
					digitalWrite ( step_pin , HIGH );
					std::this_thread::sleep_for ( std::chrono::microseconds ( step_time ) );
					digitalWrite ( step_pin , LOW );
				}
				else 
				std::this_thread::sleep_for ( std::chrono::microseconds ( 10000 ) );
			}
	}
	else{
			step_pin = DEC_step_pin;
			dir_pin = DEC_dir_pin;
			tracking_step_time = DEC_tracking_step_time;
			
			while ( mount->get_tracking_DEC() ){
				tracking_speed = ST4_read( axis , mount->get_autoguide_speed() );

				if ( tracking_speed != 0 ){
					if ( sign( tracking_speed ) ==1 )
						digitalWrite ( dir_pin , HIGH );
					else
						digitalWrite ( dir_pin , LOW );
						
					step_time = int(tracking_step_time/abs(tracking_speed) );

					std::this_thread::sleep_for ( std::chrono::microseconds ( step_time ) );
					digitalWrite ( step_pin , HIGH );
					std::this_thread::sleep_for ( std::chrono::microseconds ( step_time ) );
					digitalWrite ( step_pin , LOW );
				}
				else 
				std::this_thread::sleep_for ( std::chrono::microseconds ( 10000 ) );
			}
	}
	
	
	#ifdef DEBUG
	cout << "end of tracking along " << axis << " axis" << endl;
	#endif
}

void slew ( Mount *this_mount, Axis axis , int steps , int direction, int passed_speed, bool *abort, bool passed_relaunch_tracking, float sideral_time) {
	int step_pin, dir_pin;
	
	if( axis == 0 ){
		step_pin = AR_step_pin;
		dir_pin = AR_dir_pin;
	}
	else
	{
		step_pin = DEC_step_pin;
		dir_pin = DEC_dir_pin;
	}
	
	int ramp_steps;
	float scaled_speed;
	switch ( passed_speed ) {
		case 1:
		scaled_speed = 0.01;
		ramp_steps = 10;
		break;
		
		case 2:
		scaled_speed = 0.02;
		ramp_steps = 10;
		break;
		
		case 3:
		scaled_speed = 0.05;
		ramp_steps = 100;
		break;
		
		case 4:
		scaled_speed = 0.1;
		ramp_steps = 100;
		break;
		
		case 5:
		scaled_speed = 0.2;
		ramp_steps = 100;
		break;
		
		case 6:
		scaled_speed = 0.4;
		ramp_steps = 1000;
		break;
		
		case 7:
		scaled_speed = 0.6;
		ramp_steps = 1000;
		break;
		
		case 8:
		scaled_speed = 1;
		ramp_steps = 1000;
		break;	
	}
	
	if ( direction == 1 )
		digitalWrite ( dir_pin , HIGH );
	else
		digitalWrite ( dir_pin , LOW );
		
	int i = 1; //steps count to evaluate the final position after the slew has been stopped
	int step_time;
	
	while ( *abort == false && i < ramp_steps ){
		step_time = min_step_time*ramp_steps/i/scaled_speed;
		delayMicroseconds( step_time );
		digitalWrite ( step_pin , HIGH );
		delayMicroseconds ( step_time );
		digitalWrite ( step_pin , LOW );
		i++;
	}
	
	int j=i;
	step_time = min_step_time/scaled_speed;
	while ( *abort == false && i < steps-2*ramp_steps ){
		delayMicroseconds ( step_time );
		digitalWrite ( step_pin , HIGH );
		delayMicroseconds ( step_time );
		digitalWrite ( step_pin , LOW );
		i++;
	}
	
	#ifdef DEBUG
	if (*abort == true )
	cout << "slewing down due to abort" << endl;
	#endif
	
	for ( j ; j != 0 ; j-- ){
		step_time = min_step_time*ramp_steps/j/scaled_speed;
		delayMicroseconds ( step_time );
		digitalWrite ( step_pin , HIGH );
		delayMicroseconds ( step_time );
		digitalWrite ( step_pin , LOW );
		i++;
	}
	
	//*axis_coordinates = target_coordinates;
	if ( axis == 0 ){ //if slew was along AR axis
		double reached_position = this_mount->get_right_ascension() + map_int(i*direction, 0, AR_step_per_rev, 0.0, 24.0 );
		if ( reached_position >= 24.0 )
		this_mount->set_right_ascension ( reached_position - 24.0 );
		else if ( reached_position < 0 )
		this_mount->set_right_ascension ( reached_position + 24.0 );
		
		if ( sideral_time < 12.0) { //final position is compared to sideral time to establish if a manual forced flip move has been performed
			if ( this_mount.get_right_ascension() > sideral_time && this_mount.get_right_ascension() < sideral_time + 12.0 ) 
				if ( this_mount.get_force_flip() ) //if so, reverts the force flip status
					this_mount.set_force_flip ( false );
				else
					this_mount.set_force_flip ( true );
			}
		else  {
			if ( this_mount.get_right_ascension() < sideral_time -12.0 || this_mount.get_right_ascension() > sideral_time )
				if ( this_mount.get_force_flip() ) //if so, reverts the force flip status
					this_mount.set_force_flip ( false );
				else
					this_mount.set_force_flip ( true );
		}
	}
	
	else { //if slew was along DEC axis
		double reached_position = this_mount->get_declination() + map_int(i*direction, 0, DEC_step_per_rev, 0.0, 360.0 );
		if ( reached_position >= 90.0 ){
			this_mount->set_declination ( 180.0 - reached_position );
			this_mount->set_right_ascension ( reached_position + 12.0 );
			if ( this_mount->get_right_ascension() >= 24.0 )
			this_mount->set_right_ascension ( reached_position - 24.0 );
		}
		else if ( reached_position <= -90.0 ){
			this_mount->set_declination ( reached_position - 180.0 );
			this_mount->set_right_ascension ( reached_position + 12.0 );
			if ( this_mount->get_right_ascension() >= 24.0 )
			this_mount->set_right_ascension ( reached_position - 24.0 );
		}	
	}
	
	
	
	
	if ( axis == primary_axis && passed_relaunch_tracking ) 
	this_mount->start_track();
	
	#ifdef DEBUG
	cout << "end of slew on " << axis << " axis" << endl;
	#endif
}

void Mount::abort_slew(){
	abort=true;
	cout << "Aborting slew" << endl;
}

void Mount::slew_axis ( double passed_target_AR , double passed_target_DEC, double passed_sideral_time ){
	bool relaunch_tracking_AR = this->get_tracking_AR();
	bool relaunch_tracking_DEC = this->get_tracking_DEC();
	tracking_AR = false; //stops tracking if enabled; waits for tracking routine to destroy AR_mux unique_lock before taking control of axis
	tracking_DEC = false;
	abort = false;
	
	int AR_steps;
	int DEC_steps;
	double AR_difference;
	double DEC_difference;
	int AR_direction;
	int DEC_direction;
	
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
	
	//evaluate amplitude of slew	
	AR_steps = map_double ( AR_difference, 0, 24, 0, AR_step_per_rev);
	DEC_steps = map_double ( DEC_difference, 0, 360, 0, DEC_step_per_rev);
	
	#ifdef DEBUG
	cout << "AR nominal steps: " << abs(AR_steps) << " in direction: " << sign(AR_steps) 
	<< " \nDEC nominal steps: " << abs(DEC_steps) << " in direction: " << sign(DEC_steps) << endl; 
	#endif
	
	//if moving to object that requires inversion of pier side because westwards wrt local meridian
	if (   	( passed_sideral_time < 12 && passed_target_AR > passed_sideral_time && passed_target_AR < passed_sideral_time+12 ) ||
		( passed_sideral_time > 12 && !(passed_target_AR < passed_sideral_time-12 && passed_target_AR > passed_sideral_time ) ) ){
		
			pier_side = east;
		}
	else 
		pier_side = west;
	
	#ifdef DEBUG
	cout << "pierside: " << pier_side << endl;
	cout << "forcing: " << force_flip << endl;
	#endif
		
	//if force flip is requested inverts found pier side
	if( force_flip ){
		if ( pier_side == east )
		pier_side = west;
		else
		pier_side = east;
		
		#ifdef DEBUG
		cout << "forced pierside to: " << pier_side << endl; 
		#endif
	}
			
	//correction of slew amplitude evaluating pier side change request
	if ( pier_side != old_pier_side ){
		if( pier_side == east ){ //telescope normally watching westwards
			AR_steps = -AR_step_per_rev*0.5-AR_steps;
			DEC_steps = DEC_step_per_rev*0.5-DEC_steps;
		}
		else{
			AR_steps = AR_step_per_rev*0.5-AR_steps;
			DEC_steps = -DEC_step_per_rev*0.5-DEC_steps;
		}
		
		#ifdef DEBUG
		cout << "changed amplitude of slew due to pierside change. Now:" << pier_side << endl;
		cout << "AR nominal steps: " << abs(AR_steps) << " in direction: " << sign(AR_steps) 
		<< " \nDEC nominal steps: " << abs(DEC_steps) << " in direction: " << sign(DEC_steps) << endl; 
		#endif
		
	}
	
	AR_direction = sign ( AR_steps );
	DEC_direction = sign ( DEC_steps );
	
	old_pier_side = pier_side;	
	
	unique_lock <mutex> AR_lock ( AR_mux );
	unique_lock <mutex> DEC_lock ( DEC_mux );
	
	#ifdef DEBUG
	cout << "slewing to AR: " << passed_target_AR <<" DEC: " << passed_target_DEC << endl; 
	#endif
	
	thread AR_slew ( slew , this, Axis::primary_axis, abs(AR_steps), AR_direction, 8, &abort, relaunch_tracking_AR );
	thread DEC_slew ( slew , this, Axis::secondary_axis, abs(DEC_steps), DEC_direction, 8, &abort, relaunch_tracking_DEC );
	
	AR_slew.detach();
	DEC_slew.detach();
}

void Mount::start_track(){
	tracking_AR = true;
	tracking_DEC = true;
	
	thread AR_tracking_thread ( tracking_routine, this, primary_axis, &AR_mux);
	thread DEC_tracking_thread ( tracking_routine, this, secondary_axis, &DEC_mux);
	
	AR_tracking_thread.detach();
	DEC_tracking_thread.detach();
}

void Mount::stop_track_AR(){
	tracking_AR = false;
}

void Mount::stop_track_DEC(){
	tracking_DEC = false;
}

void Mount::jog_axis ( Axis passed_axis, int passed_direction, int passed_speed ){
	bool relaunch_tracking_AR = this->get_tracking_AR();
	bool relaunch_tracking_DEC = this->get_tracking_DEC();
	
	if ( passed_axis == primary_axis ) 
	tracking_AR = false; //stops tracking if enabled; waits for tracking routine to destroy AR_mux unique_lock before taking control of axis
	else 
	tracking_DEC = false;
	
	abort = false;
	
	
	if ( passed_axis == primary_axis ){
		unique_lock <mutex> AR_lock ( AR_mux );
		thread AR_slew ( slew , this, Axis::primary_axis, AR_step_per_rev, passed_direction, passed_speed, &abort , relaunch_tracking_AR, sideral_time );
		AR_slew.detach();
	}
	else{
		unique_lock <mutex> DEC_lock ( DEC_mux );
		thread DEC_slew ( slew , this, Axis::secondary_axis, DEC_step_per_rev, passed_direction, passed_speed, &abort, relaunch_tracking_DEC, sideral_time );
		DEC_slew.detach();
	}
		
	#ifdef DEBUG
	cout << "Jogging on " << passed_axis << " axis" << endl;
	#endif
	
	
}

void Mount::sync ( double passed_target_AR_to_sync , double passed_target_DEC_to_sync, double sideral_time_to_sync ){
	right_ascension = passed_target_AR_to_sync;
	declination = passed_target_DEC_to_sync;
	
	if ( sideral_time_to_sync > 12 && right_ascension < sideral_time_to_sync && right_ascension > sideral_time_to_sync-12 )
	pier_side = east;
	else if ( sideral_time_to_sync > 12 )
	pier_side = west;
	else if ( sideral_time_to_sync < 12 && right_ascension > sideral_time_to_sync && right_ascension < sideral_time_to_sync+12 )
	pier_side = west;
	else 
	pier_side = east;
	
	#ifdef DEBUG
	cout << "sincked mount to AR: " << right_ascension << " DEC: " << declination << " pierside: " << pier_side << endl;
	#endif
}

void Mount::home(){
	this->stop_track_AR();
	this->stop_track_DEC();
	
	bool status, direction;
	abort = false;
	
	if ( digitalRead(homing_sensor_AR) ) {
		direction = true;
		status = true;
	}		
	else {
		direction = false;
		status = false;
	}
	
	#ifdef DEBUG
	cout << "homing AR axis " << endl; 
	#endif
	
	unique_lock <mutex> AR_lock ( AR_mux );
	unique_lock <mutex> DEC_lock ( DEC_mux );
	
	thread AR_slew ( slew , this, Axis::primary_axis, int(AR_step_per_rev*0.5), direction, 8, &abort, false );
	AR_slew.detach();
	
	while ( digitalRead( homing_sensor_AR ) == status ){};
	abort =true;
	#ifdef DEBUG
	cout << "finished homing AR axis " << endl; 
	#endif
	
	std::this_thread::sleep_for ( std::chrono::seconds(5) );
	
	#ifdef DEBUG
	cout << "homing DEC axis " << endl; 
	#endif
	
	abort = false;
	if ( digitalRead(homing_sensor_DEC) ) {
		direction = true;
		status = true;
	}		
	else {
		direction = false;
		status = false;
	}
	
	thread DEC_slew ( slew , this, Axis::secondary_axis, int(DEC_step_per_rev*0.5), direction, 8, &abort, false );
	DEC_slew.detach();
	
	while ( digitalRead( homing_sensor_DEC ) == status ){};
	abort =true;
	
	#ifdef DEBUG
	cout << "finished homing DEC axis " << endl; 
	#endif
}
// ================ GETTER & SETTER MEMBER FUNCTIONS ====================

void Mount::set_force_flip( bool passed_force_flip){
	force_flip = passed_force_flip;
	
	#ifdef DEBUG
	cout << "Changed force flip to: " << force_flip ;
	#endif
}

void Mount::set_right_ascension( double passed_right_ascension){
	right_ascension = passed_right_ascension;
	
	#ifdef DEBUG
	cout << "Changed right ascension to: " << right_ascension ;
	#endif
}

void Mount::set_declination( double passed_declination){
	declination = passed_declination;	
	
	#ifdef DEBUG
	cout << "Changed declination to: " << declination ;
	#endif
}

void Mount::set_autoguide_speed ( float passed_autoguide_speed ){
	autoguide_speed = passed_autoguide_speed;
	
	#ifdef DEBUG
	cout << "Changed autoguide speed to: " << autoguide_speed ;
	#endif
}

bool Mount::get_tracking_AR () const {
	return tracking_AR;
}

bool Mount::get_tracking_DEC () const {
	return tracking_DEC;
}

double Mount::get_right_ascension() const{
	return right_ascension ;
}


double Mount::get_declination() const{
	return declination ;
}

float Mount::get_autoguide_speed() const{
	return autoguide_speed ;
}

bool Mount::get_force_flip() const{
		return force_flip ;
}
