#include "circleapproximator.h"

void CircleApproximator::processImage(QImage orig,int numCircles,int minR,int maxR){
	printf("starting circle approximation\n");
	origImage = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	keepGoing = true;
	minRadius = minR;
	maxRadius = maxR;
	precomputedDistance.clear(); // array used in the score and draw functions
	precomputedDistance.reserve(maxRadius+1);
	for(int z=0; z<maxRadius+1; z++) precomputedDistance.push_back(z*z);

	if(minRadius<1) minRadius = 1;
	if(maxRadius>origImage.width()/2) maxRadius=origImage.width()/2;
	if(maxRadius>origImage.height()/2) maxRadius=origImage.height()/2;
	if(minRadius>maxRadius) minRadius = maxRadius;

	newImage = QImage(origImage.width(),origImage.height(),QImage::Format_ARGB32_Premultiplied);
	newImage.fill(0); // clear the image

	for(int circle=0; circle<numCircles && keepGoing; circle++){
		//printf("making a circle\n");
		//find a circle that helps make things better
		currentScore=-1;
		while(currentScore<=0){
			randomSelectCurrentCircle();
			currentScore = getScore(origImage,newImage,currentColor,curtX,curtY,curtRadius);
		}
		//printf("optimising a circle\n");

		nextX=curtX; // assume it will be the best circle
		nextY=curtY;
		nextRadius=curtRadius;

		//permutate the circle until optimised
		int numTests = 0;
		while(true){
			numTests++;
			if(curtX > 0)
				tryPermutationAndMakeNextIfBetter(curtX-1,curtY,curtRadius);
			if(curtX < origImage.width())
				tryPermutationAndMakeNextIfBetter(curtX+1,curtY,curtRadius);
			if(curtY > 0)
				tryPermutationAndMakeNextIfBetter(curtX,curtY-1,curtRadius);
			if(curtY < origImage.height())
				tryPermutationAndMakeNextIfBetter(curtX,curtY+1,curtRadius);
			if(curtRadius>minRadius)
				tryPermutationAndMakeNextIfBetter(curtX,curtY,curtRadius-1);
			if(curtRadius<maxRadius){
				tryPermutationAndMakeNextIfBetter(curtX,curtY,curtRadius+1);
			}

			//if we have already reached a local maxima
			if(curtX == nextX && curtY == nextY && curtRadius == nextRadius){
				//make and save the current best approximation we have - this includes the circle we just found
				QImage tempImage = drawCircle(newImage,curtX,curtY,curtRadius,currentColor);
				newImage = tempImage;
				break;
			}else{
				//we found a new better circle so lets move over to it and try again
				curtX=nextX;
				curtY=nextY;
				curtRadius=nextRadius;
			}
		}

		double percentage = (circle+1)/(double)numCircles*100;
		emit progressMade(newImage,percentage);
		QCoreApplication::processEvents();
		//printf("emited picture %d - r %5d  - pos %5d x %5d - %5d tests - delta %5.5f\n",circle,curtRadius,curtX,curtY,numTests,currentDelta);
	}
	printf("Done Approximating\n");
	emit doneProcessing(newImage);
}

void CircleApproximator::stopProcessing(){
	if(keepGoing==true){
		keepGoing = false;
		printf("Stopping Circle Approximator\n");
	}
}

int CircleApproximator::randRange(int low, int high){
	int range = high - low;
	double percentage = (rand()/(double)RAND_MAX);
	return percentage*range+low;
}

