#pragma once
#include "..\d3d11game_win32\pch.h"
#include "Maths.h"

class Blaster
{
public:
	Blaster();
	Blaster(std::unique_ptr<DirectX::Model> blaster_model, DirectX::SimpleMath::Matrix origin_matrix, DirectX::SimpleMath::Matrix target_matrix, float spread_amount);
	
	void Update();
	std::unique_ptr<DirectX::Model> model;

	float speed = 5.f;
	float lifetime = 1.f;
	DirectX::SimpleMath::Matrix m_world;

	bool dead = false;

private:

	DirectX::SimpleMath::Vector3 v_origin;
	DirectX::SimpleMath::Vector3 v_target;

	float vdistance;
	float lerpProgress = 0;
};