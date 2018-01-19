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
#include <QList>
#include <QString>
#include <QLineEdit>
#include <QDoubleValidator>

namespace Kernel {

class Approximator : public QObject
{
	Q_OBJECT
protected:
	bool stopSignalRecieved;
	double calculateKernelDivisor(QList<double> kernel);
public:
	enum combinationTypes{AVERAGE,MAXIMUM};
	explicit Approximator(QObject *parent = nullptr);
	QImage applyKernel(QImage orig, QList<double> kernel);

	QImage combine_average(QList<QImage> images);
	QImage combine_maximum(QList<QImage> images);

public slots:
	void processImage(QImage orig, QList<QList<double> > kernels, Kernel::Approximator::combinationTypes combType);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

class Settings : public QWidget
{
	Q_OBJECT
protected:
	QHBoxLayout *globalLayout;

	QGroupBox *combinationOptions;
	QVBoxLayout *combinationLayout;
	QRadioButton *averageButton,*maxButton;

	QGroupBox *kernelNumberSelection;
	QGridLayout *kernelNumberLayout;
	QLabel *numberKernelsLabel;
	QPushButton *fewerKernels,*moreKernels;

	QList<QGroupBox*> kernelGroups;

	QDoubleValidator doubleValidator;
public:
	explicit Settings(QWidget *parent = nullptr);

	void addNewKernel(QList<double> kernel = {0,0,0,0,0,0,0,0,0}); // adds to the list a new kernel
	QList<QList<double> > getKernels();
	Approximator::combinationTypes getCombinationType();
public slots:
	void numberKernelsChange();
};

}//namespace
#endif // KERNELAPPROXIMATOR_H
