# omitting -g -Wall -Wextra
gcc -framework AudioToolbox -framework CoreAudio main.c -o main.out -lpthread && ./main.out