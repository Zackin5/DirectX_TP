#include "Blaster.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Blaster::Blaster()
{
	Blaster(nullptr, Matrix::Identity, Matrix::Identity);
}

Blaster::Blaster(std::unique_ptr<DirectX::Model> blastermodel, DirectX::SimpleMath::Matrix origin, DirectX::SimpleMath::Matrix target)
{
	// Get the origin and target vectors
	v_origin = Vector3::Transform(Vector3::Zero, origin);
	v_target = Vector3::Transform(Vector3::Zero, target);
	
	// Set the model using the passed one
	model = std::move(blastermodel);

	// Calculate rotation and the travel distance
	vdistance = Vector3::Distance(v_origin, v_target);
	m_world = Matrix::CreateLookAt(v_origin, v_target, Vector3::Up) * Matrix::CreateTranslation(v_origin);
	//m_world = Matrix::CreateTranslation(v_origin) * Matrix::CreateLookAt(v_origin, v_target, Vector3::Up);
}

void Blaster::Update()
{
	lerpProgress += vdistance / speed;

	if (lerpProgress < 1.f)
		m_world = Matrix::CreateTranslation(Vector3::Lerp(v_origin, v_target, lerpProgress));
	/*else
	{
		model.reset();
		DELETE;
	}*/
}