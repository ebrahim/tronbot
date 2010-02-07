// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>

int make_move(const Map& map)
{
	int x = map.my_x();
	int y = map.my_y();
	if (!map.is_wall(x, y - 1))
	{
		return NORTH;
	}
	if (!map.is_wall(x + 1, y))
	{
		return EAST;
	}
	if (!map.is_wall(x, y + 1))
	{
		return SOUTH;
	}
	if (!map.is_wall(x - 1, y))
	{
		return WEST;
	}
	return NORTH;
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main()
{
	while (true)
	{
		Map map;
		Map::make_move(make_move(map));
	}
	return 0;
}
