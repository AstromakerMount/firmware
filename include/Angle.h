#ifndef ANGLE_H
#define ANGLE_H

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;

//Access class
class Angle{
public:

	enum class Type {
		sexagesimal=0, astronomical
	};
	
	
	// constructors
	Angle(Type type, int first_argument, int second_argument, int third_arg);
	Angle();

	// getter functions
	Type get_type() const;
	int get_degree() const;
	int get_minute() const;
	int get_second() const;
	int get_hour() const;
	int get_arcminute() const;
	int get_arcsecond() const;

	// member funcitons
	void set_angle ( Type type, int first_argument, int second_argument, int third_argument );
	void set_hour ( int passed_hour );
	void set_arcminute ( int passed_arcminute );
	void set_arcsecond ( int passed_arcsecond );
	void set_degree( int passed_degree );
	void set_minute ( int passed_minute );
	void set_second ( int passed_second );
	
private:
	Type type;
	int hour;
	int arcminute;
	int arcsecond;
	int degree;
	int minute;
	int second;
};

// operators
std::ostream& operator<<(std::ostream& os, const Angle& angle);

#endif
