#include "soundplayer.h"

SoundPlayer::SoundPlayer()
    :MusicNum(BGM1), Mute(true)
{
    Player = new QSound(":/sound/bgm1.wav");
    Player -> setLoops(QSound::Infinite);
    play();
}

SoundPlayer::~SoundPlayer(){
    delete Player;
}

void SoundPlayer::play(SOUND sound){
    switch (sound) {
    case MUTE:{
        if(!Mute){
            Mute = true;
            Player->stop();
        }
        else{
            Mute = false;
            Player->play();
        }
        break;
    }
    case BGM1:{
        MusicNum = BGM1;
        delete Player;
        Player = new QSound(":/sound/bgm1.wav");
        Player -> setLoops(QSound::Infinite);
        if(!Mute)
            Player->play();
        break;
        }
    case BGM2:{
        MusicNum = BGM2;
        delete Player;
        Player = new QSound(":/sound/bgm2.wav");
        Player -> setLoops(QSound::Infinite);
        if(!Mute)
            Player->play();
        break;
        }
    case BGM3:{
        MusicNum = BGM3;
        delete Player;
        Player = new QSound(":/sound/bgm3.wav");
        Player -> setLoops(QSound::Infinite);
        if(!Mute)
            Player->play();
        break;
        }
    case LINK:{
        if(!Mute)
            Player->play(":/sound/link.wav");
        break;
    }
    case CLICK:{
        if(!Mute)
            Player->play(":/sound/click.wav");
        break;
    }
    default:
        break;
    }
}

void SoundPlayer::play(){
    if(!Mute)
        Player->play();
}

void SoundPlayer::stop(){
    if(!Mute)
        Player->stop();
}