double CircleApproximator::getScore(QImage wantedImage, QImage approximatedImage, QColor color, int centerX,int centerY, int radius){
	//consider distance = sqrt(x*x + y*y);
	//if you know distance and y, solving for x terms gives
	//d*d - y*y = x*x
	//so since we know radius of the circle and the current height
	//we can find the max horizontal distance to move

	//this function only counts the pixels around the circumference of the circle
	//this covers the typical case as we only really need to know if it is
	//intersecting any other colored areas on the circumference

	//it returns a score of how much BETTER the circle will be than the current approximation
	//so it gets the total difference of the current approximation and the difference the
	//circle will have if drawn. If the new<current, then we an improvment.
	//This favors larger circles as a larger circle will potentially cover more area that
	//is wrong and so has the chance to accrue more improvment over its longer circumference

	long long totalApprox=0; // where we store the combined error of the pixels
	long long totalColor=0;

	int width = wantedImage.width();
	int minx=centerX-radius;
	int maxx=centerX+radius;
	int miny=centerY-radius;
	int maxy=centerY+radius;
	if(minx<0) minx=0;
	if(miny<0) miny=0;
	if(maxx>=wantedImage.width()) maxx=wantedImage.width()-1;
	if(maxy>=wantedImage.height()) maxy=wantedImage.height()-1;

	if(maxy<=miny || maxx<=minx) return -10000; //something is not making sense

	int unSqauredRadius = precomputedDistance[radius]; // the max distance from the center anything should go
	for(int y=miny;y<=maxy;y++){
		int unSquaredHeight = precomputedDistance[abs(centerY-y)];
		int maxXDist = unSqauredRadius - unSquaredHeight;
		int x=1;
		for(; precomputedDistance[x]<maxXDist;++x); // get so x is JUST outside of the circle
		--x; // go back inside the circle
		int temp;
		//right side
		temp = centerX+x;
		if(temp>=0 && temp<width){
			QColor c1 = QColor::fromRgba(wantedImage.pixel(temp,y));
			QColor c2 = QColor::fromRgba(approximatedImage.pixel(temp,y));
			totalApprox += getColorDelta(c1,c2);
			totalColor += getColorDelta(c1,color);
		}
		//left side
		temp = centerX-x;
		if(temp>=0 && temp<width){
			QColor c1 = QColor::fromRgba(wantedImage.pixel(temp,y));
			QColor c2 = QColor::fromRgba(approximatedImage.pixel(temp,y));
			totalApprox += getColorDelta(c1,c2);
			totalColor += getColorDelta(c1,color);
		}
	}

	return totalApprox-totalColor;
}

int CircleApproximator::getColorDelta(QColor c1, QColor c2){
	int temp,total=0;

	temp = abs(c1.hslHue() - c2.hslHue());
	total += temp;
	temp = abs(c1.hslSaturation() - c2.hslSaturation());
	total += temp;
	temp = abs(c1.lightness() - c2.lightness());
	total += temp*temp;
	temp = abs(c1.alpha() - c2.alpha());
	total += temp;

	return total;
}

QImage CircleApproximator::drawCircle(QImage image, int centerX, int centerY, int radius, QColor color){
	//consider distance = sqrt(x*x + y*y);
	//if you know distance and y, solving for x terms gives
	//d*d - y*y = x*x
	//so since we know radius of the circle and the current height
	//we can find the max horizontal distance to move

	//it starts from the top of the circle and goes down
	//for every row, it starts from the middle and moves outwards

	QImage newImage = image;
	uchar *bits = newImage.bits(); // faster to directly modify bytes - the setPixel method is slow
	int width = newImage.width();
	int height = newImage.height();
	int unSqrtedDistance = precomputedDistance[radius]; // the max distance from the center anything should go

	int yStartingNum = centerY-radius; // start at the top of the circle
	if(yStartingNum<0) yStartingNum=0; // dont start outside the image

	for(int y=yStartingNum; y<centerY+radius && y<height; y++){
		int lineOffset = y*width*4;
		int unSqrtedHeight = precomputedDistance[abs(centerY-y)];
		int maxXDist = unSqrtedDistance - unSqrtedHeight;
		for(int x=0; precomputedDistance[x]<maxXDist; x++){
			int pixelOffset;
			int temp;
			//right side
			temp = centerX+x;
			if(temp<width){
				pixelOffset = lineOffset + (temp*4);
				bits[pixelOffset + 0] = color.blue();
				bits[pixelOffset + 1] = color.green();
				bits[pixelOffset + 2] = color.red();
				bits[pixelOffset + 3] = color.alpha();
			}
			//left side
			temp = centerX-x;
			if(temp >= 0){
				pixelOffset = lineOffset + (temp*4);
				bits[pixelOffset + 0] = color.blue();
				bits[pixelOffset + 1] = color.green();
				bits[pixelOffset + 2] = color.red();
				bits[pixelOffset + 3] = color.alpha();
			}
		}
	}
	return newImage;
}

void CircleApproximator::randomSelectCurrentCircle(){
	curtX=randRange(0,origImage.width()-1);
	curtY=randRange(0,origImage.height()-1);
	curtRadius = minRadius;
	currentColor = QColor::fromRgba(origImage.pixel(curtX,curtY));
}

bool CircleApproximator::tryPermutationAndMakeNextIfBetter(int x, int y, int radius){
	double score = getScore(origImage,newImage,currentColor,x,y,radius);
	if(score>currentScore){
		currentScore = score;
		nextX=x;
		nextY=y;
		nextRadius=radius;
		return true;
	}else
		return false;
}
