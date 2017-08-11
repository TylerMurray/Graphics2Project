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
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(0, XMMatrixLookAtLH(eye, at, up))));

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

	XMVECTOR camPos = newcamera.r[3];
	XMFLOAT3 camDir = XMFLOAT3(m_constantBufferData.view._31, m_constantBufferData.view._32, m_constantBufferData.view._33);
	//Rotations
	//3.14159 = 180 degrees
	//1.5708 = 90 degrees
	
	//SpotLight
	lightTime += timer.GetElapsedSeconds();
	if (LightON == true)
	{
		SpotLightConeRatio = 0.75f;
	}
	else
	{
		SpotLightConeRatio = 1.0f;
	}
	if (lightTime < 4.0f)
	{
		LightON = true;
	}
	else
	{
		LightON = !LightON;
		if (lightTime > 5.0f)
		{
			lightTime = 0.0f;
		}
	}

		////Pyramid////	
		m_constantBufferData_Pyramid = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_Pyramid.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, -2.0f, 0.0f)*XMMatrixRotationX(3.14159f)));

		////wall////																																			 
		m_constantBufferData_wall = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_wall.model, XMMatrixTranspose(XMMatrixTranslation(-2.75f, 0.0f, 0.0f)*XMMatrixRotationY(3.14159f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));
		XMStoreFloat4(&m_dirLight_wall.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.1f });
		XMStoreFloat4(&m_dirLight_wall.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_pointLight_wall.point_position, XMVECTOR{ 2.0f, 1.0f + dirY_floor, -1.0f, 0.0f });
		XMStoreFloat4(&m_pointLight_wall.point_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_spotLight_wall.spot_position, XMVECTOR{ XMVectorGetX(camPos), XMVectorGetY(camPos) - 0.5f, XMVectorGetZ(camPos), 1.0f });
		XMStoreFloat4(&m_spotLight_wall.spot_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });
		XMStoreFloat4(&m_spotLight_wall.spot_coneDir, XMVECTOR{ camDir.x, camDir.y, camDir.z, 0.0f });
		XMStoreFloat4(&m_spotLight_wall.spot_coneRatio, XMVECTOR{ SpotLightConeRatio, 0.0f, 0.0f, 0.0f });

		//Instancing
		XMStoreFloat4x4(&m_instancing_wall.world[0], XMMatrixTranspose(XMMatrixTranslation(-2.75f, 0.0f, 0.0f)*XMMatrixRotationY(3.14159f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));
		XMStoreFloat4x4(&m_instancing_wall.world[8], XMMatrixTranspose(XMMatrixTranslation(-1.0f, 0.0f, 1.75f)*XMMatrixRotationY(1.5708f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));
		XMStoreFloat4x4(&m_instancing_wall.world[9], XMMatrixTranspose(XMMatrixTranslation(-1.0f, 0.0f, 0.75f)*XMMatrixRotationY(1.5708f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));

		XMStoreFloat4x4(&m_instancing_wall.world[10], XMMatrixTranspose(XMMatrixTranslation(-2.5f, 0.0f, -0.75f)*XMMatrixRotationY(-1.5708f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));
		XMStoreFloat4x4(&m_instancing_wall.world[11], XMMatrixTranspose(XMMatrixTranslation(-2.5f, 0.0f, -1.75f)*XMMatrixRotationY(-1.5708f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));

		//XMStoreFloat4x4(&m_instancing_wall.world[12], XMMatrixTranspose(XMMatrixTranslation(-0.9f, -1.0f, 0.0f)*XMMatrixRotationZ(1.5708f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));



		for (unsigned int i = 1; i < 4; i++)
		{
			//Move -Z [1,2,3] (left wall -when looking at alien)
			XMStoreFloat4x4(&m_instancing_wall.world[i], XMMatrixTranspose(XMMatrixTranslation(-2.75f, 0.0f, 1.0f*i)*XMMatrixRotationY(3.14159f)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));
		}
		for (unsigned int i = 0; i < 4; i++)
		{
			//Move X once & Move -Z [4,5,6,7] (right wall -when looking at alien)
			XMStoreFloat4x4(&m_instancing_wall.world[i + 4], XMMatrixTranspose(XMMatrixTranslation(-0.25f, 0.0f, -1.0f*i)* XMMatrixScaling(5.0f, 7.0f, 5.0f)));
		}


		////VendingMachine////
		m_constantBufferData_objModel = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_objModel.model, XMMatrixTranspose(XMMatrixTranslation(-3.0f, -15.0f, -41.0f) * XMMatrixScaling(0.25f, 0.25f, 0.25f)*XMMatrixRotationY(-1.5708f)));

		XMStoreFloat4(&m_dirLight_objModel.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 1.1f });
		XMStoreFloat4(&m_dirLight_objModel.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_pointLight_objModel.point_position, XMVECTOR{ 9.0f + (dirX_floor * 2), 0.75f, -0.25f, 1.0f });
		XMStoreFloat4(&m_pointLight_objModel.point_color, XMVECTOR{ 1.0f, 1.0f, 0.0f, 1.0f });

		XMStoreFloat4(&m_spotLight_objModel.spot_position, XMVECTOR{ XMVectorGetX(camPos), XMVectorGetY(camPos) - 0.5f, XMVectorGetZ(camPos), 1.0f });
		XMStoreFloat4(&m_spotLight_objModel.spot_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });
		XMStoreFloat4(&m_spotLight_objModel.spot_coneDir, XMVECTOR{ camDir.x, camDir.y, camDir.z, 0.0f });
		XMStoreFloat4(&m_spotLight_objModel.spot_coneRatio, XMVECTOR{ SpotLightConeRatio, 0.0f, 0.0f, 0.0f });

		////Alien////
		m_constantBufferData_alien = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_alien.model, XMMatrixTranspose(XMMatrixTranslation(-8.5f, -4.0f, 1.0f)*XMMatrixRotationY(3.14159f)* XMMatrixScaling(0.9f, 0.9f, 0.9f)));
		//XMStoreFloat4x4(&m_constantBufferData_alien.model, XMMatrixTranspose(XMMatrixTranslation(-1.0f, -4.0f, -5.0f)*XMMatrixRotationY(-1.5708f)* XMMatrixScaling(0.9f, 0.9f, 0.9f)));

		XMStoreFloat4(&m_dirLight_alien.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.0f });
		XMStoreFloat4(&m_dirLight_alien.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_pointLight_alien.point_position, XMVECTOR{ 9.0f, 0.75f, -0.25f, 0.1f });
		XMStoreFloat4(&m_pointLight_alien.point_color, XMVECTOR{ 1.0f, 1.0f, 0.0f, 1.0f });

		XMStoreFloat4(&m_spotLight_alien.spot_position, XMVECTOR{ XMVectorGetX(camPos), XMVectorGetY(camPos) - 0.5f, XMVectorGetZ(camPos), 1.0f });
		XMStoreFloat4(&m_spotLight_alien.spot_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });
		XMStoreFloat4(&m_spotLight_alien.spot_coneDir, XMVECTOR{ camDir.x, camDir.y, camDir.z, 0.0f });
		XMStoreFloat4(&m_spotLight_alien.spot_coneRatio, XMVECTOR{ SpotLightConeRatio, 0.0f, 0.0f, 0.0f });

		////Barrel////
		m_constantBufferData_barrel = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_barrel.model, XMMatrixTranspose(XMMatrixTranslation(13.5f, -4.75f, 2.0f)* XMMatrixScaling(0.75f, 0.75f, 0.75f)));
		XMStoreFloat4(&m_dirLight_barrel.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 1.1f });
		XMStoreFloat4(&m_dirLight_barrel.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_spotLight_barrel.spot_position, XMVECTOR{ XMVectorGetX(camPos), XMVectorGetY(camPos) - 0.5f, XMVectorGetZ(camPos), 1.0f });
		XMStoreFloat4(&m_spotLight_barrel.spot_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });
		XMStoreFloat4(&m_spotLight_barrel.spot_coneDir, XMVECTOR{ camDir.x, camDir.y, camDir.z, 0.0f });
		XMStoreFloat4(&m_spotLight_barrel.spot_coneRatio, XMVECTOR{ SpotLightConeRatio, 0.0f, 0.0f, 0.0f });

		////floor////
		m_constantBufferData_floor = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_floor.model, XMMatrixTranspose(XMMatrixTranslation(1.75f, -1.2f, 0.0f)* XMMatrixScaling(5.0f, 5.0f, 5.0f)));
		if (flip_floor == false)
		{
			dirY_floor += timer.GetElapsedSeconds() * 0.15f;
			dirX_floor += timer.GetElapsedSeconds() * 0.15f;

		}
		else
		{
			dirY_floor -= timer.GetElapsedSeconds() * 0.15f;
			dirX_floor -= timer.GetElapsedSeconds() * 0.15f;

		}
		if (dirY_floor > 0.0f)
		{
			flip_floor = true;
		}
		if (dirX_floor > 1.0f)
		{
			flip_floor = true;
		}
		if (dirY_floor < -1.0f)
		{
			flip_floor = false;
		}
		if (dirX_floor < 0.0f)
		{
			flip_floor = false;
		}
		XMStoreFloat4(&m_dirLight_floor.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.1f });
		XMStoreFloat4(&m_dirLight_floor.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_pointLight_floor.point_position, XMVECTOR{ 3.0f, -3.0f + dirY_floor, -1.0f, 1.0f });
		XMStoreFloat4(&m_pointLight_floor.point_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_spotLight_floor.spot_position, XMVECTOR{ XMVectorGetX(camPos), XMVectorGetY(camPos) - 0.5f, XMVectorGetZ(camPos), 1.0f });
		XMStoreFloat4(&m_spotLight_floor.spot_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });
		XMStoreFloat4(&m_spotLight_floor.spot_coneDir, XMVECTOR{ camDir.x, camDir.y, camDir.z, 0.0f });
		XMStoreFloat4(&m_spotLight_floor.spot_coneRatio, XMVECTOR{ SpotLightConeRatio, 0.0f, 0.0f, 0.0f });

		//Instancing
		XMStoreFloat4x4(&m_instancing_floor.world[0], XMMatrixTranspose(XMMatrixTranslation(1.75f, -1.2f, 0.0f)* XMMatrixScaling(5.0f, 5.0f, 5.0f)));

		for (unsigned int i = 1; i < 4; i++)
		{
			//Move -Z (floor) [1,2,3]
			XMStoreFloat4x4(&m_instancing_floor.world[i], XMMatrixTranspose(XMMatrixTranslation(1.75f, -1.2f, -1.0f*i)* XMMatrixScaling(5.0f, 5.0f, 5.0f)));
		}
		for (unsigned int i = 0; i < 4; i++)
		{
			//Move X once & Move -Z (floor) [4,5,6,7]
			XMStoreFloat4x4(&m_instancing_floor.world[i + 4], XMMatrixTranspose(XMMatrixTranslation(0.75f, -1.2f, -1.0f*i)* XMMatrixScaling(5.0f, 5.0f, 5.0f)));

		}
		for (unsigned int i = 0; i < 4; i++)
		{
			//Ceiling 1 [8,9,10,11]
			XMStoreFloat4x4(&m_instancing_floor.world[i + 8], XMMatrixTranspose(XMMatrixTranslation(1.75f, -1.2f, -0.1f + i)* XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixRotationX(3.14159f)));
		}
		for (unsigned int i = 0; i < 4; i++)
		{
			//Ceiling 2 [12,13,14,15]
			XMStoreFloat4x4(&m_instancing_floor.world[i + 12], XMMatrixTranspose(XMMatrixTranslation(0.75f, -1.2f, -0.1f + i)* XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixRotationX(3.14159f)));
		}


		////Drone////
		m_constantBufferData_drone = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_drone.model, XMMatrixTranspose(XMMatrixScaling(0.005f, 0.005f, 0.005f)*newcamera*XMMatrixTranslation(0.0f, -0.5f, 0.0f)));

		XMStoreFloat4(&m_dirLight_drone.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.0f });
		XMStoreFloat4(&m_dirLight_drone.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_pointLight_drone.point_position, XMVECTOR{ 3.0f, -3.0f + dirY_floor, -1.0f, 0.0f });
		XMStoreFloat4(&m_pointLight_drone.point_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		XMStoreFloat4(&m_spotLight_drone.spot_position, XMVECTOR{ XMVectorGetX(camPos), XMVectorGetY(camPos) - 0.5f, XMVectorGetZ(camPos), 0.0f });
		XMStoreFloat4(&m_spotLight_drone.spot_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });
		XMStoreFloat4(&m_spotLight_drone.spot_coneDir, XMVECTOR{ camDir.x, camDir.y, camDir.z, 0.0f });
		XMStoreFloat4(&m_spotLight_drone.spot_coneRatio, XMVECTOR{ SpotLightConeRatio, 0.0f, 0.0f, 0.0f });

		////Skybox////
		m_constantBufferData_sky = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_sky.model, XMMatrixTranspose(XMMatrixScaling(100.0f, 100.0f, 100.0f)*XMMatrixTranslation(XMVectorGetX(camPos), XMVectorGetY(camPos), XMVectorGetZ(camPos))));
		XMStoreFloat4(&m_dirLight_drone.dir_direction, XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
		XMStoreFloat4(&m_dirLight_drone.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		////Station////
		m_constantBufferData_station = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_station.model, XMMatrixTranspose(XMMatrixScaling(100.0f, 100.0f, 100.0f)*XMMatrixTranslation(7.0f, 8.0f, -10.0f)));
		XMStoreFloat4(&m_dirLight_station.dir_direction, XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
		XMStoreFloat4(&m_dirLight_station.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

		//TV//
		m_constantBufferData_tv = m_constantBufferData;
		XMStoreFloat4x4(&m_constantBufferData_tv.model, XMMatrixTranspose(XMMatrixTranslation(0.075f, 1.0f, -5.0f)*XMMatrixScaling(2.5f,1.5f,2.5f)));
		XMStoreFloat4(&m_dirLight_tv.dir_direction, XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
		XMStoreFloat4(&m_dirLight_tv.dir_color, XMVECTOR{ 1.0f, 1.0f, 1.0f, 1.0f });

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

	RenderToTexture();

	auto context = m_deviceResources->GetD3DDeviceContext();

	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ResetObjectsBack2Camera1();

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

	//Draw My Objects
	DrawPyramid();
	DrawVendingMachine();
	DrawBarrel();
	DrawWall();
	DrawFloor();
	DrawDrone();
	DrawSkybox();
	DrawStation();
	DrawAlien();
	DrawTv();
	
	



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

	CD3D11_BUFFER_DESC constantBufferDesc_Pyramid(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc_Pyramid, nullptr, &m_constantBuffer_Pyramid);

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
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },

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
				CD3D11_BUFFER_DESC DirBufferDesc_objModel(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateBuffer(
						&DirBufferDesc_objModel,
						nullptr,
						&m_DirBuffer_objModel
					)
				);
				CD3D11_BUFFER_DESC PointBufferDesc_objModel(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateBuffer(
						&PointBufferDesc_objModel,
						nullptr,
						&m_PointBuffer_objModel
					)
				);
				CD3D11_BUFFER_DESC SpotBufferDesc_objModel(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateBuffer(
						&SpotBufferDesc_objModel,
						nullptr,
						&m_SpotBuffer_objModel
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

/////////////////////////////////Start of Barrel//////////////////////////////////
		std::vector<VertexPositionUVNORMAL> barrelVertices;
		std::vector<unsigned int> barrelIndices;

		bool res3 = LoadOBJModel("Assets/barrel.obj", barrelVertices, barrelIndices);

		m_indexCount_barrel = barrelIndices.size();


		if (res3 == true)
		{
			CD3D11_BUFFER_DESC constantBufferDesc_barrel(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_barrel,
					nullptr,
					&m_constantBuffer_barrel
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_barrel(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_barrel,
					nullptr,
					&m_DirBuffer_barrel
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_barrel(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_barrel,
					nullptr,
					&m_PointBuffer_barrel
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_barrel(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_barrel,
					nullptr,
					&m_SpotBuffer_barrel
				)
			);




			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_barrel = { 0 };
			vertexBufferData_barrel.pSysMem = barrelVertices.data();
			vertexBufferData_barrel.SysMemPitch = 0;
			vertexBufferData_barrel.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_barrel(sizeof(VertexPositionUVNORMAL) * barrelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_barrel,
					&vertexBufferData_barrel,
					&m_vertexBuffer_barrel
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_barrel = { 0 };
			indexBufferData_barrel.pSysMem = barrelIndices.data();
			indexBufferData_barrel.SysMemPitch = 0;
			indexBufferData_barrel.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_barrel(sizeof(unsigned int) * barrelIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_barrel,
					&indexBufferData_barrel,
					&m_indexBuffer_barrel
				)
			);
		}

		HRESULT result3 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\PropBarrel.dds",
			((ComPtr<ID3D11Resource>)barrelTexture).GetAddressOf(),
			barrelView.GetAddressOf());

///////////////////////////////////End of barrel////////////////////////////////////

/////////////////////////////////Start of Wall//////////////////////////////////
		std::vector<VertexPositionUVNORMAL> wallVertices;
		std::vector<unsigned int> wallIndices;

		bool res4 = LoadOBJModel("Assets/wall.obj", wallVertices, wallIndices);

		m_indexCount_wall = wallIndices.size();


		if (res4 == true)
		{
			// Load shaders asynchronously.
			auto loadVSTask2 = DX::ReadDataAsync(L"VertexInstancingShader.cso");

			// After the vertex shader file is loaded, create the shader and input layout.
			auto createVSTask2 = loadVSTask2.then([this](const std::vector<byte>& fileData) {
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateVertexShader(
						&fileData[0],
						fileData.size(),
						nullptr,
						&m_vertexShader_wall
					)
				);
			});

			CD3D11_BUFFER_DESC constantBufferDesc_wall(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_wall,
					nullptr,
					&m_constantBuffer_wall
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_wall(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_wall,
					nullptr,
					&m_DirBuffer_wall
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_wall(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_wall,
					nullptr,
					&m_PointBuffer_wall
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_wall(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_wall,
					nullptr,
					&m_SpotBuffer_wall
				)
			);
			CD3D11_BUFFER_DESC InstancingBufferDesc_wall(sizeof(Instancing), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&InstancingBufferDesc_wall,
					nullptr,
					&m_constantInstanceBuffer_wall
				)
			);


			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_wall = { 0 };
			vertexBufferData_wall.pSysMem = wallVertices.data();
			vertexBufferData_wall.SysMemPitch = 0;
			vertexBufferData_wall.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_wall(sizeof(VertexPositionUVNORMAL) * wallVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_wall,
					&vertexBufferData_wall,
					&m_vertexBuffer_wall
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_wall = { 0 };
			indexBufferData_wall.pSysMem = wallIndices.data();
			indexBufferData_wall.SysMemPitch = 0;
			indexBufferData_wall.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_wall(sizeof(unsigned int) * wallIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_wall,
					&indexBufferData_wall,
					&m_indexBuffer_wall
				)
			);
		}

		HRESULT result4 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\wall.dds",
			((ComPtr<ID3D11Resource>)wallTexture).GetAddressOf(),
			wallView.GetAddressOf());

/////////////////////////////////////////////End of wall////////////////////////////////////

/////////////////////////////////Start of Floor//////////////////////////////////
		std::vector<VertexPositionUVNORMAL> floorVertices;
		std::vector<unsigned int> floorIndices;

		bool res5 = LoadOBJModel("Assets/floor.obj", floorVertices, floorIndices);

		m_indexCount_floor = floorIndices.size();

		if (res5 == true)
		{
			// Load shaders asynchronously.
			auto loadVSTask2 = DX::ReadDataAsync(L"VertexInstancingShader.cso");

			// After the vertex shader file is loaded, create the shader and input layout.
			auto createVSTask2 = loadVSTask2.then([this](const std::vector<byte>& fileData) {
				DX::ThrowIfFailed(
					m_deviceResources->GetD3DDevice()->CreateVertexShader(
						&fileData[0],
						fileData.size(),
						nullptr,
						&m_vertexShader_floor
					)
				);
			});

			CD3D11_BUFFER_DESC constantBufferDesc_floor(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_floor,
					nullptr,
					&m_constantBuffer_floor
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_floor(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_floor,
					nullptr,
					&m_DirBuffer_floor
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_floor(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_floor,
					nullptr,
					&m_PointBuffer_floor
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_floor(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_floor,
					nullptr,
					&m_SpotBuffer_floor
				)
			);
			CD3D11_BUFFER_DESC InstancingBufferDesc_floor(sizeof(Instancing), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&InstancingBufferDesc_floor,
					nullptr,
					&m_constantInstanceBuffer_floor
				)
			);

			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_floor = { 0 };
			vertexBufferData_floor.pSysMem = floorVertices.data();
			vertexBufferData_floor.SysMemPitch = 0;
			vertexBufferData_floor.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_floor(sizeof(VertexPositionUVNORMAL) * floorVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_floor,
					&vertexBufferData_floor,
					&m_vertexBuffer_floor
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_floor = { 0 };
			indexBufferData_floor.pSysMem = floorIndices.data();
			indexBufferData_floor.SysMemPitch = 0;
			indexBufferData_floor.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_floor(sizeof(unsigned int) * floorIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_floor,
					&indexBufferData_floor,
					&m_indexBuffer_floor
				)
			);
		}

		HRESULT result5 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\floor.dds",
			((ComPtr<ID3D11Resource>)floorTexture).GetAddressOf(),
			floorView.GetAddressOf());

/////////////////////////////////////////////End of Floor////////////////////////////////////


/////////////////////////////////Start of Drone//////////////////////////////////
		std::vector<VertexPositionUVNORMAL> droneVertices;
		std::vector<unsigned int> droneIndices;

		bool res6 = LoadOBJModel("Assets/drone.obj", droneVertices, droneIndices);

		m_indexCount_drone = droneIndices.size();


		if (res6 == true)
		{
			CD3D11_BUFFER_DESC constantBufferDesc_drone(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_drone,
					nullptr,
					&m_constantBuffer_drone
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_drone(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_drone,
					nullptr,
					&m_DirBuffer_drone
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_drone(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_drone,
					nullptr,
					&m_PointBuffer_drone
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_drone(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_drone,
					nullptr,
					&m_SpotBuffer_drone
				)
			);



			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_drone = { 0 };
			vertexBufferData_drone.pSysMem = droneVertices.data();
			vertexBufferData_drone.SysMemPitch = 0;
			vertexBufferData_drone.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_drone(sizeof(VertexPositionUVNORMAL) * droneVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_drone,
					&vertexBufferData_drone,
					&m_vertexBuffer_drone
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_drone = { 0 };
			indexBufferData_drone.pSysMem = droneIndices.data();
			indexBufferData_drone.SysMemPitch = 0;
			indexBufferData_drone.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_drone(sizeof(unsigned int) * droneIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_drone,
					&indexBufferData_drone,
					&m_indexBuffer_drone
				)
			);
		}

		HRESULT result6 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\wall.dds",
			((ComPtr<ID3D11Resource>)droneTexture).GetAddressOf(),
			droneView.GetAddressOf());

/////////////////////////////////////////////End of Drone////////////////////////////////////

//////////////////////////////////////////Start of Skybox//////////////////////////////////////////////

//Load in obj file and get info//
		std::vector<VertexPositionUVNORMAL> skyVertices;
		std::vector<unsigned int> skyIndices;

		bool res7 = LoadSkyBox("Assets/Skybox.obj", skyVertices, skyIndices);

		m_indexCount_sky = skyIndices.size();


		if (res7 == true)
		{
			CD3D11_BUFFER_DESC constantBufferDesc_sky(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_sky,
					nullptr,
					&m_constantBuffer_sky
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_sky(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_sky,
					nullptr,
					&m_DirBuffer_sky
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_sky(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_sky,
					nullptr,
					&m_PointBuffer_sky
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_sky(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_sky,
					nullptr,
					&m_SpotBuffer_sky
				)
			);



			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_sky = { 0 };
			vertexBufferData_sky.pSysMem = skyVertices.data();
			vertexBufferData_sky.SysMemPitch = 0;
			vertexBufferData_sky.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_sky(sizeof(VertexPositionUVNORMAL) * skyVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_sky,
					&vertexBufferData_sky,
					&m_vertexBuffer_sky
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_sky = { 0 };
			indexBufferData_sky.pSysMem = skyIndices.data();
			indexBufferData_sky.SysMemPitch = 0;
			indexBufferData_sky.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_sky(sizeof(unsigned int) * skyIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_sky,
					&indexBufferData_sky,
					&m_indexBuffer_sky
				)
			);
		}

		HRESULT result7 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\Skybox1.dds",
			((ComPtr<ID3D11Resource>)skyTexture).GetAddressOf(),
			skyView.GetAddressOf());

///////////////////////////////////End of Skybox////////////////////////////////////

//////////////////////////////////////////Start of Station//////////////////////////////////////////////

//Load in obj file and get info//
		std::vector<VertexPositionUVNORMAL> stationVertices;
		std::vector<unsigned int> stationIndices;

		bool res8 = LoadOBJModel("Assets/Station.obj", stationVertices, stationIndices);

		m_indexCount_station = stationIndices.size();


		if (res8 == true)
		{
			CD3D11_BUFFER_DESC constantBufferDesc_station(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_station,
					nullptr,
					&m_constantBuffer_station
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_station(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_station,
					nullptr,
					&m_DirBuffer_station
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_station(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_station,
					nullptr,
					&m_PointBuffer_station
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_station(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_station,
					nullptr,
					&m_SpotBuffer_station
				)
			);



			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_station = { 0 };
			vertexBufferData_station.pSysMem = stationVertices.data();
			vertexBufferData_station.SysMemPitch = 0;
			vertexBufferData_station.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_station(sizeof(VertexPositionUVNORMAL) * stationVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_station,
					&vertexBufferData_station,
					&m_vertexBuffer_station
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_station = { 0 };
			indexBufferData_station.pSysMem = stationIndices.data();
			indexBufferData_station.SysMemPitch = 0;
			indexBufferData_station.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_station(sizeof(unsigned int) * stationIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_station,
					&indexBufferData_station,
					&m_indexBuffer_station
				)
			);
		}

		HRESULT result8 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\station.dds",
			((ComPtr<ID3D11Resource>)stationTexture).GetAddressOf(),
			stationView.GetAddressOf());

///////////////////////////////////End of Station////////////////////////////////////

//////////////////////////////////////////Start of Alien//////////////////////////////////////////////

//Load in obj file and get info//
		std::vector<VertexPositionUVNORMAL> alienVertices;
		std::vector<unsigned int> alienIndices;

		bool res2 = LoadOBJModel("Assets/Xenomorph.obj", alienVertices, alienIndices);

		m_indexCount_alien = alienIndices.size();


		if (res2 == true)
		{
			CD3D11_BUFFER_DESC constantBufferDesc_alien(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_alien,
					nullptr,
					&m_constantBuffer_alien
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_alien(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_alien,
					nullptr,
					&m_DirBuffer_alien
				)
			);
			CD3D11_BUFFER_DESC PointBufferDesc_alien(sizeof(point_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&PointBufferDesc_alien,
					nullptr,
					&m_PointBuffer_alien
				)
			);
			CD3D11_BUFFER_DESC SpotBufferDesc_alien(sizeof(spot_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&SpotBufferDesc_alien,
					nullptr,
					&m_SpotBuffer_alien
				)
			);



			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_alien = { 0 };
			vertexBufferData_alien.pSysMem = alienVertices.data();
			vertexBufferData_alien.SysMemPitch = 0;
			vertexBufferData_alien.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_alien(sizeof(VertexPositionUVNORMAL) * alienVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_alien,
					&vertexBufferData_alien,
					&m_vertexBuffer_alien
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_alien = { 0 };
			indexBufferData_alien.pSysMem = alienIndices.data();
			indexBufferData_alien.SysMemPitch = 0;
			indexBufferData_alien.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_alien(sizeof(unsigned int) * alienIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_alien,
					&indexBufferData_alien,
					&m_indexBuffer_alien
				)
			);
		}

		HRESULT result2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(),
			L"Assets\\Xenomorph_S.dds",
			((ComPtr<ID3D11Resource>)alienTexture).GetAddressOf(),
			alienView.GetAddressOf());

///////////////////////////////////End of Alien////////////////////////////////////

///////////////////////////////Render2Texture TV/////////////////////////////////////
		std::vector<VertexPositionUVNORMAL> tvVertices;
		std::vector<unsigned int> tvIndices;

		bool res9 = LoadOBJModel("Assets/wall.obj", tvVertices, tvIndices);

		m_indexCount_tv = tvIndices.size();


		if (res9 == true)
		{
			CD3D11_BUFFER_DESC constantBufferDesc_tv(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc_tv,
					nullptr,
					&m_constantBuffer_tv
				)
			);
			CD3D11_BUFFER_DESC DirBufferDesc_tv(sizeof(dir_light), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&DirBufferDesc_tv,
					nullptr,
					&m_DirBuffer_tv
				)
			);

			//Data setup//
			D3D11_SUBRESOURCE_DATA vertexBufferData_tv = { 0 };
			vertexBufferData_tv.pSysMem = tvVertices.data();
			vertexBufferData_tv.SysMemPitch = 0;
			vertexBufferData_tv.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc_tv(sizeof(VertexPositionUVNORMAL) * tvVertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc_tv,
					&vertexBufferData_tv,
					&m_vertexBuffer_tv
				)
			);

			D3D11_SUBRESOURCE_DATA indexBufferData_tv = { 0 };
			indexBufferData_tv.pSysMem = tvIndices.data();
			indexBufferData_tv.SysMemPitch = 0;
			indexBufferData_tv.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC indexBufferDesc_tv(sizeof(unsigned int) * tvIndices.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				m_deviceResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc_tv,
					&indexBufferData_tv,
					&m_indexBuffer_tv
				)
			);
		}

		//Render to texture

		//Setting up Screen Texture
		Size outputSize = m_deviceResources->GetOutputSize();
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = outputSize.Width;
		texDesc.Height = outputSize.Height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		texDesc.SampleDesc.Count = 1;

		m_deviceResources->GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &Render2Texture_Screen);

		//Setting up Depth Texture
		D3D11_TEXTURE2D_DESC texDesc2;
		ZeroMemory(&texDesc2, sizeof(texDesc2));
		texDesc2.Width = outputSize.Width;
		texDesc2.Height = outputSize.Height;
		texDesc2.MipLevels = 1;
		texDesc2.ArraySize = 1;
		texDesc2.Format = DXGI_FORMAT_D16_UNORM;
		texDesc2.Usage = D3D11_USAGE_DEFAULT;
		texDesc2.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc2.SampleDesc.Count = 1;


		m_deviceResources->GetD3DDevice()->CreateTexture2D(&texDesc2, NULL, &Render2Texture_Depth);

		//Render2TextureRTV;
		D3D11_RENDER_TARGET_VIEW_DESC RTV_desc;
		ZeroMemory(&RTV_desc, sizeof(RTV_desc));
		RTV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		RTV_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTV_desc.Texture2D.MipSlice = 0;

		m_deviceResources->GetD3DDevice()->CreateRenderTargetView(Render2Texture_Screen.Get(), &RTV_desc, &Render2TextureRTV);

		//Render2TextureSRV;
		D3D11_SHADER_RESOURCE_VIEW_DESC SRV_desc;
		ZeroMemory(&SRV_desc, sizeof(SRV_desc));
		SRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		SRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRV_desc.Texture2D.MostDetailedMip = 0;
		SRV_desc.Texture2D.MipLevels = 1;

		m_deviceResources->GetD3DDevice()->CreateShaderResourceView(Render2Texture_Screen.Get(), &SRV_desc, &Render2TextureSRV);

		//Render2TextureDSV;
		D3D11_DEPTH_STENCIL_VIEW_DESC DSV_desc;
		ZeroMemory(&DSV_desc, sizeof(DSV_desc));
		DSV_desc.Format = DXGI_FORMAT_D16_UNORM;
		DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		DSV_desc.Texture2D.MipSlice = 0;

		m_deviceResources->GetD3DDevice()->CreateDepthStencilView(Render2Texture_Depth.Get(), &DSV_desc, &Render2TextureDSV);


		
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


	//Reset VendingMachine Model
	m_vertexBuffer_objModel.Reset();
	m_indexBuffer_objModel.Reset();
	m_constantBuffer_objModel.Reset();

	m_inputLayout_objModel.Reset();
	m_vertexShader_objModel.Reset();
	m_pixelShader_objModel.Reset();

	diffuseTexture.Reset();
	vendingMachineTexture.Reset();
	vendingMachineView.Reset();
	vendingMachineSampler.Reset();

	m_DirBuffer_objModel.Reset();
	m_PointBuffer_objModel.Reset();
	m_SpotBuffer_objModel.Reset();

	

    //Reset Alien
	m_vertexBuffer_alien.Reset();
	m_indexBuffer_alien.Reset();
	m_constantBuffer_alien.Reset();
	alienTexture.Reset();
	alienView.Reset();

	m_DirBuffer_alien.Reset();
	m_PointBuffer_alien.Reset();
	m_SpotBuffer_alien.Reset();

	//Reset Barrel
	m_vertexBuffer_barrel.Reset();
	m_indexBuffer_barrel.Reset();
	m_constantBuffer_barrel.Reset();
	barrelTexture.Reset();
	barrelView.Reset();

	m_DirBuffer_barrel.Reset();
	m_PointBuffer_barrel.Reset();
	m_SpotBuffer_barrel.Reset();

	//Reset Wall
	m_vertexBuffer_wall.Reset();
	m_indexBuffer_wall.Reset();
	m_constantBuffer_wall.Reset();
	m_constantInstanceBuffer_wall.Reset();
	wallTexture.Reset();
	wallView.Reset();

	m_DirBuffer_wall.Reset();
	m_PointBuffer_wall.Reset();
	m_SpotBuffer_wall.Reset();
	///

	//Reset Floor
	m_vertexBuffer_floor.Reset();
	m_indexBuffer_floor.Reset();
	m_constantBuffer_floor.Reset();
	m_constantInstanceBuffer_floor.Reset();
	floorTexture.Reset();
	floorView.Reset();

	m_DirBuffer_floor.Reset();
	m_PointBuffer_floor.Reset();
	m_SpotBuffer_floor.Reset();
	///

	//Reset Drone
	m_vertexBuffer_drone.Reset();
	m_indexBuffer_drone.Reset();
	m_constantBuffer_drone.Reset();
	droneTexture.Reset();
	droneView.Reset();

	m_DirBuffer_drone.Reset();
	m_PointBuffer_drone.Reset();
	m_SpotBuffer_drone.Reset();

	//Reset Skybox
	m_vertexBuffer_sky.Reset();
	m_indexBuffer_sky.Reset();
	m_constantBuffer_sky.Reset();
	skyTexture.Reset();
	skyView.Reset();

	m_DirBuffer_sky.Reset();
	m_PointBuffer_sky.Reset();
	m_SpotBuffer_sky.Reset();

	//Reset Station
	m_vertexBuffer_station.Reset();
	m_indexBuffer_station.Reset();
	m_constantBuffer_station.Reset();
	stationTexture.Reset();
	stationView.Reset();

	m_DirBuffer_station.Reset();
	m_PointBuffer_station.Reset();
	m_SpotBuffer_station.Reset();

	//Reset Render 2 Texture
	//TV


	//R2T
	Render2Texture_Screen.Reset();
	Render2Texture_Depth.Reset();
	Render2TextureSRV.Reset();
	Render2TextureRTV.Reset();
	Render2TextureDSV.Reset();

	
	//tv
	m_vertexBuffer_tv.Reset();
	m_indexBuffer_tv.Reset();
	m_constantBuffer_tv.Reset();	
	tvTexture.Reset();
	tvView.Reset();	
	m_DirBuffer_tv.Reset();
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
				//vertex.x = -vertex.x;
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
				//normal.x = -normal.x;
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
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);

				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);

				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);


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

