//Author: Debjit Sarkar
//Date: November/December of 2016
//Description: Calculates the electric field of an object at some point by approximating the object with points

// Helpful link for matlab: https://www.mathworks.com/help/matlab/ref/fscanf.html
/*Possible additions: 
1. Fix display, to_str for the equation class? - or just ignore it altogether
2. Allow users to calculate the fields at multiple locations?
*/

//====================DIRECTIVES====================
#include <iostream> //read inputs/outputs from screen
#include <fstream> //file stream
#include <string> //strings
#include <cmath> //math
#include <cstdlib> //standard library
#include <vector> //vectors

using namespace std; //need to use outside because of vectors

//====================CLASSES====================
class Point {
	double X, Y, Z, Q; //point at (X, Y, Z) with a charge Q
  public:
	void setl(double, double, double); //set XYZ
	void setc(double); //set charge Q
	double x() {return X;} //get X val
	double y() {return Y;} //get Y val
	double z() {return Z;} //get Z val
	double charge() {return Q;} //get Q (charge) val  
};

class Equation {
	double A, B, C, D, E, F, G; //Ax^2+By^2+Cz^2+Dx+Ey+Fz+G=0
  public:
	void set(double, double, double, double, double, double, double); //set ABCDEFG
	double at(double, double, double); //evaluates the equation at some X Y Z
	double at(Point); //evaluates the equation at some X Y Z via the Point class
	string to_str(); //don't use, it sucks
	void display(); //they both suck, but to_str sucks less - actually don't use either
};

//====================FUNCTIONS====================
void get_inputs_IO(bool& is_conductor, double& obj_charge, Equation& eq, Point& start, Point& end, int& number_of_points, double& leeway); //takes in inputs from CMD

void get_inputs_file(); //takes in inputs from file, not necessary

void generate_points_insulator(Equation eq, vector <Point> & obj, Point minp, Point maxp, int hard_stop, int number_of_points); //generates points if insulator

void generate_points_conductor(Equation eq, vector <Point> & obj, Point minp, Point maxp, int hard_stop, int number_of_points, double leeway); //generates points if conductor

void points_to_file(vector <Point> obj, int number_of_points); //prints the points to a file

void generate_charges(vector <Point> & obj, int number_of_points, double obj_charge); //finds the charge per point

void get_location(Point& location); //determines where to calculate field at

void fields_at_location(vector <Point> obj, vector <Point> & fields, Point location, double KE, int number_of_points); //calculates the field due to each point at some inputted location

void sum_fields(vector <Point> fields, int number_of_points, double& net_x, double& net_y, double& net_z); //sums up individual points' fields

void output_to_IO(double net_x, double net_y, double net_z, int number_of_points, Equation eq); //displays the summed fields to CMD

void output_to_file(); //sends summed fields at some location to a file

bool point_within_function(); //checks if a point is within a function

bool point_within_boundary(); //redundant i think

void determine_limits(); //determines where to begin/end randomizing points


//====================MAIN BODY====================
int main()
{
//--------------------USER INPUTS--------------------
	//for the object:
	bool is_conductor; //choose to calculate for an insulator or conductor
	double obj_charge; //charge of the object
	Equation eq; //the equation that describes the object's shape
	Point minp, maxp; //min/max x/y/z
	int number_of_points = 0; //number of points the approximate the object with
	double leeway; //used for conductors, larger value -> thicker shell
	
	//for the location:
	Point location; //where the field is calculated at
	
//--------------------CONSTANTS--------------------
	double const KE = 9 * pow(10, 9); //Coulomb's constant
	long long int const hard_stop = 10000000; //a limiter to the number of times the point-generating loop will run, prevents the program from running infinitely if someone enters bad inputs
	
//--------------------CALCULATED VARIABLES--------------------
	vector <Point> obj, fields; //locations of the points used to generate the object and the fields at each of their locations
	double net_x = 0, net_y = 0, net_z = 0; //will store the final values of the field at some location
	
	//keeping just in case:
	//vector <Point> obj, fields;
	//obj.resize(number_of_points);
	//fields.resize(number_of_points);
	
//--------------------MISC--------------------
	//setup for the IO
	cout.setf(ios::fixed);
	cout.setf(ios::showpoint);
	cout.precision(2);

//--------------------FUNCTION CALLS--------------------
	get_inputs_IO(is_conductor, obj_charge, eq, minp, maxp, number_of_points, leeway); //gets inputs
	//get_inputs_file(); //not used
	
	obj.resize(number_of_points);
	fields.resize(number_of_points);
	
	//generates points for the object
	if(is_conductor)
		generate_points_conductor(eq, obj, minp, maxp, hard_stop, number_of_points, leeway);
	else
		generate_points_insulator(eq, obj, minp, maxp, hard_stop, number_of_points);
	
	generate_charges(obj, number_of_points, obj_charge); //assigns a charge to each point
	
	points_to_file(obj, number_of_points); //prints the points, will be used in conjuction with a separate program to display the points
	
	get_location(location); //where to find the field at
	
	fields_at_location(obj, fields, location, KE, number_of_points); //calculates the fields
	
	sum_fields(fields, number_of_points, net_x, net_y, net_z); //sums the fields
	
	output_to_IO(net_x, net_y, net_z, number_of_points, eq); //displays the results
	//output_to_file(); //not used
	
	return 0;
}

