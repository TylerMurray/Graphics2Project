#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

///
//#include "purplenebula_up.h"
#include "DDSTextureLoader.h"
#include <DirectXMath.h>
#include <vector>
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
		// Direct3D resources for Pyramid geometry.
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_Pyramid;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_Pyramid;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_Pyramid;
		//System resources for Pyramid
		ModelViewProjectionConstantBuffer m_constantBufferData_Pyramid;
		uint32 m_indexCount_Pyramid;



		//Direct3D resources for .obj loaded model VendingMachine
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_objModel;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout_objModel;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader_objModel;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader_objModel;

		//System resources for .obj loaded model VendingMachine
		ModelViewProjectionConstantBuffer m_constantBufferData_objModel;
		uint32 m_indexCount_objModel;
	
		//Texture Variables VendingMachine
		Microsoft::WRL::ComPtr<ID3D11Texture2D> diffuseTexture; //Color data mapped across polygons
		Microsoft::WRL::ComPtr<ID3D11Texture2D> vendingMachineTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vendingMachineView;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> vendingMachineSampler;
		
		//Lighting VendingMachine
		

		//wolf
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_wolf;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_wolf;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_wolf;
		ModelViewProjectionConstantBuffer m_constantBufferData_wolf;
		uint32 m_indexCount_wolf;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> wolfTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wolfView;
		



		//Alien
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_alien;
		ModelViewProjectionConstantBuffer m_constantBufferData_alien;
		uint32 m_indexCount_alien;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> alienTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> alienView;

		//Barrel
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_barrel;
		ModelViewProjectionConstantBuffer m_constantBufferData_barrel;
		uint32 m_indexCount_barrel;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> barrelTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> barrelView;

		///
	};
}

