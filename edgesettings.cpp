#include "edgesettings.h"

EdgeSettings::EdgeSettings(QWidget *parent) : QWidget(parent){
	// === Left Group - How the different images are combined after getting the kernel applied to it
	combinationOptions = new QGroupBox("Combination Options");
	averageButton = new QRadioButton("Average");
	maxButton = new QRadioButton("Max Value");
	maxButton->setChecked(true);
	combinationLayout = new QVBoxLayout;
	combinationLayout->addWidget(averageButton);
	combinationLayout->addWidget(maxButton);
	combinationLayout->addStretch(1);
	combinationOptions->setLayout(combinationLayout);

	// === Second Group - Number of kernels to apply
	kernelNumberSelection = new QGroupBox("Number of Kernels");
	numberKernelsLabel = new QLabel("2");
	fewerKernels = new QPushButton("-");
	moreKernels = new QPushButton("+");
	kernelNumberLayout = new QGridLayout;
	kernelNumberLayout->addWidget(numberKernelsLabel,0,0,1,0);
	kernelNumberLayout->addWidget(fewerKernels,1,0);
	kernelNumberLayout->addWidget(moreKernels,1,1);
	kernelNumberLayout->setRowStretch(2,1);
	kernelNumberSelection->setLayout(kernelNumberLayout);
	connect(fewerKernels,SIGNAL(clicked(bool)),this,SLOT(numberKernelsChange()) );
	connect(moreKernels,SIGNAL(clicked(bool)),this,SLOT(numberKernelsChange()) );

	globalLayout = new QHBoxLayout;
	this->setLayout(globalLayout);
	globalLayout->addWidget(combinationOptions);
	globalLayout->addWidget(kernelNumberSelection);
	globalLayout->addStretch(1);

	addNewKernel({-1,0,1,-2,0,2,-1,0,1}); // horizontal
	addNewKernel({-1,-2,-1,0,0,0,1,2,1}); // vertical
	addNewKernel({0,1,2,-1,0,1,-2,-1,0}); // diag (top right to bot left)
	addNewKernel({2,1,0,1,0,-1,0,-1,-2}); // diag (top left to bot right)
}

void EdgeSettings::numberKernelsChange(){
	int current_number = numberKernelsLabel->text().toInt();
	if(QObject::sender() == fewerKernels){
		if(current_number<=1) return;
		current_number-=1;
		while(kernelGroups.length()>current_number){
			auto temp = kernelGroups[kernelGroups.length()-1];
			globalLayout->removeWidget(temp);
			delete temp;
			kernelGroups.pop_back();
		}
	}else{
		if(current_number>=10) return;
		current_number+=1;
		while(kernelGroups.length()<current_number){
			addNewKernel();
		}
	}

	numberKernelsLabel->setText(QString::number(current_number));
}

void EdgeSettings::addNewKernel(QList<double> kernel){
	while(kernel.length()<9)
		kernel.append(0);
	while(kernel.length()>9)
		kernel.pop_back();

	QGroupBox *group = new QGroupBox;
	QGridLayout *layout = new QGridLayout;
	QLineEdit *l1,*l2,*l3,*l4,*l5,*l6,*l7,*l8,*l9;
	l1 = new QLineEdit(QString::number(kernel[0]));
	l2 = new QLineEdit(QString::number(kernel[1]));
	l3 = new QLineEdit(QString::number(kernel[2]));
	l4 = new QLineEdit(QString::number(kernel[3]));
	l5 = new QLineEdit(QString::number(kernel[4]));
	l6 = new QLineEdit(QString::number(kernel[5]));
	l7 = new QLineEdit(QString::number(kernel[6]));
	l8 = new QLineEdit(QString::number(kernel[7]));
	l9 = new QLineEdit(QString::number(kernel[8]));

	l1->setValidator(&doubleValidator);
	l2->setValidator(&doubleValidator);
	l3->setValidator(&doubleValidator);
	l4->setValidator(&doubleValidator);
	l5->setValidator(&doubleValidator);
	l6->setValidator(&doubleValidator);
	l7->setValidator(&doubleValidator);
	l8->setValidator(&doubleValidator);
	l9->setValidator(&doubleValidator);

	layout->addWidget(l1,0,0);
	layout->addWidget(l2,0,1);
	layout->addWidget(l3,0,2);

	layout->addWidget(l4,1,0);
	layout->addWidget(l5,1,1);
	layout->addWidget(l6,1,2);

	layout->addWidget(l7,2,0);
	layout->addWidget(l8,2,1);
	layout->addWidget(l9,2,2);

	layout->setRowStretch(3,1);
	group->setLayout(layout);

	kernelGroups.append(group);
	globalLayout->addWidget(group);
}

QList<QList<double> > EdgeSettings::getKernels(){
	QList<QList<double> > totalList;
	for(auto group : kernelGroups){
		QGridLayout *layout = (QGridLayout*)group->layout();
		QList<double> temp;
		for(int x=0;x<9;x++){
			temp.push_back(
				((QLineEdit*)layout->itemAtPosition(x/3,x%3)->widget())
					->text().toDouble()
			);
		}
		totalList.push_back(temp);
	}
	return totalList;
}

KernelApproximator::combinationTypes EdgeSettings::getCombinationType(){
	if(averageButton->isChecked())
		return KernelApproximator::AVERAGE;
	else
		return KernelApproximator::MAXIMUM;
}
