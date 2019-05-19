/*
Author: Ziwei Zheng
netid: zz1456
date: 05/05/2019
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

typedef Angel::vec4  color4;
typedef Angel::vec3  point3;

#define pi 3.1415926535
#define MAX_SIZE 512

/*----------------------------Globals---------------------------*/
// texture variables
GLuint textures[2];
// we initialize it with no texture 0. later on modify it to 1 for line texture, 2 for checker texture
int sphereTexSwitch = 0;
//sphere has two texture directions: verticle (false), slanted (true)
bool sphere_texture_dir = false;
//sphere has two coordinate space: object space (false), eye space (true)
bool sphere_texture_space = false;
// initialize floor to true to display in the beginning
bool checkerFloor = true;
// initialize lattice
bool lattice_on = false, lattice_upright = true;

/*	Create checkerboard texture	*/
#define checkerImageWidth  32
#define checkerImageHeight 32
GLubyte Image[checkerImageHeight][checkerImageWidth][4];

//The function image_set_up() in the file texmap.c also generates a 1D stripe image
#define	stripeWidth 32
GLubyte stripeImage[4 * stripeWidth];

// general
GLuint program; /* shader program object id */
//GLuint sphere_buffer;   /* vertex buffer object id for cube */
GLuint flat_sphere_buffer; /* vertex buffer object id for flat sphere */
GLuint smooth_sphere_buffer; /* vertex buffer object id for smooth sphere */
GLuint axis_buffer; /* vertex buffer object id for axis */
GLuint floor_buffer; /* vertex buffer object id for floor */
GLuint sphere_shadow_buffer; /* vertex buffer object id for shadow*/

GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 50.0;

float angle = 0;
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

//fog has three states: 0 for linear, 1 for exponential, 2 for exponential square
int fog = 0;

point3* sphere_points; //depending on input file, we dont know the size of sphere points yet
vec3* flat_sphere_normals;
vec3* smooth_sphere_normals;
color4* sphere_colors;
color4* sphere_shadow_colors;
int sphere_NumVertices = -1;

point3 axis_point[] = {
	point3(0.0, 0.0, 0.0),
	point3(1.0, 0.0, 0.0),

	point3(0.0, 0.0, 0.0),
	point3(0.0, 1.0, 0.0),

	point3(0.0, 0.0, 0.0),
	point3(0.0, 0.0, 1.0),
};

