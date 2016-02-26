#ifndef LINKGAMEGUI_H
#define LINKGAMEGUI_H

#include <QMainWindow>
#include <QMouseEvent>

#include "board.h"
#include "soundplayer.h"

namespace Ui {
class LinkGameGUI;
}

class LinkGameGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit LinkGameGUI(QWidget *parent = 0);
    ~LinkGameGUI();

private:
    enum RECORD_TYPE {NEWGAME, SHUFFLE, HINT};  //type of different data to record
    Ui::LinkGameGUI *ui;
    QSize UiSize;               //window size
    Board GameBoard;            //container of blocks

    int BackgroundNum;          //background number
    QPixmap Background;         //picture of background

    SoundPlayer *Music;              //music player

    void init();
    void setBackground();
    QPoint GlobalToLocal(QPoint);   //get position in the view from global position

    int MaxTime;            //maxium value of the time bar
    QTimer *Timer;          //scale time and trigger elapse
    void addTime(double);   //add time when blocks are eliminated
    void resetTime();       //set time to maxium
    void setBarColor();     //set timebar color

    int Score;              //player's score
    int ShuffleNum;         //number of shuffle the player can use
    int HintNum;            //number of hint the player can use
    void addScore(int);     //add to score
    void updateShuffle();   //update number shown on the screen
    void updateHint();

    void gameOver();        //player lose
    void win();             //player win
    void complete();        //player finish all levels
    bool Pause;             //if player press pause

    QList<int> Rank;        //rank list
    void addRank();         //add the present score to rank list

    QTimer *Projector;      //timer to control backplaying
    QList<QString> Rec;     //data to be saved/loaded
    void record(RECORD_TYPE);   //record newgame/shuffle/hint action
    void record(QPoint);        //record mousePressEvent
    void playBack();            //play back the previous game

protected:
    void mousePressEvent(QMouseEvent*);

private slots:
    //slots for the menuBar
    void newGame(int lv=1);
    void save();
    void load();
    void exit();
    void pause();
    void shuffle();
    void hint();
    void level();
    void background();
    void icons();
    void music();
    void rank();
    void instruction();
    void information();

    void elapse();      //control time of a game
    void nextFrame();   //control time of backplaying
};

#endif // LINKGAMEGUI_H
