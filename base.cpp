#include "base.h"

//==========================================================================================
// BaseApproximator
//==========================================================================================

BaseApproximator::BaseApproximator(BaseSettings *settings):settingsObject(settings){}

void BaseApproximator::stopProcessing(){
	printf("Trying to stop approximator\n");
	keepGoing = false;
}
void BaseApproximator::processImage(QImage orig){
	printf("Approximator did not define a method to process an image\n");
	emit doneProcessing(orig);
}

//==========================================================================================
// BaseSettings
//==========================================================================================

BaseSettings::BaseSettings(){
	localApproximator = NULL;
}

BaseApproximator* BaseSettings::getApproximator(){
	if(localApproximator == NULL)
		printf("ERROR - Settings Widget did not set the approximator variable\n");
	return localApproximator;
}
int BaseSettings::startApproximator(QImage original){
	return
	QMetaObject::invokeMethod(localApproximator,"processImage",
							  Q_ARG(QImage,original)
							  );
}
int BaseSettings::stopApproximator(){
	return QMetaObject::invokeMethod(localApproximator,"stopProcessing");
}
QString BaseSettings::getApproximatorName(){
	printf("Setting object did not define a name for the approximator\n");
	return "UNDEFINED";
}
