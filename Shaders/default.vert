#version 330
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 outColor;

void main(){
	outColor = vertex_color;
	gl_Position = proj * view * model * vec4(vertex_pos, 1.0);
}