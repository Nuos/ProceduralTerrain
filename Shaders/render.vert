#version 330
layout(location = 0) in vec3 triangle_position;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 tex;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec3 camOrigin;
uniform vec3 lightOrigin;

layout(std140) uniform LUT {
	int case_to_num_polys[256];
	ivec3 edge_connect_list[1280];
};

uniform MaterialProperties {
	uniform vec3 Ka;
	uniform vec3 Kd;
	uniform vec3 Ks;
	uniform float isLightSource;
	uniform float hasTexture;
};

out vec3 vert_normal_eye;
out vec3 vert_position_eye;
out vec2 tex_coords;
out vec3 lightPos;


void main(){
	
	/*Calculate normal matrix*/
	mat4 normalMat = viewMat * modelMat;
	normalMat = inverse(normalMat);
	normalMat = transpose(normalMat);
	
	/*transform vertex positions to eye space*/
	lightPos = vec3(viewMat * vec4(lightOrigin, 1.0));
	
	vert_position_eye = vec3(viewMat * modelMat * vec4(triangle_position, 1.0));
	
	vec3 vert_n_eye = normalize(vec3(normalMat * vec4(normals, 0.0)));
	
	if(length(normals) < 0.9){
		vert_normal_eye = vec3(0.0,1.0,0.0);
	}
	else {
		vert_normal_eye = vert_n_eye;
	}
	
	tex_coords = tex;
	
	gl_Position =  projMat * vec4(vert_position_eye, 1.0);
}