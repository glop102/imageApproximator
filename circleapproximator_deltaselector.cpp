#include "circleapproximator_deltaselector.h"

CircleApproximator_DeltaSelector::CircleApproximator_DeltaSelector(){
	map = new DeltaMap();
	newMap = new DeltaMap();
}
CircleApproximator_DeltaSelector::~CircleApproximator_DeltaSelector(){
	delete map;
	delete newMap;
}

void CircleApproximator_DeltaSelector::processImage(QImage orig,int numCircles,int minR,int maxR){
	printf("starting circle approximation - delta selector\n");
	QTime timer;
	timer.start();
	origImage = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	keepGoing = true;
	minRadius = minR;
	maxRadius = maxR;
	precomputedDistance.clear(); // array used in the score and draw functions
	precomputedDistance.reserve(maxRadius+2);
	for(int z=0; z<maxRadius+2; z++) precomputedDistance.push_back(z*z);
	initMap(origImage);

	if(minRadius<1) minRadius = 1;
	if(maxRadius>origImage.width()/2) maxRadius=origImage.width()/2;
	if(maxRadius>origImage.height()/2) maxRadius=origImage.height()/2;
	if(minRadius>maxRadius) minRadius = maxRadius;

	newImage = QImage(origImage.width(),origImage.height(),QImage::Format_ARGB32_Premultiplied);
	newImage.fill(0); // clear the image

	for(int circle=0; circle<numCircles && keepGoing; circle++){
		//printf("making a circle\n");
		//find a circle that helps make things better
		//currentScore=-1;
		//while(currentScore<=0){
		randomSelectCurrentCircle();
		currentScore = getScore(origImage,newImage,currentColor,curtX,curtY,curtRadius);
		//map->removePoint(curtX,curtY);
		updateMap(origImage,curtX,curtY,1,currentColor);
		//}
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
				if(currentScore>=0){
					//make and save the current best approximation we have - this includes the circle we just found
					drawCircle(newImage,curtX,curtY,curtRadius,currentColor);
					updateMap(origImage,curtX,curtY,curtRadius,currentColor);
				}
				//newImage = drawCircle(newImage,curtX,curtY,curtRadius,currentColor);
				break;
			}else{
				//we found a new better circle so lets move over to it and try again
				curtX=nextX;
				curtY=nextY;
				curtRadius=nextRadius;
			}
		}

		if(currentScore>=0){
			double percentage = (circle+1)/(double)numCircles*100;
			emit progressMade(newImage,percentage);
		}
		QCoreApplication::processEvents();
		//printf("emited picture %d - r %5d  - pos %5d x %5d - %5d tests - score %5.5f\n",circle,curtRadius,curtX,curtY,numTests,currentScore);
	}
	printf("Done Approximating after %d ms\n",timer.elapsed());
	emit doneProcessing(newImage);
}

void CircleApproximator_DeltaSelector::stopProcessing(){
	if(keepGoing==true){
		keepGoing = false;
		printf("Stopping Circle Approximator\n");
	}
}

int CircleApproximator_DeltaSelector::randRange(int low, int high){
	int range = high - low;
	double percentage = (rand()/(double)RAND_MAX);
	return percentage*range+low;
}

double CircleApproximator_DeltaSelector::getScore(QImage &wantedImage, QImage &approximatedImage, QColor color, int centerX,int centerY, int radius){
	//consider distance = sqrt(x*x + y*y);
	//if you know distance and y, solving for x terms gives
	//d*d - y*y = x*x
	//so since we know radius of the circle and the current height
	//we can find the max horizontal distance to move

	//it returns a score of how much BETTER the circle will be than the current approximation
	//so it gets the total difference of the current approximation and the difference the
	//circle will have if drawn. If the new<current, then we an improvment.
	//This favors larger circles as a larger circle will potentially cover more area that
	//is wrong and so has the chance to accrue more improvment

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
		for(; precomputedDistance[x]<=maxXDist;x++);
		x--;

		//for(int x=0; precomputedDistance[x]<=maxXDist; x++){
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
		//}
	}

	return totalApprox-totalColor;
}

