#ifndef CIRCLEAPPROXIMATOR_H
#define CIRCLEAPPROXIMATOR_H

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QCoreApplication>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <QTime>
#include <random>
#include <QWidget>
#include <QGridLayout>
#include <QSpinBox>
#include <QLabel>
#include "base.h"

namespace Circle{

using std::vector;
class Settings;

class Approximator : public BaseApproximator{
	Q_OBJECT
private:
	// temp vars for use in the processImage method - we save them externally so that i can call a method to do some work and not copy/paste the same thing 6 times
	int curtX,nextX;
	int curtY,nextY;
	int curtRadius,nextRadius;
	QColor currentColor;
	QImage newImage;
	double currentScore; // amount of change between modified image and original image
	vector<int> precomputedDistance;

	//other stuff
	QImage origImage;
	bool keepGoing;
	int minRadius;
	int maxRadius;

	int randRange(int low,int high); // warning - is inclusive of the high number
	double getScore(QImage &firstImage, QImage &secondImage, QColor color, int x,int y, int radius);
	int getColorDelta(QColor c1, QColor c2);
	void drawCircle(QImage &image,int x, int y, int radius, QColor color);
	void randomSelectCurrentCircle();
	bool tryPermutationAndMakeNextIfBetter(int x, int y, int radius);

public slots:
	void processImage(QImage orig, int numCircles, int minRadius, int maxRadius);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

class Settings : public BaseSettings{
	Q_OBJECT
	Approximator* localApproximator;
public:
	explicit Settings();
	int numCircles();
	int minRadius();
	int maxRadius();
public slots:
	void keepRadiusEntriesInSync();
	BaseApproximator* getApproximator(); //returns a valid instance
	int startApproximator(QImage orig);
	int stopApproximator();
protected:
	QGridLayout *mainLayout;
	QLabel *description;
	QSpinBox *numCirclesEntry,*minRadiusEntry,*maxRadiusEntry;
	void makeWidgets();
	void layoutWidgets();
	void makeConnections();
};

} //namespace
#endif // CIRCLEAPPROXIMATOR_H
