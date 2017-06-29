#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

///
#include <DirectXMath.h>
using namespace DirectX;
///

namespace App7
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }
		
		///
		bool LoadOBJModel(const char* path, std::vector <VertexPositionUVNORMAL> &out_verts, std::vector <unsigned int> &out_indices);


		///


	private:
		void Rotate(float radians);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;


		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		///
		XMFLOAT4X4 world, camera, proj;
		///

		///
		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_Pyramid;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_Pyramid;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_Pyramid;
		//System resources for Pyramid
		ModelViewProjectionConstantBuffer m_constantBufferData_Pyramid;
		uint32 m_indexCount_Pyramid;

		///
	};
}

