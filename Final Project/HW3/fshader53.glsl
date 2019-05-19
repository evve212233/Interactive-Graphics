/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
flat in int fFog;
in vec2 fTexCoord;
in float fTexCoord1D;
in vec4 fPosition;
in float fZ;
out vec4 fColor;

uniform sampler2D texture_2D; /* Note: If using multiple textures,
                                       each texture must be bound to a
                                       *different texture unit*, with the
                                       sampler uniform var set accordingly.
                                 The (fragment) shader can access *all texture units*
                                 simultaneously.
                              */
uniform sampler1D texture_1D; 
uniform int texture_flag;
uniform int texDimension;
uniform bool sphere;

uniform bool lattice_on;
in vec2 fLatticCoord;

void main() 
{ 

	//“no fog”: disable the fog effect,
	if (sphere && lattice_on && fract(4 * fLatticCoord.x) < 0.35 && fract(4 * fLatticCoord.y) < 0.35){
		discard;
	}
	//Define the fog color to be gray and semi-transparent
	 vec4 fogColor = vec4(0.7, 0.7, 0.7, 0.5);
	 float fog_x = 1.0;

	//“linear”: enable the fog effect, using the linear fog equation, with the fog starting and ending values 0.0 and 18.0, respectively
	if (fFog == 1){
		float fogStart = 0.0, fogEnd = 18.0;
		fog_x = (fogEnd - fZ) / (fogEnd - fogStart);
	}
	//“exponential”: enable the fog effect, using the exponential fog equation, with the fog density value 0.09
	else if (fFog == 2){
		float fdensity = 0.09;
		fog_x = exp(-fdensity * fZ);
	}
	// “exponential square”: enable the fog effect, using the exponential square fog equation, with the
	// fog density value the same as exponential one.
	else if (fFog == 3){
		float fdensity = 0.09;
		fog_x = exp(-pow(fdensity * fZ, 2));
	}
	fog_x = clamp(fog_x, 0.0, 1.0);

	//Textures
	vec4 textureColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (texture_flag == 0){
		textureColor = color;
	}
	else {
		if (texDimension == 2){
			vec4 texColor = texture( texture_2D, fTexCoord );
			if (sphere && texColor.x == 0){
				texColor = vec4(0.9, 0.1, 0.1, 1.0);
			}

			textureColor = color * texColor;
		}
		else if (texDimension == 1){
			textureColor = color * texture( texture_1D, fTexCoord1D );
		}
	}
	// yes! fog effect
	fColor = mix(fogColor, textureColor, fog_x);
} 

