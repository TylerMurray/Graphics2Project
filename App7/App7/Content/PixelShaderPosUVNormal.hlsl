//float4 main() : SV_TARGET
//{
//	return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
};


texture2D objTexture : register(t0);

SamplerState filter : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//Directional Light	
	float LightRatio;
    float3 LightDir = float3(1.0f, -1.0f, 0.0f);
	float3 SurfaceNormal = input.normal;
	float4 Result;
	float4 LightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 SurfaceColor = objTexture.Sample(filter, input.uv);
	
	LightRatio = clamp(dot(-normalize(LightDir), normalize(SurfaceNormal)),0, 1);
	Result = LightRatio * LightColor * SurfaceColor;
	return Result;
/*
	[branch] switch (typeOfLighting)
	{
	 case 0:
	 {
		return objTexture.Sample(filter, input.uv);
		break; //No Lighting
	 }
	 case 1:
	 {
		 float LightRatio;
		 float3 SurfaceNormal = input.normal;
		 float4 Result;
		 float4 SurfaceColor = objTexture.Sample(filter, input.uv);

		 LightRatio = clamp(dot(-LightDir, SurfaceNormal), 0, 1);
		 Result = LightRatio * LightColor * SurfaceColor;

		 return Result;
		break; //Directional Lighting
	 }
	 case 2:
	 {
		 float LightRatio;
		 float3 SurfaceNormal = input.normal;
		 float4 Result;
		 float4 SurfaceColor = objTexture.Sample(filter, input.uv);
		 float3 LightDirection;

		 LightDirection = normalize(LightPos - input.pos);
		 LightRatio = clamp(dot(LightDir, SurfaceNormal), 0, 1);
		 Result = LightRatio * LightColor * SurfaceColor;
		 return Result;

		break; //Point Lighting
	 }
	 case 3:
	 {
		 float LightRatio;
		 float3 SurfaceNormal = input.normal;
		 float4 Result;
		 float4 SurfaceColor = objTexture.Sample(filter, input.uv);
		 float3 LightDirection;
		 float SurfaceRatio;
		 float SpotFactor;

		 LightDirection = normalize(LightPos - input.pos);
		 SurfaceRatio = clamp(dot(-LightDir, ConeDir), 0, 1);
		 SpotFactor = (SurfaceRatio > ConeRatio) ? 1 : 0;
		 LightRatio = clamp(dot(LightDir, SurfaceNormal), 0, 1);
		 Result = SpotFactor * LightRatio * LightColor * SurfaceColor;
		 return Result;

		break; //SpotLight Lighting
	 }
	 default:
	 {
		 return objTexture.Sample(filter, input.uv);
		break;
	 }
	}
	*/

}
