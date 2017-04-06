#pragma once
#include "..\d3d11game_win32\pch.h"
#include "Maths.h"

class BlasterFlash
{
public:
	BlasterFlash();
	BlasterFlash(std::unique_ptr<DirectX::GeometricPrimitive> primitive_sphere, DirectX::SimpleMath::Matrix spawn_location);

	void Update();

	std::unique_ptr<DirectX::GeometricPrimitive> mesh;
	DirectX::SimpleMath::Matrix world;

	bool dead = false;

private:
	int lifetime;
	int lifetime_max = 1;
};