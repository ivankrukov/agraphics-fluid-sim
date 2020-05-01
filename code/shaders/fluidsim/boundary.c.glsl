#version 430 core
layout(local_size_x = 1, local_size_y = 4) in;

subroutine void boundaryConditions();


layout(location = 0) subroutine uniform boundaryConditions boundary;
layout(binding = 0, location = 1, rgba16f) uniform image2DRect updatedValue;
layout(binding = 1, location = 2) uniform sampler2DRect stateField;
layout(location = 3) uniform float scale;
layout(location = 4) uniform float time;

layout(index = 0) subroutine(boundaryConditions)
void navierBC() {
	ivec2 idx;
	// bottom
	if (gl_LocalInvocationID.y == 0) {
		idx = ivec2(gl_GlobalInvocationID.x, 0);
		imageStore(updatedValue, idx, scale * texture(stateField, idx + vec2(0, 1)));
	// top
	} else if (gl_LocalInvocationID.y == 1) {
		idx = ivec2(gl_GlobalInvocationID.x, gl_NumWorkGroups.x - 1);
		imageStore(updatedValue, idx, scale * texture(stateField, idx + vec2(0, -1)));
	// left 
	} else if (gl_LocalInvocationID.y == 2) {
		idx = ivec2(0, gl_GlobalInvocationID.x);
		imageStore(updatedValue, idx, scale * texture(stateField, idx + vec2(1, 0)));
	// right
	} else {
		idx = ivec2(gl_NumWorkGroups.x - 1, gl_GlobalInvocationID.x);
		imageStore(updatedValue, idx, scale * texture(stateField, idx + vec2(0, -1)));
	}
}

// the two functions below are custom ones mainly to make the level set
// behave more interestingly than simply slowly reaching a steady state and fading out

layout(index = 1) subroutine(boundaryConditions)
void levelFixEdge() {
	ivec2 baseIdx;
	if (gl_LocalInvocationID.y == 0) {
		baseIdx = ivec2(0, gl_GlobalInvocationID.x);
		float baseValue = texture(stateField, baseIdx).r;
		imageStore(updatedValue, baseIdx, baseValue + 
				int(baseValue < 0.5) * vec4(0.1f, 0, 0, 1));
	} else {
		baseIdx = ivec2(gl_GlobalInvocationID.x, 0);
		float baseValue = texture(stateField, baseIdx).r;
		imageStore(updatedValue, baseIdx, baseValue + 
				int(baseValue < 0.5) * vec4(0.1f, 0, 0, 1));
	}
}

layout(index = 2) subroutine(boundaryConditions)
void levelFixCenter() {
	ivec2 baseIdx;
	if (gl_LocalInvocationID.y == 0) {
		baseIdx = ivec2(gl_GlobalInvocationID.xy + vec2(scale, scale));
		float baseValue = texture(stateField, baseIdx).r;
		imageStore(updatedValue, baseIdx, vec4(baseValue - 0.3, 0, 0, 1));
	} else {
		baseIdx = ivec2(gl_GlobalInvocationID.xy + vec2(scale, scale));
		float baseValue = texture(stateField, baseIdx).r;
		imageStore(updatedValue, baseIdx, vec4(baseValue - 0.3, 0, 0, 1));
	}
}

layout(index = 3) subroutine(boundaryConditions)
void levelFixEdgeFunction() {
	ivec2 baseIdx;
	float mag = fract(sin(dot(gl_GlobalInvocationID.xy * time, vec2(12.9898,78.233))) * 43758.5453) * 0.3 + 0.3;
	if (gl_LocalInvocationID.y == 0) {
		baseIdx = ivec2(0, gl_GlobalInvocationID.x);
		float baseValue = texture(stateField, baseIdx).r;
		imageStore(updatedValue, baseIdx, vec4(mag, 0, 0, 1));
	} else {
		baseIdx = ivec2(gl_GlobalInvocationID.x, 0);
		float baseValue = texture(stateField, baseIdx).r;
		imageStore(updatedValue, baseIdx, vec4(mag, 0, 0, 1));
	}
}

void main() {
	boundary();
}
