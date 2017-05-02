//
// Base template for Windows drawing, DirectX resource setup, etc. is taken from https://github.com/walbourn/directx-vs-templates
//

//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() :
    m_window(0),
    m_outputWidth(windowX),
    m_outputHeight(windowY),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
}
Game::~Game()
{
	if (m_audEngine)
	{
		m_audEngine->Suspend();
	}
}



// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = max(width, 1) * renderrScale;
    m_outputHeight = max(height, 1) * renderrScale;

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // Custom game logic goes past here
	debugTime = timer.GetTotalSeconds();

	float introPitch = -90.f; // Intro pitch angle (inverted)
	float crawlAngle = 28.f; // angle of intro crawl text

	// Scene timings
	float t_open = 10.f; // Should be 10
	float t_panStart = t_open + 79.f; // What time does the credit pan-down start. Should be around 79 for accuracy, plus bluetext time
	float t_panEnd = t_panStart + 10.f; // What time does the pan end
	float t_scene2 = t_panEnd + 20.f; // Scene for the head-on view of the chase
	float t_scene3 = t_scene2 + 12.f; // Scene for the closeup hits on the blackade runner
	float t_dock = t_scene3 + 3.f;	// Scene for the docking/boarding
	float t_end = t_dock + 10.f;	// Scene for the ending

	bool shipChasing = false; // Flag to perform the pursuit logic

	// Ships' chance of shooting
	int stardShootChanceMod = 16;
	int runnerShootChanceMod = 16;

	// Blasters' chance of exploding
	int blasterFlashChance = 3;

	// Blaster explosion max size
	float blasterFlashSizeMax = 1.2f;
	float blasterFlashSizeMin = 0.2f;

	// Ships' rate of fire
	float stardROF = 8.f;
	float runnerROF = 4.f;

	// Ships' blaster spread
	float stardSpread = 50.f;
	float runnerSpread = 25.f;

	// Ships' movement speeds
	float stardSpeed = 1.15f;
	float runnerSpeed = 1.25f;

	// Update blaster bolts if any
	for (int i = 0; i < o_blasters.size(); i++)
	{
		if (o_blasters[i]->dead != true)
			o_blasters.at(i)->Update();
		else
		{
			// Roll if the blaster should explode
			if (rand() % blasterFlashChance == 0)
			{
				float size = clamp((rand() % (int)(blasterFlashSizeMax * 1000)) / 1000.f, blasterFlashSizeMin, 5.f);	// Calculate the blaster explosion size
				o_blasterFlashes.push_back(std::make_unique<BlasterFlash>(GeometricPrimitive::CreateGeoSphere(m_d3dContext.Get(), size, 2U, true), o_blasters[i]->m_world));
			}

			// Delete the blaster
			o_blasters.erase(o_blasters.begin() + i);
		}
	}

	// Update the blaster explosons if any
	for (int i = 0; i < o_blasterFlashes.size(); i++)
	{
		if (o_blasterFlashes[i]->dead != true)
			o_blasterFlashes[i]->Update();
		else
			o_blasterFlashes.erase(o_blasterFlashes.begin() + i);
	}

	// Scene switch logic
	if (timer.GetTotalSeconds() < t_open)
	{
		drawPrelude = true;
	}
	else if (timer.GetTotalSeconds() < t_panStart)
	{
		drawPrelude = false;
		drawTitle = true;
		m_view = Matrix::CreateTranslation(Vector3::Down * 0.f) * Matrix::CreateRotationX(degreeToRads(introPitch));

		Vector3 v_crawlangle = Vector3::Transform(Vector3::Up, Matrix::CreateRotationX(degreeToRads(crawlAngle)));
		v_crawlangle.Normalize();

		m_title_world = Matrix::CreateRotationX(degreeToRads(-90.f)) * Matrix::CreateTranslation(Vector3::Up * 1.5f) * Matrix::CreateTranslation(Vector3::Up * (timer.GetTotalSeconds() - t_open) * 1.5f);
		m_crawl_world = Matrix::CreateRotationX(degreeToRads(90.f + crawlAngle)) * Matrix::CreateTranslation(Vector3::Forward * 2.f) * Matrix::CreateTranslation(Vector3::Up * 1.f) * Matrix::CreateTranslation(v_crawlangle * (timer.GetTotalSeconds() - t_open - 15.f) * 0.25f);
		debugState = 0;
	}
	else if (timer.GetTotalSeconds() < t_panEnd)
	{
		drawTitle = false;
		float deltaT = t_panStart - timer.GetTotalSeconds(); // I think this is actually calculating everything inverted but everything is working properly with this value so I'm gonna ignore it
		
		m_view = Matrix::CreateTranslation(Vector3::Down * 0.f) * Matrix::CreateRotationX(degreeToRads(clamp(introPitch - deltaT * 8.f, introPitch, 0)));
		debugState = 1;
	}
	else if (timer.GetTotalSeconds() < t_scene2)
	{
		shipChasing = true;
		debugState = 2;
	}
	else if (timer.GetTotalSeconds() < t_scene3)
	{
		shipChasing = true;
		m_sky_world = Matrix::CreateRotationY(degreeToRads((timer.GetTotalSeconds() - t_scene2) * -0.8f));
		m_view = Matrix::CreateTranslation(Vector3(-0.05f, -0.5f, 35.f)) * Matrix::CreateRotationY(degreeToRads(170.f));
		debugState = 3;
	}
	else if (timer.GetTotalSeconds() < t_dock)
	{
		m_view = Matrix::CreateLookAt(Vector3(-0.3f, 0.4f, 1.f), Vector3::Zero, Vector3::UnitY);
		m_stard_world = Matrix::CreateTranslation(Vector3::Backward * 10.f);
		m_runner_world = Matrix::CreateTranslation(Vector3::Forward * 0.5f * (timer.GetTotalSeconds() - t_scene3 - 2.f));
		m_sky_world = Matrix::CreateRotationY(degreeToRads((timer.GetTotalSeconds() - t_scene3) * -2.2f)) * Matrix::CreateRotationX(degreeToRads((timer.GetTotalSeconds() - t_scene3) * -2.6f));
		debugState = 4;

		if (!disablingShot && timer.GetTotalSeconds() > t_scene3 + 1.0f)
		{
			disablingShot = true;
			o_blasters.push_back(std::make_unique<Blaster>(Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\Blaster.cmo", *m_fxFactory, true), Matrix::CreateTranslation(Vector3(0.f, .8f, 2.f)), Matrix::CreateTranslation(Vector3::Forward * 0.5f * (timer.GetTotalSeconds() - t_scene3 - 0.8f)), 1.f));
			o_blasters.back()->speed = 15.f;
		}

		if (!runnerExploded && timer.GetTotalSeconds() > t_scene3 + 1.6f)
		{
			float size = clamp((rand() % (int)(800)) / 1000.f, 0.1f, 0.4f);	// Calculate the explosion size
			
			// Randomized explosion vectors
			float explosionX = (rand() % 400 - 200) / 1000.f;
			float explosionY = (rand() % 400 - 200) / 1000.f;
			float explosionZ = (rand() % 400 - 200) / 1000.f;

			Matrix m_explosion = Matrix::CreateTranslation(Vector3::Forward * 0.5f * (timer.GetTotalSeconds() - t_scene3 - 1.8f)) * Matrix::CreateTranslation(Vector3(explosionX, explosionY, explosionZ)); // Calculate the explosion location

			o_blasterFlashes.push_back(std::make_unique<BlasterFlash>(GeometricPrimitive::CreateGeoSphere(m_d3dContext.Get(), size, 2U, true), m_explosion ));

			if (timer.GetTotalSeconds() > t_scene3 + 2.0f)
				runnerExploded = true;
		}
	}
	else if (timer.GetTotalSeconds() < t_end)
	{
		m_sky_world = Matrix::CreateRotationY(degreeToRads((timer.GetTotalSeconds() - t_scene2) * -0.8f));
		m_view = Matrix::CreateLookAt(Vector3(-0.5f, -1.0f, -2.f), Vector3::Zero, Vector3::UnitY);
		m_runner_world = Matrix::CreateTranslation(Vector3::Backward * (timer.GetTotalSeconds() - t_dock) * 0.016f);
		m_stard_world = Matrix::CreateTranslation(Vector3::Lerp(Vector3(0.f, 1.f, 3.f), Vector3(0,0,0), log((timer.GetTotalSeconds() - t_dock) / 5.f + 1.0f)));
	}
	else
	{
		fadeOutTime = t_end;

		if (faded)
			ExitGame();
		else
			fadeout = true;
	}

	// Pursuit logic
	if (shipChasing)
	{
		// Roll the shoot chances
		int shootChanceRollR = rand() % stardShootChanceMod;
		int shootChanceRollSD = rand() % runnerShootChanceMod;

		// Star Destroyer shoot logic
		if ((int)(timer.GetTotalSeconds() * stardROF) % 2 == 0 && !stardFrameShot && shootChanceRollSD == 0)
		{
			int turrentn = rand() % m_stard_turrents.size();
			Vector3 v_turrent = m_stard_turrents[turrentn];

			// flag that we shot and go pewpew
			stardFrameShot = true;
			o_blasters.push_back(std::make_unique<Blaster>(Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\Blaster.cmo", *m_fxFactory, true), Matrix::CreateTranslation(v_turrent) * m_stard_world, m_runner_world, stardSpread));
			o_blasters.back()->lifetime = 2.f;
		}
		else if ((int)(timer.GetTotalSeconds() * stardROF) % 2 == 1)
			stardFrameShot = false;

		// Blockade Runner shoot logic
		if ((int)(timer.GetTotalSeconds() * runnerROF) % 2 == 0 && !runnerFrameShot && shootChanceRollR == 0)
		{
			int turrentn = rand() % m_runner_turrents.size();
			Vector3 v_turrent = m_runner_turrents[turrentn];

			// flag that we shot and then go pewpew
			runnerFrameShot = true;
			o_blasters.push_back(std::make_unique<Blaster>(Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\BlasterRed.cmo", *m_fxFactory, true), Matrix::CreateTranslation(v_turrent) * m_runner_world, m_stard_world, runnerSpread));
			o_blasters.back()->lifetime = 0.6f;
		}
		else if ((int)(timer.GetTotalSeconds() * runnerROF) % 2 == 1)
			runnerFrameShot = false;

		// Update ship world positions
		m_runner_world = Matrix::CreateTranslation(Vector3(0.4f, 0.5f, 0.f)) * Matrix::CreateTranslation(Vector3::Forward * (timer.GetTotalSeconds() - t_panEnd) * runnerSpeed);
		m_stard_world = Matrix::CreateTranslation(Vector3(0.4f, 1.1f, 8.f)) * Matrix::CreateTranslation(Vector3::Forward * (timer.GetTotalSeconds() - t_panEnd) * stardSpeed);
	}

    elapsedTime;
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // TODO: Add your rendering code here.

	// Draw skybox
	m_sky_fx->SetWorld(m_sky_world);
	m_sky_fx->SetView(m_view);
	m_sky_fx->SetProjection(m_sky_proj);
	m_sky->Draw(m_sky_fx.get(), m_inputLayout.Get());

	// Put the blaster explosions into the sky projection
	m_blasterFlash_fx->SetView(m_view);
	m_blasterFlash_fx->SetProjection(m_sky_proj);

	// Draw models
	m_stard->Draw(m_d3dContext.Get(), *m_states, m_stard_world, m_view, m_proj);
	m_runner->Draw(m_d3dContext.Get(), *m_states, m_runner_world, m_view, m_proj);

	// Only draw the opening titles when we need too
	if (drawTitle)
	{
		m_title->Draw(m_d3dContext.Get(), *m_states, m_title_world, m_view, m_proj);
		m_crawl->Draw(m_d3dContext.Get(), *m_states, m_crawl_world, m_view, m_proj);
	}
	
	// Draw all of our balsterrsss
	for (int i = 0; i < o_blasters.size(); i++)
		o_blasters[i]->model->Draw(m_d3dContext.Get(), *m_states, o_blasters[i]->m_world, m_view, m_proj);

	// Draw all of our blaster explosionssss
	for (int i = 0; i < o_blasterFlashes.size(); i++)
	{
		m_blasterFlash_fx->SetWorld(o_blasterFlashes[i]->world);
		o_blasterFlashes[i]->mesh->Draw(m_blasterFlash_fx.get(), m_inputLayout.Get());
	}

	// Draw debug text
	m_spriteBatch->Begin();

	// Ending fadeout
	if (fadeout)
	{
		// Flag if we're done
		if (m_timer.GetTotalSeconds() - fadeOutTime > 1.f)
			faded = true;

		// Lerp the fade colour then render it
		Color fadeTint = Color::Lerp(Color(0.f, 0.f, 0.f, 0.f), (Color)Colors::White, m_timer.GetTotalSeconds() - fadeOutTime);
		m_spriteBatch->Draw(t_blackbg.Get(), m_fullscreenRect, fadeTint);
	}

	// Opening prelude rendering
	if (drawPrelude)
	{
		// Draw the black background
		m_spriteBatch->Draw(t_blackbg.Get(), m_fullscreenRect);

		// Draw bluetext prelude
		Color preludeTint = Colors::White * (cosf(m_timer.GetTotalSeconds() / 2.5f + 1.f) * -1.5f);
		m_spriteBatch->Draw(t_prelude.Get(), t_prelude_screen, nullptr, preludeTint, 0.f, t_prelude_origin, 0.5f);
	}

	// Draw debug info if it's enabled
	if (debug)
	{
		std::wostringstream infoTxt;
		infoTxt << std::setprecision(4) << L"Total seconds: " << debugTime << L"\nCurrent scene: " << debugState;
		m_font->DrawString(m_spriteBatch.get(), infoTxt.str().c_str(), m_fontPos, Colors::White);
	}
	m_spriteBatch->End();

    Present();
}


// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}


void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = max(width, 1) * renderrScale;
    m_outputHeight = max(height, 1) * renderrScale;

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = windowX;
    height = windowY;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
#pragma region DirectX setup and checks

	UINT creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		// TODO: Modify for supported Direct3D feature levels (see code below related to 11.1 fallback handling).
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// Create the DX11 API device object, and get a corresponding context.
	HRESULT hr = D3D11CreateDevice(
		nullptr,                                // specify nullptr to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		m_d3dDevice.ReleaseAndGetAddressOf(),   // returns the Direct3D device created
		&m_featureLevel,                        // returns feature level of device created
		m_d3dContext.ReleaseAndGetAddressOf()   // returns the device immediate context
		);

	if (hr == E_INVALIDARG)
	{
		// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it.
		hr = D3D11CreateDevice(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			creationFlags,
			&featureLevels[1],
			_countof(featureLevels) - 1,
			D3D11_SDK_VERSION,
			m_d3dDevice.ReleaseAndGetAddressOf(),
			&m_featureLevel,
			m_d3dContext.ReleaseAndGetAddressOf()
			);
	}

	DX::ThrowIfFailed(hr);

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	if (SUCCEEDED(m_d3dDevice.As(&d3dDebug)))
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// TODO: Add more message IDs here as needed.
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif

	// DirectX 11.1 if present
	if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
		(void)m_d3dContext.As(&m_d3dContext1);

	// TODO: Initialize device dependent objects here (independent of window size).
#pragma endregion

	// Custom code past here
	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());
	m_fxFactory = std::make_unique<EffectFactory>(m_d3dDevice.Get());

	// Prep the text print objects
	m_font = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"..\\..\\content\\Fonts\\Arial_14_Regular.spritefont");
	m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());

	// Prep models
	// Star Destroyer
	m_stard = Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\ProjStarD.cmo", *m_fxFactory, true);
	m_stard_world = Matrix::Identity;

	// Blockade Runner
	m_runner = Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\ProjBlockade.cmo", *m_fxFactory, true);
	m_runner_world = Matrix::Identity;

	// Title
	m_title = Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\title.cmo", *m_fxFactory, true);
	m_title_world = Matrix::Identity;

	m_crawl = Model::CreateFromCMO(m_d3dDevice.Get(), L"..\\..\\content\\Models\\titlecrawl.cmo", *m_fxFactory, true);
	m_crawl_world = Matrix::Identity;

	// Audio work

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif
	m_audEngine = std::make_unique<AudioEngine>(eflags);

	m_kazoo = std::make_unique<SoundEffect>(m_audEngine.get(), L"..\\..\\content\\Audio\\StarWarsKazoo.wav");
	auto m_kazooplayer = m_kazoo->CreateInstance();
	m_kazooplayer->Play(true);

	ComPtr<ID3D11Resource> resource;

	DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"..\\..\\content\\Textures\\longtime.png", resource.GetAddressOf(), t_prelude.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"..\\..\\content\\Textures\\theywantedblacksoigavethemblack.png", nullptr, t_blackbg.ReleaseAndGetAddressOf()));

	ComPtr<ID3D11Texture2D> prelude;
	DX::ThrowIfFailed(resource.As(&prelude));

	CD3D11_TEXTURE2D_DESC preludeDesc;
	prelude->GetDesc(&preludeDesc);

	t_prelude_origin.x = float(preludeDesc.Width / 2);
	t_prelude_origin.y = float(preludeDesc.Height / 2);

	// Model light parameters
	const DirectX::SimpleMath::Vector3 light1pos = Vector3(0.3, 0.3, -0.05);
	const DirectX::SimpleMath::Vector3 light2pos = Vector3(0.35, -0.01, 0.5);

	// Setup model lights with a lambda (oooooo aren't we fancy)
	m_runner->UpdateEffects([light1pos, light2pos](IEffect* effect)
	{
		auto lights = dynamic_cast<IEffectLights*>(effect);
		if (lights)
		{
			lights->SetLightingEnabled(true);
			lights->SetPerPixelLighting(true);
			lights->SetLightEnabled(0, true);
			lights->SetLightEnabled(1, true);
			lights->SetLightDiffuseColor(0, Colors::Azure);
			lights->SetLightDirection(0, light1pos);
			lights->SetLightDiffuseColor(1, Colors::Orange);
			lights->SetLightDirection(1, light2pos);
		}
	});

	m_stard->UpdateEffects([light1pos, light2pos](IEffect* effect)
	{
		auto lights = dynamic_cast<IEffectLights*>(effect);
		if (lights)
		{
			lights->SetLightingEnabled(true);
			lights->SetPerPixelLighting(true);
			lights->SetLightEnabled(0, true);
			lights->SetLightEnabled(1, true);
			lights->SetLightDiffuseColor(0, Colors::Azure);
			lights->SetLightDirection(0, light1pos);
			lights->SetLightDiffuseColor(1, Colors::Orange);
			lights->SetLightDirection(1, light2pos);
		}
	});

	m_title->UpdateEffects([](IEffect* effect)
	{
		auto lights = dynamic_cast<IEffectLights*>(effect);
		if (lights)
		{
			lights->SetLightingEnabled(true);
			lights->SetAmbientLightColor(Colors::White * 1.2f);
			lights->SetLightEnabled(0, false);
			lights->SetLightEnabled(1, false);
			lights->SetLightEnabled(2, false);
		}

		auto fog = dynamic_cast<IEffectFog*>(effect);
		if (fog)
		{
			fog->SetFogEnabled(true);
			fog->SetFogColor(Colors::Black);
			fog->SetFogStart(18);
			fog->SetFogEnd(22);
		}
	});

	m_crawl->UpdateEffects([](IEffect* effect)
	{
		auto lights = dynamic_cast<IEffectLights*>(effect);
		if (lights)
		{
			lights->SetLightingEnabled(true);
			lights->SetAmbientLightColor(Colors::White * 1.3f);
			lights->SetLightEnabled(0, false);
			lights->SetLightEnabled(1, false);
			lights->SetLightEnabled(2, false);
		}

		auto fog = dynamic_cast<IEffectFog*>(effect);
		if (fog)
		{
			fog->SetFogEnabled(true);
			fog->SetFogColor(Colors::Black);
			fog->SetFogStart(9);
			fog->SetFogEnd(10);
		}
	});

	// Prep the skybox
	if(debug)
		DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"..\\..\\content\\Textures\\horizonsphere.png", nullptr, m_sky_texture.ReleaseAndGetAddressOf()));
	else
		DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"..\\..\\content\\Textures\\Stars1HD.png", nullptr, m_sky_texture.ReleaseAndGetAddressOf()));

	m_sky = GeometricPrimitive::CreateGeoSphere(m_d3dContext.Get(), 100.f, 3U, false);
	m_sky_world = Matrix::Identity;
	m_sky_fx = std::make_unique<BasicEffect>(m_d3dDevice.Get());
	m_sky_fx->SetTextureEnabled(true);
	m_sky_fx->SetLightingEnabled(false);
	m_sky_fx->DisableSpecular();
	m_sky_fx->SetFogEnabled(false);
	m_sky_fx->SetTexture(m_sky_texture.Get());
	m_sky->CreateInputLayout(m_sky_fx.get(), m_inputLayout.ReleaseAndGetAddressOf());

	// Blaster fx
	m_blasterFlash_fx = std::make_unique<BasicEffect>(m_d3dDevice.Get());
	m_blasterFlash_fx->SetLightingEnabled(false);
	m_blasterFlash_fx->SetTextureEnabled(false);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
	#pragma region DirectX setup and checks

    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory1> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        ComPtr<IDXGIFactory2> dxgiFactory2;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            // DirectX 11.1 or later

            // Create a descriptor for the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
            swapChainDesc.Width = backBufferWidth;
            swapChainDesc.Height = backBufferHeight;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.SampleDesc.Count = max(MSAALevel,1);
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = backBufferCount;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
            fsSwapChainDesc.Windowed = TRUE;

            // Create a SwapChain from a Win32 window.
            DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
                m_d3dDevice.Get(),
                m_window,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                m_swapChain1.ReleaseAndGetAddressOf()
                ));

            DX::ThrowIfFailed(m_swapChain1.As(&m_swapChain));
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
            swapChainDesc.BufferCount = backBufferCount;
            swapChainDesc.BufferDesc.Width = backBufferWidth;
            swapChainDesc.BufferDesc.Height = backBufferHeight;
            swapChainDesc.BufferDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.OutputWindow = m_window;
            swapChainDesc.SampleDesc.Count = max(MSAALevel, 1);;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Windowed = TRUE;

            DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf()));
        }

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, max(MSAALevel, 1), 0);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
	#pragma endregion

	// Custom code past here
	m_fontPos = Vector2(0.f, 20.f);

	// Setup the camera viewports and positions
	m_view = Matrix::CreateTranslation(Vector3::Zero);
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), 0.1f, 50.f);

	// Create the skybox projection
	m_sky_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), .1f, 100.f);

	// Move the models away from the camera
	m_stard_world = Matrix::CreateTranslation(Vector3::Backward * 100.f);
	m_runner_world = Matrix::CreateTranslation(Vector3::Backward * 100.f);

	// Prelude center position
	t_prelude_screen.x = backBufferWidth / 2.f;
	t_prelude_screen.y = backBufferHeight / 2.f;

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = backBufferWidth;
	m_fullscreenRect.bottom = backBufferHeight;

	// Push our turrent origin vectors values to the array
	// Blender axis: X, Z, Y

	// Stardestroyer
	m_stard_turrents.push_back(Vector3(0.68f, 0.f, -2.8f));
	m_stard_turrents.push_back(Vector3(-0.68f, 0.f, -2.8f));
	m_stard_turrents.push_back(Vector3(1.5f, 0.f, -1.2f));
	m_stard_turrents.push_back(Vector3(-1.5f, 0.f, -1.2f));
	m_stard_turrents.push_back(Vector3(0.3f, -0.3f, -1.0f));
	m_stard_turrents.push_back(Vector3(-0.3f, -0.3f, -1.0f));

	// Blockade runner
	m_runner_turrents.push_back(Vector3(0.f , 0.1f, 0.4f));

}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
	m_states.reset();
	m_fxFactory.reset();
	m_font.reset();
	m_spriteBatch.reset();
	m_stard.reset();
	m_runner.reset();
	m_sky.reset();
	m_sky_fx.reset();
	m_sky_texture.Reset();
	m_inputLayout.Reset();
	m_blasterFlash_fx.reset();
	m_title.reset();
	m_crawl.reset();
	t_prelude.Reset();
	t_blackbg.Reset();
	

	for (int i = 0; i < o_blasters.size(); i++)
		o_blasters[i]->model.reset();

	for (int i = 0; i < o_blasterFlashes.size(); i++)
		o_blasterFlashes[i]->mesh.reset();
	
	if (m_audEngine)
		m_audEngine->Suspend();
	
    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain1.Reset();
    m_swapChain.Reset();
    m_d3dContext1.Reset();
    m_d3dContext.Reset();
    m_d3dDevice1.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}