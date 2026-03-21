#include "glitch.h"
#include <cstring>
namespace Glitch{

void Approximator::processImage(QImage orig){
	Settings* sett = dynamic_cast<Settings*>(settingsObject);
	bool isRotate = sett->getIsRotate();
	auto effects = sett->getEffects();

	orig = orig.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	keepGoing = true;

	QImage current = orig;
	int w = current.width();
	int h = current.height();

	for(int step = 0; step < effects.size(); step++){
		auto [freq, phase, amp] = effects[step];

		QImage next(w, h, current.format());
		next.fill(Qt::black);

		for(int y = 0; y < h; y++){
			int offset = (int)round(amp * sin(freq * y + phase));
			const uchar *src = current.scanLine(y);
			uchar *dst = next.scanLine(y);

			if(isRotate){
				// normalize offset to [0, w)
				offset = ((offset % w) + w) % w;
				// dst[0..offset-1] <- src[w-offset..w-1]
				memcpy(dst, src + (w - offset) * 4, offset * 4);
				// dst[offset..w-1] <- src[0..w-1-offset]
				memcpy(dst + offset * 4, src, (w - offset) * 4);
			} else {
				if(offset > 0){
					int count = w - offset;
					if(count > 0)
						memcpy(dst + offset * 4, src, count * 4);
				} else if(offset < 0){
					int count = w + offset;
					if(count > 0)
						memcpy(dst, src + (-offset) * 4, count * 4);
				} else {
					memcpy(dst, src, w * 4);
				}
			}
		}

		current = next;
		emit progressMade(current, (step + 1) * 100.0 / effects.size());
		QCoreApplication::processEvents();
		if(!keepGoing)
			return;
	}

	emit doneProcessing(current);
}

Approximator::Approximator(BaseSettings *sett):BaseApproximator(sett){}

//=====================================================================================================================================================
//=====================================================================================================================================================
//		Settings Class Below
//=====================================================================================================================================================
//=====================================================================================================================================================

Settings::Settings(){
	localApproximator = dynamic_cast<BaseApproximator*>(new Approximator(this));

	// === Left control group
	controlGroup = new QGroupBox;
	controlLayout = new QVBoxLayout;
	numberEffects = new QSpinBox;
	shiftRotateToggle = new QPushButton("Shift");

	controlGroup->setLayout(controlLayout);
	controlLayout->addWidget(new QLabel("Number of Effects"));
	controlLayout->addWidget(numberEffects);
	controlLayout->addWidget(shiftRotateToggle);
	controlLayout->addStretch(1);

	numberEffects->setMinimum(1);
	numberEffects->setMaximum(10);
	numberEffects->setValue(1);

	shiftRotateToggle->setCheckable(true);

	connect(numberEffects, SIGNAL(valueChanged(int)), this, SLOT(numberEffectsChange()));
	connect(shiftRotateToggle, SIGNAL(toggled(bool)), this, SLOT(shiftRotateToggled()));

	// == Finish out the layout
	globalLayout = new QHBoxLayout;
	this->setLayout(globalLayout);
	globalLayout->addWidget(controlGroup);
	globalLayout->addStretch(1);

	// Add initial effect
	addNewEffect();
}

void Settings::numberEffectsChange(){
	int target = numberEffects->value();
	while(effectGroups.length() > target){
		auto temp = effectGroups.last();
		globalLayout->removeWidget(temp);
		delete temp;
		effectGroups.pop_back();
	}
	while(effectGroups.length() < target){
		addNewEffect();
	}
}

void Settings::shiftRotateToggled(){
	if(shiftRotateToggle->isChecked())
		shiftRotateToggle->setText("Rotate");
	else
		shiftRotateToggle->setText("Shift");
}

void Settings::addNewEffect(){
	int n = effectGroups.length() + 1;
	QGroupBox *group = new QGroupBox(QString("Effect %1").arg(n));
	QFormLayout *layout = new QFormLayout;
	group->setLayout(layout);

	// Frequency slider
	QSlider *freqSlider = new QSlider(Qt::Horizontal);
	freqSlider->setRange(0, 1000);
	freqSlider->setValue(100);
	QLabel *freqLabel = new QLabel("0.0500");
	QHBoxLayout *freqRow = new QHBoxLayout;
	freqRow->addWidget(freqSlider);
	freqRow->addWidget(freqLabel);
	QWidget *freqWidget = new QWidget;
	freqWidget->setLayout(freqRow);
	layout->addRow("Frequency", freqWidget);

	connect(freqSlider, &QSlider::valueChanged, [freqLabel](int val){
		double freq = val / 1000.0 * 0.5;
		freqLabel->setText(QString::number(freq, 'f', 4));
	});

	// Phase slider
	QSlider *phaseSlider = new QSlider(Qt::Horizontal);
	phaseSlider->setRange(0, 1000);
	phaseSlider->setValue(0);
	QLabel *phaseLabel = new QLabel("0.0000");
	QHBoxLayout *phaseRow = new QHBoxLayout;
	phaseRow->addWidget(phaseSlider);
	phaseRow->addWidget(phaseLabel);
	QWidget *phaseWidget = new QWidget;
	phaseWidget->setLayout(phaseRow);
	layout->addRow("Phase", phaseWidget);

	connect(phaseSlider, &QSlider::valueChanged, [phaseLabel](int val){
		double phase = val / 1000.0 * 2.0 * M_PI;
		phaseLabel->setText(QString::number(phase, 'f', 4));
	});

	// Amplitude spinbox
	QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
	ampSpin->setRange(-2000.0, 2000.0);
	ampSpin->setDecimals(1);
	ampSpin->setSingleStep(1.0);
	ampSpin->setValue(30.0);
	layout->addRow("Amplitude", ampSpin);

	effectGroups.append(group);
	// Insert before the stretch (stretch is the last item)
	globalLayout->insertWidget(globalLayout->count() - 1, group);
}

bool Settings::getIsRotate(){
	return shiftRotateToggle->isChecked();
}

QList<std::tuple<double,double,double>> Settings::getEffects(){
	QList<std::tuple<double,double,double>> result;
	for(auto group : effectGroups){
		QFormLayout *layout = (QFormLayout*)group->layout();

		// Row 0: Frequency widget
		QWidget *freqWidget = layout->itemAt(0, QFormLayout::FieldRole)->widget();
		QSlider *freqSlider = freqWidget->findChild<QSlider*>();
		double freq = freqSlider->value() / 1000.0 * 0.5;

		// Row 1: Phase widget
		QWidget *phaseWidget = layout->itemAt(1, QFormLayout::FieldRole)->widget();
		QSlider *phaseSlider = phaseWidget->findChild<QSlider*>();
		double phase = phaseSlider->value() / 1000.0 * 2.0 * M_PI;

		// Row 2: Amplitude spinbox
		QDoubleSpinBox *ampSpin = (QDoubleSpinBox*)layout->itemAt(2, QFormLayout::FieldRole)->widget();
		double amp = ampSpin->value();

		result.append(std::make_tuple(freq, phase, amp));
	}
	return result;
}

QString Settings::getApproximatorName(){
	return "Glitch";
}

}//namespace
