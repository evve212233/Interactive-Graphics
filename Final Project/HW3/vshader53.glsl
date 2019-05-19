/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

#define PI 3.1415926535897932384626433832795

in  vec3 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
in	vec2 vTexCoord;
out vec4 color;
out vec4 fPosition;
out float fZ;
out	vec2 fTexCoord;
out float fTexCoord1D;
flat out int fFog;

uniform int Fog;

uniform bool sphere_texture_dir;
uniform bool sphere_texture_space;
uniform bool calculate_texCoord;
uniform int texDimension;

uniform bool lattice_on;
uniform bool lattice_upright;
out vec2 fLatticCoord;

uniform bool lighting;  //allows two options for the menu: enable lighting or not
uniform int LightCount;

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
	fFog = Fog;
	vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	
	if (lattice_on){
		if (lattice_upright){
			fLatticCoord = vec2(0.5 * (vPosition4.x + 1), 0.5 * (vPosition4.y + 1));
		}
		else{
			fLatticCoord = vec2(0.3 * (vPosition4.x + vPosition4.y + vPosition4.z), 0.3 * (vPosition4.x - vPosition4.y + vPosition4.z));
		}
	}

	if (calculate_texCoord){
		float s = 0.0, t = 0.0;
		vec4 position_to_calculate = vPosition4;

		if (sphere_texture_space){
			position_to_calculate = ModelView * vPosition4;
		}

		if (texDimension == 1){
			if (sphere_texture_dir){
				s = 1.5 * (position_to_calculate.x + position_to_calculate.y + position_to_calculate.z);
			}
			else{
				s = 2.5 * position_to_calculate.x;
			}
		}
		else if (texDimension == 2){
			if (sphere_texture_dir){
				s = 0.45 * (position_to_calculate.x + position_to_calculate.y + position_to_calculate.z);
				t = 0.45 * (position_to_calculate.x - position_to_calculate.y + position_to_calculate.z);
			}
			else{
				s = 0.75 * (position_to_calculate.x + 1);
				t = 0.75 * (position_to_calculate.y + 1);
			}
		}

		fTexCoord = vec2(s, t);
		fTexCoord1D = s;
	}
	else {
		fTexCoord = vTexCoord;
	}

	if (lighting){
		 // Transform vertex position into eye coordinates
		vec3 pos = (ModelView * vPosition4).xyz;
		vec3 E = normalize( -pos );

		color = GlobalAmbientProduct;

		for (int i = 0; i < LightCount; i++){
			color += calculateLight(i, pos, E);
		}

		if (LightPosition[1].z == -3.0){
			//color = vec4(0.5, 0.0, 0.0, 1.0);
		}
	}

	else{
		color = vColor;
	}

	fPosition =  ModelView * vPosition4;
	fZ = -fPosition.z;
    gl_Position = Projection *fPosition;
}