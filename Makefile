all: game

game: game.cpp player.cpp sprite.cpp zombie.cpp collision.cpp bullet.cpp log.cpp timers.cpp
	g++ game.cpp player.cpp sprite.cpp zombie.cpp collision.cpp bullet.cpp log.cpp timers.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -o game

clean:
	rm -f game
	rm -f *.o
