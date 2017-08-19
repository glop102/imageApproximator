#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QPainter>
#include <QPixmap>

class ImageLabel : public QLabel{
	Q_OBJECT
private:
	QPixmap original;
	QPixmap scaled;
public:
	ImageLabel();
public slots:
	void setPixmap(QPixmap);
	void paintEvent(QPaintEvent *event);
};

#endif // IMAGELABEL_H
