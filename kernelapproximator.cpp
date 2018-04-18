#include "kernelapproximator.h"
namespace Kernel{

// MAKE FASTER WITH DIRECT BYTE ACCESS
// orig.bits()
// orig.bytesPerLine()
QImage Approximator::applyKernel(QImage orig, const QList<double> kernel, const bool absolute){
	QImage dest(orig.width(),orig.height(),orig.format());
	dest.fill(Qt::black);
	double divisor = calculateKernelDivisor(kernel);
	int ks = sqrt(kernel.size()) / 2; // kernel size
	uchar *pix = orig.bits();

	//QElapsedTimer benchmark;
	//benchmark.start();
	#pragma omp parallel for schedule(static)
	for(int y=ks; y<orig.height()-ks; y++){ // for each line
		for(int x=ks; x<orig.width()-ks; x++){ // for each vertical
			int r=0,g=0,b=0;
			QColor temp;
			int kernelIndex = 0;
			for(int dy=-ks; dy<=ks; dy++){ // for each kernel row
				for(int dx=-ks; dx<=ks; dx++){ //for each kernel vertical
					unsigned long offset = (y+dy) * orig.bytesPerLine();
					offset += ((x+dx)*4);
					temp = orig.pixelColor(x+dx,y+dy);
					r+=kernel[kernelIndex]*pix[offset+2];
					g+=kernel[kernelIndex]*pix[offset+1];
					b+=kernel[kernelIndex]*pix[offset+0];
					kernelIndex++;
				}
			}

			if(absolute){
				temp.setRed(abs(r)/divisor);
				temp.setGreen(abs(g)/divisor);
				temp.setBlue(abs(b)/divisor);
			}else{
				temp.setRed(r/divisor/2 + 128);
				temp.setGreen(g/divisor/2 + 128);
				temp.setBlue(b/divisor/2 + 128);
			}
			dest.setPixel(x,y,temp.rgb());
		}
	}
	//long long time = benchmark.elapsed();
	//printf("Image Kernal Apply Time : %lld\n",time);
	return dest;
}

void Approximator::processImage(QImage orig, QList<QList<double> > kernels, int numPasses, bool absolute){
	printf("starting kernel approximator\n");
	QElapsedTimer timer;
	timer.start();

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
			QImage temp = applyKernel(orig,kernel,absolute);
			steps_taken++;
			emit progressMade(temp, steps_taken*progress_step );
			images.append(temp);

			QCoreApplication::processEvents();
			if(stopSignalRecieved){
				emit doneProcessing(orig);
				return;
			}
		}

		QImage temp;
		if(absolute)
			temp = combine_maximum(images);
		else
			temp = combine_extreme_pixels(images);
		steps_taken++;
		emit(progressMade(temp,steps_taken*progress_step));
		numPasses--;

		orig = temp; // time for another pass
	}

	printf("Done Approximating after %lld ms\n",timer.elapsed());
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
	#pragma omp parallel for schedule(static)
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
QImage Approximator::combine_extreme_channels(QList<QImage> images){
	//gets the value closest to 0 or 256 per channel per pixel
	//output is the max values encountered
	QImage combined(images[0].width(),images[0].height(),images[0].format());
	#pragma omp parallel for schedule(static)
	for(int y=0; y<combined.height(); y++){
		for(int x=0; x<combined.width(); x++){
			int r=0,g=0,b=0;
			for(int i=0; i<images.length(); i++){
				QColor color = images[i].pixelColor(x,y);
				if(abs(color.red()-128)>abs(r))
					r=color.red()-128;
				if(abs(color.green()-128)>abs(g))
					g=color.green()-128;
				if(abs(color.blue()-128)>abs(b))
					b=color.blue()-128;
			}
			combined.setPixelColor(x,y,QColor(r+128,g+128,b+128));
		}
	}
	return combined;
}
QImage Approximator::combine_extreme_pixels(QList<QImage> images){
	//gets the pixel closest to 0 or 256, not per channel
	//output is the max values encountered
	QImage combined(images[0].width(),images[0].height(),images[0].format());
	#pragma omp parallel for schedule(static)
	for(int y=0; y<combined.height(); y++){
		for(int x=0; x<combined.width(); x++){
			int r=0,g=0,b=0;
			int sum=0;
			for(int i=0; i<images.length(); i++){
				QColor color = images[i].pixelColor(x,y);
				int new_sum = abs(color.red()-128);
				new_sum += abs(color.green()-128);
				new_sum += abs(color.blue()-128);
				if(sum<new_sum){
					r=color.red()-128;
					g=color.green()-128;
					b=color.blue()-128;
					sum = new_sum;
				}
			}
			combined.setPixelColor(x,y,QColor(r+128,g+128,b+128));
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

	// === Left Group - Various numerical options
	quantitySelection = new QGroupBox;
	quantityLayout = new QVBoxLayout;
	numberPasses = new QSpinBox;
	numberKernels = new QSpinBox;
	quantitySelection->setLayout(quantityLayout);
	quantityLayout->addWidget(new QLabel("Number Of Passes"));
	quantityLayout->addWidget(numberPasses);
	quantityLayout->addWidget(new QLabel("Number Of Kernels"));
	quantityLayout->addWidget(numberKernels);

	numberPasses->setMaximum(10);
	numberPasses->setMinimum(1);
	numberPasses->setValue(1);
	numberKernels->setMaximum(10);
	numberKernels->setMinimum(1);
	numberKernels->setValue(4);
	connect( numberKernels,SIGNAL(valueChanged(int)),this,SLOT(numberKernelsChange()) );

	// === Middle Group - Various binary options
	toggleSelection = new QGroupBox;
	toggleLayout = new QVBoxLayout;
	signedUnsignedToggle = new QPushButton("Absolute");
	toggleSelection->setLayout(toggleLayout);
	toggleLayout->addWidget(signedUnsignedToggle);

	signedUnsignedToggle->setCheckable(true);
	connect( signedUnsignedToggle,SIGNAL(toggled(bool)),this,SLOT(signedUnsignedToggled()) );

	// == Finish out the layouts
	globalLayout = new QHBoxLayout;
	this->setLayout(globalLayout);
	globalLayout->addWidget(quantitySelection);
	globalLayout->addWidget(toggleSelection);
	globalLayout->addStretch(1);
	//kernels go after the stretch

	addNewKernel({-1,0,1,-2,0,2,-1,0,1}); // horizontal
	addNewKernel({-1,-2,-1,0,0,0,1,2,1}); // vertical
	addNewKernel({0,-1,-2,1,0,-1,2,1,0}); // diag (top right to bot left)
	addNewKernel({-2,-1,0,-1,0,1,0,1,2}); // diag (top left to bot right)
}

void Settings::numberKernelsChange(){
	int current_number = numberKernels->value();
	while(kernelGroups.length()>current_number){ // shrink if number went down
		auto temp = kernelGroups[kernelGroups.length()-1];
		globalLayout->removeWidget(temp);
		delete temp;
		kernelGroups.pop_back();
	}
	while(kernelGroups.length()<current_number){ // grow if number went up
		addNewKernel();
	}
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
	return numberPasses->value();
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
							  Q_ARG(int,getNumberPasses()),
							  Q_ARG(bool,! signedUnsignedToggle->isChecked())
							  );
}
int Settings::stopApproximator(){
	return QMetaObject::invokeMethod(localApproximator,"stopProcessing");
}
void Settings::signedUnsignedToggled(){
	if(signedUnsignedToggle->isChecked()){
		signedUnsignedToggle->setText("Signed");
	}else{
		signedUnsignedToggle->setText("Absolute");
	}
}

}//namespace
