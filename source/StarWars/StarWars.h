#pragma once

#include "DrawableGameComponent.h"

#include "Model.h"
#include "Mesh.h"
#include "WICTextureLoader.h"

using namespace Library;

namespace Library
{
	class Keyboard;
}

namespace Rendering
{
	class StarWars : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(StarWars, DrawableGameComponent)

	public:
		StarWars(Game& game, Camera& camera);
		~StarWars();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		typedef struct _BasicEffectVertex
        {
            XMFLOAT4 Position;
            XMFLOAT4 Color;

            _BasicEffectVertex() { }

            _BasicEffectVertex(XMFLOAT4 position, XMFLOAT4 color)
                : Position(position), Color(color) { }
        } BasicEffectVertex;

		typedef struct _TextureMappingVertex
		{
			XMFLOAT4 Position;
			XMFLOAT2 TextureCoordinates;

			_TextureMappingVertex() { }

			_TextureMappingVertex(XMFLOAT4 position, XMFLOAT2 textureCoordinates)
				: Position(position), TextureCoordinates(textureCoordinates) { }
		} TextureMappingVertex;

		StarWars();
		StarWars(const StarWars& rhs);
		StarWars& operator=(const StarWars& rhs);

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		ID3DX11Effect* mEffect;
        ID3DX11EffectTechnique* mTechnique;
        ID3DX11EffectPass* mPass;
        ID3DX11EffectMatrixVariable* mWvpVariable;

        ID3D11InputLayout* mInputLayout;		
        ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		XMFLOAT4X4 mWorldMatrix;	
		float mAngle;
	};
}
