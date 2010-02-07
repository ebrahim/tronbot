// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>

#define MAX_FEAR 2

class LongestPath
{
public:
	enum { MAX_DEPTH = 16384 };

	LongestPath(const Map& map)
	: width(map.width)
	, height(map.height)
	, x(map.my_x())
	, y(map.my_y())
	, enemy_x(map.opponent_x())
	, enemy_y(map.opponent_y())
	{
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
			{
				d[x][y] = map.is_wall(x, y) ? -1 : 0;
				saw[x][y] = false;
			}
	}

	int get_d(int xx, int yy)
	{
		if (xx < 0 || yy < 0 || xx >= width || yy >= height)
			return -1;
		return d[xx][yy];
	}

	void set_d(int xx, int yy, int value)
	{
		if (xx < 0 || yy < 0 || xx >= width || yy >= height || d[xx][yy] < 0)
			return;
		d[xx][yy] = value;
	}

	int run(int fear = 0)
	{
		for (int x_diff = -fear; x_diff <= fear; ++x_diff)
			for (int y_diff = -fear; y_diff <= fear; ++y_diff)
			{
				int xx = enemy_x + x_diff;
				int yy = enemy_y + y_diff;
				if (xx != x || yy != y)
					set_d(xx, yy, -1);
			}
		saw[x][y] = true;
		int res = run(0, std::string("/"));
		//fprintf(stderr, "res: %d\n", res);
		return res;
	}

	int run(int depth, std::string path)
	{
		static int x_diff[4] = { 0, 1, 0, -1 };
		static int y_diff[4] = { -1, 0, 1, 0 };
		//fprintf(stderr, "(%d, %d)\t%s\n", x, y, path.c_str());
		int me = get_d(x, y);
		if (me < 0 || depth >= MAX_DEPTH)		// If wall
			return -1;
		//fputs("salam\n", stderr);
		int x_save = x;
		int y_save = y;
		int max_child_path = me;
		int max_neighbor = -1;
		for (int neighbor = 0; neighbor <= 3; ++neighbor)
		{
			x = x_save + x_diff[neighbor];
			y = y_save + y_diff[neighbor];
			int that = get_d(x, y);
			if (me + 1 > that && that >= 0 && !saw[x][y])
			{
				set_d(x, y, me + 1);
				saw[x][y] = true;
				int this_max_child = run(depth + 1, (path + (char) (neighbor + '0')) + '/');
				//fprintf(stderr, "this max: %d\n", this_max_child);
				saw[x][y] = false;
				if (this_max_child > max_child_path)
				{
					max_child_path = this_max_child;
					max_neighbor = neighbor;
				}
			}
		}
		x = x_save;
		y = y_save;
		go = max_neighbor;
		return max_child_path;
	}

	int d[MAX][MAX];
	bool saw[MAX][MAX];
	int width;
	int height;
	int x;
	int y;
	int enemy_x;
	int enemy_y;
	int go;
};

int make_move(const Map& map)
{
	for (int fear = MAX_FEAR + map.distance() / 8; fear >= 0; --fear)
	{
		LongestPath lp(map);
		if (lp.run(fear) > 0)
			return lp.go + 1;
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