color4 axis_color[] = {
	// red
	color4(1.0, 0.0, 0.0, 1.0),
	color4(1.0, 0.0, 0.0, 1.0),
	//magenta
	color4(1.0, 0.0, 1.0, 1.0),
	color4(1.0, 0.0, 1.0, 1.0),
	//blue
	color4(0.0, 0.0, 1.0, 1.0),
	color4(0.0, 0.0, 1.0, 1.0),
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
vec3 fnormal(0.0f, 1.0f, 0.0f);// = cross(floor_points[1] - floor_points[0], floor_points[5] - floor_points[0]);
vec3 floor_normals[] = {
	vec3(fnormal), vec3(fnormal), vec3(fnormal),
	vec3(fnormal), vec3(fnormal), vec3(fnormal)
};
color4 floor_colors[] = {
	color4(0.0, 1.0, 0.0, 1.0),
	color4(0.0, 1.0, 0.0, 1.0),
	color4(0.0, 1.0, 0.0, 1.0),

	color4(0.0, 1.0, 0.0, 1.0),
	color4(0.0, 1.0, 0.0, 1.0),
	color4(0.0, 1.0, 0.0, 1.0)
};
vec2 floor_texCoord[] = {
	vec2(5.0, 6.0),
	vec2(5.0, 0.0),
	vec2(0.0, 0.0),

	vec2(5.0, 6.0),
	vec2(0.0, 0.0),
	vec2(0.0, 6.0)
};

const point3 A(-4, 1, 4), B(-1, 1, -4), C(3, 1, 5); // three turning points
// for animation
float tick = 0;
float total_ticks = 10000.0;
vec3 sphere_position = A, direction_vec;
mat4 sphere_rotation = identity();

//shadow and blending
bool if_shadow = true, if_blending = false;
const point3 L(-14.0, 12.0, -3.0);
mat4 sphere_shadow(L.y, 0.0f, 0.0f, 0.0f,
	-L.x, 0.0f, -L.z, -1.0f,
	0.0f, 0.0f, L.y, 0.0f,
	0.0f, 0.0f, 0.0f, L.y);

//Sphere and shadow draw mode
int solid_shadow = GL_FILL; // GL_LINE or GL_FILL
int animation_flag = 0; //0 - waiting to begin, 1 animation paused, 2 animation playing

//shader variables
int light_count = 2;
color4 global_ambient(1.0, 1.0, 1.0, 1.0);

//0: Ambient, 1: Distant, 2: Point, 3:Spot
int light_type2 = 3;
int light_type[] = { 1, light_type2 };

//directional light source
float ambient_light[] = { 0.0, 0.0, 0.0, 1.0,
						0.0, 0.0, 0.0, 1.0 }; //black ambient color
float diffuse_light[] = { 0.8, 0.8, 0.8, 1.0,
						1.0, 1.0, 1.0, 1.0 };
float specular_light[] = { 0.2, 0.2, 0.2, 1.0,
						1.0, 1.0, 1.0, 1.0 };

//spot light: the exponent value 15.0 and the cutoff angle 20.0
float exponent[] = { 0.0, 15.0 };
float cutoff[] = { 0.0, 20.0 };

vec4 light_position[] = { vec4(), vec4(-14.0, 12.0, -3.0, 1.0) }; //In world coordinate system
vec3 light_dir[] = { vec3(0.1, 0.0, -1.0), //in eye coordinate system
	vec3(-6.0, 0.0, -4.5) }; //Spotlight focus, in world coordinate system

/*-------------------attenuation variables---------------------*/
float const_att[] = { 0.0, 2.0 };
float linear_att[] = { 0.0, 0.01 };
float quad_att[] = { 0.0, 0.001 };

//set ground lighting color variables according to hw3 instruction
color4 ambient_ground(0.2, 0.2, 0.2, 1.0);
color4 diffuse_ground(0.0, 1.0, 0.0, 1.0); //green
color4 specular_ground(0.0, 0.0, 0.0, 1.0);
color4 global_ground_product = global_ambient * ambient_ground;
//initialize global ground variables
float ambient_ground_product[2 * 4];
float diffuse_ground_product[2 * 4];
float specular_ground_product[2 * 4];
//set sphere lighting color variables according to hw3 instruction
color4 ambient_sphere(0.2, 0.2, 0.2, 1.0);
color4 diffuse_sphere(1.0, 0.84, 0.0, 1.0); //yellow diffuse
color4 specular_sphere(1.0, 0.84, 0.0, 1.0); //yellow ambient color
float shininess = 125.0; //shininess coefficient
color4 global_sphere_product = global_ambient * ambient_sphere;
//initialize global sphere variables
float ambient_sphere_product[2 * 4];
float diffuse_sphere_product[2 * 4];
float specular_sphere_product[2 * 4];


bool lighting = true, flat = false, sphere_lighting = true;

/*----------------------methods------------------------------*/
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
void uniform_lighting_setup(mat4 mv, color4 GlobalAmbientProduct, float* AmbientProduct, float* DiffuseProduct, float* SpecularProduct);
void loadSphereFile();

// function image_set_up is from texmap.c
void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < checkerImageHeight; i++)
		for (j = 0; j < checkerImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* white */
			{
				c = 255;
				Image[i][j][0] = (GLubyte)c;
				Image[i][j][1] = (GLubyte)c;
				Image[i][j][2] = (GLubyte)c;
			}
			else  /* green */
			{
				Image[i][j][0] = (GLubyte)0;
				Image[i][j][1] = (GLubyte)150;
				Image[i][j][2] = (GLubyte)0;
			}

			Image[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/
	for (j = 0; j < stripeWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
		*/
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j > 4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
		  init() (set up texture objects),
		  display() (activate the texture object to be used, etc.)
		  and in shaders.
	 ---*/

} /* end function */


// a class for particle to produce a firework effect using shaders
class ParticleSystem {
public:
	ParticleSystem() {
		// numParticles = N particles
		velocity = new point3[numParticles];
		color = new color4[numParticles];
	}

	void init() {
		glGenBuffers(1, &particle_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(color[0]) * numParticles + sizeof(velocity[0]) * numParticles,
			NULL, GL_STATIC_DRAW);

		particleProgram = InitShader("vshader53Particle.glsl", "fshader53Particle.glsl");
	}

	void animate() {
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
		for (int i = 0; i < numParticles; i++) {
			//assign each particle a random velocity and a random color
			velocity[i].x = 2.0*((rand() % 256) / 256.0 - 0.5);
			velocity[i].y = 1.2*2.0*((rand() % 256) / 256.0);
			velocity[i].z = 2.0*((rand() % 256) / 256.0 - 0.5);

			color[i].x = (rand() % 256) / 256.0;
			color[i].y = (rand() % 256) / 256.0;
			color[i].z = (rand() % 256) / 256.0;
			color[i].w = 1.0;
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(velocity[0]) * numParticles, velocity);

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(velocity[0]) * numParticles,
			sizeof(color[0]) * numParticles, color);

		initialTime = (float)glutGet(GLUT_ELAPSED_TIME);
	}

	void draw(mat4& modelview, mat4& projection)
	{
		if (!active) return;

		glUseProgram(particleProgram);
		//--- Activate the vertex buffer object to be drawn ---//
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
		// set up view matrix and projection matrix
		GLuint mv = glGetUniformLocation(particleProgram, "ModelView");
		GLuint p = glGetUniformLocation(particleProgram, "Projection");

		glUniformMatrix4fv(mv, 1, GL_TRUE, modelview);
		glUniformMatrix4fv(p, 1, GL_TRUE, projection);
		// set up initial position
		glUniform1f(glGetUniformLocation(particleProgram, "t"), t);
		glUniform3fv(glGetUniformLocation(particleProgram, "initialPos"), 1, initialPosition);

		GLuint v = glGetAttribLocation(particleProgram, "vVelocity");
		glEnableVertexAttribArray(v);
		glVertexAttribPointer(v, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));

		GLuint color = glGetAttribLocation(particleProgram, "vColor");
		glEnableVertexAttribArray(color);
		glVertexAttribPointer(color, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(velocity[0]) * numParticles));
		// point size = 3.0
		glPointSize(3.0);
		glDrawArrays(GL_POINTS, 0, numParticles);

		glDisableVertexAttribArray(v);
		glDisableVertexAttribArray(color);
	}

	void activateParticle(bool a) {
		if (a && !active) {
			animate();
		}
		active = a;
	}

	void update() {
		if (!active) return;

		t = (float)glutGet(GLUT_ELAPSED_TIME) - initialTime;
		int counter = 0;
		//at each frame all particles have the same t.
		for (int i = 0; i < numParticles; i++) {
			if (initialPosition.y + 0.001 * velocity[i].y * t + 0.5 * -0.00000049 * t * t < 0.1) {
				counter++;
			}
		}

		if (t > 10000 || counter > numParticles - 10)
			animate();

	}
