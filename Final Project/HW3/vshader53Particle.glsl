/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec3 vVelocity;
in  vec4 vColor;
out vec4 color;
out float objPositionY;

uniform float t;
uniform vec3 initialPos;
uniform mat4 ModelView;
uniform mat4 Projection;
void main()
{
	float a = -0.00000049;
	//final formula for y
	vec4 currPosition = vec4(initialPos.x + 0.001 * vVelocity.x * t,
		initialPos.y + 0.001 * vVelocity.y * t + 0.5 * a * t * t , 
		initialPos.z + 0.001 * vVelocity.z * t, 1.0);
	//at each frame all particles have the same t
	color = vColor;

	objPositionY = currPosition.y;
    gl_Position = Projection * ModelView * currPosition;
}