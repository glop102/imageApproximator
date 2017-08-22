#include "circlesettings.h"

CircleSettings::CircleSettings(QWidget *parent) : QWidget(parent){
	numCirclesEntry = new QSpinBox();
	minRadiusEntry = new QSpinBox();
	maxRadiusEntry = new QSpinBox();

	mainLayout = new QGridLayout();
	setLayout(mainLayout);
	mainLayout->addWidget(new QLabel("Num Circles"),0,0);
	mainLayout->addWidget(numCirclesEntry,1,0);
	mainLayout->addWidget(new QLabel("Min Radius"),0,1);
	mainLayout->addWidget(minRadiusEntry,1,1);
	mainLayout->addWidget(new QLabel("Max Radius"),0,2);
	mainLayout->addWidget(maxRadiusEntry,1,2);

	numCirclesEntry->setMinimum(1);
	numCirclesEntry->setMaximum(1000000000);
	numCirclesEntry->setValue(15000);
	minRadiusEntry->setMinimum(1);
	minRadiusEntry->setMaximum(9999);
	minRadiusEntry->setValue(5);
	maxRadiusEntry->setMinimum(1);
	maxRadiusEntry->setMaximum(10000);
	maxRadiusEntry->setValue(150);

	connect(minRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
	connect(maxRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
}

void CircleSettings::keepRadiusEntriesInSync(){
	QObject *sender = QObject::sender();
	int min = minRadiusEntry->value();
	int max = maxRadiusEntry->value();

	if(min<=max) return; // makes sense, the max value is large than the min

	if(sender == minRadiusEntry){
		maxRadiusEntry->setValue(min);
	}else{
		minRadiusEntry->setValue(max);
	}
}

int CircleSettings::numCircles(){
	return numCirclesEntry->value();
}
int CircleSettings::minRadius(){
	return minRadiusEntry->value();
}
int CircleSettings::maxRadius(){
	return maxRadiusEntry->value();
}