private:
	point3 initialPosition = point3(0.0, 0.1, 0.0);
	point3* velocity;
	color4* color;
	GLuint particle_buffer, particleProgram;

	float initialTime, t;
	int numParticles = 300;
	bool active = false;
};
// global firework variable
ParticleSystem firework;

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
	image_set_up();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//initialize firework particle system
	firework.init();
	// generate objects
	glGenTextures(2, textures);
	// set an active texture's unit to 0, and bind texture to the unit
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	// texture processing using repear and nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// pass in the texture global we created
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkerImageWidth, checkerImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
	// set an active texture's unit to 1, and bind texture to the unit
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, textures[1]);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeWidth,
		0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	//Create and initialize a vertex buffer object for axis, to be used in display()
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(axis_point) * 2 + sizeof(axis_color),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(axis_point), axis_point);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(axis_point),
		sizeof(axis_color), axis_color);

	//Create and initialize a vertex buffer object for floor, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals) + sizeof(floor_texCoord),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(floor_points),
		sizeof(floor_normals), floor_normals);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(floor_points) + sizeof(floor_normals),
		sizeof(floor_colors), floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals),
		sizeof(floor_texCoord), floor_texCoord);

	//Create and initialize a vertex buffer object for flat sphere, to be used in display()
	glGenBuffers(1, &flat_sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, flat_sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3 + sizeof(flat_sphere_normals[0]) * sphere_NumVertices * 3 + sizeof(sphere_colors[0]) * sphere_NumVertices * 3,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3,
		sizeof(flat_sphere_normals[0]) * sphere_NumVertices * 3, flat_sphere_normals);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3 + sizeof(flat_sphere_normals[0]) * sphere_NumVertices * 3,
		sizeof(sphere_colors[0]) * sphere_NumVertices * 3, sphere_colors);

	//Create and initialize a vertex buffer object for smooth sphere, to be used in display()
	glGenBuffers(1, &smooth_sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, smooth_sphere_buffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3 + sizeof(smooth_sphere_normals[0]) * sphere_NumVertices * 3 + sizeof(sphere_colors[0]) * sphere_NumVertices * 3,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3,
		sizeof(smooth_sphere_normals[0]) * sphere_NumVertices * 3, smooth_sphere_normals);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3 + sizeof(smooth_sphere_normals[0]) * sphere_NumVertices * 3,
		sizeof(sphere_colors[0]) * sphere_NumVertices * 3, sphere_colors);


	//Create and initialize a vertex buffer object for sphere's, to be used in display()
	glGenBuffers(1, &sphere_shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3 + sizeof(sphere_colors[0]) * sphere_NumVertices * 3,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(sphere_points[0]) * sphere_NumVertices * 3,
		sizeof(sphere_shadow_colors[0]) * sphere_NumVertices * 3, sphere_shadow_colors);

	//Set up products
	for (int i = 0; i < light_count * 4; i++) {
		ambient_sphere_product[i] = ambient_light[i] * ambient_sphere[i % 4];
		diffuse_sphere_product[i] = diffuse_light[i] * diffuse_sphere[i % 4];
		specular_sphere_product[i] = specular_light[i] * specular_sphere[i % 4];

		ambient_ground_product[i] = ambient_light[i] * ambient_ground[i % 4];
		diffuse_ground_product[i] = diffuse_light[i] * diffuse_ground[i % 4];
		specular_ground_product[i] = specular_light[i] * specular_ground[i % 4];
	}

	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader53.glsl", "fshader53.glsl");

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.529, 0.807, 0.92, 0.0);
	glLineWidth(2.0);
}

