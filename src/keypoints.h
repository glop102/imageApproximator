#ifndef KEYPOINTS_H
#define KEYPOINTS_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <QList>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QString>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSpinBox>
#include <QElapsedTimer>
#include "base.h"

namespace Keypoints {
class Settings;

class Approximator : public BaseApproximator{
	Q_OBJECT
protected:
public:
	Approximator(BaseSettings*);

public slots:
	void processImage(QImage orig);
};

class Settings : public BaseSettings{
	Q_OBJECT
public:
	explicit Settings();

public slots:
	virtual QString getApproximatorName();
protected:
	QHBoxLayout *globalLayout;
	QSpinBox *scalingFactor; // 1.0 - 0 - What the image is scaled by for each downsampling

	// stopping point of downscaling the image can be several different things (provide radio buttons to select method)
	// - number of steps to downsample
	// - ending width/height of an image (absolute number)
	// - ending width/height of an image (multiplyer against search radius)
	// - number of keypoints found in the current downsampled layer (few or no keypoints, give up)
	QSpinBox *minImageSize;

	//search MUST be >= selection
	//if the max in a region is within search but outside select, we move in that direction (perhaps put search kernel centered on the max?)
	//if the max in a region is within both search and select then that is defined as a keypoint and noted in a list to prevent double detections
	//(remember to prevent double detections at different scaled views in addition to just the current scale) (favor the 'smaller' keypoints aka when the image is larger and so keypoint is over a smaller area)
	QSpinBox *cornerMaximum_selectionRadius; // Radius where if we find a max will allow us to select
	QSpinBox *cornerMaximum_searchRadius; // Radius where we will continue searching in a direction for a maximum

	// Minumum threshold value for the kernel detector to be able to select a point as a maximum
	QSpinBox *detectionThreshold;
	//Radio Buttons for threshold of (all channels below) or (average RGB below) or (pixel theshold amount above the kernel average)

	//Settings for colorspace shifting? - mostly useful for consistent hashing between images?
	// rotate RGB channels to have largest average channel be in R - direct channel shift, no intermediat scaling
	// rotate RGB channels - same as above but fraction rotating of the channels?
	//Second thought - this is only useful when you pull the triangles out and you want to remove colorspace transforms
};

}; // namespace Keypoints

#endif // KEYPOINTS_H
