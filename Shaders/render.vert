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

out vec3 vert_normal_eye;
out vec3 vert_position_eye;
out vec2 tex_coords;
out vec3 lightPos;

void main(){

	/*diffuse lighting*/
	/*vec3 dirLight = vec3(0.0, 0.0, -1.0);
	vec3 lightIntensity = vec3(0.8);
	vec3 diffuseSurfaceColor = vec3(0.7);*/
	
	
	
	/*vec3 normalVec =  vec3(viewMat * modelMat * vec4(normals, 0.0));
	
	float cosAngleIncidence = dot(normalVec, dirLight)/(length(normalVec)*length(dirLight));
	
	eye_normal = diffuseSurfaceColor *(lightIntensity * cosAngleIncidence);*/
	
	/*Calculate normal matrix*/
	mat4 normalMat = viewMat * modelMat;
	normalMat = inverse(normalMat);
	normalMat = transpose(normalMat);
	
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