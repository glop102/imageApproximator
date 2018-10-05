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
	double calculateKernelDivisor(const QList<double> &kernel);
public:
	Approximator(BaseSettings*);
	QImage applyKernel(QImage orig, const QList<double> &kernel, const bool &absolute);

	QImage combine_maximum(QList<QImage> images);
	QImage combine_extreme_channels(QList<QImage> images);
	QImage combine_extreme_pixels(QList<QImage> images);

public slots:
	void processImage(QImage orig);
};

class Settings : public BaseSettings{
	Q_OBJECT
public:
	explicit Settings();

	void addNewKernel(QList<double> kernel = {0,0,0,0,0,0,0,0,0}); // adds to the list a new kernel

	QList<QList<double> > getKernels();
	int getNumberPasses();
	bool getIfAbsolute();
public slots:
	void numberKernelsChange();
	void signedUnsignedToggled();

	virtual QString getApproximatorName();
protected:
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
