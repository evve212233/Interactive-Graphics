/*
Author: Ziwei Zheng
netid: zz1456
date: 04/15/2019 
*/

/************************************************************
 * Moodified by Yi-Jen Chiang to include the use of a general rotation function
   Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
   and also to include the use of the function NormalMatrix(mv) to return the
   normal matrix (mat3) of a given model-view matrix mv (mat4).

   (The functions Rotate() and NormalMatrix() are added to the file "mat-yjc-new.h"
   by Yi-Jen Chiang, where a new and correct transpose function "transpose1()" and
   other related functions such as inverse(m) for the inverse of 3x3 matrix m are
   also added; see the file "mat-yjc-new.h".)

 * Extensively modified by Yi-Jen Chiang for the program structure and user
   interactions. See the function keyboard() for the keyboard actions.
   Also extensively re-structured by Yi-Jen Chiang to create and use the new
   function drawObj() so that it is easier to draw multiple objects. Now a floor
   and a rotating cube are drawn.

** Perspective view of a color cube using LookAt() and Perspective()

** Colors are assigned to each vertex and then the rasterizer interpolates
   those colors across the triangles.
**************************************************************/
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Angel-yjc.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <math.h>

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

#define pi 3.1415926535
#define MAX_SIZE 512

GLuint program;       /* shader program object id */
GLuint sphere_buffer;   /* vertex buffer object id for cube */
GLuint axis_buffer;		/*vertex buffer object id for axis*/
GLuint floor_buffer;  /* vertex buffer object id for floor */

GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 50.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int animation_flag = 0; //0. no animation(wait to start), 1 and 2 animation

#if 0
point3 cube_points[cube_NumVertices]; // positions for all vertices
color3 cube_colors[cube_NumVertices]; // colors for all vertices
#endif
#if 1
point3* sphere_points; //depending on input file, we dont know the size of sphere points yet
color3* sphere_colors; //same as sphere points
#endif

int sphere_NumVertices = 0;

point3 axis_point[] = {
	point3(0.0, 0.0, 0.0),
	point3(1.0, 0.0, 0.0),

	point3(0.0, 0.0, 0.0),
	point3(0.0, 1.0, 0.0),

	point3(0.0, 0.0, 0.0),
	point3(0.0, 0.0, 1.0),
};

color3 axis_color[] = {
	// red
	color3(1.0, 0.0, 0.0),
	color3(1.0, 0.0, 0.0),
	//magenta
	color3(1.0, 0.0, 1.0),
	color3(1.0, 0.0, 1.0),
	//blue
	color3(0.0, 0.0, 1.0),
	color3(0.0, 0.0, 1.0),
};

// draw a quadrilateral in color green (i.e., (0, 1, 0)) with vertices at
// (5, 0, 8, 1), (5, 0, −4, 1), (−5, 0, −4, 1), and (−5, 0, 8, 1)
point3 floor_points[] = {
	//first triangle
	point3(5.0, 0.0, 8.0),
	point3(5.0, 0.0, -4.0),
	point3(-5.0, 0.0, -4.0),
	//second triangle
	point3(5.0, 0.0, 8.0),
	point3(-5.0, 0.0, -4.0),
	point3(-5.0, 0.0, 8.0)
};

point3 floor_colors[] = {
	// color green
	point3(0.0, 1.0, 0.0),
	point3(0.0, 1.0, 0.0),
	point3(0.0, 1.0, 0.0),

	point3(0.0, 1.0, 0.0),
	point3(0.0, 1.0, 0.0),
	point3(0.0, 1.0, 0.0),
};


const point3 A(-4, 1, 4), B(-1, 1, -4), C(3, 1, 5);
// for animation
float tick = 0;
float total_ticks = 10000.0;
vec3 sphere_position = A, direction_vec;
mat4 sphere_rotation = identity();

