#include "circlesettings.h"

CircleSettings::CircleSettings(QWidget *parent) : QWidget(parent){
	makeWidgets();
	layoutWidgets();
	makeConnections();
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

void CircleSettings::makeWidgets(){
	description = new QLabel("This randomly chooses where to put a circle at");
	numCirclesEntry = new QSpinBox();
	minRadiusEntry = new QSpinBox();
	maxRadiusEntry = new QSpinBox();

	description->setWordWrap(true);
	numCirclesEntry->setMinimum(1);
	numCirclesEntry->setMaximum(1000000000);
	numCirclesEntry->setValue(15000);
	minRadiusEntry->setMinimum(1);
	minRadiusEntry->setMaximum(9999);
	minRadiusEntry->setValue(5);
	maxRadiusEntry->setMinimum(1);
	maxRadiusEntry->setMaximum(10000);
	maxRadiusEntry->setValue(150);
}
void CircleSettings::layoutWidgets(){
	mainLayout = new QGridLayout();
	setLayout(mainLayout);
	mainLayout->addWidget(description,0,0,1,3);
	mainLayout->addWidget(new QLabel("Num Circles"),1,0);
	mainLayout->addWidget(numCirclesEntry,2,0);
	mainLayout->addWidget(new QLabel("Min Radius"),1,1);
	mainLayout->addWidget(minRadiusEntry,2,1);
	mainLayout->addWidget(new QLabel("Max Radius"),1,2);
	mainLayout->addWidget(maxRadiusEntry,2,2);
}
void CircleSettings::makeConnections(){
	connect(minRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
	connect(maxRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
}

//=====================================================================================================================================================
//=====================================================================================================================================================
//=====================================================================================================================================================

CircleSettings_DeltaSelector::CircleSettings_DeltaSelector(QWidget *parent) : QWidget(parent){
	makeWidgets();
	layoutWidgets();
	makeConnections();
}
void CircleSettings_DeltaSelector::keepRadiusEntriesInSync(){
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

int CircleSettings_DeltaSelector::numCircles(){
	return numCirclesEntry->value();
}
int CircleSettings_DeltaSelector::minRadius(){
	return minRadiusEntry->value();
}
int CircleSettings_DeltaSelector::maxRadius(){
	return maxRadiusEntry->value();
}
void CircleSettings_DeltaSelector::makeWidgets(){
	description = new QLabel("This chooses the pixels that are the most wrong to try to put a circle at");
	numCirclesEntry = new QSpinBox();
	minRadiusEntry = new QSpinBox();
	maxRadiusEntry = new QSpinBox();

	description->setWordWrap(true);
	numCirclesEntry->setMinimum(1);
	numCirclesEntry->setMaximum(1000000000);
	numCirclesEntry->setValue(15000);
	minRadiusEntry->setMinimum(1);
	minRadiusEntry->setMaximum(9999);
	minRadiusEntry->setValue(5);
	maxRadiusEntry->setMinimum(1);
	maxRadiusEntry->setMaximum(10000);
	maxRadiusEntry->setValue(150);
}
void CircleSettings_DeltaSelector::layoutWidgets(){
	mainLayout = new QGridLayout();
	setLayout(mainLayout);
	mainLayout->addWidget(description,0,0,1,3);
	mainLayout->addWidget(new QLabel("Num Circles"),1,0);
	mainLayout->addWidget(numCirclesEntry,2,0);
	mainLayout->addWidget(new QLabel("Min Radius"),1,1);
	mainLayout->addWidget(minRadiusEntry,2,1);
	mainLayout->addWidget(new QLabel("Max Radius"),1,2);
	mainLayout->addWidget(maxRadiusEntry,2,2);
}
void CircleSettings_DeltaSelector::makeConnections(){
	connect(minRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
	connect(maxRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
}
