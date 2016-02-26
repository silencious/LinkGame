#ifndef BLOCK_H
#define BLOCK_H

#include <QGraphicsPixmapItem>
#include "logic.h"

#define BLOCKWIDTH  44
#define BLOCKHEIGHT 44

//basic component of the gameboard
class Block : public QGraphicsPixmapItem
{
public:
    explicit Block(Coord, QGraphicsPixmapItem *parent = 0);
    ~Block();

    //some functions to get data from block
    Coord getCoord()const{return coord;}
    static QSize getSize(){return QSize(width, height);}

    static int getWidth(){return width;}
    static int getHeight(){return height;}

    int getRow() const{return coord.Row;}
    int getCol() const{return coord.Col;}

    //set the coordinate of the block
    void setCoord(Coord);

private:
    static const int width = BLOCKWIDTH;
    static const int height = BLOCKHEIGHT;
    Coord coord;
};

#endif // BLOCK_H
