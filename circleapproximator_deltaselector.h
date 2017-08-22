#ifndef CIRCLEAPPROXIMATOR_DELTASELECTOR_H
#define CIRCLEAPPROXIMATOR_DELTASELECTOR_H

#include <QObject>
#include <QImage>
#include <QColor>
#include <QTime>
#include <QCoreApplication>
#include <vector>
#include <map>
#include <unordered_map>
#include <random>
#include <stdint.h>

using std::vector;
using std::map;
using std::unordered_map;

class DeltaMap{
	/*
	 * MUHAHAHAHA
	 * This is a complicated structure that requires you know the
	 * way it works in its ENTIERTY before you work on it
	 *
	 * The point of this class is
	 * to be semi-fast for getting a point with high delta
	 * to be fast for moving points to different category of delta as we draw the circle
	 *
	 * it is two maps that are referencing pointers to seperate linked lists
	 * the map is a map of deltas to the first element of a linked list
	 * the linked list contains the x,y coords and the delta it was
	 * the unordered map is a fast lookup to the linked list using the coords - points to anywhere in the linked list
	 */
private:
	struct linked_element{
		linked_element *prev,*next;
		uint32_t x,y;
		double delta;
	};
	map<double,linked_element*> deltas;
	map<double,long long> deltas_counter; // numer of pixels with that delta
	unordered_map<uint64_t,linked_element*> points;

	//std::default_random_engine generator;
	//std::geometric_distribution<int> *distribution;
public:
	DeltaMap();
	~DeltaMap();
	void clear();
	void getHighDeltaPoint(uint32_t &x, uint32_t &y);
	void setPointDelta(uint32_t x, uint32_t y, double delta);
	void removePoint(uint32_t x, uint32_t y);
	void removeElement(linked_element *elm);
	long long numPoints(); // how many points are stored in this structure
};


/*
 * This circle approximator is a little bit more complicated. It goes after the pixel that has the most delta first.
 * It first makes a map of the image to find the delta of EVERY single pixel
 * It then selects the highests delta pixel and iterates a circle off of that (also removes the center point to prevent cycles)
 * It then removes from the map the pixels just drawn from the itterated circle
 * After all points have been removed from the map, it uses a second map that we have been writing too
 *  -- the second map is added to every time the circle is drawn
 *  -- the second map is what new delta values are and will be used to iterate on the next pass
 *  -- the first map is deleted since it does not have anything in it
 */
class CircleApproximator_DeltaSelector : public QObject{
	Q_OBJECT
	// temp vars for use in the processImage method - we save them externally so that i can call a method to do some work and not copy/paste the same thing 6 times
	int curtX,nextX;
	int curtY,nextY;
	int curtRadius,nextRadius;
	QColor currentColor;
	QImage newImage;
	double currentScore; // amount of change between modified image and original image
	vector<int> precomputedDistance;

	//other stuff
	QImage origImage;
	bool keepGoing;
	int minRadius;
	int maxRadius;

	int randRange(int low,int high); // warning - is inclusive of the high number
	double getScore(QImage &firstImage, QImage &secondImage, QColor color, int x,int y, int radius);
	int getColorDelta(QColor c1, QColor c2);
	void drawCircle(QImage &image,int x, int y, int radius, QColor color);
	void randomSelectCurrentCircle();
	bool tryPermutationAndMakeNextIfBetter(int x, int y, int radius);

	DeltaMap *map; // the current map we use to get points from
	DeltaMap *newMap; // the new map that will be used when we have exasted the current map
	void initMap(QImage &image);
	void updateMap(QImage &wantedImage, int centerX,int centerY,int radius,QColor color);
public:
	CircleApproximator_DeltaSelector();
	~CircleApproximator_DeltaSelector();
public slots:
	void processImage(QImage orig, int numCircles, int minRadius, int maxRadius);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

#endif // CIRCLEAPPROXIMATOR_DELTASELECTOR_H
