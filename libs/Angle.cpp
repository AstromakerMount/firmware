#include "Angle.h" 

// Angle constructor
Angle::Angle(Type passed_type, int first_argument, int second_argument, int third_argument)
{
	type = passed_type;
	if ( passed_type == Type::sexagesimal ){
		if ( first_argument<0 || first_argument>359 || second_argument<0 || second_argument>59 || third_argument<0 || third_argument>60) {
			cout << " ERROR --> wrong sexagesimal angle formattation" << endl;
			exit(1); 
		} 
		
		else{
			degree = first_argument;
			minute = second_argument;
			second = third_argument;
			hour=0;
			arcminute=0;
			arcsecond=0;
		}
	}
	else{
		if ( first_argument<0 || first_argument>23 || second_argument<0 || second_argument>59 || third_argument<0 || third_argument>60) {
			cout << " ERROR --> wrong astronomical angle formattation" << endl;
			exit(1); 
		} 
		
		else{
			degree=0;
			minute=0;
			second=0;
			hour = first_argument;
			arcminute = second_argument;
			arcsecond = third_argument;
			
		}
	}
}

//Angle default constructor
Angle::Angle()
	:type{Type::sexagesimal}, degree{0}, minute{0}, second{0}, hour{0}, arcminute{0}, arcsecond{0}
{
}


//getter functions
Angle::Type Angle::get_type() const {
	return type;
}

int Angle::get_degree() const {
	if(type == Type::sexagesimal)
		return degree;
	else
		return ( hour*54000 + arcminute*60 + arcsecond ) / 3600 ;
}

int Angle::get_minute() const {
	if(type == Type::sexagesimal)
		return minute;
	else
		return ( ( hour*54000 + arcminute*60 + arcsecond ) % 3600 ) / 60;
}
	
int Angle::get_second() const {
	if( type == Type::sexagesimal )
		return second;
	else
		return ( ( ( hour*54000 + arcminute*60 + arcsecond ) % 3600 ) % 60 );
}

int Angle::get_hour() const {
	if( type == Type::astronomical )
		return hour;
	else
		return (degree*3600 + minute*60 + second) / 54000;
}
	
int Angle::get_arcminute() const {
	if( type == Type::astronomical )
		return arcminute;
	else
		return ( (degree*3600 + minute*60 + second) % 54000 ) / 60;
}

int Angle::get_arcsecond() const {
	if( type == Type::astronomical )
		return arcsecond;
	else
		return ( ( ( degree*3600 + minute*60 + second) % 54000 ) % 60 );
}
	



//setter functions
void Angle::set_angle ( Type passed_type, int first_argument, int second_argument, int third_argument ){
	type = passed_type;
	if ( passed_type == Type::sexagesimal ){
		if ( first_argument<0 || first_argument>359 || second_argument<0 || second_argument>59 || third_argument<0 || third_argument>60 ) {
			cerr << " ERROR --> wrong sexagesimal angle formattation" <<endl;
			exit(1); 
		} 
		
		else{
			degree = first_argument;
			minute = second_argument;
			second = third_argument;
			hour=0;
			arcminute=0;
			arcsecond=0;
		}
	}
				
	else{
		if ( first_argument<0 || first_argument>23 || second_argument<0 || second_argument>59 || third_argument<0 || third_argument>60 ) {
			cerr << " ERROR --> wrong astronomical angle formattation" <<endl;
			exit(1); 
		} 
		
		else{
			degree=0;
			minute=0;
			second=0;
			hour = first_argument;
			arcminute = second_argument;
			arcsecond = third_argument;
		}
	}
}

void Angle::set_hour ( int passed_hour ){
	if (type == Type::astronomical) {
		if ( passed_hour<0 || passed_hour>23 )
		cerr << " ERROR --> wrong astronomical angle formattation" <<endl;
		else
		hour = passed_hour;
	}
	else{
		cerr << "ERROR --> trying to set hour on an sexagesimal-formatted angle" << endl; 
	}
}

void Angle::set_arcminute ( int passed_arcminute ){
	if (type == Type::astronomical) {
		if ( passed_arcminute<0 || passed_arcminute>59 )
		cerr << " ERROR --> wrong astronomical angle formattation" <<endl;
		else
		arcminute = passed_arcminute;
	}
	else{
		cerr << "ERROR --> trying to set arcminute on an sexagesimal-formatted angle" << endl; 
	}
}

void Angle::set_arcsecond ( int passed_arcsecond ){
	if (type == Type::astronomical) {
		if ( passed_arcsecond<0 || passed_arcsecond>60 )
		cerr << " ERROR --> wrong astronomical angle formattation" <<endl;
		else
		arcsecond = passed_arcsecond;
		}
	else{
		cerr << "ERROR --> trying to set arcsecond on an sexagesimal-formatted angle" << endl; 
	}
}

void Angle::set_degree( int passed_degree ){
	if (type == Type::sexagesimal) {
		if ( passed_degree<0 || passed_degree>359 )
		cerr << " ERROR --> wrong sexagesimal angle formattation" <<endl;
		else
		degree = passed_degree;
	}
	else{
		cerr << "ERROR --> trying to set degree on an astronomical-formatted angle" << endl; 
	}
}

void Angle::set_minute ( int passed_minute ){
	if (type == Type::sexagesimal) {
		if ( passed_minute<0 || passed_minute>59 )
		cerr << " ERROR --> wrong sexagesimal angle formattation" <<endl;
		else
		minute = passed_minute;
		}
	else{
		cerr << "ERROR --> trying to set minute on an astronomical-formatted angle" << endl; 
	}
}

void Angle::set_second ( int passed_second ){
	if (type == Type::sexagesimal) {
		if ( passed_second<0 || passed_second>60 )
		cerr << " ERROR --> wrong sexagesimal angle formattation" <<endl;
		else
		second = passed_second;
		}
	else{
		cerr << "ERROR --> trying to set second on an astronomical-formatted angle" << endl; 
	}
}


// operators
ostream& operator<<(ostream& os, const Angle& angle)
{
	if ( angle.get_type() == Angle::Type::sexagesimal )
		return os << "\n\n"  << angle.get_degree() << " °    " << angle.get_minute() << " '   " << angle.get_second() << " ''" << endl ;
	else
		return os << "\n\n"  << angle.get_hour() << " h   " << angle.get_arcminute() << " m   " << angle.get_arcsecond() << " s"<<  endl ;
		
}
