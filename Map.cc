// Map.cc

#include "Map.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>

Map::Map()
{
	read_from_file(stdin);
}

int Map::width() const
{
	return map_width;
}

int Map::height()  const
{
	return map_height;
}

bool Map::is_wall(int x, int y) const
{
	if (x < 0 || y < 0 || x >= map_width || y >= map_height)
		return true;
	else
		return map[x][y];
}

int Map::my_x() const
{
	return player_one_x;
}

int Map::my_y() const
{
	return player_one_y;
}

int Map::opponent_x() const
{
	return player_two_x;
}

int Map::opponent_y() const
{
	return player_two_y;
}

void Map::make_move(int move)
{
	fprintf(stdout, "%d\n", move);
	fflush(stdout);
}

void Map::read_from_file(FILE *file_handle)
{
	int x, y, c;
	int num_items = fscanf(file_handle, "%d %d\n", &map_width, &map_height);
	if (feof(file_handle) || num_items < 2)
		exit(0);		// End of stream means end of game. Just exit.
	memset(map, 0, sizeof(map));
	x = 0;
	y = 0;
	while (y < map_height && (c = fgetc(file_handle)) != EOF)
	{
		switch (c)
		{
			case '\r':
				break;
			case '\n':
				if (x != map_width)
				{
					fprintf(stderr, "x != width in Board_ReadFromStream\n");
					return;
				}
				++y;
				x = 0;
				break;
			case '#':
				if (x >= map_width)
				{
					fprintf(stderr, "x >= width in Board_ReadFromStream\n");
					return;
				}
				map[x][y] = true;
				++x;
				break;
			case ' ':
				if (x >= map_width)
				{
					fprintf(stderr, "x >= width in Board_ReadFromStream\n");
					return;
				}
				map[x][y] = false;
				++x;
				break;
			case '1':
				if (x >= map_width)
				{
					fprintf(stderr, "x >= width in Board_ReadFromStream\n");
					return;
				}
				map[x][y] = false;
				player_one_x = x;
				player_one_y = y;
				++x;
				break;
			case '2':
				if (x >= map_width)
				{
					fprintf(stderr, "x >= width in Board_ReadFromStream\n");
					return;
				}
				map[x][y] = false;
				player_two_x = x;
				player_two_y = y;
				++x;
				break;
			default:
				fprintf(stderr, "unexpected character %d in Board_ReadFromStream", c);
				return;
		}
	}
}
