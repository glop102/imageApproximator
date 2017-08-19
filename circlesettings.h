#ifndef CIRCLESETTIGNS_H
#define CIRCLESETTIGNS_H

#include <QWidget>
#include <QGridLayout>
#include <QSpinBox>
#include <QLabel>

class CircleSettings : public QWidget
{
	Q_OBJECT
public:
	explicit CircleSettings(QWidget *parent = 0);
	int numCircles();
	int minRadius();
	int maxRadius();
public slots:
	void keepRadiusEntriesInSync();
private:
	QGridLayout *mainLayout;
	QSpinBox *numCirclesEntry,*minRadiusEntry,*maxRadiusEntry;
};

#endif // CIRCLESETTIGNS_H