int CircleApproximator_DeltaSelector::getColorDelta(QColor c1, QColor c2){
	int temp,total=0;

	temp = abs(c1.hslHue() - c2.hslHue());
	total += temp;
	temp = abs(c1.hslSaturation() - c2.hslSaturation());
	total += temp;
	temp = abs(c1.lightness() - c2.lightness());
	total += temp;
	temp = abs(c1.alpha() - c2.alpha());
	total += temp;

	return total;
}

void CircleApproximator_DeltaSelector::drawCircle(QImage &image, int centerX, int centerY, int radius, QColor color){
	//consider distance = sqrt(x*x + y*y);
	//if you know distance and y, solving for x terms gives
	//d*d - y*y = x*x
	//so since we know radius of the circle and the current height
	//we can find the max horizontal distance to move

	//it starts from the top of the circle and goes down
	//for every row, it starts from the middle and moves outwards

	//QImage image = oldImage;
	uchar *bits = image.bits(); // faster to directly modify bytes - the setPixel method is slow
	int width = image.width();
	int height = image.height();
	int unSqrtedDistance = precomputedDistance[radius]; // the max distance from the center anything should go

	int yStartingNum = centerY-radius; // start at the top of the circle
	if(yStartingNum<0) yStartingNum=0; // dont start outside the image

	for(int y=yStartingNum; y<=centerY+radius && y<height; y++){
		int lineOffset = y*width*4;
		int unSqrtedHeight = precomputedDistance[abs(centerY-y)];
		int maxXDist = unSqrtedDistance - unSqrtedHeight;
		for(int x=0; precomputedDistance[x]<=maxXDist; x++){
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
	//return image;
}

void CircleApproximator_DeltaSelector::randomSelectCurrentCircle(){
//	curtX=randRange(0,origImage.width()-1);
//	curtY=randRange(0,origImage.height()-1);
	uint32_t x,y;
	map->getHighDeltaPoint(x,y);
	curtX = x;
	curtY = y;
	curtRadius = minRadius;
	currentColor = QColor::fromRgba(origImage.pixel(curtX,curtY));
}

bool CircleApproximator_DeltaSelector::tryPermutationAndMakeNextIfBetter(int x, int y, int radius){
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

void CircleApproximator_DeltaSelector::initMap(QImage &image){
	map->clear();
	newMap->clear();
	QColor c1,c2;
	c2.setRed(0);
	c2.setGreen(0);
	c2.setBlue(0);
	c2.setAlpha(0);
	for(int y=0;y<image.height();y++){
		for(int x=0;x<image.width();x++){
			c1 = QColor::fromRgba(image.pixel(x,y));
			double delta = getColorDelta(c1,c2);
			map->setPointDelta(x,y,delta);
		}
	}
}
void CircleApproximator_DeltaSelector::updateMap(QImage &wantedImage, int centerX, int centerY, int radius,QColor color){
	int width = wantedImage.width();
	int minx=centerX-radius;
	int maxx=centerX+radius;
	int miny=centerY-radius;
	int maxy=centerY+radius;
	if(minx<0) minx=0;
	if(miny<0) miny=0;
	if(maxx>=wantedImage.width()) maxx=wantedImage.width()-1;
	if(maxy>=wantedImage.height()) maxy=wantedImage.height()-1;

	if(maxy<=miny || maxx<=minx) return; //something is not making sense

	int unSqauredRadius = precomputedDistance[radius]; // the max distance from the center anything should go
	for(int y=miny;y<=maxy;y++){
		int unSquaredHeight = precomputedDistance[abs(centerY-y)];
		int maxXDist = unSqauredRadius - unSquaredHeight;
		for(int x=0; precomputedDistance[x]<=maxXDist;++x){
			int temp;
			double delta;
			//right side
			temp = centerX+x;
			if(temp>=0 && temp<width){
				QColor c1 = QColor::fromRgba(wantedImage.pixel(temp,y));
				delta = getColorDelta(c1,color);
				if(delta>0)
					newMap->setPointDelta(temp,y,delta);
				map->removePoint(temp,y);
			}
			//left side
			temp = centerX-x;
			if(temp>=0 && temp<width){
				QColor c1 = QColor::fromRgba(wantedImage.pixel(temp,y));
				delta = getColorDelta(c1,color);
				if(delta>0)
					newMap->setPointDelta(temp,y,delta);
				map->removePoint(temp,y);
			}
		}
	}

	if(map->numPoints()==0){
		delete map;
		map = newMap;
		newMap = new DeltaMap();
	}
}


void DeltaMap::clear(){
	//printf("clearing\n");
	auto itt = points.begin();
	while(itt != points.end()){
		linked_element *elm = itt->second;
		//removeElement(elm); // why do you need to unlink the chain if you are simply deleting the whole chain?
		itt++;
		delete elm;
	}
	deltas.clear();
	points.clear();
	//printf("cleared\n");
}
DeltaMap::~DeltaMap(){
	clear();
}
void DeltaMap::getHighDeltaPoint(uint32_t &x, uint32_t &y){
	auto temp = deltas.rbegin(); // reverse itterators start at the end - aka largest values for the in-order structure
	if(temp == deltas.rend()){
		x=0;
		y=0;
		return;
	}
	//int number = (*distribution)(generator);
	//number = (number/(double)distribution->max()) * points.size();
	//number %= points.size();

	//while(number>=deltas_counter[temp->first]){
	//	number -= deltas_counter[temp->first];
	//	temp++;
	//}

	linked_element *elm = temp->second;
	//for(;number>0;number--)
	//	elm = elm->next;
	x=elm->x;
	y=elm->y;
}
void DeltaMap::setPointDelta(uint32_t x, uint32_t y, double delta){
	uint64_t temp = x;
	temp |= ((uint64_t)y)<<32; // put the y as the higher order bits

	linked_element *elm;
	bool newElm;
	if(points.count(temp)==0){
		//is a new point, so lets make new entry
		elm = new linked_element;
		elm->x = x;
		elm->y = y;
		elm->next=NULL;
		elm->prev=NULL;
		points[temp] = elm;
		newElm = true;
	}else{
		elm=points[temp];
		newElm = false;
		//deltas_counter[elm->delta]--;
	}
	//if(deltas_counter.count(delta)!=0)
	//	deltas_counter[delta]++;
	//else
	//	deltas_counter[delta]=1;

	if(!newElm)
		removeElement(elm);

	elm->delta = delta; // now we can move the delta in

	//now add to some other list of the same delta values
	if(deltas.count(delta)!=0){
		elm->next = deltas[delta];
		elm->next->prev = elm;
		elm->prev = NULL;
		deltas[delta] = elm;
	}else{
		elm->prev=NULL;
		elm->next=NULL;
		deltas[delta]=elm;
	}
}

DeltaMap::DeltaMap(){
	//distribution = new std::geometric_distribution<int>(1.0/500); // average of doing 500 away from 0
}

void DeltaMap::removePoint(uint32_t x, uint32_t y){
	//printf("removing point ... \n");
	uint64_t temp = x;
	temp |= ((uint64_t)y)<<32; // put the y as the higher order bits
	if(points.count(temp)==0)
		return; // nothing there to delete

	linked_element *elm;
	elm = points[temp];
	removeElement(elm);

	auto itt = points.find(temp);
	if(itt != points.end())
		points.erase(itt);
	delete elm;
	//printf("removed point\n");
}
void DeltaMap::removeElement(linked_element *elm){
	//printf("removing element ... \n");
	if(elm->prev == NULL && elm->next == NULL){
		//last pixel with this delta value
		//lets remove the delta value field
		auto temp = deltas.find(elm->delta);
		if(temp != deltas.end())
			deltas.erase(temp);
	}else if(elm->prev == NULL){
		//was the first item in the list
		deltas[elm->delta] = elm->next;
		elm->next->prev = NULL;
	}else{
		//was in the middle or the end
		elm->prev->next = elm->next;
		if(elm->next!=NULL)
			elm->next->prev = elm->prev;
	}
	//printf("removed element\n");
}
long long DeltaMap::numPoints(){
	return points.size();
}
