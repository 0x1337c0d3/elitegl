# EliteGL Mac

EliteGL - Mac port of Mark Follett's EliteGL for macOS

## Build
Install the [SDL2 framework](https://www.libsdl.org/release/SDL2-2.0.22.dmg) into /Library/Frameworks.

Install the [SDL2_mixer framework](https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.1/SDL2_mixer-2.6.1.dmg) into /Library/Frameworks.

Xcode 13.2.1+ open the `elitegl.xcodeproj` build and run.

## Gameplay Differences
The function keys on the Mac are different requiring you to press Fn+F1 etc, therefore function keys are remapped to numbers: F1 -> 1, F2 -> 2 and so on.

## Bugs
1. The font kerning is not correct.
2. No music playback.

## Original

EliteGL - An OpenGL Port of Christian Pinder's "Elite: The New Kind" by Mark Follett, then forked from [davewongillies](https://github.com/davewongillies/elitegl.git) and ported to Mac
