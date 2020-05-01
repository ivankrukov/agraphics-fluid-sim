/* Fragment shader for the second pass using gbuffer data.
 *
 */
#version 430 core

in vec2 texCoord;

layout(location=0) out vec4 fragColorOut;

// gbuffer uniforms
// since these are samplerRects, texcoords are [0, dimensions of (image)]
layout(binding=0) uniform sampler2DRect velocityTex;
layout(binding=1) uniform sampler2DRect pressureTex;
layout(binding=2) uniform sampler2DRect levelSetTex;
layout(binding=3) uniform sampler2DRect debugTex;

// ***** FRAGMENT SHADER SUBROUTINES *****

subroutine vec4 processColor();

layout(location = 0) subroutine uniform processColor colorProcessor;

layout(index = 0) subroutine(processColor)
vec4 velocity() {
	vec4 color = texture(velocityTex, texCoord);
	color.a = 1;
	return color;
}

layout(index = 1) subroutine(processColor)
vec4 pressure() {
	vec4 color = texture(pressureTex, texCoord);
	color.a = 1;
	return color;
}

layout(index = 2) subroutine(processColor)
vec4 levelSet() {
	float value = texture(levelSetTex, texCoord).x;
	return vec4(int(value > 0) * value, 0, int(value < 0) * value, 1);
}

layout(index = 3) subroutine(processColor)
vec4 debug() {
	vec4 color = texture(debugTex, texCoord);
	color.a = 1;
	return color;
}

void main() {
	fragColorOut = colorProcessor();
}
