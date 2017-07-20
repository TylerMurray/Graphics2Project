#pragma once

namespace App7
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionUVNORMAL
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;

	};

	//TODO make all buffer XMFLOAT4

	struct dir_light
	{
		DirectX::XMFLOAT4 dir_direction;
		DirectX::XMFLOAT4 dir_color;

	};

	struct point_light
	{
		DirectX::XMFLOAT4 point_position;
		DirectX::XMFLOAT4 point_color;

	};

	struct spot_light
	{
		DirectX::XMFLOAT4 spot_position;
		DirectX::XMFLOAT4 spot_coneDir;
		DirectX::XMFLOAT4 spot_coneRatio;
		DirectX::XMFLOAT4 spot_color;

	};
}