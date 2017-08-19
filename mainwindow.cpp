#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
	mainLayout = new QGridLayout();
	setCentralWidget( new QWidget() );
	centralWidget()->setLayout(mainLayout);

	imageLocationLabel = new QLabel("/media/RAID/Documents/Pictures/tumblr/9cRJB3e.png");
	imageLocationButton = new QPushButton("Select Image");
	saveButton = new QPushButton("Save");
	startButton = new QPushButton("Start");
	imageDisplay = new ImageLabel();
	progressBar = new QProgressBar();

	progressBar->setRange(0,1000000);

	QHBoxLayout *topRow = new QHBoxLayout();
	mainLayout->addLayout(topRow,0,0);
	topRow->addWidget(imageLocationLabel,1);
	topRow->addWidget(imageLocationButton);
	topRow->addWidget(saveButton);
	topRow->addWidget(startButton);

	settingsMenu = new QTabWidget();
	mainLayout->addWidget(settingsMenu,1,0);
	settingsMenu->addTab(&circleSettings,"Circle");

	mainLayout->addWidget(imageDisplay,2,0);
	mainLayout->addWidget(progressBar,3,0);

	mainLayout->setRowStretch(0,0);
	mainLayout->setRowStretch(1,0);
	mainLayout->setRowStretch(2,1);
	mainLayout->setRowStretch(3,0);

	connect(imageLocationButton,SIGNAL(clicked(bool)),this,SLOT(selectImage()) );
	connect(saveButton,SIGNAL(clicked(bool)),this,SLOT(saveImageDialog()) );
	connect(startButton,SIGNAL(clicked(bool)),this,SLOT(toggleApproximator()) );
	connect(&circleApproximator,SIGNAL(progressMade(QImage,double)),this,SLOT(updateProgress(QImage,double)) );
	connect(&circleApproximator,SIGNAL(doneProcessing(QImage)),this,SLOT(finalImageApproximation(QImage)) );

	circleApproximator.moveToThread(&workerThread);
	workerThread.start();
	workerThreadBusy=false;
}

MainWindow::~MainWindow(){
	QMetaObject::invokeMethod(&circleApproximator,"stopProcessing");
	workerThread.quit();
	workerThread.wait();
}

void MainWindow::selectImage(){
	QString loc = QFileDialog::getOpenFileName();
	if(loc=="")return; // hit the cancel button
	imageLocationLabel->setText(loc);

	QImage temp(loc);
	displayImage(temp);
}

void MainWindow::toggleApproximator(){
	QString loc = imageLocationLabel->text();
	QImage temp(loc);
	if(temp.isNull()) return; //no image to work with

	if(!workerThreadBusy){
		workerThreadBusy = true;
		startButton->setText("Stop");
		imageLocationButton->setEnabled(false);
		bool ret =
		QMetaObject::invokeMethod(&circleApproximator,"processImage",
								  Q_ARG(QImage,temp),
								  Q_ARG(int,circleSettings.numCircles()),
								  Q_ARG(int,circleSettings.minRadius()),
								  Q_ARG(int,circleSettings.maxRadius())
								  );
		if(ret == false)
			printf("Callback Failed - starting thread");
	}else{
		workerThreadBusy = false;
		bool ret =
		QMetaObject::invokeMethod(&circleApproximator,"stopProcessing");
		if(ret == false)
			printf("Callback Failed - stoping thread");
	}
}

void MainWindow::updateProgress(QImage im, double percentage){
	displayImage(im);
	progressBar->setValue(percentage*10000);
}

void MainWindow::displayImage(QImage im){
	if(im.isNull()) return; //no image to work with

	displayedImage = im;
	imageDisplay->setPixmap(QPixmap::fromImage(im));
}

void MainWindow::finalImageApproximation(QImage image){
	displayImage(image);
	workerThreadBusy=false;
	startButton->setText("Start");
	imageLocationButton->setEnabled(true);
}

void MainWindow::saveImageDialog(){
	QString loc = QFileDialog::getSaveFileName();
	if(loc !="")
		displayedImage.save(loc);
}
