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
protected:
	QGridLayout *mainLayout;
	QLabel *description;
	QSpinBox *numCirclesEntry,*minRadiusEntry,*maxRadiusEntry;
	void makeWidgets();
	void layoutWidgets();
	void makeConnections();
};

class CircleSettings_DeltaSelector : public QWidget{
	Q_OBJECT
public:
	explicit CircleSettings_DeltaSelector(QWidget *parent = 0);
	int numCircles();
	int minRadius();
	int maxRadius();
public slots:
	void keepRadiusEntriesInSync();
protected:
	QGridLayout *mainLayout;
	QLabel *description;
	QSpinBox *numCirclesEntry,*minRadiusEntry,*maxRadiusEntry;
	void makeWidgets();
	void layoutWidgets();
	void makeConnections();
};

#endif // CIRCLESETTIGNS_H
