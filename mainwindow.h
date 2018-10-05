#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * ADDING A NEW APPROXIMATOR
 * you need two objects
 * - settings for the approximator
 * - the approximator itself
 * add your settings object to the constructor
 * add your files to the .pro file
 */

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
#include "imagelabel.h"

#include "circleapproximator.h"
#include "circleapproximator_deltaselector.h"
#include "kernelapproximator.h"
//#include "keypoint.h"

class MainWindow : public QMainWindow{
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
	enum STATE{IDLE,APPROXIMATING};
	STATE currentState;
	void setState(STATE);
	void constructGUI();
	void addApproximators();

	QGridLayout *mainLayout;
	QLabel *imageLocationLabel;
	QPushButton *imageLocationButton,*saveButton,*startButton;
	ImageLabel *imageDisplay;
	QProgressBar *progressBar;

	QTabWidget *settingsMenu;
	std::vector<BaseSettings*> settingsObjects;

	QImage displayedImage;

	QThread workerThread; // runs the wanted approximator in a different thread
};

#endif // MAINWINDOW_H
