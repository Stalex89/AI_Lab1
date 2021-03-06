#include "stdafx.h"
#include "Point.h"
#include "PointSet.h"
#include "Coefficient.h"
#include "Curve.h"
#include "Population.h"
#include "Functions.h"
#include "Gnuplot.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

// Define constants
constexpr int minCoefficient{ -127 };
constexpr int maxCoefficient{ 127 };
constexpr double pointMinX{ -20 };
constexpr double pointMaxX{ 20 };
constexpr double pointMinY{ -10 };
constexpr double pointMaxY{ 10 };
constexpr int populationSize{ 30 };
constexpr int positivePointsSize{ 100 };
constexpr int negativePointsSize{ 100 };
constexpr double crossoverProportion{ 0.5 };
constexpr double mutationRate{ 1.0 };
constexpr int maxGeneration{ 100 };
constexpr int polynomialDegree{ 2 };



void GeneticAlgorithm();
double GetMaxFitness(std::vector<double> &fitnesses);
double GetMinFitness(std::vector<double> &fitnesses);
double GetAvgFitness(std::vector<double> &fitnesses);
Curve* ChooseParent(Population &pop, double fitnessSum);
void ExportData(std::vector<double> &data, std::string filename);
void ExportData(std::vector<Point> points, std::string filename);
void PlotGraph(std::vector<int> &bestCoefficients);
std::string GetPlotFunction(std::vector<int> &bestCoefficients);


int main()
{
	GeneticAlgorithm();

	system("pause");
	return 0;
}

void GeneticAlgorithm()
{
	// Initialize statistical data
	std::vector<double> bestFitness;
	std::vector<double> worstFitness;
	std::vector<double> avgFitness;
	std::vector<double> popFitnesses;
	std::vector<int> bestCoefficients{};
	int bestGeneration = 0;
	double bestfit = 0.0;
	
	// Itinialize initial best coefficients
	for (unsigned int i = 0; i <= polynomialDegree; i++)
		bestCoefficients.push_back(0);

	// Initialise initial population, sets of positive and negative points
	PointSet *ppos = new PointSet(positivePointsSize, true, pointMinX, pointMaxX, pointMinY, pointMaxY);
	PointSet *pneg = new PointSet(negativePointsSize, false, pointMinX, pointMaxX, pointMinY, pointMaxY);
	Population *pop = new Population(populationSize, polynomialDegree, minCoefficient, maxCoefficient);

	// Calculate fitness for current generation
	for (int i = 0; i < pop->getPopulationSize(); i++)
	{
		Curve *currCurve = pop->getCurveAt(i);
		double fitness = calculateFitness(*currCurve, *ppos, *pneg);
		currCurve->setFitness(fitness);
		popFitnesses.push_back(fitness);

		if (fitness > bestfit)
		{
			bestGeneration = pop->getGenerationNum();
			bestfit = fitness;
			for (unsigned int k = 0; k < bestCoefficients.size(); k++)
				bestCoefficients.at(k) = currCurve->getCoefficientAt(k)->getNumber();
		}
		currCurve = nullptr;
	}

	// Set initial best, worst and average fitness;
	bestFitness.push_back(GetMaxFitness(popFitnesses));
	worstFitness.push_back(GetMinFitness(popFitnesses));
	avgFitness.push_back(GetAvgFitness(popFitnesses));
	popFitnesses.clear();


	for (unsigned int g = 1; g <= maxGeneration; g++)
	{
		// Create mating pool for crossovering individuals
		//std::vector<Curve*> matingPool = createMatingPool(*pop);

		// Calculate sum of all fitnesses of the current population
		double fitnessSum = 0;
		for (int f = 0; f < populationSize; f++)
			fitnessSum += pop->getCurveAt(f)->getFitness();

		// Create vector of new generation
		std::vector<Curve*> *newGenSet = new std::vector<Curve*>();

		// crossover 200 new individuals by randomly picking two parents from the pool
		for (int i = 0; i < populationSize; i++)
		{
			//Curve *parent1 = matingPool.at(getRandomNumber(0, matingPool.size()-1));
			//Curve *parent2 = matingPool.at(getRandomNumber(0, matingPool.size()-1));
			Curve *parent1 = ChooseParent(*pop, fitnessSum);
			Curve *parent2 = ChooseParent(*pop, fitnessSum);
			
			//Curve *child = crossoverParents(*parent1, *parent2, crossoverProportion);
			Curve *child = crossoverParents(*parent1, *parent2, crossoverProportion);

			// Mutate child
			//for (int j = 0; j < child->getDegree() + 1; j++)
			//	if (getRandomNumber(0.0, 1.0) < mutationRate)
			//	{
			//		//std::cout << "mutate child's gene #" << j << "\n";
			//		child->getCoefficientAt(j)->mutateCoefficient(mutationBits);
			//	}

			// Mutate child
			if (getRandomNumber(0.0, 1.0) < mutationRate)
				for (int j = 0; j < child->getDegree() + 1; j++)
				{
					//std::cout << "mutate child's gene #" << j << "\n";
					//child->getCoefficientAt(j)->mutateCoefficient(mutationBits);
					child->getCoefficientAt(j)->mutateCoefficient();
				}

			// Calculate fitness for the child
			double fitness = calculateFitness(*child, *ppos, *pneg);
			child->setFitness(fitness);

			// Push the fitness of the child to the statistical vector
			popFitnesses.push_back(fitness);

			// Save coefficients if the fitness was the best
			if (fitness > bestfit)
			{
				bestGeneration = pop->getGenerationNum();
				bestfit = fitness;
				for (unsigned int k = 0; k < bestCoefficients.size(); k++)
					bestCoefficients.at(k) = child->getCoefficientAt(k)->getNumber();
			}


			// Push the new induvidual to new generation vector
			newGenSet->push_back(child);

			parent1 = nullptr;
			parent2 = nullptr;
			child = nullptr;
		}

		bestFitness.push_back(GetMaxFitness(popFitnesses));
		worstFitness.push_back(GetMinFitness(popFitnesses));
		avgFitness.push_back(GetAvgFitness(popFitnesses));
		popFitnesses.clear();

		// Create new Population set using vector of new generation individuals
		delete pop;
		pop = new Population(newGenSet);
		newGenSet = nullptr;

		// If we found the best solution possible (100%), break the loop
		if (bestFitness.back() == 1.00)
			break;
	}

	// Export all statistical data to .dat files for gnuplot
	ExportData(bestFitness, "BestFitnessData.dat");
	ExportData(worstFitness, "WorstFitnessData.dat");
	ExportData(avgFitness, "AvgFitnessData.dat");
	ExportData(ppos->getPoints(), "PositivePointsData.dat");
	ExportData(pneg->getPoints(), "NegativePointsData.dat");
	
	std::cout << "Best generation: " << bestGeneration << std::endl;
	std::cout << "Best coefficients are: ";
	for (unsigned int m = 0; m < bestCoefficients.size(); m++)
		std::cout << bestCoefficients.at(m) << " ";
	std::cout << std::endl;

	// Plot function
	PlotGraph(bestCoefficients);

	// Memory deallocation
	delete ppos;
	ppos = nullptr;
	delete pneg;
	pneg = nullptr;
	delete pop;
	pop = nullptr;

}

