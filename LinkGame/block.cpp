#include "block.h"


Block::Block(Coord c, QGraphicsPixmapItem *parent)
    :QGraphicsPixmapItem(parent), coord(c)
{
    setVisible(true);   //so that the block can be seen
    setZValue(1);       //so that the block can be clicked
}

Block::~Block(){

}


void Block::setCoord(Coord c){
    coord = c;
}


