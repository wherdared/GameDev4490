all: game

game: game.cpp player.cpp zombie.cpp collision.cpp bullet.cpp log.cpp timers.cpp title.cpp
	g++ game.cpp player.cpp zombie.cpp collision.cpp bullet.cpp log.cpp timers.cpp title.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -o game

clean:
	rm -f game
	rm -f *.o