double GetMaxFitness(std::vector<double> &fitnesses)
{
	double maxFitness = 0.0;
	for (unsigned int i = 0; i < fitnesses.size(); i++)
		if (fitnesses.at(i) > maxFitness)
			maxFitness = fitnesses.at(i);
	return maxFitness;
}

double GetMinFitness(std::vector<double> &fitnesses)
{
	double minFitness = 100.0;
	for (unsigned int i = 0; i < fitnesses.size(); i++)
		if (fitnesses.at(i) < minFitness)
			minFitness = fitnesses.at(i);
	return minFitness;
}

double GetAvgFitness(std::vector<double> &fitnesses)
{
	double sumFitness = 0.0;
	for (unsigned int i = 0; i < fitnesses.size(); i++)
		sumFitness += fitnesses.at(i);
	return sumFitness / fitnesses.size();
}

Curve* ChooseParent(Population &pop, double fitnessSum)
{
	double randomNum = getRandomNumber(0.0, fitnessSum);
	double tempSum = 0;
	for (int i = 0; i < pop.getPopulationSize(); i++)
	{
		if (tempSum >= randomNum)
			return pop.getCurveAt(i);
		tempSum += pop.getCurveAt(i)->getFitness();
	}
	return pop.getCurveAt(pop.getPopulationSize() - 1);
	
}

void ExportData(std::vector<double> &data, std::string filename)
{
	ofstream outf(filename);

	if (!outf)
	{
		std::cerr << "Cannot open " << filename << " for writing" << std::endl;
		exit(1);
	}
	//std::cout << "File " << filename << " opened successfully" << std::endl;

	for (unsigned int i = 0; i < data.size(); i++)
		outf << i << '\t' << data.at(i) << endl;
	outf.close();
}

