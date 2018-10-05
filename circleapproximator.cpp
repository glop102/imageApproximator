#include "circleapproximator.h"
namespace Circle{

void Approximator::processImage(QImage orig){
	printf("starting circle approximation - random selector\n");
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

	#pragma omp parallel shared(keepGoing)
	{
		int circlesPerThread;
		#pragma omp critical
		{
			int num_threads = omp_get_num_threads();
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
			}while(circle.score<=0);

			//permutate the circle until optimized
			while(tryPermutationForBetterCircle(&circle));

			drawCircle(currentApproximation,&circle);

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

Approximator::Approximator(BaseSettings *sett):BaseApproximator(sett){}

int Approximator::randRange(int low, int high){
	int range = high - low;
	double percentage = (rand()/(double)RAND_MAX);
	return percentage*range+low;
}

double Approximator::getScore(QImage &wantedImage, QImage &approximatedImage, int centerX, int centerY, int radius, QColor color){
	//it returns a score of how much BETTER the circle will be than the current approximation
	//A postitive return is a better circle than what is currently there

	//consider distance = sqrt(x*x + y*y);
	//if you know distance and y, solving for x terms gives
	//d*d - y*y = x*x
	//so since we know radius of the circle and the current height
	//we can find the max horizontal distance to move

	//this function only counts the pixels around the circumference of the circle
	//this covers the typical case as we only really need to know if it is
	//intersecting any other colored areas on the circumference

	//so it gets the total difference of the current approximation and the difference the
	//circle will have if drawn. If the new<current, then the new is less different than current frrom the wanted
	//This also favors larger circles as a larger circle will potentially cover more area that
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

	if(maxy<=miny || maxx<=minx) return -10000; //something is not making sense, return a really bad estimate

	int unSqauredRadius = precomputedDistance[radius]; // the max distance from the center anything should go
	for(int y=miny;y<=maxy;y++){
		int unSquaredHeight = precomputedDistance[abs(centerY-y)];
		int maxXDist = unSqauredRadius - unSquaredHeight;
		int x=1;
		for(; precomputedDistance[x]<=maxXDist;++x); // get so x is JUST outside of the circle
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

int Approximator::getColorDelta(QColor c1, QColor c2){
	int temp,total=0;
	int h1,s1,l1,a1;
	c1.getHsl(&h1,&s1,&l1,&a1);
	int h2,s2,l2,a2;
	c2.getHsl(&h2,&s2,&l2,&a2);

	temp = abs(h1-h2);
	total += temp;
	temp = abs(s1-s2);
	total += temp;
	temp = abs(l1-l2);
	total += temp*temp;
	temp = abs(a1-a2);
	total += temp;

	return total;
}

void Approximator::drawCircle(QImage &image, struct Circle * circle){
	//consider distance = sqrt(x*x + y*y);
	//if you know distance and y, solving for x terms gives
	//d*d - y*y = x*x
	//so since we know radius of the circle and the current height
	//we can find the max horizontal distance to move

	//it starts from the top of the circle and goes down
	//for every row, it starts from the middle and moves outwards
	//This is used because it is faster the the QPainter drawing the circle

	int &centerX = circle->centerX;
	int &centerY = circle->centerY;
	int &radius = circle->radius;
	QColor &color = circle->color;

	int width;
	int height;
	uchar *bits;// faster to directly modify bytes - the setPixel method is slow
	#pragma omp critical
	{
		width = image.width();
		height = image.height();
		bits = image.bits(); // faster to directly modify bytes - the setPixel method is slow
	}
	int &unSqrtedDistance = precomputedDistance[radius]; // the max distance from the center anything should go

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
}

void Approximator::randomSelectCurrentCircle(struct Circle * circle){
	circle->centerX=randRange(0,origImage.width()-1);
	circle->centerY=randRange(0,origImage.height()-1);
	circle->radius = minRadius;
	circle->color = QColor::fromRgba(origImage.pixel(circle->centerX,circle->centerY));
}

bool Approximator::tryPermutationForBetterCircle(struct Circle *circle){
	double
		score1 = getScore(origImage,currentApproximation,circle->centerX+1,circle->centerY,circle->radius,circle->color),
		score2 = getScore(origImage,currentApproximation,circle->centerX-1,circle->centerY,circle->radius,circle->color),
		score3 = getScore(origImage,currentApproximation,circle->centerX,circle->centerY+1,circle->radius,circle->color),
		score4 = getScore(origImage,currentApproximation,circle->centerX,circle->centerY-1,circle->radius,circle->color),
		score5 = getScore(origImage,currentApproximation,circle->centerX,circle->centerY,circle->radius+1,circle->color),
		score6 = getScore(origImage,currentApproximation,circle->centerX,circle->centerY,circle->radius-1,circle->color)
	;

	if( circle->centerX<origImage.width()			&& score1>circle->score && score1>score2 && score1>score3 && score1>score4 && score1>score5 && score1>score6 ){
		circle->centerX += 1;
		circle->score = score1;
		return true;
	} else if( circle->centerX>0					&& score2>circle->score && score2>score3 && score2>score4 && score2>score5 && score2>score6 ){
		circle->centerX -= 1;
		circle->score = score2;
		return true;
	} else if( circle->centerY<origImage.height()	&& score3>circle->score && score3>score4 && score3>score5 && score3>score6 ){
		circle->centerY += 1;
		circle->score = score3;
		return true;
	} else if( circle->centerY>0					&& score4>circle->score && score4>score5 && score4>score6 ){
		circle->centerY -= 1;
		circle->score = score4;
		return true;
	} else if( circle->radius<maxRadius				&& score5>circle->score && score5>score6 ){
		circle->radius += 1;
		circle->score = score5;
		return true;
	} else if( circle->radius>minRadius				&& score6>circle->score ){
		circle->radius -= 1;
		circle->score = score6;
		return true;
	}

	return false;
}

//======================================================================================================================================
//======================================================================================================================================
//    Settings Class Below
//======================================================================================================================================
//======================================================================================================================================

Settings::Settings():BaseSettings(){
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

QString Settings::getApproximatorName(){
	return "Circle-Random";
}

void Settings::makeWidgets(){
	description = new QLabel("This randomly chooses where to put a circle at");
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

}//namespace
