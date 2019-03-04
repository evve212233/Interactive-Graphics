/*
Author: Ziwei Zheng
Date: 3/2/2019
NetId: zz1456
*/

/*
To run in visual studio, click Local Windows Debugger.
A console will pop up with three choices
User can choose any according to what they would like to see
glew and glut is required to run this program
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>     /* abs */

#ifdef __APPLE__  // include Mac OS X verions of headers
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glut.h>
#endif

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#define SCALE		  1.01
#define W_SCALE		  1510
#define XOFF          50
#define YOFF          50
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600

using namespace std;
//this program is modified from sample.cpp provided by the instructor

void display(void);
void myinit(void);
void drawOnePoint();
void draw_circle(int x, int y, int r);
void circlePoint(int x, int y, int x_offset, int y_offset);
/* Function to handle file input; modification may be needed */
void file_in(void);
void idle(void);
//adjust the size of point corresponding to the window size
int resize_window(int p);
//a vector to store all user input points or the file points
vector<vector<int>> points;
int frame_size = 1;
char choice;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	/* Use both double buffering and Z buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(XOFF, YOFF);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("CS6533/CS4533 Assignment 1");
	glutDisplayFunc(display);

	//part c, ask user to enter three points
	cout << "Please enter the following options:\n";
	cout << "A.Enter a point to draw a circle \n";
	cout << "B.Display the circles from input file.\n";
	cout << "C.Display animated circles\n\n";
	cout << "Your choice is: ";
	cin >> choice;
	switch (choice)
	{
	case 'A':
		drawOnePoint();
		break;
	case 'B':
		/* Function call to handle file input here */
		file_in();
		break;
	case 'C':
		file_in();
		glutIdleFunc(idle);
		break;
	default:
		break;
	}
	myinit();
	glutMainLoop();
	return 0;
}

void drawOnePoint() {
	int x, y, r, max_w, max_h;
	double scale_factor;
	cout << "Enter three integers to test (x, y, r)\n";
	cin >> x >> y >> r;

	//scale this point if it is out of window size
	//dont scale other points because it could get super small and not visible to the user
	max_w = max(x + r, abs(x - r));
	max_h = max(y + r, abs(y - r));
	while (max_w > WINDOW_WIDTH) {
		scale_factor = SCALE * (max_w / WINDOW_WIDTH);
		x = (int)(x / scale_factor);
		r = (int)(r / scale_factor);
		max_w = max(x + r, abs(x - r));
	}
	while (max_h > WINDOW_HEIGHT) {
		scale_factor = SCALE * (max_h / WINDOW_HEIGHT);
		y = (int)(y / scale_factor);
		r = (int)(r / scale_factor);
		max_h = max(y + r, abs(y - r));
	}
	points.push_back({ x, y, r });
}

void file_in(void)
{
	ifstream ifs("input_circles.txt");
	if (!ifs) {
		/*check if file does not exist*/
		cerr << "Couldn't open 'input_circles.txt'\n";
		exit(1);
	}
	else {
		//set a variable for each line
		string line;
		getline(ifs, line);
		//read line by line starting from the second line that has point values
		while (getline(ifs, line)) {
			int x, y, r;
			istringstream nums(line);
			//get x, y, r from that line
			nums >> x >> y >> r;
			//adjust the point to the screen window when add to the vector
			points.push_back({ resize_window(x), resize_window(y), resize_window(r) });
		}
	}
}

int resize_window(int p) {
	//adjust the point according to where it went out of bound
	p = (WINDOW_WIDTH <= WINDOW_HEIGHT) ? p * WINDOW_WIDTH / W_SCALE : p * WINDOW_HEIGHT / W_SCALE;
	return p;
}

void display(void)
{
	//setups
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glColor3f(1.0f, 0.84f, 0);              /* draw in golden yellow */
	glPointSize(1.0);                     /* size of each point */

	/*if we just draw a point, we dont need to resize the window*/
	if (choice != 'A') {
		glLoadIdentity();
		glTranslatef((WINDOW_WIDTH / 2), (WINDOW_HEIGHT / 2), 0.0f);
	}
	glBegin(GL_POINTS);

	/* hardcode to test points if works
	glVertex2i(300, 300);             //draw a vertex here
	draw_circle(300, 300, 15);			//hardcode to call draw circle
	draw_circle(300, 300, 50);
	draw_circle(300, 300, 100);
	draw_circle(100, 80, 80);
	*/

	//user's input three points and to test or read points from files to draw
	for (const vector<int>& i : points) {
		int r_rate = i[2];
		if (choice == 'C')
		{
			//if in animation choice, change the speed of growing radius
			r_rate = r_rate * frame_size / 1000;
		}
		draw_circle(i[0], i[1], r_rate);
	}

	glEnd();
	glFlush();                            /* render graphics */
	glutSwapBuffers();                    /* swap buffers */
}

void myinit()
{
	glClearColor(0.0f, 0.1f, 0.0f, 1.0f);    /* Dark green background*/

	/* set up viewing */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
}


void draw_circle(int x, int y, int r) {
	/*Set up for A(r,0) to start, d is decision parameter*/
	int x_offset = r, y_offset = 0, d = 1 - r;
	/*Initial eight points corresponding to x and y*/
	circlePoint(x, y, x_offset, y_offset);

	/*for each iteartion, increase y value by 1 and use the formula i derived in part a*/
	while (y_offset < x_offset) {
		y_offset++;
		/*formula from part a*/
		if (d >= 0) {
			x_offset--;
			d += 2 * y_offset - 2 * x_offset + 1;
		}
		else
		{
			d += 2 * y_offset + 1;
		}
		circlePoint(x, y, x_offset, y_offset);
	}
}

void circlePoint(int x, int y, int x_offset, int y_offset) {
	//This function draw eight symmetric points in eight regions
	//corresponding to the input x and y
	glVertex2i(x + x_offset, y + y_offset);
	glVertex2i(x + x_offset, y - y_offset);
	glVertex2i(x - x_offset, y - y_offset);
	glVertex2i(x - x_offset, y + y_offset);
	glVertex2i(x + y_offset, y + x_offset);
	glVertex2i(x + y_offset, y - x_offset);
	glVertex2i(x - y_offset, y - x_offset);
	glVertex2i(x - y_offset, y + x_offset);
}

void idle(void)
{
	/*how fast do we want our frame to grow*/
	Sleep(10);
	frame_size++;
	/*Animation cycle repeats again when */
	if (frame_size == 1000) {
		frame_size = 1; 
	}
	glutPostRedisplay();
}
