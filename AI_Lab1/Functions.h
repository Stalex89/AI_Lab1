#pragma once
#include "stdafx.h"
#include "Curve.h"
#include "PointSet.h"
#include "Population.h"
#include <vector>

// A function for calculating fitness of taken curve 
double calculateFitness(Curve &curve, PointSet &positiveSet, PointSet &negativeSet);

// A function for creating mating pool basing on the fitness of curve
// A mating pool is the array of object pointers populated according to fitness probability
std::vector<Curve*> createMatingPool(Population &population);


// A function for crossover chosen parents, creates an offspring curve from two chosen parent curves
Curve* crossoverParents(Curve &parent1, Curve &parent2, double crossoverRate);


