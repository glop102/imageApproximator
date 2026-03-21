#ifndef GLITCHAPPROXIMATOR_H
#define GLITCHAPPROXIMATOR_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QCoreApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QString>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QElapsedTimer>
#include <tuple>
#include "base.h"
#include <math.h>

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

	bool getIsRotate();
	QList<std::tuple<double,double,double>> getEffects();

public slots:
	void numberEffectsChange();
	void shiftRotateToggled();
	void addNewEffect();

	virtual QString getApproximatorName();
protected:
	QHBoxLayout *globalLayout;

	QGroupBox *controlGroup;
	QVBoxLayout *controlLayout;
	QSpinBox *numberEffects;
	QPushButton *shiftRotateToggle;

	QList<QGroupBox*> effectGroups;
};

}//namespace
#endif // GLITCHAPPROXIMATOR_H