//---------------------------------------------------------
void draw(GLuint buffer, int num_vertices, int mode = GL_TRIANGLES, bool lighting = false, bool normal = false, int texture = 0)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glUniform1i(glGetUniformLocation(program, "lighting"), lighting);
	glUniform1i(glGetUniformLocation(program, "texture_flag"), texture);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	/*----- Set up vertex attribute arrays for normal vertex attribute -----*/
	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	int NormalSize = 0;
	if (normal) {
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * num_vertices));
		NormalSize = sizeof(vec3) * num_vertices;
	}

	/*----- Set up color buffer attribute arrays for each vertex attribute -----*/
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point3) * num_vertices + NormalSize));

	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	// texture option is enabled
	if (texture) {
		glEnableVertexAttribArray(vTexCoord);
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * num_vertices + NormalSize + sizeof(color4) * num_vertices));
	}

	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	(using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(mode, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(vTexCoord);
}
//---------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//** Important: glUseProgram() must be called *before* any shader variable
	//              locations can be retrieved. This is needed to pass on values to
	//              uniform/attribute variables in shader ("variable binding" in 
	//              shader).
	glUseProgram(program); //use shader program

	GLuint  model_view;  // model-view matrix uniform shader variable location
	GLuint  projection;  // projection matrix uniform shader variable location
	// Retrieve transformation uniform variable locations
	// ** Must be called *after* glUseProgram().
	model_view = glGetUniformLocation(program, "ModelView");
	projection = glGetUniformLocation(program, "Projection");

	//Set up Projection Matrix
	mat4 p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); //GL_TRUE: matrix is row-major

	//Set up camera orientation
	vec4	at(0.0, 0.0, 0.0, 1.0);//at(-7.0, -3.0, 10.0, 0.0);
	vec4    up(0.0, 1.0, 0.0, 0.0);
	mat4 mv = LookAt(eye, at, up);

	//Fog option
	glUniform1i(glGetUniformLocation(program, "Fog"), fog);

	//Must be called after mv for light position is set up
	if (lighting) uniform_lighting_setup(mv, global_ground_product, ambient_ground_product, diffuse_ground_product, specular_ground_product);
	glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
	glUniform1i(glGetUniformLocation(program, "texDimension"), 2);

	if (if_shadow && eye.y >= 0) {

		//----------FLOOR IN FRAME BUFFER----------
		//Disable drawing to Z
		glDepthMask(GL_FALSE);
		//
		// set up model view matrix
		mv = LookAt(eye, at, up);
		mv = mv * Translate(0.0, 0.0, 0.0) * Scale(1.0, 1.0, 1.0);// * Rotate(0.0, 0.0, 0.0, 0.0);
		//Set up material for floor
		mat3 normal_matrix = NormalMatrix(mv, 1); // 1: model_view involves non-uniform scaling, 0: otherwise, 1 is always correct, 0 is faster
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

		//Draw Floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Wireframe mode, GL_FILL to fill
		draw(floor_buffer,
			sizeof(floor_points) / sizeof(floor_points[0]), GL_TRIANGLES, lighting, true, checkerFloor);

		//----------SPHERE SHADOW---------

		if (if_blending) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
		}

		glUniform1i(glGetUniformLocation(program, "sphere"), 1);
		mv = LookAt(eye, at, up);
		mv = mv * sphere_shadow * Translate(sphere_position) * Scale(1.0, 1.0, 1.0) * sphere_rotation;
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);

		glPolygonMode(GL_FRONT_AND_BACK, solid_shadow);
		draw(sphere_shadow_buffer,
			sphere_NumVertices * 3);

		//Disable drawing to frame buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//
		glUniform1i(glGetUniformLocation(program, "sphere"), 0);

		glDisable(GL_BLEND);
		//Enable drawing to Z
		glDepthMask(GL_TRUE);
		//
	}

	//----------FLOOR IN DEPTH BUFFER----------
	// set model view matrix
	mv = LookAt(eye, at, up);
	mv = mv * Translate(0.0, 0.0, 0.0) * Scale(1.0, 1.0, 1.0);// * Rotate(0.0, 0.0, 0.0, 0.0);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

	if (!if_shadow || eye.y < 0) {
		mat3 normal_matrix = NormalMatrix(mv, 1); // 1: model_view involves non-uniform scaling, 0: otherwise, 1 is always correct, 0 is faster
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

	}

	//floor
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Wireframe mode, GL_FILL to fill
	draw(floor_buffer,
		sizeof(floor_points) / sizeof(floor_points[0]), GL_TRIANGLES, lighting, true, checkerFloor);

	//Enable drawing to framebuffer
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//axis
	mv = LookAt(eye, at, up);
	mv = mv * Translate(0.0, 0.0, 0.0) * Scale(10.0, 10.0, 10.0);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode, GL_FILL to fill
	draw(axis_buffer, sizeof(axis_point) / sizeof(axis_point[0]), GL_LINES);

	//sphere
	mv = LookAt(eye, at, up);
	if (lighting) uniform_lighting_setup(mv, global_sphere_product, ambient_sphere_product, diffuse_sphere_product, specular_sphere_product);
	if (sphereTexSwitch == 1) {
		glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
		glUniform1i(glGetUniformLocation(program, "texDimension"), 1);
	}
	else if (sphereTexSwitch == 2) {
		glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
		glUniform1i(glGetUniformLocation(program, "texDimension"), 2);
	}
	glUniform1i(glGetUniformLocation(program, "sphere_texture_dir"), sphere_texture_dir);
	glUniform1i(glGetUniformLocation(program, "sphere_texture_space"), sphere_texture_space);
	glUniform1i(glGetUniformLocation(program, "calculate_texCoord"), 1);
	glUniform1i(glGetUniformLocation(program, "sphere"), 1);
	glUniform1i(glGetUniformLocation(program, "lattice_on"), lattice_on);
	glUniform1i(glGetUniformLocation(program, "lattice_upright"), lattice_upright);

	mv = mv * Translate(sphere_position) * Scale(1.0, 1.0, 1.0) * sphere_rotation;//Rotate(angle, direction_vec.z, -direction_vec.y, -direction_vec.x);
	mat3 normal_matrix = NormalMatrix(mv, 1); // 1: model_view involves non-uniform scaling, 0: otherwise, 1 is always correct, 0 is faster
	glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	glPolygonMode(GL_FRONT_AND_BACK, solid_shadow);

	if (flat) {
		draw(flat_sphere_buffer, sphere_NumVertices * 3, GL_TRIANGLES, lighting && sphere_lighting, true, sphereTexSwitch);
	}
	else {
		draw(smooth_sphere_buffer, sphere_NumVertices * 3, GL_TRIANGLES, lighting && sphere_lighting, true, sphereTexSwitch);
	}
	glUniform1i(glGetUniformLocation(program, "calculate_texCoord"), 0);
	glUniform1i(glGetUniformLocation(program, "sphere"), 0);

	//Particle System Draw
	mv = LookAt(eye, at, up);
	firework.draw(mv, p);

	glutSwapBuffers();
}

