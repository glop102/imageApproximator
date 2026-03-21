#include "imagelabel.h"

ImageLabel::ImageLabel(){
}

void ImageLabel::setPixmap(QPixmap pix){
	original = pix;
	//repaint();
	update();
}

void ImageLabel::paintEvent(QPaintEvent *event){
	QLabel::paintEvent(event);
	if(original.isNull()) return;

	float w,h; // widget size
	w=width();
	h=height();

	float pw,ph; // picture size
	pw = original.width();
	ph = original.height();

	float dw,dh; // difference ratio
	dw = pw/w;
	dh = ph/h;

	int lw,lh; // location

	if(pw<w && ph<h){
		//no scaling - the picture is small enough to just be displayed
		scaled = original;
		lw=(w-pw)/2;
		lh=(h-ph)/2;
	}else if(dw > dh){
		scaled = original.scaledToWidth(w);
		lw = 0;
		lh = (h-scaled.height())/2;
	}else{
		scaled = original.scaledToHeight(h);
		lw = (w-scaled.width())/2;
		lh = 0;
	}

	QPainter paint(this);
	paint.drawPixmap(lw,lh,scaled);
}