bool Sample3DSceneRenderer::LoadSkyBox(const char* path, std::vector <VertexPositionUVNORMAL> &out_verts, std::vector <unsigned int> &out_indices)
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
				//vertex.x = -vertex.x;
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
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
				//normal.x = -normal.x;
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

void Sample3DSceneRenderer::RenderToTexture()
{

	//BEFORE RENDERING
	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(1, Render2TextureRTV.GetAddressOf(), Render2TextureDSV.Get());
	m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(Render2TextureRTV.Get(), DirectX::Colors::CornflowerBlue);
	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(Render2TextureDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Update World,View,Projection
	world2 = world;
	//camera2 = camera;
	proj2 = proj;
	//Rotations
	//3.14159 = 180 degrees
	//1.5708 = 90 degrees

    XMStoreFloat4x4(&camera2, XMMatrixIdentity());
	XMMATRIX cameraTmp = XMLoadFloat4x4(&camera2);
	cameraTmp = XMMatrixTranspose(cameraTmp * XMMatrixTranslation(-0.3f, 0.0f, 0.3f) * XMMatrixRotationY(-1.5f));
	XMVECTOR camPos2 = cameraTmp.r[3];
	XMStoreFloat4x4(&camera2, cameraTmp);

	XMMATRIX projTmp = XMLoadFloat4x4(&proj2);
	float AspectRatio = 2.5f / 1.5f;
	projTmp = XMMatrixPerspectiveFovLH(1.5708f, AspectRatio, 0.01f, 100.0f);
	XMStoreFloat4x4(&proj2, projTmp);
	
	//Alien
	XMStoreFloat4x4(&m_constantBufferData_alien.model, XMMatrixTranspose(XMMatrixTranslation(-0.25f, -5.15f, -1.25f)*XMMatrixRotationY(-2.5708f)* XMMatrixScaling(0.9f, 0.9f, 0.9f)));
	XMStoreFloat4x4(&m_constantBufferData_alien.view, XMMatrixTranspose(XMMatrixInverse(0, cameraTmp)));
	XMStoreFloat4x4(&m_constantBufferData_alien.projection, XMMatrixTranspose(projTmp));

	if (LightON == true)
	{
		XMStoreFloat4(&m_dirLight_alien.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.1f });

	}
	else
	{
		XMStoreFloat4(&m_dirLight_alien.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.0f });
	}
	

	//Vending Machine
	m_constantBufferData_objModel.view = m_constantBufferData_alien.view;
	m_constantBufferData_objModel.projection = m_constantBufferData_alien.projection;

	//Barrel
	m_constantBufferData_barrel.view = m_constantBufferData_alien.view;
	m_constantBufferData_barrel.projection = m_constantBufferData_alien.projection;

	//wall
	m_constantBufferData_wall.view = m_constantBufferData_alien.view;
	m_constantBufferData_wall.projection = m_constantBufferData_alien.projection;

	//floor
	m_constantBufferData_floor.view = m_constantBufferData_alien.view;
	m_constantBufferData_floor.projection = m_constantBufferData_alien.projection;

	//drone
	m_constantBufferData_drone.view = m_constantBufferData_alien.view;
	m_constantBufferData_drone.projection = m_constantBufferData_alien.projection;

	//Skybox
	XMStoreFloat4x4(&m_constantBufferData_sky.model, XMMatrixTranspose(XMMatrixScaling(100.0f, 100.0f, 100.0f)*XMMatrixTranslation(XMVectorGetX(camPos2), XMVectorGetY(camPos2), XMVectorGetZ(camPos2))));
	m_constantBufferData_sky.view = m_constantBufferData_alien.view;
	m_constantBufferData_sky.projection = m_constantBufferData_alien.projection;


	//Station
	m_constantBufferData_station.view = m_constantBufferData_alien.view;
	m_constantBufferData_station.projection = m_constantBufferData_alien.projection;



	//RENDERING
	DrawVendingMachine();
	DrawBarrel();
	DrawWall();
	DrawFloor();
	DrawDrone();
	DrawSkybox();
	DrawStation();
	DrawAlien();

}



