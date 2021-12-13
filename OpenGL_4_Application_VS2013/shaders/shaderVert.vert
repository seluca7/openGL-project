#version 400

layout (location = 0) in vec3 vertexPosition;

//won't be needing color for a while
//layout (location = 1) in vec3 vertexColor;


layout (location = 2) in vec2 vertexTexture;

layout (location =3) in vec3 vertexNormal;

//out vec3 myColor;
out vec2 TexCoords;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(){ 
//myTexture= vec2(vertexTexture.x,1.0-vertexTexture.y);
//myColor=vertexColor;
gl_Position = projection * view * model * vec4(vertexPosition,1.0f);
FragPos=vec3(model* vec4(vertexPosition,1.0f));
Normal=mat3(transpose(inverse(model))) * vertexNormal;
TexCoords=vertexTexture;

}