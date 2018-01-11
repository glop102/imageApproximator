#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <stdio.h>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QProgressBar>

#include <QThread>
#include "circlesettings.h"
#include "circleapproximator.h"
#include "circleapproximator_deltaselector.h"
#include "imagelabel.h"
#include "edgesettings.h"
#include "kernelapproximator.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void selectImage();
	void toggleApproximator(); // starts/stops the approximator thread
	void updateProgress(QImage,double percentage);
	void displayImage(QImage);
	void finalImageApproximation(QImage);
	void saveImageDialog();

private:
	QGridLayout *mainLayout;
	QLabel *imageLocationLabel;
	QPushButton *imageLocationButton,*saveButton,*startButton;
	ImageLabel *imageDisplay;
	QProgressBar *progressBar;

	QTabWidget *settingsMenu;
	CircleSettings circleSettings;
	CircleSettings_DeltaSelector circleSettings2;
	EdgeSettings edgeSettings;

	QImage displayedImage;

	QThread workerThread; // runs the wanted approximator in a different thread
	bool workerThreadBusy;
	CircleApproximator circleApproximator;
	CircleApproximator_DeltaSelector circleApproximator_DeltaSelector;
	KernelApproximator kernelApproximator;
};

#endif // MAINWINDOW_H
