#version 430

struct Light
{
	vec4 m_spotColor;
	vec4 m_directColor;
	vec4 m_ambientColor;
	vec3 m_position;
	vec3 m_direction;
	float m_spotBrightness;
	float m_pointBrightness;
	float m_ambientBrightness;
	float m_directionalBrightness;
	float m_innerRadius;
	float m_outerRadius;
	float m_innerDot;
	float m_outerDot;
	
};
const int MAX_NUM_LIGHTS = 16;
uniform int u_renderFlags[8];
// u_renderFlags[0] is diffuse
uniform sampler2D u_diffuseTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_emissiveTexture;
uniform sampler2D u_specularTexture;
uniform sampler2D u_heightTexture;
uniform sampler2D u_noiseTexture;
//uniform vec3 u_lightPosition;
uniform vec3 u_cameraPosition;
uniform vec4 u_fogColor;
uniform float u_fogStartDistance;
uniform float u_fogEndDistance;
layout(location = 2) uniform Light u_Lights[MAX_NUM_LIGHTS];

uniform float u_time;

in vec2 v_texCoord;
in vec4 v_color;
in vec3 v_worldPosition;
in vec3 v_screenPosition;
in vec3 v_worldNormal;
in vec3 v_lightVector;
in vec3 v_worldTangent;
in vec3 v_worldBitangent;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_normalColor;

vec3 g_worldUnitNormal;
vec3 g_worldUnitTangent;
vec3 g_worldUnitBitangent;
int MAX_SPEC_EXP = 32;

const float surfaceShininess = 1.0;
//rename TODO
const float A = 0.1;
const float B = 0.3;
const float C = 0.6;
const float D = 1.0;

vec3 ComputeFireEmissive();
vec3 ComputeDissapearingEffect();

float CalculateLuminance( vec3 color )
{
	float luminance = ( 0.2126 * color.r ) + ( 0.7152 * color.g ) + ( 0.0722 * color.b );
	return luminance;
}