void Sample3DSceneRenderer::DrawSkybox()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Draw Skybox//
	context->UpdateSubresource1(
		m_constantBuffer_sky.Get(),
		0,
		NULL,
		&m_constantBufferData_sky,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_sky.Get(),
		0,
		NULL,
		&m_dirLight_sky,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_sky.Get(),
		0,
		NULL,
		&m_pointLight_sky,
		0,
		0,
		0
	);
	////update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_sky.Get(),
		0,
		NULL,
		&m_spotLight_sky,
		0,
		0,
		0
	);
	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_sky.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_sky.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_sky.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_sky.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_sky.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_sky.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_sky.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetShaderResources(0, 1, skyView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexed(
		m_indexCount_sky,
		0,
		0
	);
	///
}

void Sample3DSceneRenderer::DrawStation()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Draw Station//
	context->UpdateSubresource1(
		m_constantBuffer_station.Get(),
		0,
		NULL,
		&m_constantBufferData_station,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_station.Get(),
		0,
		NULL,
		&m_dirLight_station,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_station.Get(),
		0,
		NULL,
		&m_pointLight_station,
		0,
		0,
		0
	);
	////update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_station.Get(),
		0,
		NULL,
		&m_spotLight_station,
		0,
		0,
		0
	);
	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_station.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_station.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_station.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_station.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_station.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_station.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_station.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetShaderResources(0, 1, stationView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexed(
		m_indexCount_station,
		0,
		0
	);
}

