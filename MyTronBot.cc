// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>

#define FEAR_BASE 1

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
		for (int xx = 0; xx < width; ++xx)
			for (int yy = 0; yy < height; ++yy)
			{
				d[xx][yy] = map.is_wall(xx, yy) ? -1 : 0;
				saw[xx][yy] = false;
			}
		d[enemy_x][enemy_y] = -1;
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
		for (int diff = 0; diff <= fear; ++diff)
			for (int x_diff = 0; x_diff <= diff; ++x_diff)
			{
				int y_diff = diff - x_diff;
				set_d(enemy_x + x_diff, enemy_y + y_diff, -2);		// Mark for growing
				set_d(enemy_x - x_diff, enemy_y + y_diff, -2);		// Mark for growing
				set_d(enemy_x + x_diff, enemy_y - y_diff, -2);		// Mark for growing
				set_d(enemy_x - x_diff, enemy_y - y_diff, -2);		// Mark for growing
			}
		for (int xx = 0; xx < width; ++xx)
			for (int yy = 0; yy < height; ++yy)
				if (d[xx][yy] == -2)
					d[xx][yy] = -1;		// Grow marked ones
		d[x][y] = 0;		// Clear self wall if any
#if 0
		for (int yy = 0; yy < height; ++yy)
		{
			for (int xx = 0; xx < width; ++xx)
				putc(d[xx][yy] ? '+' : ' ', stderr);
			putc('\n', stderr);
		}
#endif
		saw[x][y] = true;		// Already seen self!
		int res = deepen(0);
		return res;
	}

	int deepen(int depth)
	{
		static int x_diff[4] = { 0, 1, 0, -1 };
		static int y_diff[4] = { -1, 0, 1, 0 };
		int me = get_d(x, y);
		if (me < 0 || depth >= MAX_DEPTH)		// If wall, or max depth
			return -1;
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
				int this_max_child = deepen(depth + 1);
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
	int max_score = 0;
	int move = NORTH;
	for (int fear = FEAR_BASE; fear >= 0; --fear)
	{
		LongestPath lp(map);
		int score = lp.run(fear);
		score -= score / fear;
		if (score > max_score)
		{
			max_score = score;
			move = lp.go + 1;
		}
	}
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
