#ifndef OPTIONS_H
#define OPTIONS_H

// Painel de configuracoes (resolucao, tela cheia e volume).
// E usado em dois lugares:
//  - OPTIONS_FROM_MENU:  botao OPCOES do menu inicial.
//  - OPTIONS_FROM_PAUSE: menu de pausa aberto com ESC durante o jogo.
typedef enum {
    OPTIONS_FROM_MENU,
    OPTIONS_FROM_PAUSE
} OptionsContext;

typedef enum {
    OPTIONS_NONE,       // painel continua aberto
    OPTIONS_CLOSE,      // voltar ao menu inicial (contexto menu)
    OPTIONS_RESUME,     // continuar o jogo (contexto pausa)
    OPTIONS_MAIN_MENU   // abandonar a partida e ir ao menu inicial
} OptionsAction;

void Options_Open(OptionsContext context);
OptionsAction Options_Update(void);
void Options_Draw(void);

#endif