void Sample3DSceneRenderer::DrawPyramid()
{
	////Start of Draw a Pyramid////
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	stride = sizeof(VertexPositionColor);

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
}

void Sample3DSceneRenderer::DrawVendingMachine()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Start of objModel VendingMachine
	context->UpdateSubresource1(
		m_constantBuffer_objModel.Get(),
		0,
		NULL,
		&m_constantBufferData_objModel,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_objModel.Get(),
		0,
		NULL,
		&m_dirLight_objModel,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_objModel.Get(),
		0,
		NULL,
		&m_pointLight_objModel,
		0,
		0,
		0
	);
	//update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_objModel.Get(),
		0,
		NULL,
		&m_spotLight_objModel,
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

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_objModel.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_objModel.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_objModel.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_objModel.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_objModel.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, vendingMachineView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf());

	context->DrawIndexed(
		m_indexCount_objModel,
		0,
		0
	);
	//End of objModel vending Machine
}

void Sample3DSceneRenderer::DrawBarrel()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Draw Barrel//
	context->UpdateSubresource1(
		m_constantBuffer_barrel.Get(),
		0,
		NULL,
		&m_constantBufferData_barrel,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_barrel.Get(),
		0,
		NULL,
		&m_dirLight_barrel,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_barrel.Get(),
		0,
		NULL,
		&m_pointLight_barrel,
		0,
		0,
		0
	);
	////update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_barrel.Get(),
		0,
		NULL,
		&m_spotLight_barrel,
		0,
		0,
		0
	);
	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_barrel.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_barrel.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_barrel.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_barrel.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_barrel.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_barrel.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_barrel.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetShaderResources(0, 1, barrelView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexed(
		m_indexCount_barrel,
		0,
		0
	);
	///
}

