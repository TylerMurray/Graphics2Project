// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
};

cbuffer dir_light : register(b0)
{
	float3 dir_direction;
	float4 dir_color;

};

cbuffer point_light : register(b1)
{
	float3 point_position;
	float4 point_color;

};

cbuffer spot_light : register(b2)
{
	float3 spot_position;
	float3 spot_coneDir;
	float spot_coneRatio;
	float4 spot_color;

};


texture2D objTexture : register(t0);

SamplerState filter : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{

		//Directional Light	
		float LightRatio;
		float3 LightDir = dir_direction;
		float3 SurfaceNormal = input.normal;
		float4 dirResult;
		float4 LightColor = dir_color;
		float4 SurfaceColor = objTexture.Sample(filter, input.uv);

		LightRatio = clamp(dot(-normalize(LightDir), normalize(SurfaceNormal)), 0, 1);
		dirResult = LightRatio * LightColor * SurfaceColor;
		return dirResult;

		//Point Lighting
		float LightRatio1;
		float3 SurfaceNormal1 = input.normal;
		float4 pointResult;
		float4 SurfaceColor1 = objTexture.Sample(filter, input.uv);
		float3 LightDirection1;

		LightDirection1 = normalize(point_position - input.pos);
		LightRatio1 = clamp(dot(normalize(LightDirection1), normalize(SurfaceNormal1)), 0, 1);
		pointResult = LightRatio1 * point_color * SurfaceColor1;
		return pointResult;

		//SpotLight Lighting

		float3 SurfaceNormal2 = input.normal;	
		float4 SurfaceColor2 = objTexture.Sample(filter, input.uv);

		float3 LightDirection2 = normalize(spot_position - input.pos);
		float SurfaceRatio2 = clamp(dot(-normalize(LightDirection2), normalize(spot_coneDir)), 0, 1);
		float SpotFactor2 = (SurfaceRatio2 > spot_coneRatio) ? 1 : 0;
		float LightRatio2 = clamp(dot(normalize(LightDirection2), normalize(SurfaceNormal2)), 0, 1);
		float4 spotResult = SpotFactor2 * LightRatio2 * spot_color * SurfaceColor2;
		//return spotResult;

	
	float4 finalResult = saturate(dirResult + pointResult + spotResult);
	return objTexture.Sample(filter, input.uv) * finalResult;

		////SpotLight Lighting (not working properly)

		//float LightRatio2;
		//float3 SurfaceNormal2 = input.normal;
		//float4 Result2;
		//float4 SurfaceColor2 = objTexture.Sample(filter, input.uv);
		//float3 LightDirection2;
		//float SurfaceRatio2;
		//float SpotFactor2;
		//float3 spotPos = float3(0.0f, 1.0f, 0.0f);
		//float3 spotConeDir = float3(1.0f, -1.0f, 0.0f);
		//float spotConeRatio = 0.25f;
		//float4 spotColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

		//LightDirection2 = normalize(spotPos - input.pos);
		//SurfaceRatio2 = clamp(dot(-normalize(LightDirection2), normalize(spotConeDir)), 0, 1);
		//SpotFactor2 = (SurfaceRatio2 > spot_coneRatio) ? 1 : 0;
		//LightRatio2 = clamp(dot(normalize(LightDirection2), normalize(SurfaceNormal2)), 0, 1);
		//Result2 = SpotFactor2 * LightRatio2 * spotColor * SurfaceColor2;
		//return Result2;


	////Point Lighting (not working properly)
	//float LightRatio1;
	//float3 SurfaceNormal1 = input.normal;
	//float4 Result1;
	//float4 SurfaceColor1 = objTexture.Sample(filter, input.uv);
	//float3 LightDirection1;
	//float3 pointPos = float3(0.0f, 0.0f, 0.0f);
	//float4 pointColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

	//LightDirection1 = normalize(pointPos - input.pos);
	//LightRatio1 = clamp(dot(normalize(LightDirection1), normalize(SurfaceNormal1)), 0, 1);
	//Result1 = LightRatio1 * pointColor * SurfaceColor1;
	//return Result1;

	////Directional Light	
	//float LightRatio;
 //   float3 LightDir = float3(1.0f, -1.0f, 0.0f);
	//float3 SurfaceNormal = input.normal;
	//float4 Result;
	//float4 LightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	//float4 SurfaceColor = objTexture.Sample(filter, input.uv);
	//
	//LightRatio = clamp(dot(-normalize(LightDir), normalize(SurfaceNormal)),0, 1);
	//Result = LightRatio * LightColor * SurfaceColor;
	//return Result;

}
