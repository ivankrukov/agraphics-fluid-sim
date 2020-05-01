/* Depending on the subroutine, this shader is responsible for performing:
 * 	* a step of jacobi iteration on the bound buffers (for solving things like 
 * This is used for solving poisson equations that come up in Navier Stokes, namely pressure and diffuse.
 *
 * This function should be run >20 times (the more iterations, the more accurate the result)
 *
 */
#version 430 core
layout(local_size_x = 1) in;

/*
// SSBOs for reading/storing state
// solve equations in the form Ax = b
layout(std430, binding = 0) buffer X {
	vec4 x[];
}

layout(std430, binding = 1) buffer B {
	vec4 b[];
}
*/

subroutine vec4 processUpdate(vec4 L, vec4 R, vec4 B, vec4 T, vec4 C);

layout(location = 0) subroutine uniform processUpdate computeValue;

// jacobi(general): update x, sample x patameter, center sample b parameter
// gradient: Update velocity, sample pressure, center sample velocity
// divergence: Update divergence, sample vector field, no center sample
layout(binding = 0, location=1, rgba16f) uniform image2DRect updateParameter;
layout(binding = 1, location=2) uniform sampler2DRect sampleParameter; // x
layout(binding = 2, location=3) uniform sampler2DRect centerSample; // b

/*
 *	Diffuse 
 * 	alpha = (dx)^2/(v * dt)
 * 	rBeta = 1/(4 + alpha) // my guess is the gem reciprocates it because * is faster;
 * 	x = b = velocitySSBO
 */

layout(location = 4) uniform float alpha;
layout(location = 5) uniform float rBeta;
layout(location = 6) uniform float halfrdx = 1;


/*
 *	Perform a jacobi iteration i.e stepwise solve poisson equations
 *	(form Ax = b, with A being the matrix representation of the laplacian operator)
 */
layout(index = 0) subroutine(processUpdate)
vec4 jacobi(vec4 L, vec4 R, vec4 B, vec4 T, vec4 C) {
	return (L + R + B + T + alpha * C) * rBeta;
}

/*
 *	Subtracts the pressure gradient off of the velocity
 */
layout(index = 1) subroutine(processUpdate)
vec4 gradient(vec4 L, vec4 R, vec4 B, vec4 T, vec4 C) {
	C.xy = C.xy - halfrdx * vec2(R.x - L.x, T.x - B.x);
	return C;
}

/*
 *	Divergence
 */
layout(index = 2) subroutine(processUpdate)
vec4 divergence(vec4 L, vec4 R, vec4 B, vec4 T, vec4 C) {
	return vec4(halfrdx * ((R.x - L.x) + (T.y - B.y)), 0, 0, 1);
}



void main() {
	// get 4 surrounding samples
	vec4 L = texture(sampleParameter, gl_GlobalInvocationID.xy + vec2(-1, 0));
	vec4 R = texture(sampleParameter, gl_GlobalInvocationID.xy + vec2(1, 0));
	vec4 B = texture(sampleParameter, gl_GlobalInvocationID.xy + vec2(0, -1));
	vec4 T = texture(sampleParameter, gl_GlobalInvocationID.xy + vec2(0, 1));

	vec4 C = texture(centerSample, gl_GlobalInvocationID.xy);

	// evaluate subroutine, update state
	imageStore(updateParameter, ivec2(gl_GlobalInvocationID.xy), computeValue(L, R, B, T, C));
}
