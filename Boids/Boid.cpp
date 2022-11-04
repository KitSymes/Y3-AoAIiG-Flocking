#include "Boid.h"


#define NEARBY_DISTANCE		50.0f	// how far boids can see

Boid::Boid(bool isPredator)
{
	m_position = XMFLOAT3(0, 0, 0);
	//m_direction = XMFLOAT3(0, 1, 0);
	createRandomDirection();
	setScale(1);
	m_predator = isPredator;
	m_dead = false;

	if (isPredator)
	{
		m_material.Material.Ambient = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

		m_speed = 1.15f;
		m_stamina = 1.0f;
		m_fov = 1.0f;
		m_range = 1.0f;
	}
	else
	{
		m_material.Material.Ambient = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
		//m_material.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
		//m_material.Material.SpecularPower = 32.0f;
		//m_material.Material.UseTexture = false;

		// Generate stat multipliers in range 0.9f - 1.1f
		m_speed = 0.9f + (float)(rand() % 20) / 100.0f;
		m_stamina = 0.9f + (float)(rand() % 20) / 100.0f;
		m_fov = 0.9f + (float)(rand() % 20) / 100.0f;
		m_range = 0.9f + (float)(rand() % 20) / 100.0f;
	}
}

Boid::~Boid()
{
}

void Boid::createRandomDirection()
{
	float x = (float)(rand() % 10);
	x -= 5;
	float y = (float)(rand() % 10);
	y -= 5;
	float z = 0;
	setDirection(XMFLOAT3(x, y, z));
}

void Boid::setDirection(XMFLOAT3 direction)
{
	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_direction, v);
}


#define DIRECTION_DELTA 0.1f
void Boid::update(float t, vecBoid* boidList)
{
	XMFLOAT3 vTotal = XMFLOAT3();

	if (isPredator())
	{
		XMFLOAT3  vHunt = XMFLOAT3();

		float nearestDistance = 9999.0f;
		DrawableGameObject* nearest = nullptr;
		XMFLOAT3 directionNearestStored;

		for (Boid* boid : *boidList)
		{
			// ignore predators
			if (boid == this || boid->isPredator())
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

		if (nearest != nullptr) {
			vHunt = normaliseFloat3(directionNearestStored);
		}

		vTotal = vHunt;
		//vTotal = lerpFloat3(m_direction, vTotal, t);

		if (isnan(vTotal.x))
			m_position = m_position;

		if (vTotal.x != 0.0f && vTotal.y != 0.0f)
			setDirection(vTotal);
	}
	else
	{
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

		if (isnan(vTotal.x))
			m_position = m_position;

		if (vTotal.x != 0.0f && vTotal.y != 0.0f)
			setDirection(vTotal);
	}

	XMFLOAT3 move = multiplyFloat3(m_direction, m_speed);
	m_position = addFloat3(m_position, move);

	DrawableGameObject::update(t);
}

void Boid::kill()
{
	m_dead = true;
}

XMFLOAT3 Boid::calculateSeparationVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr || boidList->size() <= 0)
		return nearby;

	// calculate average position of nearby

	float nearestDistance = 9999.0f;
	DrawableGameObject* nearest = nullptr;
	XMFLOAT3 directionNearestStored;

	for (Boid* boid : *boidList) {
		if (boid == this)
			continue;

		XMFLOAT3 mePos = m_position;
		XMFLOAT3 itPos = *boid->getPosition();

		XMFLOAT3 directionNearest = subtractFloat3(mePos, itPos);
		float d = magnitudeFloat3(directionNearest);
		if (d < nearestDistance)
		{
			nearestDistance = d;
			nearest = boid;
			directionNearestStored = directionNearest;
		}
	}

	if (nearest != nullptr) {
		directionNearestStored = normaliseFloat3(directionNearestStored);
		if (nearestDistance < 4.0f)
			directionNearestStored = multiplyFloat3(directionNearestStored, 2.0f);
		else if (nearestDistance > 20.0f)
			directionNearestStored = divideFloat3(directionNearestStored, 2.0f);
		return directionNearestStored;
	}

	return nearby;
}

XMFLOAT3 Boid::calculateAlignmentVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr || boidList->size() <= 0)
		return nearby;

	// your code here
	for (Boid* boid : *boidList)
		nearby = addFloat3(nearby, *boid->getDirection());

	nearby = divideFloat3(nearby, (float)boidList->size());

	return normaliseFloat3(nearby); // return the normalised (average) direction of nearby drawables
}

