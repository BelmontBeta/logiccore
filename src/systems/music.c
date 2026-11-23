#include "music.h"
#include "raylib.h"
#include <stddef.h>

static Music menuMusic;
static Music gameplayMusic;
static Music *currentMusic = NULL;
static float masterVolume = 1.0f;

void InitGameMusic(void)
{
    InitAudioDevice();
    SetMasterVolume(masterVolume);

    menuMusic = LoadMusicStream("assets/audio/menu.ogg");
    gameplayMusic = LoadMusicStream("assets/audio/gameplay.ogg");

    SetMusicVolume(menuMusic, 0.35f);
    SetMusicVolume(gameplayMusic, 0.35f);

    PlayMenuMusic();
}

void UpdateGameMusic(void)
{
    if (currentMusic != NULL)
    {
        UpdateMusicStream(*currentMusic);
    }
}

void PlayMenuMusic(void)
{
    if (currentMusic == &menuMusic)
        return;

    if (currentMusic != NULL)
        StopMusicStream(*currentMusic);

    currentMusic = &menuMusic;
    PlayMusicStream(menuMusic);
}

void PlayGameplayMusic(void)
{
    if (currentMusic == &gameplayMusic)
        return;

    if (currentMusic != NULL)
        StopMusicStream(*currentMusic);

    currentMusic = &gameplayMusic;
    PlayMusicStream(gameplayMusic);
}

void CloseGameMusic(void)
{
    if (currentMusic != NULL)
        StopMusicStream(*currentMusic);

    UnloadMusicStream(menuMusic);
    UnloadMusicStream(gameplayMusic);
    CloseAudioDevice();

    currentMusic = NULL;
}

void SetGameVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    masterVolume = volume;

    // Volume geral: vale para musicas e qualquer som futuro.
    if (IsAudioDeviceReady())
        SetMasterVolume(masterVolume);
}

float GetGameVolume(void)
{
    return masterVolume;
}
