#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include <direct.h>

#include "..\Common\DirectXHelper.h"

using namespace App7;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL; 
using namespace Windows::Storage;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	///
	//XMStoreFloat4x4(&view, XMMatrixIdentity());

	static const XMVECTORF32 eye = { 0.0f, 0.0f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMStoreFloat4x4(&camera, XMMatrixInverse(0, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(0, XMMatrixLookAtRH(eye, at, up))));

	///
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));

}

///
using namespace Windows::UI::Core;
extern CoreWindow^ gwindow;
#include <atomic>
extern bool mouse_move;
extern float diffx;
extern float diffy;
extern bool w_down;
extern bool a_down;
extern bool s_down;
extern bool d_down;
extern bool left_click;

extern char buttons[256];
///

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}

	///
	XMMATRIX newcamera = XMLoadFloat4x4(&camera);

	if (buttons['W'])
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[2] * timer.GetElapsedSeconds() * 5.0;
	}

	if (a_down)
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[0] * -timer.GetElapsedSeconds() *5.0;
	}

	if (s_down)
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[2] * -timer.GetElapsedSeconds() * 5.0;
	}

	if (d_down)
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[0] * timer.GetElapsedSeconds() * 5.0;
	}

	Windows::UI::Input::PointerPoint^ point = nullptr;

	//if(mouse_move)/*This crashes unless a mouse event actually happened*/
	//point = Windows::UI::Input::PointerPoint::GetCurrentPoint(pointerID);

	if (mouse_move)
	{
		// Updates the application state once per frame.
		if (left_click)
		{
			XMFLOAT4 vector = { 0, 0, 0, 1 };
			XMVECTOR pos = newcamera.r[3];
			newcamera.r[3] = XMLoadFloat4(&vector);
			newcamera = XMMatrixRotationX(diffy*0.01f) * newcamera * XMMatrixRotationY(diffx*0.01f);
			newcamera.r[3] = pos;
		}
	}

	XMStoreFloat4x4(&camera, newcamera);

	/*Be sure to inverse the camera & Transpose because they don't use pragma pack row major in shaders*/
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(0, newcamera)));

	mouse_move = false;/*Reset*/
	///

	///

	m_constantBufferData_Pyramid = m_constantBufferData;
	//m_constantBufferData_Pyramid.model = ? ? ;
	XMStoreFloat4x4(&m_constantBufferData_Pyramid.model, XMMatrixTranspose(XMMatrixTranslation(-5.0f, -5.0f, 0.0f)));

	m_constantBufferData_objModel = m_constantBufferData;
	XMStoreFloat4x4(&m_constantBufferData_objModel.model, XMMatrixTranspose(XMMatrixTranslation(5.0f, 0.0f, 0.0f)));
	//XMStoreFloat4x4(&m_constantBufferData_objModel.model, XMMatrixTranspose(XMMatrixTranspose(XMMatrixRotationY(1.5708f))));

	///

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
		);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);


	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);

	///
	//Start of Draw a Pyramid
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData_Pyramid,
		0,
		0,
		0
	);


	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_Pyramid.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_Pyramid.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetInputLayout(m_inputLayout.Get());

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	context->DrawIndexed(
		m_indexCount_Pyramid,
		0,
		0
	);
	//End of draw Pyramid

	//Start of objModel
	context->UpdateSubresource1(
		m_constantBuffer_objModel.Get(),
		0,
		NULL,
		&m_constantBufferData_objModel,
		0,
		0,
		0
	);

	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_objModel.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_objModel.Get(),
		DXGI_FORMAT_R32_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get());

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_objModel.Get(),
		nullptr,
		0
	);

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	);

	//ID3D11ShaderResourceView* texViews[] = { environmentView };
	context->PSSetShaderResources(0, 1, vendingMachineView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf());

	context->DrawIndexed(
		m_indexCount_objModel,
		0,
		0
	);
	//End of objModel


	///

}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
				)
			);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this] () {

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] = 
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};


		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
				)
			);

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices [] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
				)
			);
	});


	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});


	///
	/////////////////////////////PYRAMID//////////////////////////////

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor PyramidVertices[] =
		{
			{ XMFLOAT3(-0.5f, -1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -1.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  -1.0f, -0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  -1.0f,  -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }
		};


		D3D11_SUBRESOURCE_DATA vertexBufferData_Pyramid = { 0 };
		vertexBufferData_Pyramid.pSysMem = PyramidVertices;
		vertexBufferData_Pyramid.SysMemPitch = 0;
		vertexBufferData_Pyramid.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC vertexBufferDesc_Pyramid(sizeof(PyramidVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc_Pyramid,
				&vertexBufferData_Pyramid,
				&m_vertexBuffer_Pyramid
			)
		);


		static const unsigned short pyramidIndices[] =
		{
			//bottom face
			2,1,0, 
			1,2,3, 

			//left face
			0,4,2,

			//right face
			1,3,4,

			//front face
			3,2,4,

			//back face
			0,1,4,

		};

		m_indexCount_Pyramid = ARRAYSIZE(pyramidIndices);


		D3D11_SUBRESOURCE_DATA indexBufferData_Pyramid = { 0 };
		indexBufferData_Pyramid.pSysMem = pyramidIndices;
		indexBufferData_Pyramid.SysMemPitch = 0;
		indexBufferData_Pyramid.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC indexBufferDesc_Pyramid(sizeof(pyramidIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc_Pyramid,
				&indexBufferData_Pyramid,
				&m_indexBuffer_Pyramid
			)
		);


	///////////////////////////START OF Vending Machine OBJ MODEL///////////////////////////

		//Load in obj file and get info//
		std::vector<VertexPositionUVNORMAL> vertices;
		std::vector<unsigned int> indices;

		bool res = LoadOBJModel("Assets/VendingMachine.obj", vertices, indices);
	
		m_indexCount_objModel = indices.size();

		

		//Shaders and buffers setup//
		if (res == true)
		{
			// Load shaders asynchronously.
			auto loadVSTask1 = DX::ReadDataAsync(L"VertexShaderPosUVNormal.cso");
			auto loadPSTask1 = DX::ReadDataAsync(L"PixelShaderPosUVNormal.cso");

			// After the vertex shader file is loaded, create the shader and input layout.
			auto createVSTask1 = loadVSTask1.then([this](const std::vector<byte>& fileData) {
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateVertexShader(
						&fileData[0],
						fileData.size(),
						nullptr,
						&m_vertexShader_objModel
					)
				);

				static const D3D11_INPUT_ELEMENT_DESC vertexDesc_objModel[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

				};

				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateInputLayout(
						vertexDesc_objModel,
						ARRAYSIZE(vertexDesc_objModel),
						&fileData[0],
						fileData.size(),
						&m_inputLayout_objModel
					)
				);
			});

			// After the pixel shader file is loaded, create the shader and constant buffer.
			auto createPSTask1 = loadPSTask1.then([this](const std::vector<byte>& fileData) {
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreatePixelShader(
						&fileData[0],
						fileData.size(),
						nullptr,
						&m_pixelShader_objModel
					)
				);

				CD3D11_BUFFER_DESC constantBufferDesc_objModel(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateBuffer(
						&constantBufferDesc_objModel,
						nullptr,
						&m_constantBuffer_objModel
					)
				);
			});


			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_objModel = { 0 };
			vertexBufferData_objModel.pSysMem = vertices.data();
			vertexBufferData_objModel.SysMemPitch = 0;
			vertexBufferData_objModel.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_objModel(sizeof(VertexPositionUVNORMAL) * vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_objModel,
					&vertexBufferData_objModel,
					&m_vertexBuffer_objModel
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_objModel = { 0 };
			indexBufferData_objModel.pSysMem = indices.data();
			indexBufferData_objModel.SysMemPitch = 0;
			indexBufferData_objModel.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_objModel(sizeof(unsigned int) * indices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_objModel,
					&indexBufferData_objModel,
					&m_indexBuffer_objModel
				)
			);
		}
		
		HRESULT result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			                                      L"Assets\\T_vendMachine_D.dds",
			                                     ((ComPtr<ID3D11Resource>)vendingMachineTexture).GetAddressOf(),
			                                     vendingMachineView.GetAddressOf());

		D3D11_SAMPLER_DESC sampleDesc;
		sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.MipLODBias = 0;
		sampleDesc.MaxAnisotropy = 1;
		sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampleDesc.MinLOD = -FLT_MAX;
		sampleDesc.MaxLOD = FLT_MAX;


		m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampleDesc, vendingMachineSampler.GetAddressOf());
		

