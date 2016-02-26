#include "board.h"

#define GAPWID      3   //gap between blocks
#define BORDERWID   4

//color of selecting border
#define PEN_A       QPen(QBrush(QColor(255,200,0)),BORDERWID,style)
#define PEN_B       QPen(QBrush(QColor(255,0,0)),BORDERWID,style)

using namespace std;
Board::Board(QGraphicsScene *parent)
    :QGraphicsScene(parent),
     IconType(1),
     HintMode(SELECT),
     prev(NULL)
{
    init();
}

Board::~Board(){
    clear();
}

void Board::init(){
    //prepare icons' picture
    for(int i=1; i<=24; i++){
        QString FileName;
        FileName.sprintf(":pic/icon%d/%d.png", IconType, i);
        IconList << QPixmap(FileName).scaled(Block::getSize());
    }
    //initialize blank blocks
    for(int r=0; r<Game.getRow(); r++){
        for(int c=0; c<Game.getCol(); c++){
            Coord coord(r,c);
            Block* pB = new Block(coord);
            pB -> setPos(CoordToPos(coord));
            addItem(pB);
        }
    }
    BlockList = items(Qt::AscendingOrder);  //container of all blocks

    Timer = new QTimer(this);
    connect(Timer, SIGNAL(timeout()), this, SLOT(flash()));
    newGame();
}
//delete all allocated data and prepare for other maps(or end game)
void Board::clear(){
    if(BlockList.size()!=0){
        qDeleteAll(BlockList);
        BlockList.clear();
    }
    if(Border.size()!=0){
        qDeleteAll(Border);
        Border.clear();
    }
    if(Lines.size()!=0){
        qDeleteAll(Lines);
        Lines.clear();
    }
    if(Fire.size()!=0){
        qDeleteAll(Fire);
        Fire.clear();
    }
    delete Timer;
    prev = NULL;
}
//load icons according to the IconList
void Board::loadIcon(){
    for(int i=0; i<BlockList.size(); i++){
        Block* block = (Block *)BlockList[i];
        int Type = Game.getType(block->getCoord());
        if (Type != 0){
            block -> setPixmap(IconList[Type-1]);
            block -> show();
        }
        else
            block->hide();
    }
}
//when two blocks are linked, show the route in the form of a line,
//place fire picture on the blocks, prepare for animation
void Board::linkLine(QList<Coord> Route){
    for(int i=1; i<Route.size(); i++){
        QLine l(CoordToCenter(Route[i-1]),CoordToCenter(Route[i]));
        QGraphicsLineItem *line = new QGraphicsLineItem(l);
        QPen pen(QColor(255, 128, 0));
        pen.setWidth(10);
        line->setPen(pen);
        line->setVisible(true);
        line->setZValue(2);     //over the blocks
        addItem(line);
        Lines << line;
    }
    QPixmap pic(":pic/fire/1.png");
    for(int i=0; i<2; i++){
        QGraphicsPixmapItem *fire = new QGraphicsPixmapItem(pic.scaled(Block::getSize()));
        fire->setPos(CoordToPos(Route[i==0?0:(Route.size()-1)]));
        fire->setVisible(true);
        fire->setZValue(3);     //over the blocks and lines
        addItem(fire);
        Fire << fire;
    }
    Timer->start(15);           //start timer, let flash() change the line and fire to get animation
}
//start a new game
void Board::newGame(int lv){
    if(Border.size()!=0){
        qDeleteAll(Border);
        Border.clear();
    }
    Game.setLevel(lv);  //pass the level number to logic part
    Game.generate();    //generate map <coord, type>
    loadIcon();         //load icons
}
//let logic part do the work
QList<QString> Board::saveData(){
    return Game.saveData();
}
//let logic load the data, update blocks' picture
void Board::loadData(QList<QString> data){
    Game.loadData(data);
    loadIcon();
}
//shuffle the blocks until there is a linkable pair
void Board::shuffle(){
    if(Border.size()!=0){
        qDeleteAll(Border);
        Border.clear();
    }
    do{
        Game.shuffle();
    }while(!search());
    loadIcon();         //reload icon
}

