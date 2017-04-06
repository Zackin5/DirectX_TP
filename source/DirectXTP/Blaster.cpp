#include "Blaster.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

// Yo this constructor will likely break things so if you're calling it prepare for a wild wide of CRASHES, use the full one
Blaster::Blaster()
{
	Blaster(nullptr, Matrix::Identity, Matrix::Identity, 0.f);
}

Blaster::Blaster(std::unique_ptr<DirectX::Model> blastermodel, DirectX::SimpleMath::Matrix origin, DirectX::SimpleMath::Matrix target, float spread)
{
	Matrix m_spread = Matrix::CreateTranslation(Vector3((rand() % (int)(spread * 200) - (spread * 100)) / 10000,
														(rand() % (int)(spread * 200) - (spread * 100)) / 10000,
														(rand() % (int)(spread * 200) - (spread * 100)) / 10000));

	// Get the origin and target vectors
	v_origin = Vector3::Transform(Vector3::Zero, origin);
	v_target = Vector3::Transform(Vector3::Zero, m_spread * target);
	
	// Set the model using the passed one
	model = std::move(blastermodel);
	
	// Calculate rotation and the travel distance
	vdistance = Vector3::Distance(v_origin, v_target);
	m_world = Matrix::Identity;

	// Calcuate flight angle
	Vector3 v_normal = v_target - v_origin;
	v_normal.Normalize();

	m_rotation = Matrix::CreateLookAt(Vector3::Zero, v_normal, Vector3::Down);
	m_rotation.Transpose();
}

void Blaster::Update()
{
	lerpProgress += vdistance * speed * 0.001f;

	if (lerpProgress < lifetime)
		m_world = m_rotation * Matrix::CreateTranslation(Vector3::Lerp(v_origin, v_target, lerpProgress));
	else
		dead = true;
}