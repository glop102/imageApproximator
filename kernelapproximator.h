#ifndef KERNELAPPROXIMATOR_H
#define KERNELAPPROXIMATOR_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QCoreApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QString>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSpinBox>
#include <QElapsedTimer>
#include "base.h"
#include <math.h>
#include <omp.h>

namespace Kernel {
class Settings;

class Approximator : public BaseApproximator{
	Q_OBJECT
protected:
	bool stopSignalRecieved;
	double calculateKernelDivisor(QList<double> kernel);
public:
	QImage applyKernel(QImage orig, const QList<double> kernel, const bool absolute);

	QImage combine_maximum(QList<QImage> images);
	QImage combine_extreme_channels(QList<QImage> images);
	QImage combine_extreme_pixels(QList<QImage> images);

public slots:
	//void processImage(QImage orig, Settings settingsHolder);
	void processImage(QImage orig, QList<QList<double> > kernels, int numberPasses, bool absolute);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

class Settings : public BaseSettings{
	Q_OBJECT
public:
	explicit Settings();

	void addNewKernel(QList<double> kernel = {0,0,0,0,0,0,0,0,0}); // adds to the list a new kernel
	QList<QList<double> > getKernels();
	int getNumberPasses();
public slots:
	void numberKernelsChange();
	void signedUnsignedToggled();

	BaseApproximator* getApproximator(); //returns a valid instance
	int startApproximator(QImage orig);
	int stopApproximator();
protected:
	Approximator* localApproximator;
	QHBoxLayout *globalLayout;

	QGroupBox *quantitySelection;
	QVBoxLayout *quantityLayout;
	QSpinBox *numberPasses,*numberKernels;

	QGroupBox *toggleSelection;
	QVBoxLayout *toggleLayout;
	QPushButton *signedUnsignedToggle;

	QList<QGroupBox*> kernelGroups;

	QDoubleValidator doubleValidator;
};

}//namespace
#endif // KERNELAPPROXIMATOR_H