//block's position in the gameboard
QPoint Board::CoordToPos(Coord coord){
    return QPoint((coord.Col+1)*(Block::getWidth() + GAPWID),
                  (coord.Row+1)*(Block::getHeight() + GAPWID));
}
//block's center's position
QPoint Board::CoordToCenter(Coord coord){
    return QPoint((coord.Col+1)*(Block::getWidth() + GAPWID)+Block::getWidth()/2,
                  (coord.Row+1)*(Block::getHeight() + GAPWID)+Block::getHeight()/2);
}
//search linkable pair, add to the pair buffer list
//if succeed, return true
bool Board::search(){
    QList<Coord> coords = Game.search();
    PairBuf.clear();
    if(coords.size()!=0){
        PairBuf << (Block*)itemAt(CoordToPos(coords[0]), QTransform());
        PairBuf << (Block*)itemAt(CoordToPos(coords[1]), QTransform());
        return true;
    }
    return false;
}
//search and show linkable pair
bool Board::hint(){
    if(!stalemate()){
        if(HintMode == SELECT){         //mode 1, show a selecting border
            select(PairBuf[0], Qt::DashDotLine);
            select(PairBuf[1], Qt::DashDotLine);
        }
        else if(HintMode == ELIMINATE){ //mode 2, eliminate the pair
            prev = PairBuf[0];
            link(PairBuf[1]);
            PairBuf.clear();
        }
        return true;
    }
    return false;
}
//change iconlist and reload icon
void Board::changeIcon(int icon){
    IconType = icon;
    IconList.clear();
    for(int i=1; i<=24; i++){
        QString FileName;
        FileName.sprintf(":pic/icon%d/%d.png", IconType, i);
        IconList << QPixmap(FileName).scaled(Block::getSize());
    }
    loadIcon();
}
//if the block has been eliminated
bool Board::isEmpty(Block *block){
    return Game.isEmpty(block->getCoord());
}
//if all blocks have been eliminated
bool Board::allClear(){
    return Game.allClear();
}
//triggered when a mousePressEvent happen, check if the block is linkable
//if so, eliminate it and the previous block
bool Board::link(Block *block){
    if(prev == block)
        return false;

    if(Border.size() != 0){     //clear all borders drawn before
        qDeleteAll(Border);
        Border.clear();
    }

    if(prev == NULL){           //there hasn't been any selected block
        select(block);
        prev = block;
    }
    else{
        Block *curr = block;
        Coord coord1 = prev->getCoord(), coord2 = curr->getCoord();
        QList<Coord> Route = Game.link(coord1, coord2);
        if(Route.size()!=0){      //previous and current blocks are linkable
            linkLine(Route);
            loadIcon();
            search();
            prev = NULL;
            return true;
        }
        else{
            select(block);          //select current
            prev = curr;            //set current to previous
        }
    }
    return false;
}
//set selectiong border
void Board::select(Block *block, Qt::PenStyle style){
    QRectF rect = QRectF(block->pos(),Block::getSize());
    if(Border.find(rect) == Border.end()){  //not selected by player or hint
        QGraphicsRectItem *border = new QGraphicsRectItem(rect);
        Border.insert(rect, border);

        if(style == Qt::SolidLine)          //select by player
            border->setPen(PEN_A);
        else
            border->setPen(PEN_B);          //select by hint

        border->setVisible(true);
        border->setZValue(0);               //below the blocks
        addItem(border);
    }
    else{   //player select marked block
        Border[rect]->setPen(PEN_B);
    }
}
//animation of the linking line and fire picture
void Board::flash(){
    static int count = 0;   //count how many times has the function been called
    QTime time = QTime::currentTime();
    qsrand(time.msec() + time.second() * 1000);
    int color = qrand();
    QPen pen(QColor(color%150+100, color/150%150+100, color/150/150%150+100));  //random color
    pen.setWidth(10);
    for(int i=0; i<Lines.size(); i++){
        Lines[i]->setPen(pen);
    }
    if(count>15){           //the function has been called 16 times, stop the animation, reset count
        count = 0;
        Timer->stop();
        qDeleteAll(Lines);
        Lines.clear();
        qDeleteAll(Fire);
        Fire.clear();
    }
    else{
        int n = count/4 + 1;    //decide which picture of fire to show
        QString name;
        name.sprintf(":pic/fire/%d.png",n);
        QPixmap pic(name);
        for(int i=0; i<2; i++)
            Fire[i]->setPixmap(pic.scaled(Block::getSize()));
        count++;
    }
}
