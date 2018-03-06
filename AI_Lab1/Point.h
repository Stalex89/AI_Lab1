#pragma once

// A Point class represents point on the coordinate system
class Point
{
	double m_x;
	double m_y;
public:
	Point(bool isPositive, double minX, double maxX, double minY, double maxY);
	Point(double x, double y);
	void setX(double x);
	void setY(double y);
	double getX();
	double getY();
	void printPoint();
};
