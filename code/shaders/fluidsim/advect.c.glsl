/*
 * Advect describes the moving of quantities through a fluid (i.e dye through a liquid, velocity, etc)
 * This is acheived through the implicit method of forward integration (trace back our velocity, bilerp cells)
 */
#version 430 core
// Work groups are 2d
layout(local_size_x = 1) in;

/*
 * DEPRECATED: The GPU gem I was looking at was using clamped textures.
 * While I can modify coordinates to be clamped, it's likely not computationally efficient
// SSBOs for reading/storing fluid sim information
// while SSBOs follow the incoherent memory access model,
// OpenGL provides a guarantee that written values are ignored by an invocation
// As such, I don't think separate buffers are necessary for read/write that are swapped per-iteration
layout(std430, binding = 0) buffer Pres {
	vec4 pressure[1000][1000];
};
layout(std430, binding = 1) buffer Vel {
	vec4 velocity[1000][1000];
};
layout(std430, binding = 2) buffer Vorac {
	vec4 voracity[1000][1000];
};
*/

// per-invocation, newVelocity and velocity texture units are swapped
// Rect textures remove the need for me to normalize things in coordinate calculation
layout(binding = 0, location = 0, rgba16f) uniform image2DRect advected; 
layout(binding = 1, location = 9, rgba16f) uniform image2DRect debug;
layout(binding = 1, location = 1) uniform sampler2DRect toAdvect; 
layout(binding = 2, location = 2) uniform sampler2DRect velocity; 

// simulation parameters
layout(location = 3) uniform float timeStep = 0.1;
// since the simulation works with a grid, components need to be scaled accordingly
// 1/gridScale
layout(location = 4) uniform float rdx = 1;
layout(location = 5) uniform float dissipation = 0.9;

vec4 advect(vec4 v) {
	// this could be changed to a vec3 with a 3d texture
	// but then one would need to do ray marching to render the volume
	vec2 traceCoord = gl_GlobalInvocationID.xy - timeStep * rdx * v.xy;

	// perform bilinear interpolation to advect quantity[] SSBO
	// We effectively fetch a 2x2 grid around the traceCoord to determine our new advection
	// f4texRECTbilerp(velocity, traceCoord)
	// CLAMP_TO_EDGE helps us for stuff that falls out of bounds
	
	// take fractional as a parameter for lerping our four corners
	vec2 lerpParams = fract(traceCoord);
	// roundoff errors cause weird "float to the upper right hand corner" behavior with v=0
	// vec2 ceilC = ceil(traceCoord);
	// vec2 floorC = floor(traceCoord);
	// round to four corners; flatten coordinates

	vec4 ul = textureOffset(toAdvect, traceCoord, ivec2(0, 1));
	vec4 ur = textureOffset(toAdvect, traceCoord, ivec2(1, 1));
	vec4 ll = texture(toAdvect, traceCoord);
	vec4 lr = textureOffset(toAdvect, traceCoord, ivec2(1, 0));

	vec4 advectBilerp = mix(mix(ll, lr, lerpParams.s), mix(ul, ur, lerpParams.s), lerpParams.t);
	imageStore(debug, ivec2(gl_GlobalInvocationID.xy), vec4(timeStep * rdx * v.xy, 0, 1));
	return dissipation * advectBilerp;
}

void main() {
	vec4 xNew = advect(texture(velocity, ivec2(gl_GlobalInvocationID.xy)));
	imageStore(advected, ivec2(gl_GlobalInvocationID.xy), xNew);
}