XMFLOAT3 Boid::calculateCohesionVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr || boidList->size() <= 0)
		return nearby;

	// calculate average position of nearby
	for (Boid* boid : *boidList)
		nearby = addFloat3(nearby, *boid->getPosition());

	nearby = divideFloat3(nearby, (float)boidList->size());
	nearby = subtractFloat3(nearby, m_position);

	if (magnitudeFloat3(nearby) > NEARBY_DISTANCE * m_range / 2.0f)
	{
		nearby = normaliseFloat3(nearby);
		return multiplyFloat3(nearby, 2.0f);
	}

	return normaliseFloat3(nearby); // nearby is the direction to where the other drawables are
}

XMFLOAT3 Boid::calculateEscapeVector(vecBoid* boidList)
{
	XMFLOAT3 escape = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr || boidList->size() <= 0)
		return escape;

	for (Boid* boid : *boidList)
		if (boid->isPredator())
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



// use but don't alter the methods below

XMFLOAT3 Boid::addFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x + f2.x;
	out.y = f1.y + f2.y;
	out.z = f1.z + f2.z;

	return out;
}

XMFLOAT3 Boid::subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x - f2.x;
	out.y = f1.y - f2.y;
	out.z = f1.z - f2.z;

	return out;
}

XMFLOAT3 Boid::multiplyFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x * scalar;
	out.y = f1.y * scalar;
	out.z = f1.z * scalar;

	return out;
}

XMFLOAT3 Boid::divideFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x / scalar;
	out.y = f1.y / scalar;
	out.z = f1.z / scalar;

	return out;
}

XMFLOAT3 Boid::lerpFloat3(XMFLOAT3& f1, XMFLOAT3& f2, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x + (f2.x - f1.x) * scalar;
	out.y = f1.y + (f2.y - f1.y) * scalar;
	out.z = f1.z + (f2.z - f1.z) * scalar;

	return out;
}

float Boid::magnitudeFloat3(XMFLOAT3& f1)
{
	return sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
}

XMFLOAT3 Boid::normaliseFloat3(XMFLOAT3& f1)
{
	// DO NOT normalise if magnitude 0
	if (f1.x == 0.0f && f1.y == 0.0f && f1.z == 0.0f)
		return f1;

	float length = sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));

	f1.x /= length;
	f1.y /= length;
	f1.z /= length;

	return f1;
}

vecBoid Boid::nearbyBoids(vecBoid* boidList)
{
	vecBoid nearBoids;
	if (boidList->size() == 0)
		return nearBoids;

	XMFLOAT3 dirNormal = normaliseFloat3(m_direction);

	for (Boid* boid : *boidList) {
		// ignore self
		if (boid == this || boid->isPredator())
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


void Boid::checkIsOnScreenAndFix(const XMMATRIX& view, const XMMATRIX& proj)
{
	XMFLOAT4 v4;
	v4.x = m_position.x;
	v4.y = m_position.y;
	v4.z = m_position.z;
	v4.w = 1.0f;

	XMVECTOR vScreenSpace = XMLoadFloat4(&v4);
	XMVECTOR vScreenSpace2 = XMVector4Transform(vScreenSpace, view);
	XMVECTOR vScreenSpace3 = XMVector4Transform(vScreenSpace2, proj);

	XMFLOAT4 v;
	XMStoreFloat4(&v, vScreenSpace3);
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;
	v.w /= v.w;

	float fOffset = 10; // a suitable distance to rectify position within clip space
	if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	{
		if (v.x < -1 || v.x > 1) {
			v4.x = -v4.x + (fOffset * v.x);
		}
		else if (v.y < -1 || v.y > 1) {
			v4.y = -v4.y + (fOffset * v.y);
		}

		// throw a bit of randomness into the mix
		//createRandomDirection();

		// method 1 - appear on the other side
		m_position.x = v4.x;
		m_position.y = v4.y;
		m_position.z = v4.z;

		// method2 - bounce off sides and head to centre
		/*if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
		{
			m_direction = multiplyFloat3(m_direction, -1);;
			m_direction = normaliseFloat3(m_direction);
		}*/
	}

	return;
}