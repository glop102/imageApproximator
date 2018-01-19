#include "base.h"

void BaseApproximator::stopProcessing(){
	printf("Unable To Stop - Approximator did not implement a stop method\n");
}

BaseApproximator* BaseSettings::getApproximator(){
	printf("ERROR - Settings Widget did not implement returns a localy valid Approximator\n");
	return NULL;
}
int BaseSettings::startApproximator(QImage original){
	printf("Unable To Start - Settings Widget did not implement a way to start the Approximator\n");
	return true;
}
int BaseSettings::stopApproximator(){
	printf("Unable To Stop - Settings Widget did not implement a way to stop the Approximator\n");
	return true;
}
