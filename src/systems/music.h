#ifndef MUSIC_H
#define MUSIC_H

void InitGameMusic(void);
void UpdateGameMusic(void);
void PlayMenuMusic(void);
void PlayGameplayMusic(void);
void CloseGameMusic(void);

// Volume geral do jogo, de 0.0 a 1.0.
void SetGameVolume(float volume);
float GetGameVolume(void);

#endif
