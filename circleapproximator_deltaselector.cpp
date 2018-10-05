#include "circleapproximator_deltaselector.h"
namespace Circle_DeltaSelector {

Approximator::Approximator(BaseSettings*sett): ::Circle::Approximator(sett){
	map = new DeltaMap();
	newMap = new DeltaMap();
}
Approximator::~Approximator(){
	delete map;
	delete newMap;
}

void Approximator::processImage(QImage orig){
	printf("starting circle approximation - delta selector\n");
	QTime timer;
	timer.start();

	//Some work to be done before getting started
	origImage = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied); // make sure we know the format
	keepGoing = true;

	Settings* sett = dynamic_cast<Settings*>(settingsObject);
	int numCircles = sett->numCircles();
	minRadius = sett->minRadius();
	maxRadius = sett->maxRadius();
	if(minRadius<1) minRadius = 1;
	if(maxRadius>origImage.width()/2) maxRadius=origImage.width()/2;
	if(maxRadius>origImage.height()/2) maxRadius=origImage.height()/2;
	if(minRadius>maxRadius) minRadius = maxRadius;

	precomputedDistance.clear(); // array used in the score and draw functions
	precomputedDistance.reserve(maxRadius+2);
	for(int z=0; z<maxRadius+2; z++) precomputedDistance.push_back(z*z);

	currentApproximation = QImage(origImage.width(),origImage.height(),QImage::Format_ARGB32_Premultiplied);
	currentApproximation.fill(0); // clear the image
	initMap(origImage);

	#pragma omp parallel shared(keepGoing)
	{
		int num_threads;
		int circlesPerThread;
		#pragma omp critical
		{
			num_threads = omp_get_num_threads();
			circlesPerThread = numCircles / num_threads;
			if(omp_get_thread_num() == num_threads-1)
				circlesPerThread += numCircles % num_threads; //the last thread gets the left over number of circles
		}
		int circle_num = 0;

		while(circle_num<circlesPerThread){

			struct Circle circle;
			do{ //find a circle that helps make things better
				randomSelectCurrentCircle(&circle);
				circle.score = getScore(origImage,currentApproximation,
											   circle.centerX,circle.centerY,circle.radius,circle.color);
				updateMap(origImage,&circle,1);
			}while(circle.score<=0);

			//permutate the circle until optimized
			while(tryPermutationForBetterCircle(&circle));

			drawCircle(currentApproximation,&circle);

			updateMap(origImage,&circle);

			if(!keepGoing)break;

			#pragma omp master
			{ // emit details of progress
				double percentage = (circle_num+1)/(double)circlesPerThread*100;
				emit progressMade(currentApproximation,percentage);
				QCoreApplication::processEvents();
			}
			circle_num++;
		}
	}
	printf("Done Approximating after %d ms\n",timer.elapsed());
	emit doneProcessing(currentApproximation);
}

void Approximator::randomSelectCurrentCircle(struct Circle* circle){
	uint32_t x,y;
	map->getHighDeltaPoint(x,y);
	circle->centerX = x;
	circle->centerY = y;
	circle->radius = minRadius;
	circle->color = QColor::fromRgba(origImage.pixel(x,y));
}

