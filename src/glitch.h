#ifndef GLITCHAPPROXIMATOR_H
#define GLITCHAPPROXIMATOR_H

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

namespace Glitch {
class Settings;

class Approximator : public BaseApproximator{
	Q_OBJECT
public:
	Approximator(BaseSettings*);

public slots:
	void processImage(QImage orig);
};

class Settings : public BaseSettings{
	Q_OBJECT
public:
	explicit Settings();

public slots:
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
#endif // GLITCHAPPROXIMATOR_H
