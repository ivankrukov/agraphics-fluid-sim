#version 430 core
layout(local_size_x = 1) in;

layout(binding = 0, rgba16f) uniform image2DRect normalMap;
layout(binding = 1) uniform sampler2DRect levelSet;
const ivec3 off = ivec3(-1, 0, 1);
const vec2 size = vec2(2.0, 0.0);

void main() {
	ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
	float height = texture(levelSet, texCoord).r;
	float s01 = textureOffset(levelSet, texCoord, off.xy).r;
	float s21 = textureOffset(levelSet, texCoord, off.zy).r;
	float s10 = textureOffset(levelSet, texCoord, off.yx).r;
	float s12 = textureOffset(levelSet, texCoord, off.yz).r;
	vec3 va = normalize(vec3(size.xy, s21 - s01));
	vec3 vb = normalize(vec3(size.yx, s12 - s10));

	imageStore(normalMap, texCoord, vec4(cross(va, vb), height));
}