void Sample3DSceneRenderer::DrawWall()
{
	//Draw wall//
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_wall.Get(),
		0,
		NULL,
		&m_dirLight_wall,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_wall.Get(),
		0,
		NULL,
		&m_pointLight_wall,
		0,
		0,
		0
	);
	//update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_wall.Get(),
		0,
		NULL,
		&m_spotLight_wall,
		0,
		0,
		0
	);
	context->UpdateSubresource1(
		m_constantInstanceBuffer_wall.Get(),
		0,
		NULL,
		&m_instancing_wall,
		0,
		0,
		0
	);
	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_wall.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_wall.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	context->VSSetShader(
		m_vertexShader_wall.Get(),
		nullptr,
		0
	); //Thats fine using same shader as floor for instancing
	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->VSSetConstantBuffers1(
		1,
		1,
		m_constantInstanceBuffer_wall.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_wall.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_wall.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_wall.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_wall.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetShaderResources(0, 1, wallView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexedInstanced(
		m_indexCount_wall,
		64,
		0,
		0,
		0
	);
}

void Sample3DSceneRenderer::DrawFloor()
{
	//Draw floor//
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;

	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_floor.Get(),
		0,
		NULL,
		&m_dirLight_floor,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_floor.Get(),
		0,
		NULL,
		&m_pointLight_floor,
		0,
		0,
		0
	);
	//update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_floor.Get(),
		0,
		NULL,
		&m_spotLight_floor,
		0,
		0,
		0
	);
	//update instancing buffer
	context->UpdateSubresource1(
		m_constantInstanceBuffer_floor.Get(),
		0,
		NULL,
		&m_instancing_floor,
		0,
		0,
		0
	);

	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_floor.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_floor.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_floor.Get(),
		nullptr,
		0
	);

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	   //Set Instance cBuffer
	context->VSSetConstantBuffers1(
		1,
		1,
		m_constantInstanceBuffer_floor.GetAddressOf(),
		nullptr,
		nullptr
	);

	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_floor.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_floor.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_floor.GetAddressOf(),
		nullptr,
		nullptr
	);


	context->PSSetShaderResources(0, 1, floorView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexedInstanced(
		m_indexCount_floor,
		64,
		0,
		0,
		0
	);

	///
}

