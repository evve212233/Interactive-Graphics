/* 
File Name: "vshader53.glsl":
Vertex shader: Modified by Ziwei Zheng
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

#define PI 3.1415926535897932384626433832795

in  vec3 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
out vec4 color;

uniform bool lighting; //allows two options for the menu: enable lighting or not
uniform int lighting_count;

uniform vec4 GlobalAmbientProduct; //will be using to set up a global ambient light with white color (1.0, 1.0, 1.0, 1.0) in source.cpp
uniform vec4 AmbientProduct[2 * 4], DiffuseProduct[2 * 4], SpecularProduct[2 * 4];
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition[2 * 4];
uniform vec3 LightDirection[2 * 3]; //directional light source
uniform int LightType[2]; //point light or spot light
uniform float Shininess;

uniform float Cutoff[2]; //spot light variables
uniform float Exponent[2]; //spot light variables

uniform float ConstAtt[2];  // Constant Attenuation
uniform float LinearAtt[2]; // Linear Attenuation
uniform float QuadAtt[2];   // Quadratic Attenuation

vec4 specularCheck(vec3 L, vec3 N, vec4 specular){
	if( dot(L, N) < 0.0 ) { // when perpendicular to each other
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}
	return specular;
}

vec4 calculateLight(int i, vec3 pos, vec3 eye){

	float att; //attenuation

	vec4 ambient; //ambient light
	vec4 diffuse; //diffuse light
	vec4 specular; //specular light

	if (LightType[i] == 0) // if ambient light
	{
		return AmbientProduct[i];
	}

	if (LightType[i] == 1) //if directional light
	{
		vec3 L = -1 * LightDirection[i]; // //light position in eye frame
		vec3 H = normalize(L + eye);
		vec3 N = normalize(Normal_Matrix * vNormal);

		att = 1.0; //set attenuation to 1.0
		// illumination calculations
		ambient = AmbientProduct[i];
		diffuse = max(dot(L, N), 0.0) * DiffuseProduct[i];
		specular = pow(max(dot(N, H), 0.0), Shininess) * SpecularProduct[i];
		specular = specularCheck(L,N,specular); //check whether need to reset specular to 0.0f depending on the dot product of L and N
	}

	if (LightType[i] == 2) //if point Light
	{
		vec3 light_pos = LightPosition[i].xyz; //light position in eye frame
		vec3 L = normalize( light_pos - pos );
		vec3 H = normalize( L + eye );
		vec3 N = normalize(Normal_Matrix * vNormal);
		// illumination calculations
		float dist = length(light_pos - pos);
		att = 1 / (ConstAtt[i] + LinearAtt[i] * dist + QuadAtt[i] * pow(dist, 2)); 
		ambient = AmbientProduct[i];
		diffuse = max(dot(L, N), 0.0) * DiffuseProduct[i];
		specular = pow(max(dot(N, H), 0.0), Shininess) * SpecularProduct[i];
		specular = specularCheck(L,N,specular);
	}

	if (LightType[i] == 3) //if spotlight
	{
		vec3 light_pos = LightPosition[i].xyz; //light position in eye frame
		vec3 L = normalize( light_pos - pos );
		vec3 H = normalize( L + eye );
		vec3 N = normalize(Normal_Matrix * vNormal);
		//illumination calculations
		float dist = length(light_pos - pos);
		att = 1 / (ConstAtt[i] + LinearAtt[i] * dist + QuadAtt[i] * pow(dist, 2)); 
		ambient = AmbientProduct[i];
		diffuse = max( dot(L, N), 0.0 ) * DiffuseProduct[i];
		specular = pow(max(dot(N, H), 0.0), Shininess) * SpecularProduct[i];
		specular = specularCheck(L,N,specular);
		// light direction is the same as the spot light local position
		vec3 center_focus = normalize(LightDirection[i].xyz - light_pos);
		float lenCF = dot(center_focus, -L);
		// calculate attenuation depending on dot product of spotlight center focus and -L, and cos of cutoff
		if (lenCF >= cos(Cutoff[i] * PI / 180.0)) att = att * pow(lenCF, Exponent[i]);
		else att = 0;

	}

	return att * (ambient + diffuse + specular);
}


void main()
{
	vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);

	if (lighting){
		 // Transform vertex position into eye coordinates
		vec3 pos = (ModelView * vPosition4).xyz;
		vec3 eye = normalize( -pos );

		color = GlobalAmbientProduct;

		for (int i = 0; i < lighting_count; i++){
			color += calculateLight(i, pos, eye);
		}
	}

	else{
		color = vColor;
	}

    gl_Position = Projection * ModelView * vPosition4;
}