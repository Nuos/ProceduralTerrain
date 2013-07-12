#version 330
layout(location = 0) in vec3 triangle_position;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 tex;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec3 camOrigin;
uniform vec3 lightOrigin;
/*uniform bool VS_light; /*if true, lighting calculations are done in Vertex Shader*/

/*layout(std140) uniform LUT {
	int case_to_num_polys[256];
	ivec3 edge_connect_list[1280];
};*/

uniform MaterialProperties {
	uniform vec3 Ka;
	uniform vec3 Kd;
	uniform vec3 Ks;
	uniform float isLightSource;
	uniform float hasTexture;
};

out vec3 diffuseTerm;
out vec3 specTerm;
out float tex_coord;


void main(){
	
	/*Calculate normal matrix*/
	mat4 normalMat = viewMat * modelMat;
	normalMat = inverse(normalMat);
	normalMat = transpose(normalMat);
	
	/*transform vertex positions to eye space*/
	vec3 lightPosEye = vec3(viewMat * vec4(lightOrigin, 1.0));
	vec3 vertPosEye = vec3(viewMat * modelMat * vec4(triangle_position, 1.0));
	vec3 vertNormEye = normalize(vec3(normalMat * vec4(normals, 0.0)));
	
	


	vec3 Ls = vec3(1.0);
	vec3 Ld = vec3(0.7);
	vec3 La = vec3(0.3);
	
	if(isLightSource > 0.5) {
		Ld = vec3(1.0);
		La = vec3(1.0);
	}
	if(hasTexture > 0.5)
		tex_coord = 1.0;
	
	vec3 Ia = La * Ka;
	
	/*Diffuse calculation*/
	vec3 dirToLightEye = normalize(lightPosEye - vertPosEye); 
	vec3 Id =  Ld * vec3(Kd * dot(dirToLightEye, vertNormEye));
	diffuseTerm = Id  + Ia;
	
	/*Specular*/
	float specularExponent = 100.0;
	vec3 reflection_eye = reflect(-dirToLightEye, vertNormEye);
	vec3 surface_to_cam = normalize(-vertPosEye);
	float Is = pow(clamp( dot(reflection_eye, surface_to_cam) , 0.0, 1.0), specularExponent);
	specTerm = Is * Ls * Ks;
	
	/*textures*/
	tex_coord = tex.t;
	
	
	gl_Position =  projMat * vec4(vertPosEye, 1.0);
}