vec3 lerp(const vec3& begin, const vec3& end, float percent) {
	return vec3(begin.x + (end.x - begin.x) * percent,
		begin.y + (end.y - begin.y) * percent,
		begin.z + (end.z - begin.z) * percent);
}
mat4 lerp(const mat4& begin, const mat4& end, float percent) {
	return begin + (end - begin) * percent;
}
float find_dist(const vec3& a, const vec3& b) {
	return sqrt((b.x - a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y) + (b.z - a.z)*(b.z - a.z));
}

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

void loadSphereFile();

void init()
{
	//Ask User to input file
	loadSphereFile();
#if 0 //YJC: The following is not needed
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif
	//Create and initialize a vertex buffer object for axis, to be used in display()
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(axis_point) + sizeof(axis_color),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(axis_point), axis_point);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(axis_point),
		sizeof(axis_color), axis_color);
	//Create and initialize a vertex buffer object for floow, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(floor_points) + sizeof(floor_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(floor_points),
		sizeof(floor_colors), floor_colors);

	//Create and initialize a vertex buffer object for sphere, to be used in display()
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
#if 0
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_point) + sizeof(axis_color),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_point), axis_color);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_point), sizeof(axis_color),
		cube_colors);
#endif
#if 1
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3 + sizeof(sphere_colors[0]) * sphere_NumVertices * 3,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3,
		sizeof(sphere_colors[0]) * sphere_NumVertices * 3, sphere_colors);
#endif
	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader42.glsl", "fshader42.glsl");

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.529, 0.807, 0.92, 0.0);
	glLineWidth(2.0);
}
void drawObj(GLuint buffer, int num_vertices, int mode = GL_TRIANGLES)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point3) * num_vertices));

	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	(using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(mode, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}
//---------------------------------------------------------
void display(void)
{
	GLuint  model_view;  // model-view matrix uniform shader variable location
	GLuint  projection;  // projection matrix uniform shader variable location

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program); // use shader program

	model_view = glGetUniformLocation(program, "model_view");
	projection = glGetUniformLocation(program, "projection");

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4 p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); //GL_TRUE: matrix is row-major

	//Set up camera orientation
	//vec4	at(-7.0, -3.0, 10.0, 0.0); part b
	vec4	at(0.0, 0.0, 0.0, 1.0);
	vec4    up(0.0, 1.0, 0.0, 0.0);

	mat4 mv; //Model-view matrix

	// Axis
	mv = LookAt(eye, at, up);
	mv = mv * Translate(0.0, 0.0, 0.0) * Scale(5.0, 5.0, 8.0); //transformation
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe axis
	drawObj(axis_buffer, sizeof(axis_point) / sizeof(axis_point[0]), GL_LINES);

	// Floor
	mv = LookAt(eye, at, up);
	mv = mv * Translate(0.0, 0.0, 0.0) * Scale(1.0, 1.0, 1.0);// * Rotate(0.0, 0.0, 0.0, 0.0);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

	// Draw Floor
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Filled floor
	// for hw3, we can set floor flag here to fill to floor or wireframe the floor.
	drawObj(floor_buffer, sizeof(floor_points) / sizeof(floor_points[0]));

	//Sphere
	mv = LookAt(eye, at, up);
	mv = mv * Translate(sphere_position) * Scale(1.0, 1.0, 1.0) * sphere_rotation;//transformation of sphere
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode, GL_FILL to fill
	drawObj(sphere_buffer, sphere_NumVertices * 3); // for hw3, we can set sphere flag here to fill to sphere or wireframe the sphere.
	glutSwapBuffers();
}
// for animation
void idle(void)
{
	point3 old_position;
	float percent;
	tick++;
	int phase_num = (int)(tick / total_ticks) % 3;
	vec3 start, dest;

	if (phase_num == 0) {
		start = A;
		dest = B;
	}
	else if (phase_num == 1) {
		start = B;
		dest = C;
	}
	else {
		start = C;
		dest = A;
	}
	percent = (float)((int)tick % (int)total_ticks) / total_ticks;
	old_position = sphere_position;
	sphere_position = lerp(start, dest, percent);

	angle = find_dist(old_position, sphere_position) * 180.0 / pi;
	direction_vec = sphere_position - old_position;

	vec3 rotation = cross(vec3(0.0f, 1.0f, 0.0f), direction_vec); //Floor normal cross direction vec
	sphere_rotation = Rotate(angle, rotation.x, rotation.y, rotation.z) * sphere_rotation;

	glutPostRedisplay();
}