//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv):
// Set up lighting parameters that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------
void uniform_lighting_setup(mat4 mv, color4 GlobalAmbientProduct, float* AmbientProduct, float* DiffuseProduct, float* SpecularProduct) {
	glUniform4fv(glGetUniformLocation(program, "GlobalAmbientProduct"), 1, GlobalAmbientProduct);
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), light_count, AmbientProduct);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), light_count, DiffuseProduct);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), light_count, SpecularProduct);
	glUniform1fv(glGetUniformLocation(program, "Exponent"), 2, exponent);
	glUniform1fv(glGetUniformLocation(program, "Cutoff"), 2, cutoff);
	glUniform1i(glGetUniformLocation(program, "LightCount"), light_count);

	//The Light Position in Eye Frame
	float li_pos[2 * 4];
	for (int i = 0; i < light_count; i++) {
		vec4 light_position4(light_position[i].x, light_position[i].y, light_position[i].z, 1.0);
		vec4 light_position_eyeFrame = mv * light_position4;
		li_pos[i * 4 + 0] = light_position_eyeFrame.x;
		li_pos[i * 4 + 1] = light_position_eyeFrame.y;
		li_pos[i * 4 + 2] = light_position_eyeFrame.z;
		li_pos[i * 4 + 3] = light_position_eyeFrame.w;
	}

	//Light direction in Eye Frame
	vec4 light_dir4(light_dir[1].x, light_dir[1].y, light_dir[1].z, 1.0);
	vec4 light_dir_eye = mv * light_dir4;
	float li_dir[] = { light_dir[0].x, light_dir[0].y, light_dir[0].z,
		light_dir_eye.x, light_dir_eye.y, light_dir_eye.z };
	//pass calculated variables to vertext shader
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), light_count, li_pos);
	glUniform3fv(glGetUniformLocation(program, "LightDirection"), light_count, li_dir);
	glUniform1iv(glGetUniformLocation(program, "LightType"), light_count, light_type);
	glUniform1fv(glGetUniformLocation(program, "ConstAtt"), 2, const_att);
	glUniform1fv(glGetUniformLocation(program, "LinearAtt"), 2, linear_att);
	glUniform1fv(glGetUniformLocation(program, "QuadAtt"), 2, quad_att);
	glUniform1f(glGetUniformLocation(program, "Shininess"), shininess);
}

