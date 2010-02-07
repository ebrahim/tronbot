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

bool Map::is_wall(int x, int y) const
{
	if (x < 0 || y < 0 || x >= width || y >= height)
		return true;
	else
		return map[x][y];
}

bool Map::is_wall(int move) const
{
	switch (move)
	{
		case NORTH:
			return is_wall(player_one_x, player_one_y - 1);
		case EAST:
			return is_wall(player_one_x + 1, player_one_y);
		case SOUTH:
			return is_wall(player_one_x, player_one_y + 1);
		case WEST:
			return is_wall(player_one_x - 1, player_one_y);
	}
	return true;
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
	int num_items = fscanf(file_handle, "%d %d\n", &width, &height);
	if (feof(file_handle) || num_items < 2)
		exit(0);		// End of stream means end of game. Just exit.
	memset(map, 0, sizeof(map));
	x = 0;
	y = 0;
	while (y < height && (c = fgetc(file_handle)) != EOF)
	{
		switch (c)
		{
			case '\r':
				break;
			case '\n':
				if (x != width)
				{
					fprintf(stderr, "x != width in Board_ReadFromStream\n");
					return;
				}
				++y;
				x = 0;
				break;
			case '#':
				if (x >= width)
				{
					fprintf(stderr, "x >= width in Board_ReadFromStream\n");
					return;
				}
				map[x][y] = true;
				++x;
				break;
			case ' ':
				if (x >= width)
				{
					fprintf(stderr, "x >= width in Board_ReadFromStream\n");
					return;
				}
				map[x][y] = false;
				++x;
				break;
			case '1':
				if (x >= width)
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
				if (x >= width)
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

bool Map::stuck() const
{
	return is_wall(NORTH) && is_wall(EAST) && is_wall(SOUTH) && is_wall(WEST);
}
