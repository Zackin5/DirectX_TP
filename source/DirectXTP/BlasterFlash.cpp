#include "BlasterFlash.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

BlasterFlash::BlasterFlash()
{
	BlasterFlash(nullptr, Matrix::Identity);
}

BlasterFlash::BlasterFlash(std::unique_ptr<DirectX::GeometricPrimitive> m_primitive, DirectX::SimpleMath::Matrix location)
{
	mesh = std::move(m_primitive);
	world = location;
}

void BlasterFlash::Update()
{
	lifetime++;
	if (lifetime > lifetime_max)
		dead = true;
}