///////////////////////////////////////End of Vending Machine OBJ Model////////////////////////////////////////////////

	///
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();

	///
	//Reset Pyramid
	m_vertexBuffer_Pyramid.Reset();
	m_indexBuffer_Pyramid.Reset();
	m_constantBuffer_Pyramid.Reset();

	//Reset Model
	m_vertexBuffer_objModel.Reset();
	m_indexBuffer_objModel.Reset();
	m_constantBuffer_objModel.Reset();

	m_inputLayout_objModel.Reset();
	m_vertexShader_objModel.Reset();
	m_pixelShader_objModel.Reset();


	//Reset texture
	diffuseTexture.Reset();
	vendingMachineTexture.Reset();
	vendingMachineView.Reset();
	vendingMachineSampler.Reset();
	///
}

bool Sample3DSceneRenderer::LoadOBJModel(const char* path, std::vector <VertexPositionUVNORMAL> &out_verts,std::vector <unsigned int> &out_indices)
{
	//Temp variables to store obj data
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<XMFLOAT3> temp_vertices;
	std::vector<XMFLOAT2> temp_uvs;
	std::vector<XMFLOAT3> temp_normals;

	char* curPath;
	curPath = _getcwd(NULL, 0);
	

	//Try to open the file
	FILE* file = fopen(path, "r");

	if (file == NULL)
	{
		printf("Can not open the file. \n");
		return false;
	}

	//Read the file
	while (true)
	{
		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF)
		{
			//End of the file exit the loop
			break;
		}
		else
		{
			if (strcmp(lineHeader, "v") == 0)
			{
				XMFLOAT3 vertex;
				fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				vertex.x = -vertex.x;
				temp_vertices.push_back(vertex);
			}
			else if(strcmp(lineHeader, "vt") == 0)
			{
				XMFLOAT2 uv;
				fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
				uv.y = 1.0f - uv.y;
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				XMFLOAT3 normal;
				fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				normal.x = -normal.x;
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0],
					&uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

				if (matches != 9)
				{
					printf("File can't be read by parser");
					return false;
				}

				//push-back all found data
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[2]);
				vertexIndices.push_back(vertexIndex[1]);

				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[2]);
				uvIndices.push_back(uvIndex[1]);

				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[2]);
				normalIndices.push_back(normalIndex[1]);


			}
		}

	}

	//filling in input-parameters
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		XMFLOAT3 vertex = temp_vertices[vertexIndex - 1];
		XMFLOAT2 uv = temp_uvs[uvIndex - 1];
		XMFLOAT3 normal = temp_normals[normalIndex - 1];

		VertexPositionUVNORMAL temp;
		temp.pos = vertex;
		temp.uv = uv;
		temp.normal = normal;
		out_verts.push_back(temp);
		out_indices.push_back(i);
	}

	fclose(file);
	return true;

}

