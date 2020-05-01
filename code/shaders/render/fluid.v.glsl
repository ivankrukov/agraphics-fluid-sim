#version 430 core

layout(location = 0) in vec2 vPos;

// normal map is defined by a compute shader run on the level set
// using similar techniques for deriving normals from a heightmap
layout(binding = 0) uniform sampler2DRect bumpMap;

layout(location = 0) uniform vec3 scaleFactors;
layout(location = 1) uniform mat4 mvpMatrix;
layout(location = 2) uniform mat4 mvMatrix;
layout(location = 3) uniform mat4 mMatrix;
layout(location = 4) uniform mat3 normalMatrix;

layout(location = 5) uniform vec3 worldCamera;

out vec3 position;
out vec3 normal;
out vec3 reflectDir;

void main() {
	// position/gl_Position/normal intermediaries
	vec2 texCoord = vPos.xy * scaleFactors.xy;
	vec4 normalHeight = texture(bumpMap, texCoord);
	vec4 truePosition = vec4(vPos.x, normalHeight.w * scaleFactors.z - 1, vPos.y, 1);

	// reflection intermediaries
	vec3 worldPos = vec3(mMatrix * truePosition);
	vec3 worldNorm = vec3(mMatrix * vec4(normalHeight.xyz, 0.0));
	vec3 worldView = normalize(worldCamera - worldPos);

	// final computations of each out
	position = (mvMatrix * truePosition).xyz;
	normal = normalize(normalMatrix * normalHeight.xyz);
	reflectDir = reflect(-worldView, normalize(worldNorm));
	reflectDir.y = -reflectDir.y;
	gl_Position = mvpMatrix * truePosition;
}
