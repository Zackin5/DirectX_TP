#include "StarWars.h"
#include "Game.h"
#include "GameException.h"
#include "MatrixHelper.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "Utility.h"
#include "D3DCompiler.h"

namespace Rendering
{
	RTTI_DEFINITIONS(StarWars)

	StarWars::StarWars(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera),  
		  mEffect(nullptr), mTechnique(nullptr), mPass(nullptr), mWvpVariable(nullptr),
		  mInputLayout(nullptr), mWorldMatrix(MatrixHelper::Identity), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0)
	{
	}

	StarWars::~StarWars()
	{
		ReleaseObject(mWvpVariable);
        ReleaseObject(mPass);
        ReleaseObject(mTechnique);
        ReleaseObject(mEffect);		
        ReleaseObject(mInputLayout);
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);
	}

	void StarWars::Initialize()
	{

		#pragma region DirectX setup and checks


		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Compile the shader
        UINT shaderFlags = 0;

#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3D10Blob* compiledShader = nullptr;
        ID3D10Blob* errorMessages = nullptr;		
        HRESULT hr = D3DCompileFromFile(L"Content\\Effects\\BasicEffect.fx", nullptr, nullptr, nullptr, "fx_5_0", shaderFlags, 0, &compiledShader, &errorMessages);
		if (FAILED(hr))
		{
			char* errorMessage = (errorMessages != nullptr ? (char*)errorMessages->GetBufferPointer() : "D3DX11CompileFromFile() failed");
			GameException ex(errorMessage, hr);
			ReleaseObject(errorMessages);

			throw ex;
		}

        // Create an effect object from the compiled shader
        hr = D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, mGame->Direct3DDevice(), &mEffect);
        if (FAILED(hr))
        {
            throw GameException("D3DX11CreateEffectFromMemory() failed.", hr);
        }

        ReleaseObject(compiledShader);

        // Look up the technique, pass, and WVP variable from the effect
        mTechnique = mEffect->GetTechniqueByName("main11");
        if (mTechnique == nullptr)
        {
            throw GameException("ID3DX11Effect::GetTechniqueByName() could not find the specified technique.", hr);
        }

        mPass = mTechnique->GetPassByName("p0");
        if (mPass == nullptr)
        {
            throw GameException("ID3DX11EffectTechnique::GetPassByName() could not find the specified pass.", hr);
        }

        ID3DX11EffectVariable* variable = mEffect->GetVariableByName("WorldViewProjection");
        if (variable == nullptr)
        {
            throw GameException("ID3DX11Effect::GetVariableByName() could not find the specified variable.", hr);
        }

        mWvpVariable = variable->AsMatrix();
        if (mWvpVariable->IsValid() == false)
        {
            throw GameException("Invalid effect variable cast.");
        }

        // Create the input layout
        D3DX11_PASS_DESC passDesc;
        mPass->GetDesc(&passDesc);

        D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        if (FAILED(hr = mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout)))
        {
            throw GameException("ID3D11Device::CreateInputLayout() failed.", hr);
        }
	#pragma endregion

		// Create the vertex and index buffers
		BasicEffectVertex vertices[] =
		{
			// Square vertices
			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))),	//0
			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))),	//1
			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))),	//2
			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))),	//3

			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Purple))),	//4
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Purple))),	//5
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Purple))),	//6
			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Purple))),	//7

			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))),	//8
			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))),	//9
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))),	//10
			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))),	//11

			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Yellow))),	//2, 12
			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Yellow))),	//1, 13
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Yellow))),	//6, 14
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Yellow))),	//5, 15

			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::BlueGreen))),	//1, 16
			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::BlueGreen))),	//7, 17
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::BlueGreen))),	//6, 18
			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::BlueGreen))),	//0, 19

			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))),	//0, 20
			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))),	//3, 21
			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))),	//4, 22
			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))),	//7, 23

			// Tetra faces
			BasicEffectVertex(XMFLOAT4(+4.000f, -0.866f, +0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::White))), //24
			BasicEffectVertex(XMFLOAT4(+2.000f, -0.866f, +0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::White))), //25
			BasicEffectVertex(XMFLOAT4(+3.000f, -0.866f, -0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::White))), //26

			BasicEffectVertex(XMFLOAT4(+4.000f, -0.866f, +0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))), //27
			BasicEffectVertex(XMFLOAT4(+2.000f, -0.866f, +0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))), //28
			BasicEffectVertex(XMFLOAT4(+3.000f, +0.866f, +0.292f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))), //29

			BasicEffectVertex(XMFLOAT4(+4.000f, -0.866f, +0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))), //30
			BasicEffectVertex(XMFLOAT4(+3.000f, -0.866f, -0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))), //31
			BasicEffectVertex(XMFLOAT4(+3.000f, +0.866f, +0.292f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))), //32

			BasicEffectVertex(XMFLOAT4(+2.000f, -0.866f, +0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))), //33
			BasicEffectVertex(XMFLOAT4(+3.000f, -0.866f, -0.866f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))), //34
			BasicEffectVertex(XMFLOAT4(+3.000f, +0.866f, +0.292f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))), //35

		};

		D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
        vertexBufferDesc.ByteWidth = sizeof(BasicEffectVertex) * ARRAYSIZE(vertices);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;		
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubResourceData;
        ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
        vertexSubResourceData.pSysMem = vertices;
        if (FAILED(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, &mVertexBuffer)))
        {
            throw GameException("ID3D11Device::CreateBuffer() failed.");
        }

		UINT indices[] =
		{
			// cube faces
			// top
			0, 1, 2,
			0, 2, 3,

			// bottom
			4, 5, 6,
			4, 6, 7,

			//front
			8, 9, 10,
			8, 10, 11,

			//right
			12, 13, 14,
			12, 14, 15,

			//back
			16, 17, 18,
			16, 19, 17,

			//left
			20, 21, 22,
			20, 22, 23,

			// tetra faces
			24, 25, 26,
			27, 28, 29,
			30, 31, 32,
			33, 34, 35
		};

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(UINT) * ARRAYSIZE(indices);
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;		
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = indices;
		if (FAILED(mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, &mIndexBuffer)))
		{
			throw GameException("ID3D11Device::CreateBuffer() failed.");
		}

		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

		Mesh* mesh = model->Meshes().at(0);
		CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = mesh->Indices().size();

	}

	void StarWars::Update(const GameTime& gameTime)
	{
		mAngle += XM_PI * static_cast<float>(gameTime.ElapsedGameTime());
		XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationAxis({ 1.0f,1.0f,1.0f,1.0f }, mAngle));
	}

	void StarWars::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();        
        direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(BasicEffectVertex);
        UINT offset = 0;
        direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);		
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		        
        XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
        XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
        mWvpVariable->SetMatrix(reinterpret_cast<const float*>(&wvp));

        mPass->Apply(0, direct3DDeviceContext);

        direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}
}