#pragma once
#include "Boid.h"
class Predator :
    public Boid
{
public:
	Predator();

	void update(float t, vecBoid* boidList);

protected:
	vecBoid nearbyBoids(vecBoid* boidList);
};

