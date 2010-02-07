// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>

int make_move(const Map& map)
{
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
	++target_col;
	int x = map.my_x();
	int y = map.my_y();
	int direction = x < target_col ? EAST : x > target_col ? WEST : SOUTH;
	bool can[4] = { 0 };
	for (int i = 1; i < 5; ++i)
		can[i] = !map.is_wall(i);
	switch (direction)
	{
		case EAST:
			if (can[EAST])
				return EAST;
			if (can[SOUTH])
				return SOUTH;
			if (can[NORTH])
				return NORTH;
			return WEST;
		case WEST:
			if (can[WEST])
				return WEST;
			if (can[SOUTH])
				return SOUTH;
			if (can[NORTH])
				return NORTH;
			return EAST;
		case SOUTH:
			if (can[SOUTH])
				return SOUTH;
			if (can[NORTH])
				return NORTH;
			if (map.opponent_x() < x && can[EAST] || !can[WEST])
				return EAST;
			else
				return WEST;
	}
	return NORTH;
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
