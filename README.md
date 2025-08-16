# MorseGen

MorseGen is a small SDL2-based program that repeatedly sends a fixed Morse code message through the computer's audio output. It demonstrates basic audio generation and event handling with SDL.

## Build

Before building, run the `configure` script to verify that all required
dependencies are installed:

```bash
./configure
```

### Linux

```bash
make
```

### Windows

```bash
make -f Makefile.win
```

## Controls
- Press `Esc` or close the window to stop playback.
- Press `Ctrl+C` in the terminal to exit immediately.

## Roadmap
- Allow configuring the message and tone parameters.
- Support real-time keyboard input for custom messages.
- Provide a simple graphical interface.
