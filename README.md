# Tetris_SDL_C
Tetris Clone made from scratch using C and SDL in a weekend.

# Build Process
Currently, there is only Windows build.bat file and VS-compatible libraries inluded for building the game. For Linux and MacOS, you will need to download SDL2 and SDL_TTF for your compiler of choice.
Considering you have a Windows PC with Visual Studio Installed, follow the steps below:
1. Locate your vcvars64.bat file. (Usually inside C:\Program Files (x86)\Microsoft Visual Studio\Your_Visual_Studio_Version_Year\Community_Enterprise_or_Professional\VC\Auxiliary\Build\)
2. Copy the path to vcvars64.bat and paste it on the path in the first line of the build.bat file.
3. Run build.bat.
4. Enjoy the game!

# Keybindings
- Up Arrow: Rotate the falling tetromino.
- Down Arrow: Move the falling tetromino one cell below.
- Right and Left Arrow: Move the falling tetromino right and left.

# Screenshots
![image](https://user-images.githubusercontent.com/42971567/113599210-f78ca980-9646-11eb-8e1f-369be476b265.png)
