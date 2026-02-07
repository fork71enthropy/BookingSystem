# Émulateur CHIP-8

Un émulateur CHIP-8 complet écrit en C avec SDL2.

## Fonctionnalités

✅ Tous les 35 opcodes CHIP-8 implémentés
✅ Affichage 64x32 pixels avec mise à l'échelle
✅ Clavier 16 touches
✅ Timers de délai et de son
✅ Support complet des sprites
✅ Générateur de nombres aléatoires

## Prérequis

Vous devez avoir SDL2 installé :

### Ubuntu/Debian
```bash
sudo apt-get install libsdl2-dev
```

### macOS
```bash
brew install sdl2
```

### Fedora/RedHat
```bash
sudo dnf install SDL2-devel
```

## Compilation

```bash
make
```

Ou manuellement :
```bash
gcc -Wall -Wextra -O2 chip8_emulator.c -o chip8 -lSDL2
```

## Utilisation

```bash
./chip8 <fichier_rom>
```

Exemple :
```bash
./chip8 pong.ch8
```

## Mapping du clavier

Le CHIP-8 utilise un clavier hexadécimal 4x4 mappé comme suit :

```
CHIP-8 Keyboard:        Votre clavier:
1 2 3 C                 1 2 3 4
4 5 6 D        →        Q W E R
7 8 9 E                 A S D F
A 0 B F                 Z X C V
```

## Où trouver des ROMs

Vous pouvez trouver des ROMs CHIP-8 gratuites ici :
- https://github.com/kripod/chip8-roms
- https://www.zophar.net/pdroms/chip8.html
- https://johnearnest.github.io/chip8Archive/

## Jeux populaires à essayer

- **PONG** - Le classique jeu de tennis
- **TETRIS** - Le célèbre jeu de puzzle
- **SPACE INVADERS** - Tire sur les aliens
- **BREAKOUT** - Casse les briques
- **MAZE** - Génère un labyrinthe aléatoire

## Architecture de l'émulateur

### Mémoire
- 4096 octets de RAM
- 0x000-0x1FF : Interpréteur (contient le fontset)
- 0x200-0xFFF : Espace programme

### Registres
- 16 registres 8-bit (V0-VF)
- VF est utilisé comme flag
- 1 registre d'index 16-bit (I)
- Compteur de programme (PC)
- Pointeur de pile (SP)

### Timers
- Delay timer : décompte à 60Hz
- Sound timer : émet un son quand > 0

### Affichage
- 64x32 pixels monochrome
- Mise à l'échelle 10x pour un affichage 640x320

## Détails techniques

### Cycle d'exécution
L'émulateur exécute ~600 instructions par seconde (10 cycles par frame à 60 FPS).

### Opcodes implémentés
Tous les 35 opcodes CHIP-8 standards sont implémentés :
- 0nnn, 00E0, 00EE (contrôle)
- 1nnn, 2nnn, Bnnn (saut/appel)
- 3xkk, 4xkk, 5xy0, 9xy0 (conditions)
- 6xkk, 7xkk, 8xy0-8xyE (arithmétique)
- Annn, Cxkk (registres)
- Dxyn (affichage)
- Ex9E, ExA1 (clavier)
- Fx07-Fx65 (timers/mémoire)

## Dépannage

### Pas de fenêtre affichée
Vérifiez que SDL2 est correctement installé :
```bash
sdl2-config --version
```

### ROM ne se charge pas
- Vérifiez que le fichier existe
- Vérifiez que c'est bien un fichier CHIP-8 (.ch8)
- Vérifiez que le fichier fait moins de 3584 octets

### Jeu trop rapide/lent
Modifiez la constante dans la boucle principale :
```c
for (int i = 0; i < 10; i++)  // Changez 10 pour ajuster la vitesse
```

## Améliorations possibles

- [ ] Support des variantes SUPER-CHIP
- [ ] Débogueur intégré
- [ ] Sauvegarde d'états
- [ ] Son amélioré avec des fréquences
- [ ] Configuration du clavier personnalisable

## License

Ce projet est du domaine public. Utilisez-le librement !

## Ressources

- [Spécification CHIP-8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Guide CHIP-8](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [SDL2 Documentation](https://wiki.libsdl.org/SDL2/FrontPage)
