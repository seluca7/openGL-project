#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;


struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
    vec3 position;	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct SpotLight{
      vec3 position;
	  vec3 direction;

	  vec3 ambient;
	  vec3 diffuse;
	  vec3 specular;

          float constant;
         float linear;
         float quadratic;

	  float cutOff;
	  float outerCutOff;
};

#define NR_POINT_LIGHTS 4
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;


uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int useDirLight;
uniform int usePointLight;
uniform int useSpotLight;
uniform int useFog;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,float shadow,vec3 lightDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light,vec3 normal,vec3 fragPos,vec3 viewDir);

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

float computeFog(vec4 fragmentPosEyeSpace)
{
float fogDensity = 0.1f;
float fragmentDistance = length(fragmentPosEyeSpace);
float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{           
   
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(dirLight.position - fs_in.FragPos);
   /*
    vec3 lightColor = vec3(1.0);
    // Ambient
    vec3 ambient = 4.0 * color;
    // Diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // Specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;    
   // vec3 lighting = (materil.ambient + (1.0 - shadow) * (diffuse + specular)) * color;  
      */

   // Calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace,normal,lightDir); 
 // Properties
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    
    // == ======================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == ======================================
    // Phase 1: Directional lighting

  
    vec3 result = vec3(0.0,0.0,0.0);
    if (useDirLight == 1)
    result+= CalcDirLight(dirLight, normal, viewDir,shadow,lightDir);
              
    // Phase 2: Point lights
     if (usePointLight == 1)
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);    
      
    // Phase 3: Spot light

   if (useSpotLight == 1)
    result += CalcSpotLight(spotLight, normal, fs_in.FragPos, viewDir);      
 
    vec4 fColor = vec4(result, 1.0f);
   if (useFog == 1)
    {
    float fogFactor = computeFog(fs_in.FragPosLightSpace);
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    FragColor = mix(fogColor, fColor, fogFactor);
    }  
   else FragColor = fColor;
}


// Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,float shadow,vec3 lightDir)
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
  //  vec3 lightDir = normalize(-light.direction);
   // Ambient
    vec3 ambient = light.ambient * color;
	 vec3 lightColor = vec3(1.0);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // Combine results
    vec3 diffuse = lightColor * diff; //* vec3(texture(diffuseTexture, fs_in.TexCoords));
    vec3 specular = lightColor * spec ;//* vec3(texture(material.specular, fs_in.TexCoords));
    return (vec3(min((ambient + (1.0f-shadow)*diffuse) + (1.0f -shadow)*specular, 1.0f))*color);
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light,vec3 normal,vec3 fragPos, vec3 viewDir)
{      
    vec3 lightDir = normalize(light.position - fragPos);
	// Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	// Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    //Intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation;   
	return(ambient + diffuse + specular);

}