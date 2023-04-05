#pragma once
#include "Boid.h"
class Fish :
    public Boid
{
public:
	Fish(int id);

	void update(float t, vecBoid* boidList);
protected:
	vecBoid nearbyBoids(vecBoid* boidList);
	XMFLOAT3 calculateEscapeVector(vecBoid* boidList);
};