void idle(void)
{
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
	point3 sphere_old_position = sphere_position;
	sphere_position = lerp(start, dest, percent);

	angle = find_dist(sphere_old_position, sphere_position) * 180.0 / pi;
	direction_vec = sphere_position - sphere_old_position;

	vec3 rotation = cross(vec3(0.0f, 1.0f, 0.0f), direction_vec); //Floor normal cross direction vec
	sphere_rotation = Rotate(angle, rotation.x, rotation.y, rotation.z) * sphere_rotation;
	// constantly update firework's position
	firework.update();

	glutPostRedisplay();
}

// read input file drom the user, and store data into sphere_points
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
	int index = 0, point_count = 0;
	int sphere_size;
	int pos;
	float number;

	while (file >> number) {
		if (sphere_NumVertices == -1) {
			// first number is total number of triangles 
			sphere_NumVertices = number;
			sphere_points = new point3[number * 3];
		}
		else {
			// take in 3 numbers at a time for x, y, z values
			pos = index % 3;
			if (pos == 0)
				x = number;
			else if (pos == 1)
				y = number;
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
	sphere_colors = new color4[sphere_size];
	// we want to initialize the normals
	//Flat sphere
	flat_sphere_normals = new vec3[sphere_size];
	for (int i = 0; i < sphere_size; i += 3) {
		vec3 u = sphere_points[i + 1] - sphere_points[i],
			v = sphere_points[i + 2] - sphere_points[i];
		//find normal vector thats perpendicular to u and v
		vec3 perpendicular_normal = normalize(cross(u, v));
		//all three vertices of the sphere has the same normal vector
		flat_sphere_normals[i] = flat_sphere_normals[i + 1] = flat_sphere_normals[i + 2] = perpendicular_normal;
	}
	//Smooth sphere
	smooth_sphere_normals = new vec3[sphere_size];
	for (int i = 0; i < sphere_size; i++)
		smooth_sphere_normals[i] = normalize(sphere_points[i]);

	//Initialize colors
	for (int i = 0; i < sphere_size; i++)
		sphere_colors[i] = color4(1.0, 0.84, 0.0, 1.0);

	//Initialize sphere shadow colors
	sphere_shadow_colors = new color4[sphere_size];
	for (int i = 0; i < sphere_size; i++)
		sphere_shadow_colors[i] = color4(0.25, 0.25, 0.25, 0.65);
}
// User-interface capabilities
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		// eye view options
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
		// animation option
	case 'b': case 'B':
		if (animation_flag == 0) {
			glutIdleFunc(idle);
			animation_flag = 2;
		}
		break;
		// sphere texture options
	case 'e': case 'E':
		sphere_texture_space = 1;
		break;
	case 'o': case 'O':
		sphere_texture_space = 0;
		break;
	case 's': case 'S':
		sphere_texture_dir = 1;
		break;
	case 'v': case 'V':
		sphere_texture_dir = 0;
		break;
		// lattice options
	case 'u': case 'U':
		lattice_upright = true;
		break;
	case 't': case 'T':
		lattice_upright = false;
		break;
	case 'l': case 'L':
		lattice_on = !lattice_on;
		break;
	}
	glutPostRedisplay();
}

