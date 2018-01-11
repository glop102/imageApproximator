#include "kernelapproximator.h"

KernelApproximator::KernelApproximator(QObject *parent) : QObject(parent){

}

QImage KernelApproximator::applyKernel(QImage orig, QList<double> kernel){
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

void KernelApproximator::processImage(QImage orig, QList<QList<double> > kernels, combinationTypes combType){
	QList<QImage> images;
	stopSignalRecieved = false;
	for(auto kernel : kernels){
		QImage temp = applyKernel(orig,kernel);
		emit progressMade(temp,   100 * ((images.length()+1)/((double)kernels.length()+1))   );
		images.append(temp);

		QCoreApplication::processEvents();
		if(stopSignalRecieved){
			emit doneProcessing(orig);
			return;
		}
	}

	if(combType == AVERAGE){
		QImage temp = combine_average(images);
		emit(doneProcessing(temp));
		return;
	}else if(combType == MAXIMUM){
		QImage temp = combine_maximum(images);
		emit(doneProcessing(temp));
		return;
	}

	emit doneProcessing(orig);

}

void KernelApproximator::stopProcessing(){
	stopSignalRecieved = true;
	printf("Stopping Kernel Approximator\n");
}

double KernelApproximator::calculateKernelDivisor(QList<double> kernel){
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

QImage KernelApproximator::combine_maximum(QList<QImage> images){
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

QImage KernelApproximator::combine_average(QList<QImage> images){
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
