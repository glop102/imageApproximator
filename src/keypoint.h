#ifndef KEYPOINT_H
#define KEYPOINT_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QCoreApplication>
#include <QString>
#include <math.h>
#include "base.h"

namespace Keypoint {

class Approximator : public BaseApproximator{
	Q_OBJECT
protected:
	bool stopSignalRecieved;
	double calculateKernelDivisor(QList<double> kernel);
	QList<double> createKernel(double sigma, int size);
public:
	QImage applyKernel(QImage orig, QList<double> kernel, bool absolute);

	QImage combine_maximum(QList<QImage> images);
	QImage combine_extreme(QList<QImage> images);
	QImage combine_subtract(QImage,QImage);

public slots:
	void processImage(QImage orig);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

class Settings : public BaseSettings{
	Q_OBJECT
public:
	explicit Settings();
public slots:
	BaseApproximator* getApproximator(); //returns a valid instance
	int startApproximator(QImage orig);
	int stopApproximator();
protected:
	Approximator* localApproximator;
};

}//namespace
#endif // KEYPOINT_H
