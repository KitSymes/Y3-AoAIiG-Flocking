#include "Fish.h"

// Generate stat multipliers in range 0.9f - 1.1f
#define MIN 0.9f
#define VARIATION 20

Fish::Fish()
{
	m_material.Material.Ambient = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	m_speed = MIN + (float)(rand() % VARIATION) / 100.0f;
	m_stamina = MIN + (float)(rand() % VARIATION) / 100.0f;
	m_fov = MIN + (float)(rand() % VARIATION) / 100.0f;
	m_range = MIN + (float)(rand() % VARIATION) / 100.0f;
}

void Fish::update(float t, vecBoid* boidList)
{
	XMFLOAT3 vTotal = XMFLOAT3();

	// create a list of nearby boids
	vecBoid nearBoids = nearbyBoids(boidList);

	XMFLOAT3  vSeparation = calculateSeparationVector(&nearBoids);
	XMFLOAT3  vAlignment = calculateAlignmentVector(&nearBoids);
	XMFLOAT3  vCohesion = calculateCohesionVector(&nearBoids);
	XMFLOAT3  vEscape = calculateEscapeVector(boidList);

	if (vEscape.x != 0.0f || vEscape.y != 0.0f)
		vTotal = vEscape;
	else
	{
		vTotal = addFloat3(vSeparation, vAlignment);
		vTotal = addFloat3(vTotal, vCohesion);
	}
	vTotal = normaliseFloat3(vTotal);
	vTotal = lerpFloat3(m_direction, vTotal, t);

	if (vTotal.x != 0.0f && vTotal.y != 0.0f)
		setDirection(vTotal);

	XMFLOAT3 move = multiplyFloat3(m_direction, m_speed);
	m_position = addFloat3(m_position, move);

	DrawableGameObject::update(t);
}

vecBoid Fish::nearbyBoids(vecBoid* boidList)
{
	vecBoid nearBoids;
	if (boidList->size() == 0)
		return nearBoids;

	XMFLOAT3 dirNormal = normaliseFloat3(m_direction);

	for (Boid* boid : *boidList) {
		// ignore self
		if (boid == this || dynamic_cast<Fish*>(boid) == nullptr)
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

XMFLOAT3 Fish::calculateEscapeVector(vecBoid* boidList)
{
	XMFLOAT3 escape = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr || boidList->size() <= 0)
		return escape;

	for (Boid* boid : *boidList)
		if (dynamic_cast<Fish*>(boid) == nullptr)
		{
			XMFLOAT3 vDiff = subtractFloat3(m_position, *boid->getPosition());
			float dist = magnitudeFloat3(vDiff);
			if (dist < NEARBY_DISTANCE * m_range) {
				XMFLOAT3 direction = subtractFloat3(m_position, *boid->getPosition());
				escape = addFloat3(escape, direction);
			}
		}

	return normaliseFloat3(escape); // escape is the direction away from all predators
}
