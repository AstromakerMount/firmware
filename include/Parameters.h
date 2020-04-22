#ifndef PARAMETERS_H
#define PARAMETERS_H

//driver parameters
int const limit_frequency { 200000 }; 

extern int const ramp_steps { 10000 }; //acceleration/deceleration ramp amplitude

int const AR_stepper_step_per_rev { 8000 }; 
int const DEC_stepper_step_per_rev { 8000 };

//GPIO wiring
extern int const AR_step_pin { 7 };
extern int const AR_dir_pin { 0 };
extern int const AR_enable_pin { 2 };
extern int const AR_axis_error_pin { 3 };
extern int const AR_error_enable_pin { 4 };

extern int const DEC_step_pin { 22 };
extern int const DEC_dir_pin { 23 };
extern int const DEC_enable_pin { 24 };
extern int const DEC_axis_error_pin { 25 };
extern int const DEC_error_enable_pin { 5 };

//mechanical parameters
int const AR_mechanical_ratio {88};
int const DEC_mechanical_ratio {88};










//derivate parameters: self-adapted

extern int const min_step_time = 1000000 / limit_frequency ; //steptime limit due to driver max frequency

extern int const AR_step_per_rev = AR_stepper_step_per_rev * AR_mechanical_ratio; //step/rev on mount output axis
extern int const DEC_step_per_rev = DEC_stepper_step_per_rev * DEC_mechanical_ratio; 

extern long const AR_tracking_step_time = 1296000 / ( AR_step_per_rev * 15 ) * 500000; //step half time at sideral speed
extern long const DEC_tracking_step_time = 1296000 / ( DEC_step_per_rev * 15 ) * 500000;

#endif