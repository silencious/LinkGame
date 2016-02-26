#include "linkgamegui.h"
#include "ui_linkgamegui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>

#define WORDSIZE    QSize(100, 40)
#define PICSIZE     QSize(50, 50)

#define UIWIDTH     800                 //width of window
#define UIHEIGHT    600                 //height of window
#define BDWIDTH     650                 //width of gameboard
#define BDHEIGHT    450                 //height of gameboard
#define UNITTIME    100                 //update per 100 millisec

#define CONNECTACTION(action, slot) connect(ui->action, SIGNAL(triggered()), this, SLOT(slot))

using namespace std;

LinkGameGUI::LinkGameGUI(QWidget *parent)
    :QMainWindow(parent),
     ui(new Ui::LinkGameGUI),
     UiSize(QSize(UIWIDTH, UIHEIGHT)),    //default window size
     MaxTime(60*1000),  //60 seconds totally
     Score(0),
     ShuffleNum(1),
     HintNum(2),
     Pause(true)        //show the shade at first
{
    ui->setupUi(this);

    init();
}

LinkGameGUI::~LinkGameGUI()
{
    delete ui;
    delete Music;
    delete Timer;
    delete Projector;
}

void LinkGameGUI::init(){
    //initialize background
    BackgroundNum = 1;
    setBackground();
    //initialize widgets in the mainwindow
    ui->Level->setStyleSheet("border-image:url(:/pic/level.png)");
    ui->LevelNum->setNum(GameBoard.getLevel());
    ui->LevelNum->setStyleSheet("font-size:40px; color:#FF8000; font-family:\"Times\"");

    ui->Score->setStyleSheet("border-image:url(:/pic/score.png)");
    ui->ScoreNum->setNum(Score);
    ui->ScoreNum->setStyleSheet("font-size:28px; color:#800040; font-family:\"Times\"");

    ui->Shuffle->setStyleSheet("border-image:url(:/pic/shuffle.png)");
    ui->ShuffleNum->setStyleSheet("font-size:28px; color:#8080FF; font-family:\"Times\"");
    updateShuffle();
    connect(ui->Shuffle, SIGNAL(clicked()), this, SLOT(shuffle()));

    ui->Hint->setStyleSheet("border-image:url(:/pic/hint.png)");
    ui->HintNum->setStyleSheet("font-size:28px; color:#8080FF; font-family:\"Times\"");
    updateHint();
    connect(ui->Hint, SIGNAL(clicked()), this, SLOT(hint()));

    ui->actionBackground1->setChecked(true);
    ui->actionIcon1->setChecked(true);

    //initialize music
    Music = new SoundPlayer;
    ui->actionMusic1->setChecked(true);
    ui->actionMute->setChecked(true);

    //initialize ui view, to store and show gameboard
    GameBoard.setSceneRect(0, 0, BDWIDTH, BDHEIGHT);
    ui->View->setScene(&GameBoard);
    ui->View->setFixedSize(BDWIDTH, BDHEIGHT);
    ui->View->setStyleSheet("background: transparent; border:Opx");

    //initialize action in the menubar and slots
    CONNECTACTION(actionNew_Game, newGame());
    CONNECTACTION(actionSave, save());
    CONNECTACTION(actionLoad, load());
    CONNECTACTION(actionExit, exit());
    CONNECTACTION(actionPause, pause());
    CONNECTACTION(actionShuffle, shuffle());
    CONNECTACTION(actionHint, hint());
    CONNECTACTION(actionLevel1, level());
    CONNECTACTION(actionLevel2, level());
    CONNECTACTION(actionLevel3, level());
    CONNECTACTION(actionLevel4, level());
    CONNECTACTION(actionLevel5, level());
    CONNECTACTION(actionBackground1, background());
    CONNECTACTION(actionBackground2, background());
    CONNECTACTION(actionBackground3, background());
    CONNECTACTION(actionIcon1, icons());
    CONNECTACTION(actionIcon2, icons());
    CONNECTACTION(actionIcon3, icons());
    CONNECTACTION(actionMute, music());
    CONNECTACTION(actionMusic1, music());
    CONNECTACTION(actionMusic2, music());
    CONNECTACTION(actionMusic3, music());
    CONNECTACTION(actionRank, rank());
    CONNECTACTION(actionInstruction, instruction());
    CONNECTACTION(actionInformation, information());

    //initialize TimeBar and Timer
    ui->TimeBar->setMaximum(MaxTime);
    ui->TimeBar->setValue(MaxTime);
    setBarColor();
    Timer = new QTimer(this);
    connect(Timer, SIGNAL(timeout()), this, SLOT(elapse()));
    //initialize Shade when pause
    QPixmap pic(":/pic/shade.jpg");
    ui->Shade->setPixmap(pic.scaled(UiSize));
    //load ranking
    std::ifstream fin("rank.ini");
    if(fin){
        int temp;
        for(int i=0; i<3; i++){
            fin >> temp;
            Rank << temp;
        }
    }
    else
        Rank << 0 << 0 <<0;
    fin.close();
    //initialize timer of backplaying
    Projector = new QTimer(this);
    connect(Projector, SIGNAL(timeout()), this, SLOT(nextFrame()));
    record(NEWGAME);    //write to file
}
//convert from global position to local position
QPoint LinkGameGUI::GlobalToLocal(QPoint p){
    return QPoint((p.x()-(ui->View->pos().x())),
                  p.y()-(ui->View->pos().y())-(ui->menuBar->height()));
}
//monitor player's click
void LinkGameGUI::mousePressEvent(QMouseEvent *event){
    if(Projector->isActive())   //if it is backplaying, forbid mouseEvent
        return;
    QPoint p = GlobalToLocal(event->pos());
    if(event->button() == Qt::LeftButton){  //only clicking LeftButton is valid
        if(!Pause){ //the player is actually clicking on the gameboard
            Block *b = (Block*)GameBoard.itemAt(p, QTransform());
            if(b==NULL)     //there's no block at the player's clicking position, return
                return;

            if(!GameBoard.isEmpty(b)){          //b is not an eliminated block
                record(event->pos());           //record the pressing event
                if(GameBoard.link(b)){          //b can be linked with previous block
                    Music->play(SoundPlayer::LINK);
                    addTime(1.5);
                    addScore(ui->TimeBar->value()/1000);

                    if(GameBoard.allClear()){           //link all blocks, player win
                        addScore(ui->TimeBar->value()/50);
                        if(GameBoard.getLevel()!=10)
                            win();
                        else
                            complete();                 //complete all 10 levels
                    }
                    else{
                        if(GameBoard.stalemate()){      //no linkable pairs, auto shuffle
                            if(ShuffleNum>0)
                                shuffle();
                            else                        //no left chance of shuffle, player lose
                                gameOver();
                        }
                    }
                }
                else{   //not linkable
                    Music->play(SoundPlayer::CLICK);
                }
            }
        }
        else{       //clicking on pausing picture
            if((p.x()>260) && (p.x()<390) && (p.y()>260) && (p.y()<340)){
                pause();        //return to game
            }
            else if((p.x()>440) && (p.x()<550) && (p.y()>260) && (p.y()<340)){
                load();         //load game
            }
            else if((p.x()>260) && (p.x()<390) && (p.y()>360) && (p.y()<430)){
                information();  //show information
                instruction();
            }
            else if((p.x()>440) && (p.x()<550) && (p.y()>360) && (p.y()<430)){
                exit();         //exit game
            }
        }
    }
}
//start a new game according to level and if the player has won
void LinkGameGUI::newGame(int lv){    //start new game
    GameBoard.newGame(lv);

    if(lv==0){          //the player won
        ShuffleNum++;
        HintNum+=2;
    }
    else{               //player start a new game
        addScore(-1);
        ShuffleNum=1;
        HintNum=2;
    }

    //if player stop backplaying before it's finished
    Projector->stop();
    //belows were disabled when backplaying(if there was)
    ui->menuBar->setEnabled(true);
    ui->actionPause->setEnabled(true);
    ui->actionShuffle->setEnabled(true);
    ui->Shuffle->setEnabled(true);
    ui->actionHint->setEnabled(true);
    ui->Hint->setEnabled(true);

    updateShuffle();
    updateHint();

    resetTime();
    Timer->start(UNITTIME);

    ui->LevelNum->setNum(GameBoard.getLevel()); //update level
    if(Pause)           //hide shade
        pause();
    record(NEWGAME);    //record the start of a new game
}
//save data of present situation
void LinkGameGUI::save(){
    if(!Pause)      //temporarily stop from the game
        pause();
    //some data about GUI
    ofstream fout("game.sav");
    fout << "[GUI]" << endl
         << "BackgroundNum= " << BackgroundNum << endl
         << "TimeValue= " << ui->TimeBar->value() << endl
         << "Score= " << Score << endl
         << "ShuffleNum= " << ShuffleNum << endl
         << "HintNum= " << HintNum << endl;
    QList<QString> data = GameBoard.saveData();     //data about game logic
    for(int i=0; i<data.size(); i++)
        fout << data[i].toStdString() << endl;

    //if the player want to play back the game after he load a game, these are useful
    fout << "[Record]" << endl;
    ifstream fin("rec.log");
    string temp;
    while(getline(fin, temp))
        fout << temp << endl;
    fout.close();
    fin.close();

    QMessageBox box;
    box.setText("Save Successfully.");
    box.addButton(QMessageBox::Ok);
    box.setVisible(true);
    box.exec();
    pause();    //quit pausing
}
//load from saved data
void LinkGameGUI::load(){
    ifstream fin("game.sav");
    if(!fin)        //cannot find load file
        return;
    int time;       //store time value
    string temp;    //ignore useless information
    getline(fin, temp);
    fin >> temp >> BackgroundNum
        >> temp >> time
        >> temp >> Score
        >> temp >> ShuffleNum
        >> temp >> HintNum;
    QList<QString> data;
    while(getline(fin, temp)){
        if(temp[1] == 'R')
            break;
        data.push_back(QString::fromStdString(temp));
    }
    GameBoard.loadData(data);   //pass to gameboard to use

    ofstream fout("rec.log", ios::out | ios::trunc);
    while(getline(fin, temp)){
        fout << temp << endl;
    }
    fin.close();
    fout.close();

    //set all parameter's and GUI to the state of savefile
    setBackground();
    ui->TimeBar->setValue(time);
    ui->LevelNum->setNum(GameBoard.getLevel());
    ui->ScoreNum->setNum(Score);
    updateShuffle();
    updateHint();

    if(Pause)
        pause();    //quit pausing
}
//exit game
void LinkGameGUI::exit(){
    ofstream fout("rank.ini");
    for(int i=0; i<3; i++)
        fout << Rank[i] << endl;    //write ranklist
    fout.close();
    close();
}
//pause, show a Shade so that player can't see the gameboard when pausing
void LinkGameGUI::pause(){
    if(!Pause){         //not paused, set pause
        Pause = true;
        ui->actionPause->setChecked(true);
        Timer -> stop();
        Music -> stop();
        ui->Shade->show();
    }
    else{               //paused, set not pause
        Pause = false;
        ui->actionPause->setChecked(false);
        Timer -> start(UNITTIME);
        Music -> play();
        ui->Shade->hide();
    }
}
//shuffle, randomly reorganize the blocks in the board
void LinkGameGUI::shuffle(){
    if(ShuffleNum>0){
        GameBoard.shuffle();
        ShuffleNum--;
        updateShuffle();
        if(!Projector->isActive())  //not playing back
            record(SHUFFLE);
    }
}
//hint, find linkable pair and select/eliminate
void LinkGameGUI::hint(){
    if(Pause)
        return;
    if(HintNum>0){
        GameBoard.hint();
        HintNum--;
        updateHint();
        if(!Projector->isActive())  //not playing back
            record(HINT);
        if(GameBoard.allClear()){           //link all blocks, player win
            if(GameBoard.getLevel()!=10)
                win();
            else
                complete();
        }
    }
}
//player select level, up to 5
void LinkGameGUI::level(){
    QAction* action = (QAction*)sender();

    if(action == ui->actionLevel1){
        newGame(1);
    }
    else if(action == ui->actionLevel2){
        newGame(2);
    }
    else if(action == ui->actionLevel3){
        newGame(3);
    }
    else if(action == ui->actionLevel4){
        newGame(4);
    }
    else if(action == ui->actionLevel5){
        newGame(5);
    }
}
//set background picture
void LinkGameGUI::setBackground(){
    QString FileName;
    FileName.sprintf(":pic/bg%d.jpg", BackgroundNum);
    Background.load(FileName);
    ui->Background->setPixmap(Background.scaled(UiSize));
}
//switch background according to player's choice
void LinkGameGUI::background(){
    QAction* action = (QAction*)sender();

    if(action == ui->actionBackground1){
        ui->actionBackground1->setChecked(true);
        if(BackgroundNum != 1){
            ui->actionBackground2->setChecked(false);
            ui->actionBackground3->setChecked(false);
            BackgroundNum = 1;
            setBackground();
        }
    }
    else if(action == ui->actionBackground2){
        ui->actionBackground2->setChecked(true);
        if(BackgroundNum != 2){
            ui->actionBackground1->setChecked(false);
            ui->actionBackground3->setChecked(false);
            BackgroundNum = 2;
            setBackground();
        }
    }
    else if(action == ui->actionBackground3){
        ui->actionBackground3->setChecked(true);
        if(BackgroundNum != 3){
            ui->actionBackground1->setChecked(false);
            ui->actionBackground2->setChecked(false);
            BackgroundNum = 3;
            setBackground();
        }
    }
}
//switch icons according to player's choice
void LinkGameGUI::icons(){
    QAction* action = (QAction*)sender();

    if(action == ui->actionIcon1){
        ui->actionIcon1->setChecked(true);
        if(GameBoard.getIcon() != 1){
            ui->actionIcon2->setChecked(false);
            ui->actionIcon3->setChecked(false);
            GameBoard.changeIcon(1);
        }
    }
    else if(action == ui->actionIcon2){
        ui->actionIcon2->setChecked(true);
        if(GameBoard.getIcon() != 2){
            ui->actionIcon1->setChecked(false);
            ui->actionIcon3->setChecked(false);
            GameBoard.changeIcon(2);
        }
    }
    else if(action == ui->actionIcon3){
        ui->actionIcon3->setChecked(true);
        if(GameBoard.getIcon() != 3){
            ui->actionIcon1->setChecked(false);
            ui->actionIcon2->setChecked(false);
            GameBoard.changeIcon(3);
        }
    }
}
//switch music according to player's choice
void LinkGameGUI::music(){
    QAction* action = (QAction*)sender();

    if(action == ui->actionMute)
        Music->play(SoundPlayer::MUTE);
    else if(action == ui->actionMusic1){
        ui->actionMusic1->setChecked(true);
        ui->actionMusic2->setChecked(false);
        ui->actionMusic3->setChecked(false);
        Music->play(SoundPlayer::BGM1);
    }
    else if(action == ui->actionMusic2){
        ui->actionMusic1->setChecked(false);
        ui->actionMusic2->setChecked(true);
        ui->actionMusic3->setChecked(false);
        Music->play(SoundPlayer::BGM2);
    }
    else if(action == ui->actionMusic3){
        ui->actionMusic1->setChecked(false);
        ui->actionMusic2->setChecked(false);
        ui->actionMusic3->setChecked(true);
        Music->play(SoundPlayer::BGM3);
    }
}
//show ranklist
void LinkGameGUI::rank(){
    QMessageBox box;
    box.setText("Rank List");
    QString text;
    text.sprintf("1st:\t%d\n2nd:\t%d\n3rd:\t%d", Rank[0], Rank[1], Rank[2]);
    box.setInformativeText(text);
    QPushButton *Ok = box.addButton(tr("Ok"), QMessageBox::ActionRole);
    QPushButton *Clear = box.addButton(tr("Clear"), QMessageBox::ActionRole);
    box.setVisible(true);
    box.exec();
    if((QPushButton *)box.clickedButton() == Ok)
        return;
    else if((QPushButton *)box.clickedButton() == Clear){
        Rank.clear();
        Rank << 0 << 0 << 0;
        rank();
    }
}
//show instruction
void LinkGameGUI::instruction(){
    if(!Pause)
        pause();
    QMessageBox box(QMessageBox::NoIcon,
                    "Link Game",
                    "Connect same blocks that can be connected\nwith a polyline of no more than two turns.\nEliminate all blocks before time is up.",
                    QMessageBox::Ok);
    box.setVisible(true);
    box.exec();
    pause();
}
//show information
void LinkGameGUI::information(){
    if(!Pause)
        pause();
    QMessageBox box(QMessageBox::NoIcon,
                    "Information",
                    "This game is designed by Kevin Tian.\nAll rights reserved",
                    QMessageBox::Ok);
    box.setVisible(true);
    box.exec();
    pause();
}
//time elapses
void LinkGameGUI::elapse(){
    QProgressBar *Bar = ui->TimeBar;
    int Value = Bar->value() - UNITTIME;    //decrease remaining time
    Bar->setValue(Value);

    if(Value == 0){
        gameOver();     //time up and player hasn't clear the board, player lose
    }
    setBarColor();
}
//add time if player link one pair
void LinkGameGUI::addTime(double sec){
    QProgressBar *Bar = ui->TimeBar;
    int Value = Bar->value();

    if(Value < Bar->maximum())
        Bar->setValue(Value+10*sec*UNITTIME);
    setBarColor();
}
//set time back to max time
void LinkGameGUI::resetTime(){
    QProgressBar *Bar = ui->TimeBar;
    Bar->setMaximum((GameBoard.getLevel()>5)? 90*1000:60*1000);
    Bar->setValue(Bar->maximum());
    setBarColor();
}
//change TimeBar color according to remaining time
void LinkGameGUI::setBarColor(){
    QProgressBar *Bar = ui->TimeBar;
    int Value = Bar->value();
    if(Value <= MaxTime/2){
        QString Sheet;
        int n = 300 * Value / MaxTime;
        Sheet.sprintf("QProgressBar::chunk {background-color: rgb(%d, %d, 0)}", 255-n, n);
        Bar->setStyleSheet(Sheet);
    }
    else{
        Bar->setStyleSheet("QProgressBar::chunk {background-color: rgb(0, 150, 0)}");
    }
}
//add score when player link a pair or clear a board
void LinkGameGUI::addScore(int n){
    if(n>=0)
        Score += n;
    else
        Score = 0;
    ui->ScoreNum->setNum(Score);
}
//update the number showing the remaining shuffle number
void LinkGameGUI::updateShuffle(){
    QString text;
    text.sprintf("x %d", ShuffleNum);
    ui->ShuffleNum->setText(text);

}
//update the number showing the remaining hint number
void LinkGameGUI::updateHint(){
    QString text;
    text.sprintf("x %d", HintNum);
    ui->HintNum->setText(text);
}
//time up, game over, show information
void LinkGameGUI::gameOver(){
    Timer->stop();
    Projector->stop();
    addRank();
    QMessageBox box;
    box.setText("Sorry , You Lose!");
    box.setInformativeText("Would you like to start a new game?");
    QPushButton *Yes = box.addButton(tr("Yes"), QMessageBox::ActionRole);
    QPushButton *PlayBack = box.addButton(tr("Play Back"), QMessageBox::ActionRole);
    QPushButton *No = box.addButton(tr("Exit"), QMessageBox::ActionRole);
    box.setVisible(true);
    box.exec();
    if((QPushButton *)box.clickedButton() == Yes)
        newGame(1);         //start new game from level 1
    else if((QPushButton *)box.clickedButton() == PlayBack)
        playBack();
    else if((QPushButton *)box.clickedButton() == No)
        exit();
}
//all clear, player win, show information
void LinkGameGUI::win(){
    Timer->stop();
    Projector->stop();
    QMessageBox box;
    box.setText("Congratulations! You win!");
    box.setInformativeText("Would you like to enter the next level?");
    QPushButton *Yes = box.addButton(tr("Yes"), QMessageBox::ActionRole);
    QPushButton *PlayBack = box.addButton(tr("Play Back"), QMessageBox::ActionRole);
    QPushButton *No = box.addButton(tr("Exit"), QMessageBox::ActionRole);
    box.setVisible(true);
    box.exec();
    if((QPushButton *)box.clickedButton() == Yes)
        newGame(0);         //start new game from next level
    else if((QPushButton *)box.clickedButton() == PlayBack)
        playBack();
    else if((QPushButton *)box.clickedButton() == No)
        exit();
}
//complete all levels
void LinkGameGUI::complete(){
    Timer->stop();
    Projector->stop();
    addRank();
    QMessageBox box;
    box.setText("Congratulations! You have completed all levels!");
    QString text;
    text.sprintf("Your final score is %d.\nWould you like to start from the beginning?", Score);
    box.setInformativeText(text);
    QPushButton *Yes = box.addButton(tr("Yes"), QMessageBox::ActionRole);
    QPushButton *PlayBack = box.addButton(tr("Play Back"), QMessageBox::ActionRole);
    QPushButton *No = box.addButton(tr("Exit"), QMessageBox::ActionRole);
    box.setVisible(true);
    box.exec();
    if((QPushButton *)box.clickedButton() == Yes)
        newGame(1);         //start new game from level 1
    else if((QPushButton *)box.clickedButton() == PlayBack)
        playBack();
    else if((QPushButton *)box.clickedButton() == No)
        exit();
}
//add the final rank of current player
void LinkGameGUI::addRank(){
    for(int i=0; i<3; i++){
        if(Rank[i] < Score){
            Rank.insert(i, Score);
            Rank.pop_back();
            return;
        }
    }
    Rank << Score;
}
//record player's action
void LinkGameGUI::record(RECORD_TYPE Type){
    ofstream fout;
    if(Type == NEWGAME){    //player starts new game, record the information of the game
        fout.open("rec.log", ios::out | ios::trunc);
        fout << "[GUI]\n"
             << "Score= " << Score << endl
             << "ShuffleNum= " << ShuffleNum << endl
             << "HintNum= " << HintNum << endl;
        QList<QString> data = GameBoard.saveData();
        for(int i=0; i<data.size(); i++)
            fout << data[i].toStdString() << endl;
    }
    else{                   //not new game, need not extra information
        fout.open("rec.log", ios::out | ios::app);
        int time = ui->TimeBar->value();
        if(Type == SHUFFLE){    //player use shuffle
            fout << time << "\tS" << endl;
            QList<QString> data = GameBoard.saveData();
            for(int i=0; i<data.size(); i++)
                fout << data[i].toStdString() << endl;
        }
        else if(Type == HINT)   //player use hint
            fout << time << "\tH" << endl;
    }
    fout.close();
}
//record player's click position(global)
void LinkGameGUI::record(QPoint p){
    ofstream fout("rec.log", ios::out | ios::app);
    fout << ui->TimeBar->value() << '\t' << p.x() << '\t' << p.y() << endl;
    fout.close();
}
//back play the previous game
void LinkGameGUI::playBack(){
    ifstream fin("rec.log");
    if(!fin)
        return;
    string temp;
    fin >> temp
        >> temp >> Score            //set back the basic data
        >> temp >> ShuffleNum
        >> temp >> HintNum;
    getline(fin, temp);
    QList<QString> data;
    for(int i=0; i<4; i++){
        getline(fin, temp);
        data.push_back(QString::fromStdString(temp));
    }
    GameBoard.loadData(data);
    Rec.clear();
    while(getline(fin, temp)){
        Rec.push_back(QString::fromStdString(temp));
    }
    fin.close();

    resetTime();
    ui->LevelNum->setNum(GameBoard.getLevel());
    ui->ScoreNum->setNum(Score);
    updateShuffle();
    updateHint();
    Projector->start(UNITTIME);
    //disable player's operation when back playing
    ui->menuBar->setDisabled(true);
    ui->actionPause->setDisabled(true);
    ui->actionShuffle->setDisabled(true);
    ui->Shuffle->setDisabled(true);
    ui->actionHint->setDisabled(true);
    ui->Hint->setDisabled(true);
}
//catch the projector's signal, imitate player's operation
void LinkGameGUI::nextFrame(){
    QProgressBar *Bar = ui->TimeBar;
    if(!Rec.isEmpty()){
        string line = Rec[0].toStdString();
        stringstream ss;
        int time;
        ss << line;
        ss >> time;
        if(time == Bar->value()){   //the time matches
            Rec.pop_front();

            char ch = ss.get();
            while(isblank(ch))
                ch = ss.get();

            if(isdigit(ch)){        //mouse click, imitate mousePressEvent
                ss.putback(ch);
                int x, y;
                ss >> x >> y;
                QPoint p = GlobalToLocal(QPoint(x, y));
                Block *b = (Block*)GameBoard.itemAt(p, QTransform());
                if(GameBoard.link(b)){          //b is linked with previous block
                    Music->play(SoundPlayer::LINK);
                    addTime(1.5);
                    addScore(ui->TimeBar->value()/1000);

                    if(GameBoard.allClear()){           //link all blocks, player win
                        addScore(ui->TimeBar->value()/50);
                        if(GameBoard.getLevel()!=10)
                            win();
                        else
                        complete();
                    }
                    else{
                        if(GameBoard.stalemate()){
                            if(ShuffleNum>0)
                                shuffle();
                            else
                                gameOver();
                        }
                    }
                }
                else
                    Music->play(SoundPlayer::CLICK);
            }
            else if(ch == 'S'){         //player used shuffle
                QList<QString> data;
                for(int i=0; i<4; i++){
                    data.push_back(Rec[0]);
                    Rec.pop_front();
                }
                GameBoard.loadData(data);
                ShuffleNum --;
                updateShuffle();
            }
            else if(ch == 'H'){         //player used hint
                hint();
            }
            return;
        }
    }
    //time doesn't match, decrease time
    int Value = Bar->value() - UNITTIME;
    Bar->setValue(Value);
    setBarColor();
    if(Value == 0){
        gameOver();
    }
}
