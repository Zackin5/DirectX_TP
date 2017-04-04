//
// Base template for Windows drawing, DirectX resource setup, etc. is taken from https://github.com/walbourn/directx-vs-templates
//

//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "..\DirectXTP\Blaster.h"
#include "..\DirectXTP\Maths.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice1;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext1;

    Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;
    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;


	///////////////////////////////////
	// Homemade stuff goes past here //
	///////////////////////////////////

	// Renderer stuff
	// Resolution (minimum is 320x200)
	int windowX = 1280;
	int windowY = 800;
	float renderrScale = 1; // Resolution scale multiplier, allows for supersampling or undersampling (altough the lack of filtering really makes the former look lacking)
	int MSAALevel = 8; // MSAA Level, a value of 1 disables it

	// Debug stuff
	bool debug = false;
	int debugState;
	float debugTime;

	// Logic stuff
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;
	DirectX::SimpleMath::Vector2 m_fontPos;

	DirectX::SimpleMath::Matrix m_view_title;
	DirectX::SimpleMath::Matrix m_view_scene1;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::IEffectFactory> m_fxFactory;
	std::unique_ptr<DirectX::SpriteFont> m_font;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// Resources
	std::unique_ptr<DirectX::GeometricPrimitive> m_sky;
	std::unique_ptr<DirectX::BasicEffect> m_sky_fx;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sky_texture;
	DirectX::SimpleMath::Matrix m_sky_world;
	DirectX::SimpleMath::Matrix m_sky_proj;

	std::unique_ptr<DirectX::Model> m_stard;
	DirectX::SimpleMath::Matrix m_stard_world;

	std::unique_ptr<DirectX::Model> m_runner;
	DirectX::SimpleMath::Matrix m_runner_world;

	std::vector<std::unique_ptr<Blaster>> o_blasters;

	//std::vector<std::unique_ptr<DirectX::Model>> m_blasters;
	//DirectX::SimpleMath::Matrix m_blaster_world;
};