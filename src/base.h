#ifndef BASE_H
#define BASE_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <atomic>

class BaseSettings;
class BaseApproximator;


/* Making Your Own Setting Class
 *
 * Inherit from this BaseSettings class
 * Your constructor should create the Approximator object saved to localApproximator
 *     localApproximator = dynamic_cast<BaseSettings*>(new Approximator(this));
 * Your constructor should create the widgets inside itself that give the settings to be passed to the approximator
 * Your settings should have get methods for the settings so the approximator can ask
 * Make sure to return your own name for the GUI to use
 */
class BaseSettings : public QWidget{
	Q_OBJECT
protected:
	BaseApproximator* localApproximator;
public:
	BaseSettings();
public slots:
	virtual BaseApproximator* getApproximator(); //returns a valid instance
	virtual QString getApproximatorName(); // this gets used for the text of the tab
	virtual int startApproximator(QImage original); //returns success value give by invokeMethod()
	virtual int stopApproximator();
};


/* Making your own Approximator
 *
 * Inherit from this base class
 * Your constructor needs to take a settings object and pass it to the base constructor
 * Your processImage method
 *  - each major loop itteration should check that keepGoing is still true
 *  - emit progressMade semi-often, when it makes sense to do
 *  - cast the settings object tto your specific object with dynamic_cast<>() and then pull the
 *    setting you need from it, prefferably with public getter methods (best practices)
 */
class BaseApproximator : public QObject{
	Q_OBJECT
protected:
	BaseSettings* settingsObject;
	std::atomic<bool> keepGoing;
public:
	explicit BaseApproximator(BaseSettings* settings);
public slots:
	virtual void processImage(QImage orig);
	virtual void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage); //out of 100% aka 0-100 float
	void doneProcessing(QImage);
};

#endif // BASE_H