void main()
{
	g_worldUnitTangent = normalize( v_worldTangent);
	vec3 worldUnitBitangent = normalize( v_worldBitangent);
	g_worldUnitNormal = normalize( v_worldNormal );

	g_worldUnitBitangent = normalize( cross( g_worldUnitNormal, g_worldUnitTangent ) );
	g_worldUnitTangent = normalize( cross( g_worldUnitBitangent, g_worldUnitNormal ) );
	if( dot( g_worldUnitBitangent, worldUnitBitangent ) < 0.0 )
	{
		g_worldUnitBitangent *= -1.0;
	}

	mat3 tangentToWorldMat = mat3( g_worldUnitTangent, g_worldUnitBitangent, g_worldUnitNormal );
	vec3 lightDirections[MAX_NUM_LIGHTS];
	float lightDistances[MAX_NUM_LIGHTS];
	for( int i = 0; i < MAX_NUM_LIGHTS; ++i )
	{
		vec3 lightPos = u_Lights[i].m_position - v_worldPosition;
		lightDistances[i] = length( lightPos );
		lightDirections[i] = normalize( lightPos );
	}
	
	vec3 cameraDirection = u_cameraPosition - v_worldPosition;
	float cameraDistance = length( cameraDirection );
	vec3 unitCameraDirection = normalize( cameraDirection );
	vec2 texCoord = v_texCoord;
	mat3 worldToTangentMat = transpose( tangentToWorldMat );
	vec3 unitCameraDirectionTangentSpace = normalize(worldToTangentMat * unitCameraDirection);
	unitCameraDirectionTangentSpace.y *= -1;

	//Parallax
	if( u_renderFlags[5] == 1 )
	{
		float bumbHeight = texture2D( u_heightTexture, v_texCoord).r;
		vec2 parallaxedTexCoords = v_texCoord + ( unitCameraDirectionTangentSpace.xy * bumbHeight * 0.03 );
		texCoord = parallaxedTexCoords;
	}
	
	//Diffuse
	if( u_renderFlags[0] == 1 )
	{
		vec3 baseColor = texture2D(u_diffuseTexture, texCoord).rgb;
		o_fragColor = vec4( baseColor, 1.0f ); 
	}

	vec4 lightColorSum = vec4(0,0,0,1);
	vec4 lightColor;
	vec4 spotColor;
	vec4 directColor;
	vec4 directionalColor;
	vec4 ambientColor;
	float coneFallOff[MAX_NUM_LIGHTS];
	float distanceFallOff[MAX_NUM_LIGHTS];
	vec3 lightDirectionForNormalCalc;

	for(int i = 0; i < MAX_NUM_LIGHTS; ++i )
	{
		float tempDistanceFallOff = ( lightDistances[i] - u_Lights[i].m_outerRadius) / ( u_Lights[i].m_innerRadius - u_Lights[i].m_outerRadius );
		distanceFallOff[i] = clamp( tempDistanceFallOff, 0.0, 1.0 );
		float tempconeFallOff = dot( -u_Lights[i].m_direction, lightDirections[i]);
		tempconeFallOff = ( tempconeFallOff - u_Lights[i].m_outerDot ) / (u_Lights[i].m_innerDot - u_Lights[i].m_outerDot );
		tempconeFallOff = clamp( tempconeFallOff, 0.0, 1.0 );
		coneFallOff[i] = smoothstep( 0, 1, tempconeFallOff );
		spotColor = u_Lights[i].m_spotColor * u_Lights[i].m_spotBrightness * coneFallOff[i] * distanceFallOff[i];// * coneFallOff;
		directColor = u_Lights[i].m_directColor *  u_Lights[i].m_pointBrightness * distanceFallOff[i];
		ambientColor = u_Lights[i].m_ambientColor * u_Lights[i].m_ambientBrightness * distanceFallOff[i];
		directionalColor = u_Lights[i].m_directColor *  u_Lights[i].m_directionalBrightness;
		lightColor = directColor + spotColor + directionalColor;
		lightDirectionForNormalCalc = ( u_Lights[i].m_spotBrightness + u_Lights[i].m_pointBrightness ) * lightDirections[i] + (u_Lights[i].m_directionalBrightness * -u_Lights[i].m_direction);

		//Normal
		if( u_renderFlags[1] == 1 )
		{
			vec3 tangentSpaceNormal =  texture2D(u_normalTexture, texCoord).rgb* 2 - 1.0;
			//tangentSpaceNormal = vec3( tangentSpaceNormal.x, 0, tangentSpaceNormal.z );
			tangentSpaceNormal = normalize( tangentSpaceNormal );
			g_worldUnitNormal = normalize(tangentToWorldMat * tangentSpaceNormal);
			o_normalColor = vec4( 0.5 + ( 0.5 * g_worldUnitNormal ), 1 );

			float df =  dot( g_worldUnitNormal, lightDirectionForNormalCalc ); 
			df = clamp( df, 0.0, 1.0 );
			if( u_renderFlags[7] == 1 )
			{
				float E = fwidth(df);
				if (df > A - E && df < A + E)
					df = mix(A, B, smoothstep(A - E, A + E, df));
				else if (df > B - E && df < B + E)
					df = mix(B, C, smoothstep(B - E, B + E, df));
				else if (df > C - E && df < C + E)
					df = mix(C, D, smoothstep(C - E, C + E, df));
				else if (df < A) df = 0.0;
				else if (df < B) df = B;
				else if (df < C) df = C;
				else df = D;
			}
			lightColor =  vec4(lightColor.rgb * df, 1.0 );
		}

		lightColor += ambientColor;
		lightColorSum += lightColor;
	}
	o_fragColor = vec4( ( o_fragColor * lightColorSum ).rgb, o_fragColor.a );
	
	//Specular
	if( u_renderFlags[3] == 1 )
	{
		vec3 cameraReflection = -1 * reflect( unitCameraDirection, g_worldUnitNormal );
		cameraReflection = normalize( cameraReflection );

		vec3 specularLightColor;
		vec3 sumOfSpecularLight;
		for( int i = 0; i < MAX_NUM_LIGHTS; ++i )
		{
			specularLightColor = u_Lights[i].m_directColor.xyz * ( u_Lights[i].m_pointBrightness + u_Lights[i].m_directionalBrightness )
				+ ( u_Lights[i].m_spotColor.xyz * u_Lights[i].m_spotBrightness * coneFallOff[i] );

			float specPowerPosition = clamp( dot( cameraReflection, lightDirections[i] ), 0.0001, 1.0 ) * ( u_Lights[i].m_pointBrightness + u_Lights[i].m_spotBrightness );
			float specPowerDirection = clamp(  dot( cameraReflection, -u_Lights[i].m_direction ), 0.0001, 1.0 ) * u_Lights[i].m_directionalBrightness;
			float specPower = specPowerPosition + specPowerDirection;

			vec4 specTexelColor = texture2D( u_specularTexture, texCoord );
			float shininess = CalculateLuminance( specTexelColor.rgb );

			specPower = pow( specPower, MAX_SPEC_EXP );
			
			specPower *= shininess;

			//TOON
			if( u_renderFlags[7] == 1 )
			{

				float specTier = 2;
				specPower *= specTier;
				specPower = round( specPower );
				specPower /= specTier;
			}

			//specPower *= distanceFallOff[i];
			sumOfSpecularLight += specularLightColor * specPower;
		}
		o_fragColor += vec4( sumOfSpecularLight, 1.0 );

		
	}
	
	

	//Emissive
	vec3 emissiveColor = texture2D( u_emissiveTexture, texCoord).rgb;
	if( u_renderFlags[2] == 1 )
	{
		o_fragColor += vec4( emissiveColor * .75, 1.0 );
	}
	

	//Fog
	if( u_renderFlags[4] == 1 )
	{
		float fogIntensity = ( cameraDistance - u_fogStartDistance ) / ( u_fogEndDistance - u_fogStartDistance );
		
		fogIntensity = clamp( fogIntensity, 0, 1 );
		fogIntensity *= u_fogColor.a;
		o_fragColor = vec4( (fogIntensity * u_fogColor.rgb ) + ( ( 1 - fogIntensity ) * ( o_fragColor.rgb) ), o_fragColor.a );
		if( u_renderFlags[2] == 1 )
		{
			
			o_fragColor += vec4( emissiveColor * .25, 1.0 );
		}
	}

	//Fire fragment effect
	if( u_renderFlags[6] == 1 )
	{
		vec3 fireEffect = ComputeDissapearingEffect();
		//vec3 fireEffect = ComputeFireEmissive();
		o_fragColor += vec4( fireEffect, 1.0 );
	}

}

