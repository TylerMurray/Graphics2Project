// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer Instancing : register(b1)
{
	matrix world[10];
}

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
	float3 worldpos : POSITION;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
	float3 worldpos : POSITION;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input, unsigned int index : SV_InstanceID) 
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	float4 worldpos = float4(input.worldpos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, world[index]);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	worldpos = mul(pos, world[index]);
	output.worldpos = worldpos;

	output.uv = input.uv;
	output.normal = mul(input.normal, (float3x3)world[index]);


	return output;
}