//====================POINT CLASS FUCTIONS====================
void Point::setl(double x, double y, double z) //set the location of a point
{
	X = x;
	Y = y;
	Z = z;
}

void Point::setc(double q) //set the charge of a point
{
	Q = q;
}

//====================EQUATION CLASS FUNCTIONS====================
void Equation::set(double a, double b, double c, double d, double e, double f, double g) //set the values for ABCDEFG
{
	A = a;
	B = b;
	C = c;
	D = d;
	E = e;
	F = f;
	G = g;
}

double Equation::at(double x, double y, double z) //evaluates the function at some XYZ
{
	return (A*x*x + B*y*y + C*z*z + D*x + E*y + F*z + G);
}

double Equation::at(Point p) //evaluates the function at some point, basically the same as using XYZ
{
	return (A*p.x()*p.x() + B*p.y()*p.y() + C*p.z()*p.z() + D*p.x() + E*p.y() + F*p.z() + G);
}

string Equation::to_str() //this is a complete mess so I'm discarding it - there's not much benefit to this anyway (or maybe just sour grapes)
{
	string s = "";
	//if(A!=0)
	return s;
	//if(A!=0)
	//	output += to_string(A) + "x^2"
	//return (to_string(A) + "x^2"
	//return (to_string(A).erase(to_string(A).find_last_not_of('0') + 1, string::npos) + "x^2 + " + to_string(B).erase(to_string(B).find_last_not_of('0') + 1, string::npos) + "y^2 + " + to_string(C).erase(to_string(C).find_last_not_of('0') + 1, string::npos) + "z^2 + " + to_string(D).erase(to_string(D).find_last_not_of('0') + 1, string::npos) + "x + " + to_string(E).erase(to_string(E).find_last_not_of('0') + 1, string::npos) + "y + " + to_string(F).erase(to_string(F).find_last_not_of('0') + 1, string::npos) + "z + " + to_string(G).erase(to_string(G).find_last_not_of('0') + 1, string::npos) + " = 0");
}

//.erase( to_string(A).find_last_not_of('0') + 1, string::npos)

void Equation::display() //same as the to_str function, this is such a mess to clear up that I'm not using it
{
	if(A != 0)
		cout << A << "x^2+";
	if(B != 0)
		cout << B << "y^2+";
	if(C != 0)
		cout << C << "z^2+";
		cout << D << "x+";
		cout << E << "y+";
		cout << F << "z+";
		cout << G; 
	cout << "=0";
}

