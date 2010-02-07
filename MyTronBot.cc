// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>

int vertical(const Map& map, int fallback)
{
	if (map.is_wall(NORTH) && map.is_wall(SOUTH))
		return fallback;
	if (map.is_wall(NORTH) && !map.is_wall(SOUTH))
		return SOUTH;
	if (map.is_wall(SOUTH) && !map.is_wall(NORTH))
		return NORTH;
	int x = map.my_x();
	int north = 0;
	int south = 0;
	for (int y = map.my_y() - 1; !map.is_wall(x, y); --y)
	{
		++north;
		if (!map.is_wall(x - 1, y))
			++south;
		if (!map.is_wall(x + 1, y))
			++south;
	}
	for (int y = map.my_y() + 1; !map.is_wall(x, y); ++y)
	{
		++south;
		if (!map.is_wall(x - 1, y))
			++north;
		if (!map.is_wall(x + 1, y))
			++north;
	}
	if (north < south)
		return NORTH;
	else
		return SOUTH;
}

int make_move(const Map& map)
{
	static int walling = 0;
	if (walling && !map.is_wall(walling))
		return walling;
	else
		walling = 0;
	int space[MAX] = { 0 };
	for (int col = 0; col < map.width; ++col)
		for (int row = 0; row < map.height; ++row)
			space[col] += map.is_wall(col, row) ? 0 : 1;
	for (int col = 1; col < map.width; ++col)
		space[col] += space[col - 1];
	int target_col;
	for (target_col = 0; target_col < map.width; ++target_col)
		if (space[target_col] > space[map.width - 1] / 2)
			break;
	int x = map.my_x();
	int y = map.my_y();
	if (map.opponent_x() > x)
		++target_col;
	if (x < target_col)
	{
		if (!map.is_wall(EAST))
			return EAST;
		return vertical(map, WEST);
	}
	if (x > target_col)
	{
		if (!map.is_wall(WEST))
			return WEST;
		return vertical(map, EAST);
	}
	int move = vertical(map, map.opponent_x() < x && !map.is_wall(EAST) || map.is_wall(WEST) ? EAST : WEST);
	if (move == NORTH || move == SOUTH)
		walling = move;
	return move;
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main()
{
	for (;;)
	{
		Map map;
		Map::make_move(make_move(map));
	}
	return 0;
}
