# LOGIC CORE

**O GRANDE ASSALTO LOGICO**

LOGIC CORE é um jogo de aventura e lógica desenvolvido em C com [raylib](https://www.raylib.com/). O jogador controla BIT, explora três salas e resolve desafios para recuperar a verdade e impedir o apagamento do núcleo lógico.

## Sobre o jogo

A campanha combina exploração, narrativa e minigames. Em cada sala, o jogador deve localizar cofres ou estações de acesso, coletar energia e completar desafios antes que a bateria de BIT se esgote.

### Recursos

- Exploração em três salas
- Sistema de bateria com recarga através de coletáveis
- Sistema de vidas e tela de game over
- Sequências narrativas entre as fases
- Menu inicial e reinício da campanha
- Três tipos de minigame:
  - Cifra
  - Doodle
  - Terminal
- Sprites e cenários carregados a partir da pasta `assets`

## Controles

| Ação | Controle |
|---|---|
| Mover BIT | Teclas direcionais ou `A` / `D` |
| Interagir | `E` |
| Navegar no terminal | Conforme indicado na interface |
| Reiniciar após o fim da partida | Botão `REINICIAR` |

## Requisitos

- GCC compatível com C11
- Raylib
- Bibliotecas usadas pelo Makefile em ambientes Linux:
  - OpenGL
  - pthread
  - dl
  - rt
  - X11

## Compilação

O projeto usa um `Makefile` com os arquivos-fonte de `src/` e gera o executável em `build/logiccore`.

```bash
make
```

Para limpar o executável:

```bash
make clean
```

## Execução

É necessário executar o jogo a partir da raiz do repositório para que os arquivos em `assets/` sejam encontrados corretamente:

```bash
./build/logiccore
```

Uma versão de teste baseada em raylib também está disponível:

```bash
gcc -std=c11 raylib_test.c -o raylib_test \
  -I/usr/local/include -L/usr/local/lib \
  -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

./raylib_test
```

## Estrutura do projeto

```text
logiccore/
├── assets/
│   ├── backgrounds/
│   └── sprites/
├── build/
├── src/
│   ├── core/
│   ├── minigames/
│   │   ├── cipher/
│   │   ├── doodle/
│   │   └── terminal/
│   ├── scenes/
│   └── systems/
├── Makefile
├── raylib_test.c
└── README.md
```

## Desenvolvimento

O loop principal fica em `src/main.c`. A lógica da campanha, transições entre cenas e integração dos minigames estão em `src/core/game.c`.

- `src/scenes/`: menu, salas e cenas narrativas
- `src/systems/`: sistemas do jogador
- `src/minigames/`: desafios de cifra, doodle e terminal
- `assets/`: imagens e recursos visuais

## Licença

Nenhuma licença foi definida para este projeto. Para utilizar, modificar ou redistribuir o código, entre em contato com o mantenedor ou adicione uma licença ao repositório.
