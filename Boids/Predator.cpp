#include "Predator.h"

Predator::Predator(int id) : Boid(id)
{
	m_material.Material.Ambient = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	m_speed = 1.15f;
	m_stamina = 1.0f;
	m_fov = 1.0f;
	m_range = 1.0f;
}

void Predator::update(float t, vecBoid* boidList)
{
	XMFLOAT3 vTotal = XMFLOAT3();
	XMFLOAT3  vHunt = XMFLOAT3();

	float nearestDistance = 9999.0f;
	DrawableGameObject* nearest = nullptr;
	XMFLOAT3 directionNearestStored;

	// Checking to see if it is touching a boid, then finding the nearest target
	for (Boid* boid : *boidList)
	{
		// ignore predators
		if (boid == this || dynamic_cast<Predator*>(boid) != nullptr)
			continue;

		// get the distance between the two
		XMFLOAT3 vB = *(boid->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(vB, m_position);

		// Accounting for screen wrapping
		if (vDiff.x < -260)
			vDiff.x += 520;
		if (vDiff.y < -200)
			vDiff.y += 400;
		if (vDiff.x > 260)
			vDiff.x -= 520;
		if (vDiff.y > 200)
			vDiff.y -= 400;
		// -------------------------------

		float d = magnitudeFloat3(vDiff);

		if (d < nearestDistance)
		{
			if (d <= 1.0f)
			{
				boid->kill();
				continue;
			}

			nearestDistance = d;
			nearest = boid;
			directionNearestStored = vDiff;
		}
	}

	vHunt = normaliseFloat3(directionNearestStored);

	vTotal = multiplyFloat3(vHunt, m_speed);
	//vTotal = lerpFloat3(m_direction, vTotal, t);

	//if (isnan(vTotal.x))
		//m_position = m_position;

	if (vTotal.x != 0.0f && vTotal.y != 0.0f)
		setDirection(vTotal);

	XMFLOAT3 move = multiplyFloat3(m_direction, m_speed);
	m_position = addFloat3(m_position, move);

	/*float m = magnitudeFloat3(move);
	if (m != m_speed)
	{
		OutputDebugStringA(std::to_string(m_speed - m).c_str());
		OutputDebugStringA("\n");
	}*/

	DrawableGameObject::update(t);
}

vecBoid Predator::nearbyBoids(vecBoid* boidList)
{
	vecBoid nearBoids;
	if (boidList->size() == 0)
		return nearBoids;

	XMFLOAT3 dirNormal = normaliseFloat3(m_direction);

	for (Boid* boid : *boidList) {
		// ignore self
		if (boid == this || dynamic_cast<Predator*>(boid) != nullptr)
			continue;

		// get the distance between the two
		XMFLOAT3 vB = *(boid->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(m_position, vB);

		// Accounting for screen wrapping
		if (vDiff.x < -260)
			vDiff.x += 520;
		if (vDiff.y < -200)
			vDiff.y += 400;
		if (vDiff.x > 260)
			vDiff.x -= 520;
		if (vDiff.y > 200)
			vDiff.y -= 400;
		// -------------------------------

		float l = magnitudeFloat3(vDiff);
		XMFLOAT3 vDiffNormal = normaliseFloat3(vDiff);


		if (l < NEARBY_DISTANCE * m_range) {
			float dot = vDiffNormal.x * dirNormal.x + vDiffNormal.y * dirNormal.y + vDiffNormal.z * dirNormal.z;
			if (dot > -0.5f - (m_fov - 1.0f))
				nearBoids.push_back(boid);
		}
	}

	return nearBoids;
}
