#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out VS_OUT
    {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
     }vs_out;

//out vec3 normal;
//out vec4 fragPosEye;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() 
{
	//compute eye space coordinates
	//fragPosEye = view * model * vec4(vPosition, 1.0f);
	//normal = vNormal;
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	vs_out.FragPos = vec3(model*vec4(vPosition,1.0f));
	vs_out.Normal =transpose(inverse(mat3(model))) * vNormal;
	vs_out.TexCoords = vTexCoords;
	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

}
