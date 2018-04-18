#include "keypoint.h"
namespace Keypoint{

// MAKE FASTER WITH DIRECT BYTE ACCESS
// orig.bits()
// orig.bytesPerLine()
QImage Approximator::applyKernel(QImage orig, QList<double> kernel, bool absolute){
	QImage dest(orig.width(),orig.height(),orig.format());
	dest.fill(Qt::black);
	double divisor = calculateKernelDivisor(kernel);
	printf("divisor: %f\n",divisor);
	int ks = sqrt(kernel.size()) / 2; // kernel size
	uchar *pix = orig.bits();
	for(int y=ks; y<orig.height()-ks; y++){
		for(int x=ks; x<orig.width()-ks; x++){
			int r=0,g=0,b=0;
			QColor temp;
			int kernelIndex = 0;
			for(int dy=-ks; dy<=ks; dy++){
				for(int dx=-ks; dx<=ks; dx++){
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
	return dest;
}

void Approximator::processImage(QImage orig){
	orig = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	stopSignalRecieved = false;

	int levels_per_scale = 5;
	int num_scales = 5;
	double sigma = 1.6;

	QList<QList<double> > kernels;
	for(double x=0; x<levels_per_scale; x++)
		kernels.push_back(createKernel( pow(2,x/levels_per_scale)*sigma ,5));

	QList<QImage> images;
	for(QList<double> kernel : kernels){
		for(auto x:kernel) printf("%f  ",x);
		printf("\n");
		QImage temp = applyKernel(orig,kernel,true);
		emit progressMade(temp,10);
		images.push_back(temp);
	}
	for(int x=0; x<levels_per_scale-1; x++){
		QImage temp = combine_subtract(images[x],images[x+1]);
		emit progressMade(temp,20);
		images[x] = temp;
		getchar();
	}
	images.pop_back();

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
QImage Approximator::combine_extreme(QList<QImage> images){
	//gets the value closest to 0 or 256 per channel per pixel
	//output is the max values encountered
	QImage combined(images[0].width(),images[0].height(),images[0].format());
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
QImage Approximator::combine_subtract(QImage f, QImage s){
	QImage out(f.width(),f.height(),f.format());
	uchar *pix = out.bits();
	uchar *fp=f.bits() , *sp=s.bits();

	for(int y=0; y<f.height(); y++){
		for(int x=0; x<f.width(); x++){
			pix[(x*4)+0] = abs(fp[(x*4)+0] - (int)sp[(x*4)+0]);
			pix[(x*4)+1] = abs(fp[(x*4)+1] - (int)sp[(x*4)+1]);
			pix[(x*4)+2] = abs(fp[(x*4)+2] - (int)sp[(x*4)+2]);
			//pix[(x*4)+3] = abs(((int)fp[(x*4)+3]) - ((int)sp[(x*4)+3]));
			pix[(x*4)+3] = 255;
		}
		fp+=f.bytesPerLine();
		sp+=s.bytesPerLine();
		pix+=out.bytesPerLine();
	}
	return out;
}
QList<double> Approximator::createKernel(double sigma, int size){
	//took equation from https://courses.cs.washington.edu/courses/cse576/06sp/notes/Interest2.pdf
	QList<double> kernel;
	size /= 2; // eg if 5 then turns to 2 so we use as a ends for -2,-1,0,1,2
	for(int y=-size; y<=size; y++){
		for(int x=-size; x<=size; x++){
			double temp;
			temp = (x*x + y*y) * -1;
			temp /= 2*sigma*sigma;
			temp = exp(temp);
			temp /= 2*M_PI*sigma*sigma;
			kernel.push_back(temp);
		}
	}
	return kernel;
}

//=====================================================================================================================================================
//=====================================================================================================================================================
//		Settings Class Below
//=====================================================================================================================================================
//=====================================================================================================================================================

Settings::Settings(){
	localApproximator = new Approximator;
}

BaseApproximator* Settings::getApproximator(){
	return localApproximator;
}
int Settings::startApproximator(QImage orig){
	return
	QMetaObject::invokeMethod(localApproximator,"processImage",
							  Q_ARG(QImage,orig)
							  );
}
int Settings::stopApproximator(){
	return QMetaObject::invokeMethod(localApproximator,"stopProcessing");
}

}//namespace