void ExportData(std::vector<Point> points, std::string filename)
{
	ofstream outf(filename);

	if (!outf)
	{
		std::cerr << "Cannot open " << filename << " for writing" << std::endl;
		exit(1);
	}
	//std::cout << "File " << filename << " opened successfully" << std::endl;

	for (unsigned int i = 0; i < points.size(); i++)
		outf << points.at(i).getX() << '\t' << points.at(i).getY() << endl;
	outf.close();
}

void PlotGraph(std::vector<int> &bestCoefficients)
{
	std::string function = GetPlotFunction(bestCoefficients);

	Gnuplot fitPlot;
	fitPlot("set title \"Genetic Algorithm fitness values\"");
	fitPlot("set grid");
	fitPlot("set xlabel 'generation'");
	fitPlot("set ylabel 'fitness value'");
	fitPlot("set xtics 1");
	fitPlot("set ytics 0.1");
	fitPlot("plot [:] [0.00:1.50] \'C:\\Users\\aleks\\Documents\\Visual Studio 2017\\Projects\\AI_Lab1\\AI_Lab1\\BestFitnessData.dat\' lc rgb \'green\' title \'Best Fitness\' with lines, \'C:\\Users\\aleks\\Documents\\Visual Studio 2017\\Projects\\AI_Lab1\\AI_Lab1\\WorstFitnessData.dat\' lc rgb \'blue\' title \'Worst Fitness\' with lines, \'C:\\Users\\aleks\\Documents\\Visual Studio 2017\\Projects\\AI_Lab1\\AI_Lab1\\AvgFitnessData.dat\' lc rgb \'red\' title \'Average Fitness\' with lines");
	
	Gnuplot pointPlot;
	pointPlot("set title \"Best Curve\"");
	pointPlot("set grid");
	pointPlot("set xtics 1");
	pointPlot("set ytics 1");
	pointPlot("plot [-15:15] [-1:15] \'C:\\Users\\aleks\\Documents\\Visual Studio 2017\\Projects\\AI_Lab1\\AI_Lab1\\PositivePointsData.dat\' lc rgb \'red\' title \'Positive Points\', \'C:\\Users\\aleks\\Documents\\Visual Studio 2017\\Projects\\AI_Lab1\\AI_Lab1\\NegativePointsData.dat\' lc rgb \'blue\' title \'Negative Points\'," + function);
}

std::string GetPlotFunction(std::vector<int> &bestCoefficients)
{
	int coefNum = bestCoefficients.size();
	switch (coefNum)
	{
	case 2:
		return "(" +
			std::to_string(bestCoefficients.at(0)) + "*x)+(" +
			std::to_string(bestCoefficients.at(1)) + ") lc rgb \'black\' title \'Best Function'";
	case 3:
		return "(" + 
			std::to_string(bestCoefficients.at(0)) + "*x**2)+(" + 
			std::to_string(bestCoefficients.at(1)) + "*x)+(" +
			std::to_string(bestCoefficients.at(2)) + ") lc rgb \'black\' title \'Best Function'";
	case 4:
		return "(" + 
			std::to_string(bestCoefficients.at(0)) + "*x**3)+(" + 
			std::to_string(bestCoefficients.at(1)) + "*x**2)+(" +
			std::to_string(bestCoefficients.at(2)) + "*x)+(" + 
			std::to_string(bestCoefficients.at(3)) + ") lc rgb \'black\' title \'Best Function'";
	case 5:
		return "(" +
			std::to_string(bestCoefficients.at(0)) + "*x**4)+(" +
			std::to_string(bestCoefficients.at(1)) + "*x**3)+(" +
			std::to_string(bestCoefficients.at(2)) + "*x**2)+(" +
			std::to_string(bestCoefficients.at(3)) + "*x)+(" +
			std::to_string(bestCoefficients.at(4)) + ") lc rgb \'black\' title \'Best Function'";
	case 6:
		return "(" +
			std::to_string(bestCoefficients.at(0)) + "*x**5)+(" +
			std::to_string(bestCoefficients.at(1)) + "*x**4)+(" +
			std::to_string(bestCoefficients.at(2)) + "*x**3)+(" +
			std::to_string(bestCoefficients.at(3)) + "*x**2)+(" +
			std::to_string(bestCoefficients.at(4)) + "*x)+(" +
			std::to_string(bestCoefficients.at(5)) + ") lc rgb \'black\' title \'Best Function'";
	default:
		return "";
	}
}
