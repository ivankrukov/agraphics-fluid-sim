// Responsible for initializing the Level Set
#version 430 core

layout(local_size_x = 1) in;

layout(binding = 0, location = 0, rgba16f) uniform image2DRect target; 

void main() {
	// float mag = int((gl_GlobalInvocationID.x < 100 && gl_GlobalInvocationID.y < 30) || 
	// 		(gl_GlobalInvocationID.x < 30 && gl_GlobalInvocationID.y < 100));
	float mag = fract(sin(dot(gl_GlobalInvocationID.xy ,vec2(12.9898,78.233))) * 43758.5453);
	imageStore(target, ivec2(gl_GlobalInvocationID.xy), vec4(mag, 0, 0, 1));
}
