/*
 * handles applying a uniform force to velocity
 */
#version 430 core
layout(local_size_x = 1) in;

subroutine vec4 updateVelocity(vec4 v);

layout(location = 0) subroutine uniform updateVelocity getVelocity;
// below used to be 16 bit, but 
// https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-30-real-time-simulation-and-rendering-3d-fluids
// mentioned that there was no visual difference/better bandwidth
layout(binding = 0, location = 0, rgba16f) uniform image2DRect newVelocity; 
layout(binding = 1, location = 1) uniform sampler2DRect velocity; 
layout(location = 2) uniform vec4 force;
layout(location = 3) uniform float timeStep;

/*
// 1000 * 1000 texture -> 1M points of varying force
layout(std430, binding=0) buffer Force {
	vec3 force[1000][1000];
};
*/


layout(index = 0) subroutine(updateVelocity)
vec4 river(vec4 v) {
	// vec4 magnitude = v +
	//	int(gl_GlobalInvocationID.x < 80 && gl_GlobalInvocationID.y < 80) * force * timeStep;
	vec4 magnitude = v + force * timeStep;
	magnitude.a = 1;
	return magnitude;
}

layout(index = 1) subroutine(updateVelocity)
vec4 whirlpool(vec4 v) {
	vec2 coord = (gl_GlobalInvocationID.xy / float(gl_NumWorkGroups.x)) * 50;
	vec2 fieldValue = vec2(coord.x + coord.y, coord.y - coord.x);
	return vec4(v.xy - fieldValue * timeStep, 0, 1);
}

layout(index = 2) subroutine(updateVelocity)
vec4 ripple(vec4 v) {
	vec2 coord = (gl_GlobalInvocationID.xy / 150.0f - 0.5) * 40;
	vec2 fieldValue = vec2(coord.x / 2, coord.y / 2);
	return vec4(fieldValue, 0, 1);
}

layout(index = 3) subroutine(updateVelocity)
vec4 inward(vec4 v) {
	vec2 inVec = clamp((gl_NumWorkGroups.xy / 2.0f) - gl_GlobalInvocationID.xy, -30, 30);
	return vec4(inVec, 0, 1);
}

layout(index = 4) subroutine(updateVelocity)
vec4 nochange(vec4 v) {
	return v;
}


void main() {
	vec4 current = texture(velocity, gl_GlobalInvocationID.xy);
	imageStore(newVelocity, ivec2(gl_GlobalInvocationID.xy), getVelocity(current));
}
