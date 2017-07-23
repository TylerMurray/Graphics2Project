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

		bool LoadSkyBox(const char * path, std::vector<VertexPositionUVNORMAL>& out_verts, std::vector<unsigned int>& out_indices);


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



		//VendingMachine
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_objModel;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout_objModel;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader_objModel;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader_objModel;

		ModelViewProjectionConstantBuffer m_constantBufferData_objModel;
		uint32 m_indexCount_objModel;
		float dirY_vend = -1.0f;
		float dirX_vend = 0.0f;
		bool flip_vend = false;
	
		//Texture Variables VendingMachine
		Microsoft::WRL::ComPtr<ID3D11Texture2D> diffuseTexture; //Color data mapped across polygons
		Microsoft::WRL::ComPtr<ID3D11Texture2D> vendingMachineTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vendingMachineView;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> vendingMachineSampler;
		dir_light m_dirLight_objModel;
		point_light m_pointLight_objModel;
		spot_light m_spotLight_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_objModel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_objModel;


		//Wall
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_wall;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_wall;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_wall;
		ModelViewProjectionConstantBuffer m_constantBufferData_wall;
		uint32 m_indexCount_wall;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> wallTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wallView;
		dir_light m_dirLight_wall;
		point_light m_pointLight_wall;
		spot_light m_spotLight_wall;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_wall;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_wall;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_wall;



		//Alien
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_alien;
		ModelViewProjectionConstantBuffer m_constantBufferData_alien;
		uint32 m_indexCount_alien;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> alienTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> alienView;
		dir_light m_dirLight_alien;
		point_light m_pointLight_alien;
		spot_light m_spotLight_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_alien;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_alien;

		//Barrel
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_barrel;
		ModelViewProjectionConstantBuffer m_constantBufferData_barrel;
		uint32 m_indexCount_barrel;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> barrelTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> barrelView;
		dir_light m_dirLight_barrel;
		point_light m_pointLight_barrel;
		spot_light m_spotLight_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_barrel;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_barrel;

		//floor
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_floor;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_floor;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_floor;
		ModelViewProjectionConstantBuffer m_constantBufferData_floor;
		uint32 m_indexCount_floor;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> floorTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorView;

		//Lighting Floor
		dir_light m_dirLight_floor;
		point_light m_pointLight_floor;
		spot_light m_spotLight_floor;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_floor;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_floor;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_floor;

		float dirY_floor = -1.0f;
		float dirX_floor = 0.0f;
		bool flip_floor = false;



		//Drone
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_drone;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_drone;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_drone;
		ModelViewProjectionConstantBuffer m_constantBufferData_drone;
		uint32 m_indexCount_drone;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> droneTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> droneView;
		dir_light m_dirLight_drone;
		point_light m_pointLight_drone;
		spot_light m_spotLight_drone;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_drone;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_drone;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_drone;

		//Skybox
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer_sky;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer_sky;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer_sky;
		ModelViewProjectionConstantBuffer m_constantBufferData_sky;
		uint32 m_indexCount_sky;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> skyTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyView;
		dir_light m_dirLight_sky;
		point_light m_pointLight_sky;
		spot_light m_spotLight_sky;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DirBuffer_sky;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PointBuffer_sky;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SpotBuffer_sky;
		///
	};
}

