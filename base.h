#ifndef BASE_H
#define BASE_H

#include <QObject>
#include <QWidget>
#include <QImage>

class BaseApproximator : public QObject{
	Q_OBJECT
public slots:
	//void processImage(QImage orig, other args);
	virtual void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage); //out of 100% aka 0-100 float
	void doneProcessing(QImage);
};

class BaseSettings : public QWidget{
	Q_OBJECT
public slots:
	virtual BaseApproximator* getApproximator(); //returns a valid instance
	virtual int startApproximator(QImage original); //returns success value give by invokeMethod()
	virtual int stopApproximator();
};

#endif // BASE_H
