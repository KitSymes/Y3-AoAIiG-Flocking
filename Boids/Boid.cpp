#include "Boid.h"



Boid::Boid()
{
	m_position = XMFLOAT3(0, 0, 0);
	//m_direction = XMFLOAT3(0, 1, 0);
	createRandomDirection();
	setScale(1);
	m_dead = false;
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

	for (Boid* boid : *boidList)
		nearby = addFloat3(nearby, *boid->getDirection());

	nearby = divideFloat3(nearby, (float)boidList->size());
	nearby = normaliseFloat3(nearby);
	nearby = multiplyFloat3(nearby, 4.0f);

	return nearby;
}

XMFLOAT3 Boid::calculateCohesionVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr || boidList->size() <= 0)
		return nearby;

	for (Boid* boid : *boidList)
		nearby = addFloat3(nearby, *boid->getPosition());

	nearby = divideFloat3(nearby, (float)boidList->size());
	nearby = subtractFloat3(nearby, m_position);

	if (magnitudeFloat3(nearby) > NEARBY_DISTANCE * m_range / 3.0f)
	{
		nearby = normaliseFloat3(nearby);
		return multiplyFloat3(nearby, 2.0f);
	}

	return normaliseFloat3(nearby);
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
	return *boidList;
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