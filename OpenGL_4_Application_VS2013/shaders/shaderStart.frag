#version 410 core

//in vec3 normal;
//in vec4 fragPosEye;
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;

uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform sampler2D diffuseTexture;
//uniform sampler2D ambientTexture;
uniform sampler2D specularTexture;


uniform sampler2D shadowMap;


vec3 ambient;
float ambientStrength = 3.9f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.2f;
float shininess = 64.0f;

float ShadowCalculation(vec4 fragPosLightSpace,vec3 normal,vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
   // float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
    //add bias to solve fragment
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 
	float shadow = 0.0;
   vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
for(int x = -1; x <= 1; ++x)
{
    for(int y = -1; y <= 1; ++y)
    {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    }    
}
       shadow /= 9.0;
	//extra check the z value
	if(projCoords.z > 1.0) shadow = 0.0;
    return shadow;
}  


vec3 computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = fs_in.Normal;	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fs_in.FragPos);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light

	
         vec3 halfwayDir = normalize(lightDirN + viewDirN);  
         float  spec = pow(max(dot(normalEye, halfwayDir), 0.0), shininess);
         specular = spec * lightColor;    


	ambient *= vec3(texture(diffuseTexture, fs_in.TexCoords));
	diffuse *= vec3( texture(diffuseTexture, fs_in.TexCoords));
	specular *= vec3(texture(specularTexture, fs_in.TexCoords));
	vec3 color = min((ambient + diffuse) + specular, 1.0f);

	 float shadow = ShadowCalculation(fs_in.FragPosLightSpace,normalEye,lightDir);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color; 
	return lighting;


	//vec3 reflection = reflect(-lightDirN, normalEye);
	//float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);

	//vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;

}



void main() 
{
	vec3 lighting=computeLightComponents();
	/*
	vec3 baseColor = vec3(1.0f, 0.55f, 0.0f);//orange
	
	ambient *= baseColor;
	diffuse *= baseColor;
	specular *= baseColor;
	*/

	ambient *= vec3(texture(diffuseTexture, fs_in.TexCoords));
	diffuse *= vec3( texture(diffuseTexture, fs_in.TexCoords));
	specular *= vec3(texture(specularTexture, fs_in.TexCoords));
	vec3 color = min((ambient + diffuse) + specular, 1.0f);
    
	// Calculate shadow
      
    
    fColor = vec4(lighting, 1.0f);
  
}
