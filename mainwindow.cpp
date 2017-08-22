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
	settingsMenu->addTab(&circleSettings,"Circle-Random");
	settingsMenu->addTab(&circleSettings2,"Circle-Stable");

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
	connect(&circleApproximator_DeltaSelector,SIGNAL(progressMade(QImage,double)),this,SLOT(updateProgress(QImage,double)) );
	connect(&circleApproximator_DeltaSelector,SIGNAL(doneProcessing(QImage)),this,SLOT(finalImageApproximation(QImage)) );

	circleApproximator.moveToThread(&workerThread);
	circleApproximator_DeltaSelector.moveToThread(&workerThread);
	workerThread.start();
	workerThreadBusy=false;
}

MainWindow::~MainWindow(){
	QMetaObject::invokeMethod(&circleApproximator,"stopProcessing");
	QMetaObject::invokeMethod(&circleApproximator_DeltaSelector,"stopProcessing");
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
	static QWidget *currentTab;
	bool ret; // return status

	if(!workerThreadBusy){
		workerThreadBusy = true;
		startButton->setText("Stop");
		imageLocationButton->setEnabled(false);
		currentTab = settingsMenu->currentWidget();
		if(currentTab == &circleSettings){
			ret =
			QMetaObject::invokeMethod(&circleApproximator,"processImage",
									  Q_ARG(QImage,temp),
									  Q_ARG(int,circleSettings.numCircles()),
									  Q_ARG(int,circleSettings.minRadius()),
									  Q_ARG(int,circleSettings.maxRadius())
									  );
		}else if(currentTab == &circleSettings2){
			ret =
			QMetaObject::invokeMethod(&circleApproximator_DeltaSelector,"processImage",
									  Q_ARG(QImage,temp),
									  Q_ARG(int,circleSettings2.numCircles()),
									  Q_ARG(int,circleSettings2.minRadius()),
									  Q_ARG(int,circleSettings2.maxRadius())
									  );
		}
		if(ret == false)
			printf("Callback Failed - starting thread");
	}else{
		workerThreadBusy = false;
		if(currentTab == &circleSettings){
			ret =
			QMetaObject::invokeMethod(&circleApproximator,"stopProcessing");
		}else if(currentTab == &circleSettings2){
			ret=
			QMetaObject::invokeMethod(&circleApproximator_DeltaSelector,"stopProcessing");
		}
		if(ret == false)
			printf("Callback Failed - stoping thread");
	}
}

void MainWindow::updateProgress(QImage im, double percentage){
	displayImage(im);
	int amount = percentage * progressBar->maximum() / 100;
	progressBar->setValue(amount);
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
	progressBar->setValue(progressBar->maximum());
}

void MainWindow::saveImageDialog(){
	QString loc = QFileDialog::getSaveFileName();
	if(loc !="")
		displayedImage.save(loc);
}
