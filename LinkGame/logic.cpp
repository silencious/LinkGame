#include "logic.h"

using namespace std;

Coord::Coord(int r, int c)
    :Row(r), Col(c)
{
}

Logic::Logic()
    :RowNum(ROWNUM), ColNum(COLNUM), Level(1)
{
    initMap();
}

//return the picture number of the block at coord
int Logic::getType(Coord coord){
    if(legal(coord))
        return map[coord];
    else
        return 0;
}
//initialize
void Logic::initMap(){
    for(int i=0;i<RowNum;i++){
        for(int j=0;j<ColNum;j++) {
            Coord coord(i,j);
            map.insert(coord,0);
        }
    }
}
//set size of the gameboard, not used in the final version
void Logic::setSize(int r, int c){
    RowNum = r;
    ColNum = c;
}
//generate map of <coord, type> randomly
void Logic::generate(){
    QList<Coord> CoordList;
    QList<int> TypeList;

    int Count = (Level<=5)? 6:4;                //number of blocks for each type
    int typenum = RowNum * ColNum / Count;      //number of total types

    QMap<Coord, int>::const_iterator it=map.begin();
    int i=0;
    while(it!=map.end()){                       //get lists of coords and types
        i = (i%typenum) + 1;
        CoordList << it.key();
        TypeList << i;
        it++;
    }
    Remain = CoordList.size();

    QTime time = QTime::currentTime();
    qsrand(time.msec() + time.second() * 1000);     //use current millisec as seed
    for(int i=0; i<CoordList.size(); i++){
        int index = qrand() % TypeList.size();      //random index, disorganize the map
        map[CoordList[i]] = TypeList[index];        //set map <coord, type>
        TypeList.removeAt(index);                   //remove from typelist
    }
}
//shuffle
void Logic::shuffle(){
    QList<Coord> CoordList;
    QList<int> TypeList;

    QMap<Coord, int>::const_iterator it=map.begin();
    while(it != map.end()){
        if(it.value() != 0){            //ignore blank blocks
            CoordList << it.key();
            TypeList << it.value();
        }
        it++;
    }

    QTime time = QTime::currentTime();
    qsrand(time.msec() + time.second() * 1000);     //use current millisec as seed
    for(int i=0; i<CoordList.size(); i++){
        int index = qrand() % TypeList.size();      //random index, disorganize the map
        map[CoordList[i]] = TypeList[index];
        TypeList.removeAt(index);
    }
}
//if the block at coord has been eliminated
bool Logic::isEmpty(Coord coord){
    if(legal(coord))
        return map[coord]==0;
    else
        return true;
}
//if all blocks have been eliminated
bool Logic::allClear(){
    return Remain==0;
}
//set blank, use type=0 to represent
void Logic::eliminate(Coord coord1, Coord coord2){
    map[coord1]=0;
    map[coord2]=0;
}
//to be used by other class, return route of the linkage if the two blocks can be linked
QList<Coord> Logic::link(Coord coord1, Coord coord2){
    QList<Coord> Route = linkable(coord1, coord2);
    if(Route.size() != 0){
        eliminate(coord1, coord2);  //eliminate the two blocks
        move(coord1, coord2);       //move blocks according to level
        Remain -= 2;                //decrease remaining block count
    }
    return Route;
}
//swap the types of two blocks, to be used by move function
void Logic::swapType(Coord coord1, Coord coord2){
    int temp = map[coord1];
    map[coord1] = map[coord2];
    map[coord2] = temp;
}
//if the block is within the map
bool Logic::legal(Coord coord){
    return ((coord.Row>=0) && (coord.Row<RowNum) &&
            (coord.Col>=0) && (coord.Col<ColNum));
}
//moving blocks according to the level
void Logic::move(Coord coord1, Coord coord2){
    switch(Level){
    case 2: case 7:{    //level 2&7, move upward
        //if the blocks are on the same column, move the lower blank block
        //to the bottom first, so that the other block won't change position
        Coord coord = MAX(coord1, coord2);
        for(int r = coord.Row; r < RowNum-1; r++){
            Coord coord3(r, coord.Col), coord4(r+1, coord.Col);
            swapType(coord3, coord4);
        }
        coord = MIN(coord1, coord2);
        for(int r = coord.Row; r < RowNum-1; r++){
            Coord coord3(r, coord.Col), coord4(r+1, coord.Col);
            swapType(coord3, coord4);
        }
        break;
    }
    case 3: case 8:{    //level 3&8, move leftward
        //if the blocks are on the same row, move the block on the right first
        Coord coord = MAX(coord1, coord2);
        for(int c = coord.Col; c <ColNum-1; c++){
            Coord coord1(coord.Row, c), coord2(coord.Row, c+1);
            swapType(coord1, coord2);
        }
        coord = MIN(coord1, coord2);
        for(int c = coord.Col; c <ColNum-1; c++){
            Coord coord1(coord.Row, c), coord2(coord.Row, c+1);
            swapType(coord1, coord2);
        }
        break;
    }
    case 4: case 9:{    //level 4&9, move all blocks to apart to top and bottom
        Coord coord;
        int r1 = coord1.Row, r2 = coord2.Row;
        if(r1 < RowNum/2){
            if(r2 < RowNum/2){
                //the two blank blocks are both in the upper half region,
                //move the one below first, to the middle
                coord = MAX(coord1, coord2);
                for(int r=coord.Row; r < RowNum/2-1; r++){
                    Coord coord3(r, coord.Col), coord4(r+1, coord.Col);
                    swapType(coord3, coord4);
                }
                coord = MIN(coord1, coord2);
                for(int r=coord.Row; r < RowNum/2-1; r++){
                    Coord coord3(r, coord.Col), coord4(r+1, coord.Col);
                    swapType(coord3, coord4);
                }
            }
            else{   //one up, one down, no risk of interfering with each other
                for(int r=r1; r < RowNum/2-1; r++){
                    Coord coord3(r, coord1.Col), coord4(r+1, coord1.Col);
                    swapType(coord3, coord4);
                }
                for(int r=r2; r > RowNum/2; r--){
                    Coord coord3(r, coord2.Col), coord4(r-1, coord2.Col);
                    swapType(coord3, coord4);
                }
            }
        }
        else{
            if(r2 < RowNum/2){  //one up, one down
                for(int r=r1; r > RowNum/2; r--){
                    Coord coord3(r, coord1.Col), coord4(r-1, coord1.Col);
                    swapType(coord3, coord4);
                }
                for(int r=r2; r < RowNum/2-1; r++){
                    Coord coord3(r, coord2.Col), coord4(r+1, coord2.Col);
                    swapType(coord3, coord4);
                }
            }
            else{
                //the two blank blocks are both in the lower region,
                //move the upper one first
                coord = MIN(coord1, coord2);
                for(int r=coord.Row; r > RowNum/2; r--){
                    Coord coord3(r, coord.Col), coord4(r-1, coord.Col);
                    swapType(coord3, coord4);
                }
                coord = MAX(coord1, coord2);
                for(int r=coord.Row; r > RowNum/2; r--){
                    Coord coord3(r, coord.Col), coord4(r-1, coord.Col);
                    swapType(coord3, coord4);
                }
            }
        }
        break;
    }
    case 5: case 10:{    //level 5&10, move all blocks to the middle column
        Coord coord;
        int c1 = coord1.Col, c2 = coord2.Col;
        if(c1 < ColNum/2){
            if(c2 < ColNum/2){
                //both blocks are in the left part, move the one on the left first
                coord = MIN(coord1, coord2);
                for(int c=coord.Col; c>0; c--){
                    Coord coord3(coord.Row, c), coord4(coord.Row, c-1);
                    swapType(coord3, coord4);
                }
                coord = MAX(coord1, coord2);
                for(int c=coord.Col; c>0; c--){
                    Coord coord3(coord.Row, c), coord4(coord.Row, c-1);
                    swapType(coord3, coord4);
                }
            }
            else{   //one left, one right, no risk
                for(int c=c1; c>0; c--){
                    Coord coord3(coord1.Row, c), coord4(coord1.Row, c-1);
                    swapType(coord3, coord4);
                }
                for(int c=c2; c < ColNum-1; c++){
                    Coord coord3(coord2.Row, c), coord4(coord2.Row, c+1);
                    swapType(coord3, coord4);
                }
            }
        }
        else{
            if(c2 < ColNum/2){  //one left, one right, no risk
                for(int c=c1; c < ColNum-1; c++){
                    Coord coord3(coord1.Row, c), coord4(coord1.Row, c+1);
                    swapType(coord3, coord4);
                }
                for(int c=c2; c>0; c--){
                    Coord coord3(coord2.Row, c), coord4(coord2.Row, c-1);
                    swapType(coord3, coord4);
                }
            }
            else{
                //both blocks are on the right side, move the right one first
                coord = MAX(coord1, coord2);
                for(int c=coord.Col; c < ColNum-1; c++){
                    Coord coord3(coord.Row, c), coord4(coord.Row, c+1);
                    swapType(coord3, coord4);
                }
                coord = MIN(coord1, coord2);
                for(int c=coord.Col; c < ColNum-1; c++){
                    Coord coord3(coord.Row, c), coord4(coord.Row, c+1);
                    swapType(coord3, coord4);
                }
            }
        }
        break;
    }
    default:{
        break;
    }
    }

}
//set game level
void Logic::setLevel(int lv){
    if(lv==0)       //finished a level, add level
        Level++;
    else
        Level=lv;   //player-chosen level
}
//directly linked two blocks, return a list of coords of start and end
QList<Coord> Logic::noTurnLink(Coord coord1, Coord coord2){
    QList<Coord> Route;
    if((coord1.Row != coord2.Row)&&(coord1.Col != coord2.Col))
        return Route;
    else{
        if(coord1.Row == coord2.Row){   //same row
            int r=coord1.Row;
            int c=coord1.Col;
            (c<coord2.Col)? c++:c--;
            while(c!=coord2.Col){
                Coord coord(r, c);
                if(!isEmpty(coord))     //no block between two blocks
                    return Route;
                (c<coord2.Col)? c++:c--;
            }
        }
        else{                           //same column
            int c=coord1.Col;
            int r=coord1.Row;
            (r<coord2.Row)? r++:r--;
            while(r!=coord2.Row){
                Coord coord(r, c);
                if(!isEmpty(coord))
                    return Route;
                (r<coord2.Row)? r++:r--;
            }
        }
        Route << coord1 << coord2;
        return Route;            //reach the second block, linkable
    }
}
//linked with one turn, return a list of coords from start to end with turns
QList<Coord> Logic::oneTurnLink(Coord coord1, Coord coord2){
    //turn1 on the same row with block1, same column with block2
    //turn2 on the same column with block2, same row with block1
    QList<Coord> Route;
    Coord turn1(coord1.Row, coord2.Col),
             turn2(coord2.Row, coord1.Col);
    if(isEmpty(turn1) &&
       (noTurnLink(coord1, turn1).size()!=0) &&
       (noTurnLink(coord2, turn1).size()!=0))
        Route << coord1 << turn1 << coord2;     //linked via turn1
    else if(isEmpty(turn2) &&
            (noTurnLink(coord1, turn2).size()!=0 &&
            (noTurnLink(coord2, turn2).size()!=0)))
        Route << coord1 << turn2 << coord2;     //linked via turn2
    return Route;
}
//linked with two turns, return a list of coords from start to end with turns
QList<Coord> Logic::twoTurnLink(Coord coord1, Coord coord2){
    QList<Coord> Route;
    int r1=coord1.Row, c1=coord1.Col, r2=coord2.Row, c2=coord2.Col;
    if(r1!=0){                  //not on the top of the gameboard
        int r=r1-1;
        Coord turn1(r, c1);
        Coord turn2(r, c2);
        for(; r>0; r--){
            turn1.Row = r;
            turn2.Row = r;

            if(!isEmpty(turn1)) //cannot link this way
                break;

            if(r==r2)           //oneTurnLink, have tested
                continue;

            if(isEmpty(turn2) &&
               (noTurnLink(turn1, turn2).size()!=0) &&
               (noTurnLink(turn2, coord2).size()!=0)){      //find the route
                Route << coord1 << turn1 << turn2 << coord2;
                return Route;
            }
        }
        if(r==0){               //reach topmost row with no obstacles
            turn1.Row = r;
            turn2.Row = r;
            if(isEmpty(turn1)){
                if((r2==0) ||
                   (isEmpty(turn2) &&
                    (noTurnLink(turn2, coord2).size()!=0))){
                    Route << coord1 << Coord(-1, c1) << Coord(-1, c2) << coord2;    //use -1 to represent outside the gameboard
                   return Route;
                }
            }
        }
    }
    else{                       //block 1 is on the topmost row
        Coord turn2(r1, c2);
        if((r2==0) ||
           (isEmpty(turn2) &&
            (noTurnLink(turn2, coord2).size()!=0))){
            Route << coord1 << Coord(-1, c1) << Coord(-1, c2) << coord2;
            return Route;
        }
    }
    if(r1!=RowNum-1){           //not at the bottom of the gameboard
        int r=r1+1;
        Coord turn1(r, c1);
        Coord turn2(r, c2);
        for(; r<RowNum-1; r++){
            turn1.Row = r;
            turn2.Row = r;

            if(!isEmpty(turn1)) //cannot link this way
                break;

            if(r==r2)           //oneTurnLink, have tested
                continue;

            if(isEmpty(turn2) &&
               (noTurnLink(turn1, turn2).size()!=0) &&
               (noTurnLink(turn2, coord2).size()!=0)){   //find the route
                Route << coord1 << turn1 << turn2 << coord2;
                return Route;
            }
        }
        if(r==RowNum-1){        //reach the bottom with no obstacle
            turn1.Row = r;
            turn2.Row = r;
            if(isEmpty(turn1)){
               if((r2==RowNum-1) ||
                  (isEmpty(turn2) &&
                   (noTurnLink(turn2, coord2).size()!=0))){
                   Route << coord1 << Coord(RowNum, c1) << Coord(RowNum, c2) << coord2; //use RowNum to represent outside the gameboard
                   return Route;
               }
            }
        }
    }
    else{                       //block1 is at the bottom
        Coord turn2(r1, c2);
        if((r2==RowNum-1) ||
           (isEmpty(turn2) &&
            (noTurnLink(turn2, coord2).size()!=0))){
            Route << coord1 << Coord(RowNum, c1) << Coord(RowNum, c2) << coord2;
            return Route;
        }
    }

    if(c1!=0){                  //not on the leftmost of the gameboard
        int c=c1-1;
        Coord turn1(r1, c);
        Coord turn2(r2, c);
        for(; c>0; c--){
            turn1.Col = c;
            turn2.Col = c;

            if(!isEmpty(turn1)) //cannot go this way
                break;

            if(c==c2)           //oneTurnLink, have tested
                continue;

            if(isEmpty(turn2) &&
               (noTurnLink(turn1, turn2).size()!=0) &&
               (noTurnLink(turn2, coord2).size()!=0)){   //find the route
                Route << coord1 << turn1 << turn2 << coord2;
                return Route;
            }
        }
        if(c==0){               //reach leftmost column with no obstacles
            turn1.Col = c;
            turn2.Col = c;
            if(isEmpty(turn1)){
                if((c2==0) ||
                   (isEmpty(turn2) &&
                    (noTurnLink(turn2, coord2).size()!=0))){
                    Route << coord1 << Coord(r1, -1) << Coord(r2, -1) << coord2;    //use -1 to represent outside the gameboard
                    return Route;
                }
            }
        }
    }
    else{                       //block1 is on the leftmost column
        Coord turn2(r2, c1);
        if((c2==0) ||
           (isEmpty(turn2) &&
            (noTurnLink(turn2, coord2).size()!=0))){
            Route << coord1 << Coord(r1, -1) << Coord(r2, -1) << coord2;
            return Route;
        }
    }

    if(c1!=ColNum-1){           //not on the rightmost of the gameboard
        int c=c1+1;
        Coord turn1(r1, c);
        Coord turn2(r2, c);
        for(; c<ColNum-1; c++){
            turn1.Col = c;
            turn2.Col = c;

            if(!isEmpty(turn1)) //cannot go this way
                break;

            if(c==c2)           //oneTurnLink, have tested
                continue;

            if(isEmpty(turn2) &&
               (noTurnLink(turn1, turn2).size()!=0) &&
               (noTurnLink(turn2, coord2).size()!=0)){   //find the route
                Route << coord1 << turn1 << turn2 << coord2;
                return Route;
            }
        }
        if(c==ColNum-1){        //reach the rightmost with no obstacles
            turn1.Col = c;
            turn2.Col = c;
            if(isEmpty(turn1)){
                if((c2==ColNum-1) ||
                   (isEmpty(turn2) &&
                    (noTurnLink(turn2, coord2).size()!=0))){
                    Route << coord1 << Coord(r1, ColNum) << Coord(r2, ColNum) << coord2;    //use ColNum to represent outside the gameboard
                    return Route;
                }
            }
        }
    }
    else{                       //block1 is on the rightmost column
        Coord turn2(r2, c1);
        if((c2==ColNum-1) ||
           (isEmpty(turn2) &&
            (noTurnLink(turn2, coord2).size()!=0))){
            Route << coord1 << Coord(r1, ColNum) << Coord(r2, ColNum) << coord2;
            return Route;
        }
    }
    return Route;
}
//if the two coords are linkable, return a list from start to end with turns,
//else return an empty list
QList<Coord> Logic::linkable(Coord coord1, Coord coord2){
    QList<Coord> Route;
    if(coord1==coord2)          //duplicated block
        return Route;
    if(map[coord1]!=map[coord2])//not the same type, cannot match
        return Route;

    Route = noTurnLink(coord1, coord2);
    if(Route.size()!=0)
        return Route;
    Route = oneTurnLink(coord1, coord2);
    if(Route.size()!=0)
        return Route;

    return twoTurnLink(coord1, coord2);
}
//autofind linkable blocks, return a list of the pair
//simply traverse all blocks
QList<Coord> Logic::search(){
    QList<Coord> coords;
    for(int i=0; i<RowNum; i++){
        for(int j=0; j<ColNum; j++){
            if(isEmpty(Coord(i, j)))
                continue;
            for(int k=i; k<RowNum; k++){
                int l;
                l = (k==i)? j+1:0;
                for(; l<ColNum; l++){
                    if(isEmpty(Coord(k, l)))
                        continue;
                    if(linkable(Coord(i, j), Coord(k, l)).size()!=0)
                        coords << Coord(i, j) << Coord(k, l);
                }
            }
        }
    }
    return coords;
}
//save map, remaining blocks number and present level to a list and return
//to be used by Save/Load or PlayBack
QList<QString> Logic::saveData(){
    QList<QString> data;
    data.push_back("[Game Data]");
    QString line, temp;
    for(QMap<Coord, int>::const_iterator it=map.begin(); it!=map.end(); it++){
        temp.sprintf("%d ", it.value());
        line.append(temp);
    }
    data.push_back(line);               //save all types in a line
    line.sprintf("Remain= %d", Remain);
    data.push_back(line);
    line.sprintf("Level= %d", Level);
    data.push_back(line);
    return data;
}
//load Data from a list
void Logic::loadData(QList<QString> data){
    while(!(data[0][0].isDigit()))
        data.pop_front();
    stringstream ss;
    ss << data[0].toStdString();
    for(int i=0; i<RowNum; i++){
        for(int j=0; j<ColNum; j++){
            ss >> map[Coord(i, j)];
        }
    }
    string temp;
    ss << data[1].toStdString();
    ss >> temp >> Remain;           //ignore string information
    ss.clear();
    ss << data[2].toStdString();
    ss >> temp >> Level;
}