void Approximator::initMap(QImage &image){
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
		emit progressMade(currentApproximation,
						  ( (image.height()-y)/(double )image.height() ) * 100.0
						  );
	}
}
void Approximator::updateMap(QImage &wantedImage, struct Circle* circle, int radius_override){
	int width = wantedImage.width();
	int minx=circle->centerX - circle->radius;
	int maxx=circle->centerX + circle->radius;
	int miny=circle->centerY - circle->radius;
	int maxy=circle->centerY + circle->radius;
	if(minx<0) minx=0;
	if(miny<0) miny=0;

	int unSqauredRadius;
	if(radius_override)
		unSqauredRadius = precomputedDistance[radius_override];
	else
		unSqauredRadius = precomputedDistance[circle->radius]; // the max distance from the center anything should go

	#pragma omp critical
	{
		if(maxx>=wantedImage.width()) maxx=wantedImage.width()-1;
		if(maxy>=wantedImage.height()) maxy=wantedImage.height()-1;
		if(maxy<=miny || maxx<=minx) goto skip_map_update; //something is not making sense

		for(int y=miny;y<=maxy;y++){
			int unSquaredHeight = precomputedDistance[abs(circle->centerY-y)];
			int maxXDist = unSqauredRadius - unSquaredHeight;
			for(int x=0; precomputedDistance[x]<=maxXDist;++x){
				int temp;
				double delta;
				//right side
				temp = circle->centerX+x;
				if(temp>=0 && temp<width){
					QColor c1 = QColor::fromRgba(wantedImage.pixel(temp,y));
					delta = getColorDelta(c1,circle->color);
					if(delta>0)
						newMap->setPointDelta(temp,y,delta);
					map->removePoint(temp,y);
				}
				//left side
				temp = circle->centerX-x;
				if(temp>=0 && temp<width){
					QColor c1 = QColor::fromRgba(wantedImage.pixel(temp,y));
					delta = getColorDelta(c1,circle->color);
					if(delta>0)
						newMap->setPointDelta(temp,y,delta);
					map->removePoint(temp,y);
				}
			}
		}

		if(map->numPoints()==0){
			printf("Ran through all points, flipping map\n");
			delete map;
			map = newMap;
			newMap = new DeltaMap();
		}
		skip_map_update:;
	}
}

//=====================================================================================================================================================
//=====================================================================================================================================================
//		DeltaMap Class Below
//=====================================================================================================================================================
//=====================================================================================================================================================

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

//=====================================================================================================================================================
//=====================================================================================================================================================
//		Settings Class Below
//=====================================================================================================================================================
//=====================================================================================================================================================

Settings::Settings() : BaseSettings(){
	localApproximator = dynamic_cast<BaseApproximator*>(new Approximator(this));
	makeWidgets();
	layoutWidgets();
	makeConnections();
}

void Settings::keepRadiusEntriesInSync(){
	QObject *sender = QObject::sender();
	int min = minRadiusEntry->value();
	int max = maxRadiusEntry->value();

	if(min<=max) return; // makes sense, the max value is large than the min

	if(sender == minRadiusEntry){
		maxRadiusEntry->setValue(min);
	}else{
		minRadiusEntry->setValue(max);
	}
}

int Settings::numCircles(){
	return numCirclesEntry->value();
}
int Settings::minRadius(){
	return minRadiusEntry->value();
}
int Settings::maxRadius(){
	return maxRadiusEntry->value();
}

void Settings::makeWidgets(){
	description = new QLabel("This chooses the pixels that are the most wrong to try to put a circle at");
	numCirclesEntry = new QSpinBox();
	minRadiusEntry = new QSpinBox();
	maxRadiusEntry = new QSpinBox();

	description->setWordWrap(true);
	numCirclesEntry->setMinimum(1);
	numCirclesEntry->setMaximum(1000000000);
	numCirclesEntry->setValue(15000);
	minRadiusEntry->setMinimum(1);
	minRadiusEntry->setMaximum(9999);
	minRadiusEntry->setValue(5);
	maxRadiusEntry->setMinimum(1);
	maxRadiusEntry->setMaximum(10000);
	maxRadiusEntry->setValue(150);
}
void Settings::layoutWidgets(){
	mainLayout = new QGridLayout();
	setLayout(mainLayout);
	mainLayout->addWidget(description,0,0,1,3);
	mainLayout->addWidget(new QLabel("Num Circles"),1,0);
	mainLayout->addWidget(numCirclesEntry,2,0);
	mainLayout->addWidget(new QLabel("Min Radius"),1,1);
	mainLayout->addWidget(minRadiusEntry,2,1);
	mainLayout->addWidget(new QLabel("Max Radius"),1,2);
	mainLayout->addWidget(maxRadiusEntry,2,2);
}
void Settings::makeConnections(){
	connect(minRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
	connect(maxRadiusEntry,SIGNAL(valueChanged(int)),this,SLOT(keepRadiusEntriesInSync()) );
}

QString Settings::getApproximatorName(){
	return "Circle-Delta";
}

} //namespace
