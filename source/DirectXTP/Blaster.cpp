#include "Blaster.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Blaster::Blaster()
{
	Blaster(nullptr, nullptr, Matrix::Identity, Matrix::Identity);
}

Blaster::Blaster(Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, std::unique_ptr<DirectX::IEffectFactory> fxFactory, DirectX::SimpleMath::Matrix origin, DirectX::SimpleMath::Matrix target)
{
	v_origin = Vector3::Transform(Vector3::Up, origin);
	v_target = Vector3::Transform(Vector3::Up, target);
	
	model = Model::CreateFromCMO(d3dDevice.Get(), L"Blaster.cmo", fxFactory, false);

	vdistance = Vector3::Distance(v_origin, v_target);
	m_world = Matrix::CreateLookAt(v_origin, v_target, Vector3::Up) * Matrix::CreateTranslation(v_origin);
}

void Blaster::Update()
{
	lerpProgress += vdistance * speed;

	if (lerpProgress < 1.f)
		m_world = Matrix::CreateTranslation(Vector3::Lerp(v_origin, v_target, lerpProgress));
	else
	{
		model.reset();
		DELETE;
	}
}