// mouse options
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
// main menu
void main_menu(int id)
{
	if (id == 1) {
		eye = init_eye;
	}
	else if (id == 2) {
		exit(0);
	}
	else if (id == 3) {
		if (solid_shadow == GL_FILL) {
			solid_shadow = GL_LINE;
			sphere_lighting = false;
		}
		else {
			solid_shadow = GL_FILL;
			sphere_lighting = true;
		}
	}
	glutPostRedisplay();
}

// shadow options
void shadow_menu(int id)
{
	if (id == 1) {
		if_shadow = true;
	}
	else if (id == 2) {
		if_shadow = false;
	}
	glutPostRedisplay();
}

void shading_menu(int id)
{
	if (id == 1) {
		flat = true;
	}
	else if (id == 2) {
		flat = false;
	}
	glutPostRedisplay();
}

void blending_shadow_menu(int id)
{
	if (id == 1) {
		if_blending = false;
	}
	if (id == 2) {
		if_blending = true;
	}
	glutPostRedisplay();
}

//light options
void light_menu(int id)
{
	if (id == 1) {
		lighting = true;
		if (solid_shadow != GL_LINE)
			sphere_lighting = true;
	}
	else if (id == 2) {
		lighting = false;
		sphere_lighting = false;
	}
	glutPostRedisplay();
}

