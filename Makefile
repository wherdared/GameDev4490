all: game

game: game.cpp player.cpp sprite.cpp zombie.cpp rounds.cpp collision.cpp bullet.cpp log.cpp timers.cpp title.cpp
	g++ game.cpp player.cpp sprite.cpp zombie.cpp rounds.cpp collision.cpp bullet.cpp log.cpp timers.cpp title.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -o game
>>>>>>> 6987f49db7e05adecba9137e5cd7004896071ef7

clean:
	rm -f game
	rm -f *.o