//====================FUNCTION BODIES====================
void get_inputs_IO(bool& is_conductor, double& obj_charge, Equation& eq, Point& minp, Point& maxp, int& number_of_points, double& leeway)
{
	//Description: set the values for all the variables that are needed from the user
	
	using namespace std;
	
	string insul_conduc; //stores user input of conductor vs insulator to determine the value of is_conductor
	
	double A, B, C, D, E, F, G, xi, xf, yi, yf, zi, zf;
	cout << "Describe the object according to \nAx^2+By^2+Cz^2+Dx+Ey+Fz+G=0\nEnter the values of A, B, C, D, E, F, and G." << endl;
	cin >> A;
	cin >> B;
	cin >> C;
	cin >> D;
	cin >> E;
	cin >> F;
	cin >> G;
	
	eq.set(A, B, C, D, E, F, G);
	
	cout << endl << "If conductor, type \"C\". If insulator, type \"I\"" << endl;
	cin >> insul_conduc;
	if(insul_conduc == "C" || insul_conduc == "c")
	{
		is_conductor = true;
		cout << "Leeway (relates to the thickness of the shell, suggestion is 0.1):";
		cin >> leeway;
	}
	else
		is_conductor = false;
	
	cout << endl << "Enter any boundary conditions for the object." << endl << "Min: -10  Max:+10  Anything outisde the min/max will be set to 10/-10." << endl;
	
	//could/should probably simplify this, all it does is force the limits to be between -10 and 10 if they are too large
	cout << "X: ";
	cin >> xi;
	if(xi < -10)
		xi = -10;
	cout << "To: ";
	cin >> xf;
	if(xf > 10)
		xf = 10;
	cout << "Y: ";
	cin >> yi;
	if(yi < -10)
		yi = -10;
	cout << "To: ";
	cin >> yf;
	if(yf > 10)
		yf = 10;
	cout << "Z: ";
	cin >> zi;
	if(zi < -10)
		zi = -10;
	cout << "To: ";
	cin >> zf;
	if(zf > 10)
		zf = 10;
	
	minp.setl(xi, yi, zi);
	maxp.setl(xf, yf, zf);
	
	cout << "Enter the charge of the object: ";
	cin >> obj_charge;
	cout << "Enter the number of points to approximate with: ";
	cin >> number_of_points;
	
	cout << "Done getting inputs." << endl;
}

void generate_points_insulator(Equation eq, vector <Point> & obj, Point minp, Point maxp, int hard_stop, int number_of_points)
{
	//Description: generate the points for an insulator
	
	double rand_x, rand_y, rand_z; //random x, y, and z values between the minimum and maximum
	int counter = 0; //counts up to the number_of_points
	int hard_counter = 0; //counts up the the hard_stop
	Point p; //used to add the randomly generated points to the obj vector
	
	while(counter < number_of_points && hard_counter < hard_stop)
	{
		//random values between the minimum and maximum
		rand_x = ((1.0 * rand()) / RAND_MAX) * (maxp.x() - minp.x()) + minp.x();
		rand_y = ((1.0 * rand()) / RAND_MAX) * (maxp.y() - minp.y()) + minp.y();
		rand_z = ((1.0 * rand()) / RAND_MAX) * (maxp.z() - minp.z()) + minp.z();
		
		//stores random values to a point
		p.setl(rand_x, rand_y, rand_z);
		
		//checks if the point is within the function with relation to the origin
		if(eq.at(p) <= 0)
		{
			obj[counter] = p;
			counter++;
		}
		hard_counter++;
	}
	cout << "Done generating points." << endl;
}

void generate_points_conductor(Equation eq, vector <Point> & obj, Point minp, Point maxp, int hard_stop, int number_of_points, double leeway)
{
	//Description: generate the points for a conductor
	
	double rand_x, rand_y, rand_z;
	//leeway: larger value -> thicker shell, not sure if this should be internal or adjustable, probably adjustable
	//this leeway variable is sorta difficult to explain in how it works, but it basically determines the range of points that are acceptable
	double upper = abs(leeway / 2.0);
	double lower = -upper;
	int counter = 0; //keep looping until counter = number_of_points
	int hard_counter = 0; //keep looping until hard_counter = hard_stop
	Point p;
	
	cout << "NOTE: This will only generate points based on the function." << endl;
	cout << "Ex. the top/bottom of a cylinder are not included." << endl;
	
	while(counter < number_of_points && hard_counter < hard_stop)
	{
		//random values between the minimum and maximum
		rand_x = ((1.0 * rand()) / RAND_MAX) * (maxp.x() - minp.x()) + minp.x();
		rand_y = ((1.0 * rand()) / RAND_MAX) * (maxp.y() - minp.y()) + minp.y();
		rand_z = ((1.0 * rand()) / RAND_MAX) * (maxp.z() - minp.z()) + minp.z();
		
		//stores the random values to a point
		p.setl(rand_x, rand_y, rand_z);
		
		if(lower < eq.at(p) && eq.at(p) < upper) //so long as the evaluation of the function is close to 0, use the point
		{
			obj[counter] = p;
			counter++;
			//cout << "Point: " << rand_x << " " << rand_y << " " << rand_z << endl;
		}
		hard_counter++;
	}
	cout << "Done generating points." << endl;
}

