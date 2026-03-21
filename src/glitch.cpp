#include "glitch.h"
namespace Glitch{

// MAKE FASTER WITH DIRECT BYTE ACCESS
// orig.bits()
// orig.bytesPerLine()
/*
QImage Approximator::applyKernel(QImage orig, const QList<double> &kernel, const bool &absolute){
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
*/

/* Idea for this aproximator:
 *
 * for a given input image, apply a glitchy filter over top of it
 * if the input image is a multi-frame image, apply it independantly to every frame
 *
 * 1) select random effect (random regions for each)
 *    1a) color shift rectangle
 *	  1b) drop a color channel in a rectangle
 *    1c) shift the rectangle a certain amount
 *    1d) random noise in a rectangle
 * 2) apply global effects - just the same effects as above but covers the whole image instead of just small rectangles
 *
 * should have flags in the settings to turn certain effects on/off
*/
//QImage Approximator::applySingleGlitchFrame(QImage orig){
//}

void Approximator::processImage(QImage orig){
	printf("starting kernel approximator\n");
	QElapsedTimer timer;
	timer.start();

	Settings* sett = dynamic_cast<Settings*>(settingsObject);
	orig = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	keepGoing = true; // flag to signal approximator should stop - the approximator runs in a seperate thread from the GUI

	emit(progressMade(orig, 50.0 )); //some idiot decided to make it 0-100 for progress

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
	//connect( numberKernels,SIGNAL(valueChanged(int)),this,SLOT(numberKernelsChange()) );

	// === Middle Group - Various binary options
	toggleSelection = new QGroupBox;
	toggleLayout = new QVBoxLayout;
	signedUnsignedToggle = new QPushButton("Absolute");
	toggleSelection->setLayout(toggleLayout);
	toggleLayout->addWidget(signedUnsignedToggle);

	signedUnsignedToggle->setCheckable(true);
	//connect( signedUnsignedToggle,SIGNAL(toggled(bool)),this,SLOT(signedUnsignedToggled()) );

	// == Finish out the layouts
	globalLayout = new QHBoxLayout;
	this->setLayout(globalLayout);
	globalLayout->addWidget(quantitySelection);
	globalLayout->addWidget(toggleSelection);
	globalLayout->addStretch(1);
	//kernels go after the stretch
}

QString Settings::getApproximatorName(){
	return "Glitch";
}

}//namespace