//-----------------------------------------------------------------------------------------------------
vec3 ComputeFireEmissive()
{
	float relativeHeight = 0.5 + 0.5 * v_worldPosition.y;

	float relativeTime1 = (0.21 * u_time ) + ( 0.11 * v_worldPosition.y ) - ( 0.01 * sin( -1.0 * u_time + 13.0 * v_worldPosition.x ) ) + ( 0.02 * sin( 1.3 * u_time + 7.0 * v_worldPosition.z ) );
	float relativeTime2 = (0.37 * u_time ) + ( 0.17 * v_worldPosition.y) - ( sin( -1.1 * u_time * v_worldPosition.z ) );
	float relativeTime3 = (0.19 * u_time ) + ( 0.23 * v_worldPosition.y ) - ( 0.03 * sin( -1.0 * u_time + 11.0 * v_worldPosition.x ) ) + ( 0.02 * sin( 1.1 * u_time + 4.5 * v_worldPosition.z ) );
	float relativeTime4 = (0.41 * u_time );


	vec2 noiseTexCoords1 = vec2( relativeTime1, 0.1 );
	vec2 noiseTexCoords2 = vec2( relativeTime2, 0.3 );
	vec2 noiseTexCoords3 = vec2( relativeTime3, 0.4 );
	vec2 noiseTexCoords4 = vec2( relativeTime4, 0.6 );

	vec4 noiseTexel1 = texture2D( u_noiseTexture, noiseTexCoords1 );
	vec4 noiseTexel2 = texture2D( u_noiseTexture, noiseTexCoords2 );
	vec4 noiseTexel3 = texture2D( u_noiseTexture, noiseTexCoords2 );

	float noiseIntensity1 = noiseTexel1.r;
	float noiseIntensity2 = noiseTexel2.r;
	float noiseIntensity3 = noiseTexel2.r;


	float totalNoise = ( noiseIntensity1 * noiseIntensity1 );// + ( noiseIntensity2 * noiseIntensity2 ) + ( noiseIntensity3 * noiseIntensity3 );
	float fireIntensity = max( 0.0, (0.3 + totalNoise ) * ( relativeHeight ) );
	float blue = fireIntensity;
	float green = blue * blue * blue;
	float red = green * green * green;
	return vec3( red, green, blue);

}

vec3 ComputeDissapearingEffect()
{
	float relativeTime2 = (0.21 * u_time ) + ( 0.11 * v_worldPosition.y ) - ( 0.01 * sin( -1.0 * u_time + 13.0 * v_worldPosition.x ) ) + ( 0.02 * sin( 1.3 * u_time + 7.0 * v_worldPosition.z ) );
	float relativeTime3 = (0.37 * u_time ) + ( 0.19 * v_worldPosition.y ) - ( 0.03 * sin( -1.0 * u_time + 7.0 * v_worldPosition.x ) ) + ( 0.04 * sin( 0.3 * u_time + 3.0 * v_worldPosition.z ) );
	float relativeTime1 = (0.19 * u_time ) + ( 0.27 * v_worldPosition.y );
	vec2 noiseTexCoords1 = vec2( relativeTime1, 0.1 );
	vec2 noiseTexCoords2 = vec2( relativeTime2, 0.3 );
	vec4 noiseAmount1 = texture2D( u_noiseTexture, noiseTexCoords1 );
	vec4 noiseAmount2 = texture2D( u_noiseTexture, noiseTexCoords2 );
	float red = (1.1 + sin( u_time)) * 1.7 * noiseAmount1.r;
	
	float blue = red ;
	float green = red * red;
	if( red >= 1.0 && green >= 1.0 && blue >= 1.0 )
	{
		discard;
	}
	return vec3( red, green, blue );
}