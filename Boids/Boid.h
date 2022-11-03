#pragma once
#include "DrawableGameObject.h"
class Boid :
	public DrawableGameObject
{
public:
	Boid(bool isPredator);
	~Boid();

	XMFLOAT3*							getDirection() { return &m_direction; }
	void								checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj);
	void								update(float t, vecBoid* drawList);
	bool								isPredator() { return m_predator; }
	void								kill();
	bool								isDead() { return m_dead; }

protected:
	void								setDirection(XMFLOAT3 direction);

	vecBoid								nearbyBoids(vecBoid* boidList);
	XMFLOAT3							calculateSeparationVector(vecBoid* drawList);
	XMFLOAT3							calculateAlignmentVector(vecBoid* drawList);
	XMFLOAT3							calculateCohesionVector(vecBoid* drawList);
	XMFLOAT3							calculateEscapeVector(vecBoid* drawList);
	void								createRandomDirection();

	XMFLOAT3							addFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							normaliseFloat3(XMFLOAT3& f1);
	float								magnitudeFloat3(XMFLOAT3& f1);
	XMFLOAT3							multiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							divideFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							lerpFloat3(XMFLOAT3& f1, XMFLOAT3& f2, const float scalar);

	XMFLOAT3							m_direction;
	bool								m_predator, m_dead;
	float								m_speed, m_stamina, m_fov, m_range;
	//unsigned int*						m_nearbyDrawables;
};

