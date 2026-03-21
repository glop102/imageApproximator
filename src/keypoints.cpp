#include "keypoints.h"

namespace Keypoints{

void Approximator::processImage(QImage orig){
	printf("starting kernel approximator\n");
	QElapsedTimer timer;
	timer.start();

	Settings* sett = dynamic_cast<Settings*>(settingsObject);

	orig = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	keepGoing = true;
	double progress_step = 0.0; // make an actual amount of step per stage
	int steps_taken = 0;

	while(true){
		QCoreApplication::processEvents();
		if(!keepGoing){
			emit doneProcessing(orig);
			return;
		}

		steps_taken++;
		emit(progressMade(orig,steps_taken*progress_step));
	}

	printf("Done Approximating after %lld ms\n",timer.elapsed());
	emit doneProcessing(orig);
}

Approximator::Approximator(BaseSettings *sett):BaseApproximator(sett){}

//=====================================================================================================================================================
//=====================================================================================================================================================
//		Settings Class Below
//=====================================================================================================================================================
//=====================================================================================================================================================

Settings::Settings(){
	localApproximator = dynamic_cast<BaseApproximator*>(new Approximator(this));

	// == Settings Get added Here

	// == Finish out the layouts
	globalLayout = new QHBoxLayout;
	this->setLayout(globalLayout);
	//Left Justified Settings
	//globalLayout->addWidget(something);
	globalLayout->addStretch(1);
	//Right Justified Settings
}

QString Settings::getApproximatorName(){
	return "Keypoints";
}

}//namespace
