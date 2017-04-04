#pragma once
#include "..\d3d11game_win32\pch.h"
#include "Maths.h"

class Blaster
{
public:
	Blaster();
	Blaster(std::unique_ptr<DirectX::Model> blaster_model, DirectX::SimpleMath::Matrix origin_matrix, DirectX::SimpleMath::Matrix target_matrix);
	
	void Update();
	std::unique_ptr<DirectX::Model> model;

	float speed = 0.25;
	DirectX::SimpleMath::Matrix m_world;

private:

	DirectX::SimpleMath::Vector3 v_origin;
	DirectX::SimpleMath::Vector3 v_target;

	float vdistance;
	float lerpProgress = 0;
};