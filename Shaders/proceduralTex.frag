#version 330
in vec3 diffuseTerm;
in vec3 specTerm;
in float tex_coord;
out vec4 frag_colour;


uniform float width;
uniform float fuzz;
uniform float scale;


void main () {
	vec3 BackColor = vec3(0.3, 0.5, 0.4);
	vec3 StripeColor = vec3(0.8, 0.8, 0.0);
	
	float scaledT = fract(tex_coord * scale);
	float frac1 = clamp(scaledT/fuzz, 0.0, 1.0);
	float frac2 = clamp((scaledT - width)/fuzz, 0.0, 1.0);
	
	frac1 = frac1 * (1.0 - frac2);
	frac1 = frac1 * frac1 * (3.0 - (2.0 * frac1));
	
	vec3 finalColor = mix(BackColor, StripeColor, frac1);
	finalColor = finalColor*diffuseTerm + specTerm;
	
	frag_colour = vec4(finalColor, 1.0);
}