void Sample3DSceneRenderer::DrawDrone()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Draw drone//
	context->UpdateSubresource1(
		m_constantBuffer_drone.Get(),
		0,
		NULL,
		&m_constantBufferData_drone,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_drone.Get(),
		0,
		NULL,
		&m_dirLight_drone,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_drone.Get(),
		0,
		NULL,
		&m_pointLight_drone,
		0,
		0,
		0
	);
	////update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_drone.Get(),
		0,
		NULL,
		&m_spotLight_drone,
		0,
		0,
		0
	);
	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_drone.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_drone.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_drone.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_drone.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_drone.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_drone.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_drone.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetShaderResources(0, 1, droneView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexed(
		m_indexCount_drone,
		0,
		0
	);
	///
}

void Sample3DSceneRenderer::DrawAlien()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Draw Alien//
	context->UpdateSubresource1(
		m_constantBuffer_alien.Get(),
		0,
		NULL,
		&m_constantBufferData_alien,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_alien.Get(),
		0,
		NULL,
		&m_dirLight_alien,
		0,
		0,
		0
	);
	//update point_buffer
	context->UpdateSubresource1(
		m_PointBuffer_alien.Get(),
		0,
		NULL,
		&m_pointLight_alien,
		0,
		0,
		0
	);
	////update spot_buffer
	context->UpdateSubresource1(
		m_SpotBuffer_alien.Get(),
		0,
		NULL,
		&m_spotLight_alien,
		0,
		0,
		0
	);
	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_alien.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_alien.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);

	context->IASetInputLayout(m_inputLayout_objModel.Get()); //Thats fine using same layout

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(
		m_vertexShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->PSSetShader(
		m_pixelShader_objModel.Get(),
		nullptr,
		0
	); //Thats fine using same shader

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_alien.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_alien.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_alien.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		1,
		1,
		m_PointBuffer_alien.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		2,
		1,
		m_SpotBuffer_alien.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetShaderResources(0, 1, alienView.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf()); //thats fine using same sampler

	context->DrawIndexed(
		m_indexCount_alien,
		0,
		0
	);
}

