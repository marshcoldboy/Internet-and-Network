#pragma once
#include "opencv2/opencv.hpp"
#include <vector>
#include <algorithm>
using namespace cv;
using namespace std;

struct AlignmentPattern
{
	Point2d position;
	int count;
	double estimatedModuleSize;
	AlignmentPattern():count(1) {};
	AlignmentPattern(double posX, double posY, double estimatedModuleSize)
		:count(1), position(posX, posY), estimatedModuleSize(estimatedModuleSize) {};
	bool AboutEquals(double moduleSize, double i, double j)
	{
		if (abs(i - position.y) <= moduleSize && abs(j - position.x) <= moduleSize)
		{
			double moduleSizeDiff = abs(moduleSize - estimatedModuleSize);
			return moduleSizeDiff <= 1.0 || moduleSizeDiff / estimatedModuleSize <= 1.0;
		}
		return false;
	}
};

class AlignmentPatternFinder
{
public:
	AlignmentPatternFinder();
	AlignmentPattern* Find(Mat image, int startX, int startY, int width, int height, double moduleSize,int bound);
private:
	Mat image;
	static bool FinderPatternSort(AlignmentPattern a, AlignmentPattern b);
	int crossCheckStateCount[5] = { 0,0,0,0,0 };
	vector<AlignmentPattern> possibleCenters;
	double moduleSize;
	bool HandlePossibleCenter(int statecount[], int i, int j);
	double CrossCheckVertical(int startI, int centerJ, int maxCount, int originalStateCountTotal);
	bool FoundPatternCross(int statecount[]);
	double CenterFromEnd(int stateCount[], int end);
};