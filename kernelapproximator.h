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
#include "base.h"

namespace Kernel {
class Settings;

class Approximator : public BaseApproximator{
	Q_OBJECT
protected:
	bool stopSignalRecieved;
	double calculateKernelDivisor(QList<double> kernel);
public:
	QImage applyKernel(QImage orig, QList<double> kernel);

	QImage combine_average(QList<QImage> images);
	QImage combine_maximum(QList<QImage> images);

public slots:
	//void processImage(QImage orig, Settings settingsHolder);
	void processImage(QImage orig, QList<QList<double> > kernels, int numberPasses);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

class Settings : public BaseSettings{
	Q_OBJECT
	Approximator* localApproximator;
protected:
	QHBoxLayout *globalLayout;

	QGroupBox *numberPassesSelection;
	QGridLayout *numberPassesLayout;
	QLabel *numberPassesLabel;
	QPushButton *fewerPasses,*morePasses;

	QGroupBox *kernelNumberSelection;
	QGridLayout *kernelNumberLayout;
	QLabel *numberKernelsLabel;
	QPushButton *fewerKernels,*moreKernels;

	QList<QGroupBox*> kernelGroups;

	QDoubleValidator doubleValidator;
public:
	explicit Settings();

	void addNewKernel(QList<double> kernel = {0,0,0,0,0,0,0,0,0}); // adds to the list a new kernel
	QList<QList<double> > getKernels();
	int getNumberPasses();
public slots:
	void numberKernelsChange();
	void numberPassesChanged();

	BaseApproximator* getApproximator(); //returns a valid instance
	int startApproximator(QImage orig);
	int stopApproximator();
};

}//namespace
#endif // KERNELAPPROXIMATOR_H
