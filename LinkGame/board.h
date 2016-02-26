#ifndef BOARD_H
#define BOARD_H

#include <QGraphicsScene>
#include <QApplication>
#include <QRectF>
#include <QTimer>
#include "block.h"
#include "logic.h"

//to be used in the QMap arrangement
inline bool operator <(const QRectF& rect1, const QRectF& rect2){
    if (rect1.y()==rect2.y())
        return rect1.x()<rect2.x();
    else
        return rect1.y()<rect2.y();
}

class Board : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit Board(QGraphicsScene *parent = 0);
    ~Board();

    void newGame(int lv=1);
    QList<QString> saveData();      //save data to be loaded later
    void loadData(QList<QString>);  //load data from a list
    void shuffle();
    bool hint();
    void changeIcon(int icon=1);    //change the icon type

    bool isEmpty(Block *);          //if the block has been eliminated
    bool allClear();                //if all blocks have been eliminated
    bool link(Block *);             //try to link a block with a previous clicked block
    void select(Block *, Qt::PenStyle style=Qt::SolidLine);     //select the block when it's clicked or hinted

    int getIcon()const{return IconType;}
    int getLevel()const{return Game.getLevel();}
    bool stalemate(){return ((PairBuf.size()==0) && !search());}    //if there is no linkable pairs
private:
    enum HINT_MODE {SELECT, ELIMINATE}; //two modes of hint, select or automatically eliminate them

    void init();
    void clear();       //clear all allocated data
    void loadIcon();    //load icons according to icon type and map

    void linkLine(QList<Coord>);    //show a line when two blocks are linked
    QTimer *Timer;                  //this timer is used to control the animation of linking

    QPoint CoordToPos(Coord);       //convert from coord to point in the board
    QPoint CoordToCenter(Coord);    //convert from coord to central point's position

    int IconType;           //Icon sets, 1, 2 or 3
    HINT_MODE HintMode;     //mode of hint
    Block* prev;            //previous clicked block
    bool search();          //auto find linkable pair

    Logic Game;                                 //game logic
    QList<QPixmap> IconList;                    //store icons' picture
    QList<QGraphicsItem *> BlockList;           //store blocks
    QList<Block *> PairBuf;                     //buffer for hint
    QMap<QRectF,QGraphicsRectItem *> Border;    //store borders of selection
    QList<QGraphicsLineItem *> Lines;           //store linking lines
    QList<QGraphicsPixmapItem *>Fire;           //store fire animation
signals:

private slots:
    void flash();                               //invoked according to timer after two blocks are linked
};

#endif // BOARD_H
