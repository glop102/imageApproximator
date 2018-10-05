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
#include <omp.h>
#include <atomic>
#include <mutex>

namespace Circle{

using std::vector;
class Settings;
class Approximator;

//================================================================
//================================================================

class Approximator : public BaseApproximator{
	Q_OBJECT
protected:
	struct Circle{
		int centerX=0,centerY=0;
		int radius=0;
		QColor color;
		double score=0; // amount of change between modified image and original image
	};

	vector<int> precomputedDistance;
	QImage currentApproximation;

	QImage origImage;
	int minRadius;
	int maxRadius;

	int randRange(int low,int high); // warning - is inclusive of the high number
	double getScore(QImage &firstImage, QImage &secondImage, int centerX, int centerY, int radius, QColor color);
	int getColorDelta(QColor c1, QColor c2);
	void drawCircle(QImage &image,struct Circle *);
	void randomSelectCurrentCircle(struct Circle *);
	bool tryPermutationForBetterCircle(struct Circle *);

public:
	Approximator(BaseSettings*);
public slots:
	virtual void processImage(QImage orig);
};

//================================================================
//================================================================

class Settings : public BaseSettings{
	Q_OBJECT
public:
	explicit Settings();
	int numCircles();
	int minRadius();
	int maxRadius();
public slots:
	void keepRadiusEntriesInSync();
	QString getApproximatorName();
protected:
	QGridLayout *mainLayout;
	QLabel *description;
	QSpinBox *numCirclesEntry,*minRadiusEntry,*maxRadiusEntry;
	virtual void makeWidgets();
	virtual void layoutWidgets();
	virtual void makeConnections();
};

} //namespace
#endif // CIRCLEAPPROXIMATOR_H