// User-interface capabilities
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 'b': case 'B':
			if (animation_flag == 0) {
				glutIdleFunc(idle);
				animation_flag = 2;
			}
			break;
		case 'x':
			eye[0] -= 1.0;
			break;
		case 'X': 
			eye[0] += 1.0; 
			break;
		case 'y':
			eye[1] -= 1.0;
			break;
		case 'Y': 
			eye[1] += 1.0; 
			break;
		case 'z':
			eye[2] -= 1.0;
			break;
		case 'Z': 
			eye[2] += 1.0; 
			break;
	}
	glutPostRedisplay();
}


// read input file drom the user, and store data into  sphere_points
void loadSphereFile()
{
	std::ifstream file;
	char file_path[MAX_SIZE];
	std::cout << "\nWelcome to my sphere program\n";
	while (!file.is_open()) {
		std::cout << "Please enter your file name to draw: ";
		// get file name from user
		std::cin >> file_path;
		// open the file
		file.open(file_path);
		//check if valid file path
		if (!file.is_open()) {
			std::cout << "\nCannot find file name, please try again\n";
		}
	}

	GLfloat x, y, z;
	int sphere_size;
	int index = 0;
	int point_count = 0;
	int pos;
	float number;
	while (file >> number) {
		// std::cout << reader;
		if (sphere_NumVertices == 0) {
			// first number is total number of triangles 
			sphere_NumVertices = number;
			sphere_points = new point3[number * 3];
		}
		else {
			// take in 3 numbers at a time for x, y, z values
			pos = index % 3;
			if (pos == 0) {
				x = number;
			}
			else if (pos == 1) {
				y = number;
			}
			else {
				z = number;
				sphere_points[point_count] = point3(x, y, z);
				point_count++;
			}
			index++;
		}
		if (index % 3 == 0 && point_count % 3 == 0)
			file >> number;
	}
	// now we have size of vertices we need to create sphere
	sphere_size = sphere_NumVertices * 3;
	sphere_colors = new color3[sphere_size];

	for (int i = 0; i < sphere_size; i++)
		sphere_colors[i] = color3(1.0, 0.84, 0); //golden yellow color
}

void main_menu(int id)
{
	if (id == 1) {
		eye = init_eye;
	}
	else if (id == 2) {
		exit(0);
	}
	glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			// menu
			break;
		case GLUT_RIGHT_BUTTON:
			// enable or disable rolling
			if (animation_flag == 2) {
				glutIdleFunc(NULL);
			}
			else if (animation_flag > 0) {
				glutIdleFunc(idle);
			}
			animation_flag = (animation_flag * 2) % 3;
			break;
		}
	}
	glutPostRedisplay();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowSize(512, 512);
	glutCreateWindow("Welcome to Rolling Sphere~(o^_^o)");

#ifdef __APPLE__ // on macOS
	// Core profile requires to create a Vertex Array Object (VAO).
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
	/* Call glewInit() and error checking */
	int err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}
#endif

	// Get info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(NULL);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(keyboard);

	// mouse menu
	glutCreateMenu(main_menu);
	glutAddMenuEntry("Default View Point", 1);
	glutAddMenuEntry("Quit", 2);
	glutAttachMenu(GLUT_LEFT_BUTTON);

	init();
	glutMainLoop();
	return 0;
}