// spot light or point source option
void lightsource_menu(int id)
{
	if (id == 1) {
		light_type[1] = 3;
	}
	else if (id == 2) {
		light_type[1] = 2;
	}
	glutPostRedisplay();
}


void checker_menu(int id)
{
	if (id == 1) checkerFloor = false;
	else if (id == 2) checkerFloor = true;
	glutPostRedisplay();
}
// switch between line and checker according to id
void sphere_texture_menu(int id)
{
	sphereTexSwitch = id;
	glutPostRedisplay();
}

void particle_menu(int id)
{
	firework.activateParticle(id);
	glutPostRedisplay();
}

void fog_menu(int id) {
	if (id == 1) { // if there is no fog
		fog = 0;
	}
	else if (id == 2) { // if linear fog
		fog = 1;
	}
	else if (id == 3) { // if exponential fog
		fog = 2;
	}
	else if (id == 4) { // if exponential square
		fog = 3;
	}
	glutPostRedisplay();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}


//---------------------------------------------------------
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
	glutIdleFunc(NULL); //sphere waiting for user to input rolling
	glutMouseFunc(myMouse);
	glutKeyboardFunc(keyboard);

	// create mouse menu options
	int shadowMenu = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);
	// different sphere surface texture
	int shadingMenu = glutCreateMenu(shading_menu);
	glutAddMenuEntry("Flat shading", 1);
	glutAddMenuEntry("Smoothing shading", 2);
	// blend shadow option
	int shadowBlendingMenu = glutCreateMenu(blending_shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);
	// enable lighting
	int lightMenu = glutCreateMenu(light_menu);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);
	// light type
	int lightSourceMenu = glutCreateMenu(lightsource_menu);
	glutAddMenuEntry("Spot Light", 1);
	glutAddMenuEntry("Point Source", 2);
	// fog options
	int fogMenu = glutCreateMenu(fog_menu);
	glutAddMenuEntry("No Fog", 1);
	glutAddMenuEntry("Linear", 2);
	glutAddMenuEntry("Exponential", 3);
	glutAddMenuEntry("Exponential Square", 4);
	// different texture options
	int sphereTexMenu = glutCreateMenu(sphere_texture_menu);
	glutAddMenuEntry("No", 0);
	glutAddMenuEntry("Yes - Contour Lines", 1);
	glutAddMenuEntry("Yes - Checkerboard", 2);
	// ground checker texture
	int checkerMenu = glutCreateMenu(checker_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);
	// firework effect
	int particleMenu = glutCreateMenu(particle_menu);
	glutAddMenuEntry("No", 0);
	glutAddMenuEntry("Yes", 1);
	// mouse menu
	glutCreateMenu(main_menu);
	glutAddMenuEntry("Toggle wire frame sphere", 3);
	glutAddSubMenu("Shadow", shadowMenu);
	glutAddSubMenu("Shading", shadingMenu);
	glutAddSubMenu("Light Source", lightSourceMenu);
	glutAddSubMenu("Enable Lighting", lightMenu);
	// newly added
	glutAddSubMenu("Blending Shadow", shadowBlendingMenu);
	glutAddSubMenu("Fog Options", fogMenu);
	glutAddSubMenu("Texture Mapped Ground", checkerMenu);
	glutAddSubMenu("Texture Mapped Sphere", sphereTexMenu);
	glutAddSubMenu("Firework", particleMenu);
	glutAddMenuEntry("Default View Point", 1);
	glutAddMenuEntry("Quit", 2);
	glutAttachMenu(GLUT_LEFT_BUTTON);

	init();
	glutMainLoop();
	return 0;
}