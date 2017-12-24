#include "lib/CompGate.h"

CompGate::CompGate():FuncGate(50,50,QColor(255,180,180),2){}

void CompGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    FuncGate::paint(painter,option, widget);
}

void GTGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    CompGate::paint(painter,option,widget);
    QRectF rect=boundingRect();
    painter->drawText(rect.x()+rect.width()/2-3,rect.y()+rect.height()/2+3,">");
}
void NEGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    CompGate::paint(painter,option,widget);
    QRectF rect=boundingRect();
    painter->drawText(rect.x()+rect.width()/2-3,rect.y()+rect.height()/2+3,"!=");
}
void EQGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    CompGate::paint(painter,option,widget);
    QRectF rect=boundingRect();
    painter->drawText(rect.x()+rect.width()/2-3,rect.y()+rect.height()/2+3,"=");
}
void LEGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    CompGate::paint(painter,option,widget);
    QRectF rect=boundingRect();
    painter->drawText(rect.x()+rect.width()/2-3,rect.y()+rect.height()/2+3,"<=");
}
void GEGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    CompGate::paint(painter,option,widget);
    QRectF rect=boundingRect();
    painter->drawText(rect.x()+rect.width()/2-3,rect.y()+rect.height()/2+3,">=");
}
void LTGate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    CompGate::paint(painter,option,widget);
    QRectF rect=boundingRect();
    painter->drawText(rect.x()+rect.width()/2-3,rect.y()+rect.height()/2+3,"<");
}
data_t GTGate::eval()const{
    return input[0]->eval() > input[1]->eval();
}

data_t LTGate::eval()const{
    return input[0]->eval() < input[1]->eval();
}

data_t GEGate::eval()const{
    return input[0]->eval() >= input[1]->eval();
}

data_t LEGate::eval()const{
    return input[0]->eval() <= input[1]->eval();
}

data_t EQGate::eval()const{
    return input[0]->eval() == input[1]->eval();
}

data_t NEGate::eval()const{
    return input[0]->eval() != input[1]->eval();
}
