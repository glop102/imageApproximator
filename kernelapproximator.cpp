#include "kernelapproximator.h"
namespace Kernel{

QImage Approximator::applyKernel(QImage orig, QList<double> kernel){
	QImage dest(orig.width(),orig.height(),orig.format());
	dest.fill(Qt::black);
	double divisor = calculateKernelDivisor(kernel);
	for(int y=1; y<orig.height()-1; y++){
		for(int x=1; x<orig.width()-1; x++){
			int r=0,g=0,b=0;
			QColor temp;

			temp = orig.pixelColor(x-1,y-1);
			r+=temp.red()*kernel[0];g+=temp.green()*kernel[0];b+=temp.blue()*kernel[0];
			temp = orig.pixelColor(x  ,y-1);
			r+=temp.red()*kernel[1];g+=temp.green()*kernel[1];b+=temp.blue()*kernel[1];
			temp = orig.pixelColor(x+1,y-1);
			r+=temp.red()*kernel[2];g+=temp.green()*kernel[2];b+=temp.blue()*kernel[2];

			temp = orig.pixelColor(x-1,y  );
			r+=temp.red()*kernel[3];g+=temp.green()*kernel[3];b+=temp.blue()*kernel[3];
			temp = orig.pixelColor(x  ,y  );
			r+=temp.red()*kernel[4];g+=temp.green()*kernel[4];b+=temp.blue()*kernel[4];
			temp = orig.pixelColor(x+1,y  );
			r+=temp.red()*kernel[5];g+=temp.green()*kernel[5];b+=temp.blue()*kernel[5];

			temp = orig.pixelColor(x-1,y+1);
			r+=temp.red()*kernel[6];g+=temp.green()*kernel[6];b+=temp.blue()*kernel[6];
			temp = orig.pixelColor(x  ,y+1);
			r+=temp.red()*kernel[7];g+=temp.green()*kernel[7];b+=temp.blue()*kernel[7];
			temp = orig.pixelColor(x+1,y+1);
			r+=temp.red()*kernel[8];g+=temp.green()*kernel[8];b+=temp.blue()*kernel[8];

			temp.setRed(abs(r)/divisor);
			temp.setGreen(abs(g)/divisor);
			temp.setBlue(abs(b)/divisor);
			dest.setPixel(x,y,temp.rgb());
		}
	}
	return dest;
}

//void Approximator::processImage(QImage orig, Settings settingsHolder){
//	processImage(
//				orig,
//				settingsHolder.getKernels(),
//				settingsHolder.getNumberPasses()
//			);
//}
void Approximator::processImage(QImage orig, QList<QList<double> > kernels, int numPasses){
	orig = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	stopSignalRecieved = false;
	double progress_step =  // how much each step is worth percentage wise
			100.0 / ( //100% divided by number of steps
			(kernels.length()+1) // steps per loop (kernels + combine)
			* numPasses); // number of loops
	int steps_taken = 0;

	while(numPasses){
		QList<QImage> images;
		for(auto kernel : kernels){
			QImage temp = applyKernel(orig,kernel);
			steps_taken++;
			emit progressMade(temp, steps_taken*progress_step );
			images.append(temp);

			QCoreApplication::processEvents();
			if(stopSignalRecieved){
				emit doneProcessing(orig);
				return;
			}
		}

		QImage temp = combine_maximum(images);
		steps_taken++;
		emit(progressMade(temp,steps_taken*progress_step));
		numPasses--;

		orig = temp; // time for another pass
	}

	emit doneProcessing(orig);

}

void Approximator::stopProcessing(){
	if(stopSignalRecieved==false){
		stopSignalRecieved = true;
		printf("Stopping Kernel Approximator\n");
	}
}

double Approximator::calculateKernelDivisor(QList<double> kernel){
	//got from here - https://stackoverflow.com/questions/40444560/opencvs-sobel-filter-why-does-it-look-so-bad-especially-compared-to-gimp
	double neg=0,pos=0;
	for(auto x : kernel){
		if(x<0)
			neg+=x;
		else
			pos+=x;
	}
	neg = neg * -1;

	if(neg ==0 && pos == 0)
		return 1;
	else if(neg>pos)
		return neg;
	else
		return pos;
}

QImage Approximator::combine_maximum(QList<QImage> images){
	//gets the largest value per channel per pixel from all the canidate input images
	//output is the max values encountered
	QImage combined(images[0].width(),images[0].height(),images[0].format());
	for(int y=0; y<combined.height(); y++){
		for(int x=0; x<combined.width(); x++){
			int r=0,g=0,b=0;
			for(int i=0; i<images.length(); i++){
				QColor color = images[i].pixelColor(x,y);
				if(color.red()>r)
					r=color.red();
				if(color.green()>g)
					g=color.green();
				if(color.blue()>b)
					b=color.blue();
			}
			combined.setPixelColor(x,y,QColor(r,g,b));
		}
	}
	return combined;
}

QImage Approximator::combine_average(QList<QImage> images){
	QImage combined(images[0].width(),images[0].height(),images[0].format());
	for(int y=0; y<combined.height(); y++){
		for(int x=0; x<combined.width(); x++){
			int r=0,g=0,b=0;
			for(int i=0; i<images.length(); i++){
				QColor color = images[i].pixelColor(x,y);
				if(color.red()>r)
					r=color.red();
				if(color.green()>g)
					g=color.green();
				if(color.blue()>b)
					b=color.blue();
			}
			r/=images.length();
			g/=images.length();
			b/=images.length();
			combined.setPixelColor(x,y,QColor(r,g,b));
		}
	}
	return combined;
}

//=====================================================================================================================================================
//=====================================================================================================================================================
//		Settings Class Below
//=====================================================================================================================================================
//=====================================================================================================================================================

Settings::Settings(){
	localApproximator = new Approximator;

	// === Left Group - How the different images are combined after getting the kernel applied to it
	numberPassesSelection = new QGroupBox("Number of Passes");
	numberPassesLabel = new QLabel("2");
	fewerPasses = new QPushButton("-");
	morePasses = new QPushButton("+");
	numberPassesLayout = new QGridLayout;
	numberPassesLayout->addWidget(numberPassesLabel,0,0,1,0);
	numberPassesLayout->addWidget(fewerPasses,1,0);
	numberPassesLayout->addWidget(morePasses,1,1);
	numberPassesLayout->setRowStretch(2,1);
	numberPassesSelection->setLayout(numberPassesLayout);
	connect(fewerPasses,SIGNAL(clicked(bool)),this,SLOT(numberPassesChanged()) );
	connect(morePasses,SIGNAL(clicked(bool)),this,SLOT(numberPassesChanged()) );

	// === Second Group - Number of kernels to apply
	kernelNumberSelection = new QGroupBox("Number of Kernels");
	numberKernelsLabel = new QLabel("4");
	fewerKernels = new QPushButton("-");
	moreKernels = new QPushButton("+");
	kernelNumberLayout = new QGridLayout;
	kernelNumberLayout->addWidget(numberKernelsLabel,0,0,1,0);
	kernelNumberLayout->addWidget(fewerKernels,1,0);
	kernelNumberLayout->addWidget(moreKernels,1,1);
	kernelNumberLayout->setRowStretch(2,1);
	kernelNumberSelection->setLayout(kernelNumberLayout);
	connect(fewerKernels,SIGNAL(clicked(bool)),this,SLOT(numberKernelsChange()) );
	connect(moreKernels,SIGNAL(clicked(bool)),this,SLOT(numberKernelsChange()) );

	globalLayout = new QHBoxLayout;
	this->setLayout(globalLayout);
	globalLayout->addWidget(numberPassesSelection);
	globalLayout->addWidget(kernelNumberSelection);
	globalLayout->addStretch(1);

	addNewKernel({-1,0,1,-2,0,2,-1,0,1}); // horizontal
	addNewKernel({-1,-2,-1,0,0,0,1,2,1}); // vertical
	addNewKernel({0,1,2,-1,0,1,-2,-1,0}); // diag (top right to bot left)
	addNewKernel({2,1,0,1,0,-1,0,-1,-2}); // diag (top left to bot right)
}

void Settings::numberKernelsChange(){
	int current_number = numberKernelsLabel->text().toInt();
	if(QObject::sender() == fewerKernels){
		if(current_number<=1) return;
		current_number-=1;
		while(kernelGroups.length()>current_number){
			auto temp = kernelGroups[kernelGroups.length()-1];
			globalLayout->removeWidget(temp);
			delete temp;
			kernelGroups.pop_back();
		}
	}else{
		if(current_number>=10) return;
		current_number+=1;
		while(kernelGroups.length()<current_number){
			addNewKernel();
		}
	}

	numberKernelsLabel->setText(QString::number(current_number));
}
void Settings::numberPassesChanged(){
	int current_number = numberPassesLabel->text().toInt();
	if(QObject::sender() == fewerPasses){
		if(current_number<=1) return;
		current_number-=1;
	}else{
		if(current_number>=10) return;
		current_number+=1;
	}

	numberPassesLabel->setText(QString::number(current_number));
}

void Settings::addNewKernel(QList<double> kernel){
	while(kernel.length()<9)
		kernel.append(0);
	while(kernel.length()>9)
		kernel.pop_back();

	QGroupBox *group = new QGroupBox;
	QGridLayout *layout = new QGridLayout;
	QLineEdit *l1,*l2,*l3,*l4,*l5,*l6,*l7,*l8,*l9;
	l1 = new QLineEdit(QString::number(kernel[0]));
	l2 = new QLineEdit(QString::number(kernel[1]));
	l3 = new QLineEdit(QString::number(kernel[2]));
	l4 = new QLineEdit(QString::number(kernel[3]));
	l5 = new QLineEdit(QString::number(kernel[4]));
	l6 = new QLineEdit(QString::number(kernel[5]));
	l7 = new QLineEdit(QString::number(kernel[6]));
	l8 = new QLineEdit(QString::number(kernel[7]));
	l9 = new QLineEdit(QString::number(kernel[8]));

	l1->setValidator(&doubleValidator);
	l2->setValidator(&doubleValidator);
	l3->setValidator(&doubleValidator);
	l4->setValidator(&doubleValidator);
	l5->setValidator(&doubleValidator);
	l6->setValidator(&doubleValidator);
	l7->setValidator(&doubleValidator);
	l8->setValidator(&doubleValidator);
	l9->setValidator(&doubleValidator);

	layout->addWidget(l1,0,0);
	layout->addWidget(l2,0,1);
	layout->addWidget(l3,0,2);

	layout->addWidget(l4,1,0);
	layout->addWidget(l5,1,1);
	layout->addWidget(l6,1,2);

	layout->addWidget(l7,2,0);
	layout->addWidget(l8,2,1);
	layout->addWidget(l9,2,2);

	layout->setRowStretch(3,1);
	group->setLayout(layout);

	kernelGroups.append(group);
	globalLayout->addWidget(group);
}

QList<QList<double> > Settings::getKernels(){
	QList<QList<double> > totalList;
	for(auto group : kernelGroups){
		QGridLayout *layout = (QGridLayout*)group->layout();
		QList<double> temp;
		for(int x=0;x<9;x++){ // parse every text box
			temp.push_back(
				((QLineEdit*)layout->itemAtPosition(x/3,x%3)->widget())
					->text().toDouble()
			);
		}
		totalList.push_back(temp);
	}
	return totalList;
}

int Settings::getNumberPasses(){
	return numberPassesLabel->text().toInt();
}

BaseApproximator* Settings::getApproximator(){
	return localApproximator;
}
int Settings::startApproximator(QImage orig){
	qRegisterMetaType<QList<QList<double> > >("QList<QList<double> >");
	return
	QMetaObject::invokeMethod(localApproximator,"processImage",
							  Q_ARG(QImage,orig),
							  Q_ARG(QList<QList<double> >, getKernels()),
							  Q_ARG(int,getNumberPasses())
							  );
}
int Settings::stopApproximator(){
	return QMetaObject::invokeMethod(localApproximator,"stopProcessing");
}

}//namespace
