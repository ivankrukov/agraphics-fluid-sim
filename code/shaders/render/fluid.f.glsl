/*	The Color of the ocean is based on a physically based rendering model
 *	Optimally, the BRDF function defined in https://hal.inria.fr/inria-00443630/file/article-1.pdf
 *	should be used for microfacet modeling.
 *
 * Most of the PBR code is from Chapter 4 Page 133-141 of the textbook
 * Reflections are a mashup from the reflection section of the textbook added to the material color
 * and properly scaled based on roughness
 *
 * 	The paper does use a Gertnert wave approach for approximating surfaces
 * 	(similar to the GPU gem in http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch01.html), but I'm assuming the visual results would look just as good with Navier Stokes
 *
 */
#version 430 core
#define PI 3.1415926538


in vec3 position;
in vec3 normal;
in vec3 reflectDir;

layout(binding = 0) uniform LightInfo {
	vec4 position;
	vec3 L;
} light;

layout(binding = 1) uniform MaterialInfo {
	float Rough;
	bool Metal;
	vec3 Color;
} material;

layout(binding = 1) uniform samplerCube cubeMap;


subroutine vec3 brdfFunction(vec3 position, vec3 n);
layout(location = 0) subroutine uniform brdfFunction brdf;

out vec4 fragColorOut;


vec3 reflectedColor() {
	vec3 reflectColor = texture(cubeMap, reflectDir).rgb;
	return mix(material.Color, reflectColor, max(1 - material.Rough, 0.04));
	// return reflectColor;
}

// schlick approximation functions

// Fresnel Term
vec3 schlickFresnel(float lDotH) {
	vec3 f0 = vec3(0.04);
	if (material.Metal) {
		f0 = reflectedColor();
	}
	return f0 + (1 - f0) * pow(1.0 - lDotH, 5);
}

// Geometry term
float geomSmith(float dotProd) {
	float k = (material.Rough + 1.0) * (material.Rough + 1.0) / 8.0;
	float denom = dotProd * (1 - k) + k;
	return 1.0 / denom;
}

// normal distribution function D
float ggxDistribution(float nDotH) {
	float alpha2 = material.Rough * material.Rough * material.Rough * material.Rough;
	float d = (nDotH * nDotH) * (alpha2 - 1) + 1;
	return alpha2 / (PI * d * d);
}

// fresnel microfacet model calc.
layout(index = 0) subroutine(brdfFunction)
vec3 schlickApproximation(vec3 position, vec3 n) {
	vec3 diffuseBrdf = vec3(0.0);
	if (!material.Metal) {
		diffuseBrdf = reflectedColor(); // TODO check
	}

	vec3 l = vec3(0.0), lightI = light.L;
	if (light.position.w == 0.0) { // directional
		l = normalize(light.position.xyz);
	} else { // positional
		l = light.position.xyz - position;
		float dist = length(l);
		l = normalize(l);
		lightI /= (dist * dist);
	}
	
	vec3 v = normalize(-position);
	vec3 h = normalize(v + l);
	float nDotH = dot(n, h);
	float lDotH = dot(l, h);
	float nDotL = max(dot(n, l), 0.0);
	float nDotV = dot(n, v);
	vec3 specBrdf = 0.25 * ggxDistribution(nDotH) *
		schlickFresnel(lDotH) * geomSmith(nDotL) *
		geomSmith(nDotV);

	return (diffuseBrdf + PI * specBrdf) * lightI * nDotL;
}

// Gaussian distribution based BRDF functions

layout(index = 1) subroutine(brdfFunction)
vec3 gaussian(vec3 position, vec3 n) {
	// brdf(v, l) = (qvn(zeta_h, v, l) * F(v * h))/(4h^3_zcos(theta_l v * h))
	// zeta = probability to see microfaces of slopes
	// v = viewer (view vector?)
	// l = source (light vector?)
	// qvn = normalized visibility probability distribution function
	// h = halfway vector (v + l)
	// F = fresnel factor
	return vec3(0, 0, 1);
}

void main() {
	vec3 sum = brdf(position, normalize(normal));

	// gamma correction
	sum = pow(sum, vec3(1.0/2.2));

	fragColorOut = vec4(sum, 1);
}
