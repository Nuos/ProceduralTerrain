#version 330
in vec3 vert_normal_eye;
in vec3 vert_position_eye;
in vec2 tex_coords;
in vec3 lightPos;

out vec4 frag_colour;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform vec3 camOrigin;
uniform sampler2D m_texture;


uniform MaterialProperties {
	uniform vec3 Ka;
	uniform vec3 Kd;
	uniform vec3 Ks;
	uniform float isLightSource;
	uniform float hasTexture;
};

/*Light intensity*/
vec3 Ia;
vec3 Id;
vec3 Is;

void main () {
	/*frag_colour = vec4 (outColor, 1.0);*/
	
	vec4 texel;
	vec4 tex_amb;
	vec4 tex_diff;
	vec4 tex_spec;
	
	
	/*Phong lighting*/
	/*vec3 lightPos = vec3(5.0, 1.0, 10.0);*/
	
	vec3 Ls = vec3(1.0);
	vec3 Ld = vec3(0.7);
	vec3 La = vec3(0.3);
	
	if(hasTexture > 0.5){
		texel = texture(m_texture, tex_coords);
	}
	else{
		texel = vec4(1.0);
	}

	if(isLightSource > 0.5) {
		Ld = vec3(1.0);
		La = vec3(1.0);
	}

	/* surface reflectance*/
	/*vec3 Kdd = texel;*/
	vec3 K_s = Ks; /*vec3(0.8); /* fully reflect specular light*/
	vec3 K_d = Kd; /* diffuse surface reflectance*/
	vec3 K_a = Ka; /*vec3 (1.0, 1.0, 1.0); /* fully reflect ambient light*/
	float specular_exponent = 100.0; /* the 'shininess' factor*/
	
	
	/*
	Phong Reflection Model:
		Ip = ka*ia + ( kd * (Lm * normal)*id  +  ks * (Rm * V)^specular_exponent * is)
	*/
	
	/*ambient light intensity*/
	Ia = La * K_a;
	
	tex_amb = vec4(Ia, 1.0) * texel;
	
	/*diffuse*/
	vec3 light_position_eye = lightPos;
	vec3 distanceToLightEye = light_position_eye - vert_position_eye;
	vec3 dirToLightEye = normalize(distanceToLightEye);
	float dotProd = dot(dirToLightEye, vert_normal_eye);

	dotProd = max(dotProd, 0.0);
	Id = Ld * K_d * dotProd;
	
	tex_diff = vec4(Id, 1.0) * texel;
	
	/*specular*/
	vec3 reflection_eye = reflect(-dirToLightEye, vert_normal_eye);
	
	/*Since we are in eye space, camera origin is at {0,0,0} so surface_to_cam
		can be represented by the negative of vert_position_eye*/
	vec3 surface_to_cam = normalize(-vert_position_eye);
	float dotProdSpec = dot(reflection_eye, surface_to_cam);
	dotProdSpec = max(dotProdSpec, 0.0);
	float specFactor = pow(dotProdSpec, specular_exponent);
	Is = Ls * K_s * specFactor;
	
	tex_spec = vec4(Is, 1.0) * texel;

	frag_colour = min(tex_amb + tex_diff + tex_spec, vec4(1.0));
}