void generate_charges(vector <Point> & obj, int number_of_points, double obj_charge)
{
	//Description: give each point a charge (in this case, by using a simple average)
	//this could be expanded if there's a non-constant charge density
	
	double avg_charge = (1.0 * obj_charge) / number_of_points;
	
	for(int i = 0; i < number_of_points; i++)
	{
		obj[i].setc(avg_charge);
	}
	cout << "Done generating charges." << endl;
}

void points_to_file(vector <Point> obj, int number_of_points)
{
	//Description: send the points to a file so that they can be displayed using another program
	
	string file_name = "points.txt";
	ofstream output;
	
	cout << "Writing points to " << file_name << endl;
	
	output.open(file_name);
	
	//write each point in an "X Y Z\nX Y Z" format
	for(int i = 0; i < number_of_points; i++)
	{
		output << obj[i].x() << " " << obj[i].y() << " " << obj[i].z() /*<< " " << obj[i].charge()*/ << endl;
	}
	
	output.close();
	
	//writes the x, y, and z of the points to 3 different files to be read in by the matlab program
	ofstream xs, ys, zs;
	xs.open("xpoints.txt");
	ys.open("ypoints.txt");
	zs.open("zpoints.txt");
	
	for(int i = 0; i < number_of_points; i++)
	{
		xs << obj[i].x() << endl;
		ys << obj[i].y() << endl;
		zs << obj[i].z() << endl;
	}
	
	xs.close();
	ys.close();
	zs.close();
	
	cout << "Done writing points to file." << endl;
}

void get_location(Point& location)
{
	//Description: finds where to calculate the points at
	
	double loc_x, loc_y, loc_z;
	cout << "Find field at (X, Y, Z): " << endl;
	cout << "X: ";
	cin >> loc_x;
	cout << "Y: ";
	cin >> loc_y;
	cout << "Z: ";
	cin >> loc_z;
	
	location.setl(loc_x, loc_y, loc_z);
}

void fields_at_location(vector <Point> obj, vector <Point> & fields, Point location, double KE, int number_of_points)
{
	//Description: calculates the fields at each point in each axial direction
	
	double field_mag = 0;
	double distance = 0;
	double field_x, field_y, field_z;
	
	for(int i = 0; i < number_of_points; i++)
	{
		distance = sqrt(pow((location.x() - obj[i].x()), 2) + pow((location.y() - obj[i].y()), 2) + pow((location.z() - obj[i].z()), 2)); //the distance between the point and the location
		field_mag = KE * obj[i].charge() / (distance * distance); //the magnitude of the electric field at the point
		
		//resolves the field into the various directions
		field_x = field_mag * (obj[i].x() - location.x()) / distance;
		field_y = field_mag * (obj[i].y() - location.y()) / distance;
		field_z = field_mag * (obj[i].z() - location.z()) / distance;
		
		//sets the field generated by each point charge
		fields[i].setl(field_x, field_y, field_z);
	}
	
	cout << "Done calculating fields." << endl;
}

void sum_fields(vector <Point> fields, int number_of_points, double& net_x, double& net_y, double& net_z)
{
	//Description: arithmetic sum of the fields in each direction
	
	net_x = 0;
	net_y = 0;
	net_z = 0;
	
	for(int i = 0; i < number_of_points; i++)
	{
		net_x += fields[i].x();
		net_y += fields[i].y();
		net_z += fields[i].z();
	}
	
	cout << "Done summing fields." << endl;
}

void output_to_IO(double net_x, double net_y, double net_z, int number_of_points, Equation eq)
{
	//Description: display the results
	
	cout << endl;
	cout << "Net electric fields in each axial direction:" << endl;
	cout << "X: " << net_x << " N/C" << endl;
	cout << "Y: " << net_y << " N/C" << endl;
	cout << "Z: " << net_z << " N/C" << endl;
}