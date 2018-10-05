#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
	settingsObjects.push_back(new Circle::Settings() );
	settingsObjects.push_back(new Circle_DeltaSelector::Settings() );
	settingsObjects.push_back(new Kernel::Settings() );
	constructGUI();
	addApproximators();
}

void MainWindow::addApproximators(){
	for(auto sett : settingsObjects){
		settingsMenu->addTab(sett,sett->getApproximatorName());
		connect(sett->getApproximator(),SIGNAL(progressMade(QImage,double)),this,SLOT(updateProgress(QImage,double)) );
		connect(sett->getApproximator(),SIGNAL(doneProcessing(QImage)),this,SLOT(finalImageApproximation(QImage)) );

		sett->getApproximator()->moveToThread(&workerThread);
	}
	workerThread.start();
}
void MainWindow::constructGUI(){
	mainLayout = new QGridLayout();
	setCentralWidget( new QWidget() );
	centralWidget()->setLayout(mainLayout);

	//imageLocationLabel = new QLabel("/media/RAID/Documents/Pictures/tumblr/9cRJB3e.png");	//misato room service
	imageLocationLabel = new QLabel("/media/RAID/Documents/Pictures/Wallpapers/phlshu.png");
	imageLocationButton = new QPushButton("Select Image");
	saveButton = new QPushButton("Save");
	startButton = new QPushButton("Start");
	imageDisplay = new ImageLabel();
	progressBar = new QProgressBar();

	progressBar->setRange(0,1000000); // big number gives lots of granularity

	QHBoxLayout *topRow = new QHBoxLayout();
	mainLayout->addLayout(topRow,0,0);
	topRow->addWidget(imageLocationLabel,1);
	topRow->addWidget(imageLocationButton);
	topRow->addWidget(saveButton);
	topRow->addWidget(startButton);

	settingsMenu = new QTabWidget();
	mainLayout->addWidget(settingsMenu,1,0);
	mainLayout->addWidget(imageDisplay,2,0);
	mainLayout->addWidget(progressBar,3,0);

	mainLayout->setRowStretch(0,0);
	mainLayout->setRowStretch(1,0);
	mainLayout->setRowStretch(2,1);
	mainLayout->setRowStretch(3,0);

	connect(imageLocationButton,SIGNAL(clicked(bool)),this,SLOT(selectImage()) );
	connect(saveButton,SIGNAL(clicked(bool)),this,SLOT(saveImageDialog()) );
	connect(startButton,SIGNAL(clicked(bool)),this,SLOT(toggleApproximator()) );
	setState(IDLE);
}

MainWindow::~MainWindow(){
	for(auto sett : settingsObjects)
		sett->stopApproximator();
	workerThread.quit();
	workerThread.wait();

	for(auto sett : settingsObjects)
		delete sett;
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
	static BaseSettings *currentTab;
	bool ret; // return status

	if(currentState == IDLE){
		setState(APPROXIMATING);
		currentTab = (BaseSettings*)settingsMenu->currentWidget();
		ret = currentTab->startApproximator(temp);
		if(ret == false){
			printf("Callback Failed - starting thread\n");
			setState(IDLE);
		}
	}else{
		ret = currentTab->stopApproximator();
		if(ret == false){
			printf("Callback Failed - stoping thread\n");
			setState(IDLE);
		}
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
	setState(IDLE);
}

void MainWindow::saveImageDialog(){
	QString loc = QFileDialog::getSaveFileName();
	if(loc !="")
		displayedImage.save(loc);
}

void MainWindow::setState(STATE s){
	currentState = s;

	switch(s){
	case IDLE:
		startButton->setText("Start");
		imageLocationButton->setEnabled(true);
		progressBar->setValue(progressBar->maximum());
		break;
	case APPROXIMATING:
		startButton->setText("Stop");
		imageLocationButton->setEnabled(false);
		progressBar->setValue(progressBar->minimum());
		break;
	}
}
