#ifndef EDGESETTINGS_H
#define EDGESETTINGS_H

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
#include "kernelapproximator.h"

class EdgeSettings : public QWidget
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
	explicit EdgeSettings(QWidget *parent = nullptr);

	void addNewKernel(QList<double> kernel = {0,0,0,0,0,0,0,0,0}); // adds to the list a new kernel
	QList<QList<double> > getKernels();
	KernelApproximator::combinationTypes getCombinationType();
public slots:
	void numberKernelsChange();
};

#endif // EDGESETTINGS_H
