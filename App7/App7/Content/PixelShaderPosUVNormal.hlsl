// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
	float3 worldpos : POSITION;
};

cbuffer dir_light : register(b0)
{
	float4 dir_direction;
	float4 dir_color;

};

cbuffer point_light : register(b1)
{
	float4 point_position;
	float4 point_color;

};

cbuffer spot_light : register(b2)
{
	float4 spot_position;
	float4 spot_coneDir;
	float4 spot_coneRatio;
	float4 spot_color;

};


texture2D objTexture : register(t0);

SamplerState filter : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//0 in dir_direction.w = No lighting applied, 1 or higher = do that desired lighting
	if (dir_direction.w == 0.0f)
	{
		return objTexture.Sample(filter, input.uv);
	}
	float4 dirResult = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 pointResult = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spotResult = float4(0.0f, 0.0f, 0.0f, 0.0f);

if (dir_direction.w >= 1.0f)
{

		//Directional Light	
		float LightRatio;
		float3 LightDir = float3(dir_direction.x, dir_direction.y, dir_direction.z);
		float3 SurfaceNormal = input.normal;
	
		float4 SurfaceColor = objTexture.Sample(filter, input.uv);
		float4 LightColor = dir_color;

		LightRatio = clamp(dot(-normalize(LightDir), normalize(SurfaceNormal)), 0, 1);
		dirResult = LightRatio * LightColor * SurfaceColor;
		//return dirResult;
}
if (point_position.w >= 1.0f)
{
	//Point Lighting
	float3 LightDir1;
	float3 LightPos1 = float3(point_position.x, point_position.y, point_position.z);
	float3 SurfacePos1 = input.worldpos;
	float LightRatio1;
	float3 SurfaceNormal1 = input.normal;
	
	float4 SurfaceColor1 = objTexture.Sample(filter, input.uv);
	float4 LightColor1 = point_color;

	if (point_position.w == 1.1f)
	{
		LightPos1 = float3(input.worldpos.x + point_position.x, input.worldpos.y + point_position.y, input.worldpos.z + point_position.z);
	}
	float3 ToLight = LightPos1 - SurfacePos1;
	float ToLightDis = length(ToLight);
	//LightDir1 = normalize(LightPos1 - SurfacePos1);
	LightDir1 = ToLight / ToLightDis;
	float DisAtt = 1.0f - saturate(ToLightDis / 2.0f);
	DisAtt *= DisAtt;
	LightRatio1 = clamp(dot(normalize(LightDir1), normalize(SurfaceNormal1)), 0, 1);
	pointResult = LightRatio1 * LightColor1 * SurfaceColor1 * DisAtt;
	//return pointResult;
}

if (spot_position.w >= 1.0f)
{
	//SpotLight Lighting
	float3 LightDir2;
	float3 LightPos2 = float3(spot_position.x, spot_position.y, spot_position.z);
	float3 SurfacePos2 = input.worldpos;
	float3 SurfaceNormal2 = input.normal;
	float4 SurfaceColor2 = objTexture.Sample(filter, input.uv);
	float SurfaceRatio2;
	float3 ConeDir = float3(spot_coneDir.x, spot_coneDir.y, spot_coneDir.z);
	float SpotFactor2;
	float ConeRatio = spot_coneRatio.x;
	float LightRatio2;
	float4 LightColor2 = spot_color;

	float3 ToLight = LightPos2 - SurfacePos2;
	float ToLightDis = length(ToLight);
	LightDir2 = ToLight / ToLightDis;
	float DisAtt = 1.0f - saturate(ToLightDis / 2.0f);
	DisAtt *= DisAtt;
	//LightDir2 = normalize(LightPos2 - SurfacePos2);
	SurfaceRatio2 = clamp(dot(-normalize(LightDir2), normalize(ConeDir)), 0, 1);
	SpotFactor2 = (SurfaceRatio2 > ConeRatio) ? 1 : 0;
	LightRatio2 = clamp(dot(normalize(LightDir2), normalize(SurfaceNormal2)), 0, 1);
	spotResult = SpotFactor2 * LightRatio2 * LightColor2 * SurfaceColor2 * DisAtt;
	//return spotResult;
}

float4 finalResult = saturate(dirResult + pointResult + spotResult);
return objTexture.Sample(filter, input.uv) * finalResult;



}
