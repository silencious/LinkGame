#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QSound>

class SoundPlayer
{
public:
    SoundPlayer();
    ~SoundPlayer();
    enum SOUND {MUTE, BGM1, BGM2, BGM3, LINK, CLICK};
    void play(SOUND);
    void play();
    void stop();
private:
    SOUND MusicNum;
    bool Mute;
    QSound *Player;
};

#endif // SOUNDPLAYER_H
