#ifndef KERNELAPPROXIMATOR_H
#define KERNELAPPROXIMATOR_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QCoreApplication>

class KernelApproximator : public QObject
{
	Q_OBJECT
protected:
	bool stopSignalRecieved;
	double calculateKernelDivisor(QList<double> kernel);
public:
	enum combinationTypes{AVERAGE,MAXIMUM};
	explicit KernelApproximator(QObject *parent = nullptr);
	QImage applyKernel(QImage orig, QList<double> kernel);

	QImage combine_average(QList<QImage> images);
	QImage combine_maximum(QList<QImage> images);

public slots:
	void processImage(QImage orig, QList<QList<double> > kernels, KernelApproximator::combinationTypes combType);
	void stopProcessing(); // cancels the operation
signals:
	void progressMade(QImage,double percentage);
	void doneProcessing(QImage);
};

#endif // KERNELAPPROXIMATOR_H
