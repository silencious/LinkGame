#ifndef LOGIC_H
#define LOGIC_H

#include <QTime>
#include <QMap>
#include <sstream>

#define MAX(a, b) a<b? b:a
#define MIN(a, b) a<b? a:b

#define ROWNUM  8
#define COLNUM  12

//integer coordinate of the block in the gameboard
struct Coord
{
    int Row;
    int Col;
    Coord(int r=0, int c=0);
};

inline bool operator ==(const Coord& coord1, const Coord& coord2){
    return (coord1.Row == coord2.Row) && (coord1.Col == coord2.Col);
}

//to be used in the QMap arrangement
inline bool operator <(const Coord& coord1, const Coord& coord2){
    if (coord1.Row == coord2.Row)
        return coord1.Col<coord2.Col;
    else
        return coord1.Row<coord2.Row;
}

class Logic
{
public:
    Logic();

    int getRow() const {return RowNum;}
    int getCol() const {return ColNum;}
    int getType(Coord);
    int getLevel() const {return Level;}

    void generate();                //generate a new map
    void shuffle();

    bool isEmpty(Coord);
    bool allClear();
    void eliminate(Coord, Coord);
    QList<Coord> link(Coord, Coord);
    void setLevel(int);

    QList<Coord> search();          //find linkable pairs
    QList<QString> saveData();      //to be used by Save/Load and PlayBack functions
    void loadData(QList<QString>);

private:
    void initMap();
    void setSize(int, int);

    void move(Coord, Coord);        //to be used in diffenrent levels
    void swapType(Coord, Coord);    //to be used by moving blocks
    bool legal(Coord);              //if the coord is in the map

    QList<Coord> noTurnLink(Coord, Coord);
    QList<Coord> oneTurnLink(Coord, Coord);
    QList<Coord> edgeLink(Coord, Coord);
    QList<Coord> twoTurnLink(Coord, Coord);
    QList<Coord> linkable(Coord, Coord);

    QMap<Coord, int> map;   //<coord, type> container

    int RowNum;
    int ColNum;
    int Remain;       //the number of remaining blocks
    int Level;
};

#endif // LOGIC_H