void Sample3DSceneRenderer::DrawTv()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = 0;
	UINT offset = 0;
	//Draw TV//
	context->UpdateSubresource1(
		m_constantBuffer_tv.Get(),
		0,
		NULL,
		&m_constantBufferData_tv,
		0,
		0,
		0
	);
	//update dir_buffer
	context->UpdateSubresource1(
		m_DirBuffer_tv.Get(),
		0,
		NULL,
		&m_dirLight_tv,
		0,
		0,
		0
	);

	stride = sizeof(VertexPositionUVNORMAL);
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer_tv.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer_tv.Get(),
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

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_tv.GetAddressOf(),
		nullptr,
		nullptr
	);
	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer_tv.GetAddressOf(),
		nullptr,
		nullptr
	);
	//Set lighting buffers
	context->PSSetConstantBuffers1(
		0,
		1,
		m_DirBuffer_tv.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, Render2TextureSRV.GetAddressOf());
	context->PSSetSamplers(0, 1, vendingMachineSampler.GetAddressOf());

	context->DrawIndexed(
		m_indexCount_tv,
		0,
		0
	);

	ID3D11ShaderResourceView* pNull = nullptr;
	context->PSSetShaderResources(0, 1, &pNull);
	


	///
}

void Sample3DSceneRenderer::ResetObjectsBack2Camera1()
{
	XMMATRIX newcamera = XMLoadFloat4x4(&camera);
	XMVECTOR camPos = newcamera.r[3];


	//VendingMachine
	m_constantBufferData_objModel.view = m_constantBufferData.view;
	m_constantBufferData_objModel.projection = m_constantBufferData.projection;

	//barrel
	m_constantBufferData_barrel.view = m_constantBufferData.view;
	m_constantBufferData_barrel.projection = m_constantBufferData.projection;

	//wall
	m_constantBufferData_wall.view = m_constantBufferData.view;
	m_constantBufferData_wall.projection = m_constantBufferData.projection;

	//floor
	m_constantBufferData_floor.view = m_constantBufferData.view;
	m_constantBufferData_floor.projection = m_constantBufferData.projection;

	//drone
	m_constantBufferData_drone.view = m_constantBufferData.view;
	m_constantBufferData_drone.projection = m_constantBufferData.projection;

	//alien
	XMStoreFloat4x4(&m_constantBufferData_alien.model, XMMatrixTranspose(XMMatrixTranslation(-8.5f, -4.0f, 1.0f)*XMMatrixRotationY(3.14159f)* XMMatrixScaling(0.9f, 0.9f, 0.9f)));
	m_constantBufferData_alien.view = m_constantBufferData.view;
	m_constantBufferData_alien.projection = m_constantBufferData.projection;
	XMStoreFloat4(&m_dirLight_alien.dir_direction, XMVECTOR{ dirX_floor, dirY_floor, 0.0f, 0.0f });


	//skybox
	XMStoreFloat4x4(&m_constantBufferData_sky.model, XMMatrixTranspose(XMMatrixScaling(100.0f, 100.0f, 100.0f)*XMMatrixTranslation(XMVectorGetX(camPos), XMVectorGetY(camPos), XMVectorGetZ(camPos))));
	m_constantBufferData_sky.view = m_constantBufferData.view;
	m_constantBufferData_sky.projection = m_constantBufferData.projection;

	//station
	m_constantBufferData_station.view = m_constantBufferData.view;
	m_constantBufferData_station.projection = m_constantBufferData.projection;

}
