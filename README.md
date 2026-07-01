# Shadow Arena

A 2D animated fighting game built with **SFML 2.5+**, featuring four real
hand-drawn pixel-art fighters (sourced as free, CC0-licensed sprite packs —
see Credits below), full animation, movement, local 2-player support, and an
original menu/arena look.

- **Real pixel-art sprite animations** — each fighter has Idle / Punch / Kick /
  Rage / Hurt / KO / Walk animations, built from actual hand-drawn frame sets
  (not procedurally generated stick figures).
- **4 distinct fighters**: Martial Hero, Hero Knight, Evil Wizard, Medieval King
  — each with different stats, sprite scale, and attack damage.
- **Movement** — walk forward/back, can't walk through your opponent, attacks
  only land within real striking range.
- **Local 2-Player mode** — play against a friend on the same keyboard, or
  vs a simple AI in 1-Player mode.
- **Custom backgrounds**: a lightning/starburst menu screen and a sunset
  city-skyline fight arena with spotlights and a crowd.
- A full game flow: **Menu → Character Select → Fight → Win/Lose screen**,
  with health bars and a K.O. screen.

## 1. Install SFML

**Windows (recommended: vcpkg)**
```
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install sfml
```
Then open this folder with CMake (Visual Studio's "Open Folder" supports
CMake projects directly) and pass
`-DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake`.

**macOS**
```
brew install sfml cmake
```

**Linux (Debian/Ubuntu)**
```
sudo apt-get install libsfml-dev cmake g++
```

## 2. Build

```
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

This produces a `ShadowArena` (or `ShadowArena.exe`) executable and
automatically copies the `assets/` folder next to it (see the
`add_custom_command` at the bottom of `CMakeLists.txt`). **Run the
executable from inside that folder** so it can find `assets/...`.

If you'd rather skip CMake, compile directly:
```
g++ -std=c++17 -Isrc src/*.cpp -lsfml-graphics -lsfml-window -lsfml-system -o ShadowArena
```
(then manually copy the `assets/` folder next to the resulting binary).

## 3. Controls

| Scene            | Keys |
|-------------------|------|
| Menu              | `Enter` 1 Player (vs AI), `Tab` 2 Player (vs Friend), `Esc` quit |
| Character Select  | `←`/`→` choose, `Enter` confirm, `Esc` back. In 2-Player mode this screen runs twice — Player 1 picks first, then Player 2. |
| Fight (Player 1)  | `←`/`→` move, `A` punch, `S` kick, `D` rage (only once your HP ≤ 30%) |
| Fight (Player 2, 2P mode only) | `J`/`L` move, `U` punch, `I` kick, `O` rage |
| Win/Lose screen   | `Enter` back to menu, `Esc` quit |

In 1-Player mode the opponent is chosen randomly. The AI closes the distance
if you're too far away to fight, then attacks on a random cooldown once in
range, using Rage as soon as it's available.

## 4. The roster

| Character | HP | Punch | Kick | Rage | Personality |
|---|---|---|---|---|---|
| Martial Hero | 110 | 15 | 20 | 40 | Fast, agile sword fighter |
| Hero Knight | 150 | 20 | 18 | 50 | Heavy armoured tank |
| Evil Wizard | 90 | 18 | 18 | 55 | Glass cannon — low HP, high Rage damage |
| Medieval King | 130 | 16 | 25 | 45 | Balanced all-rounder |

None of these packs ship a dedicated "Rage" animation, so Rage reuses each
character's strongest Attack animation — the damage and unlock condition
(HP ≤ 30%) still work exactly like a real special move.

## 5. Project layout

```
src/
  Animation.hpp        – one row of a sprite sheet = one animation
  AnimatedSprite.*      – steps an sf::Sprite through an Animation's frames,
                          anchored at the character's feet (not frame bottom)
  Character.*           – stats, state machine, hit-frame timing
  Game.*                – scenes (Menu/Select/Fight/Over), HUD, enemy AI,
                          movement & range-gated combat
  main.cpp               – window + game loop
assets/
  characters/*.png      – packed sprite sheets (one PNG per character,
                          7 rows: Idle/Punch/Kick/Rage/Hurt/KO/Walk)
  backgrounds/*.png      – menu_bg.png, fight_bg.png
  fonts/font.ttf
pack_sprites.py           – the script that built assets/characters/*.png
                          from the raw downloaded sprite packs (see below)
CMakeLists.txt
```

## 6. How the sprite sheets were built / how to add more characters

Each character originally ships as several separate strip PNGs (`Idle.png`,
`Attack1.png`, `Run.png`, etc.), one frame-width per pose, all with a fixed
frame size per character but a *different* frame size between characters and
a *different* number of frames per animation.

`pack_sprites.py` reads those raw strips and packs them into one sheet per
character, in the fixed row order the C++ engine expects (Idle, Punch, Kick,
Rage, Hurt, KO, Walk), padding every row to the same column count so the
sheet has one consistent grid. It also prints the exact per-row frame counts
you need to paste into the `roster` array in `Game.cpp`.

To add a 5th character from another free CC0 pack:
1. Download it, note its frame size and which strip file maps to which row
2. Add an entry to the `CHARS` dict at the top of `pack_sprites.py`
3. Run `python3 pack_sprites.py` — it'll add the new sheet to `assets/characters/`
4. Add a matching entry to the `roster` vector in `Game::loadAssets()`
   (`Game.cpp`), including `frameW`/`frameH`/`frameCounts` from the script's
   printed output, and a `displayScale`/`originY` — see the comment at the
   top of that function for how those two are measured (they account for
   transparent padding around the actual pixel art in each frame).

## 7. Credits / licensing

The four character sprite packs are by **LuizMelo** on itch.io, all licensed
**CC0 (Creative Commons Zero)** — free for any use, commercial or otherwise,
no attribution required:
- Martial Hero 2 — https://luizmelo.itch.io/martial-hero-2
- Hero Knight — https://luizmelo.itch.io/hero-knight
- Evil Wizard 2 — https://luizmelo.itch.io/evil-wizard-2
- Medieval King Pack 2 — https://luizmelo.itch.io/medieval-king-pack-2

Backgrounds, fonts, and all code are original to this project.

## 8. Gameplay notes

- A punch/kick/rage animation locks input until it finishes (no spamming),
  and getting hit interrupts whatever you were doing into a Hurt animation.
- Movement: both fighters can walk forward/back within the arena and can't
  walk through each other. Attacks only land within a real striking range —
  walking apart is a valid way to create breathing room.
- In 2-Player mode there's no AI: Player 2 is purely controlled by the
  J/L/U/I/O keys.
- Pixel art is rendered with nearest-neighbour filtering (no blur) so it
  stays crisp even scaled up several times its native size.
