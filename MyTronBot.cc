// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>

#define INFINITY 16384
#define FEAR_BASE 2
#define FEAR_MULT 2

static int x_diff[4] = { 0, 1, 0, -1 };
static int y_diff[4] = { -1, 0, 1, 0 };

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
	, max_neighbor(-1)
	{
		for (int xx = 0; xx < width; ++xx)
			for (int yy = 0; yy < height; ++yy)
				wall[xx][yy] = map.is_wall(xx, yy);
	}

	bool is_wall(int xx, int yy)
	{
		if (xx < 0 || yy < 0 || xx >= width || yy >= height)
			return true;
		return wall[xx][yy];
	}

	void set_wall(int xx, int yy, bool value)
	{
		if (xx < 0 || yy < 0 || xx >= width || yy >= height)
			return;
		wall[xx][yy] = value;
	}

	int reachables(int xx, int yy, int& depth, bool& reached_enemy)
	{
		static bool reached[MAX][MAX];
		struct Pos
		{
			Pos()
			{
			}

			Pos(int x, int y, int depth)
			: x(x)
			, y(y)
			, depth(depth)
			{
			}

			int x, y, depth;
		};
		static Pos neighbors[MAX * MAX];
		reached_enemy = false;
		depth = 0;
		if (is_wall(xx, yy))
			return 0;
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
				reached[x][y] = false;
		int tail = 0;
		int head = 0;
		reached[xx][yy] = true;
		neighbors[tail++] = Pos(xx, yy, 0);
		int count = 1;
		do
		{
			int x = neighbors[head].x;
			int y = neighbors[head].y;
			int d = neighbors[head].depth;
			for (int diff = 0; diff < 4; ++diff)
			{
				int xx = x + x_diff[diff];
				int yy = y + y_diff[diff];
				if (xx == enemy_x && yy == enemy_y)
					reached_enemy = true;
				if (!is_wall(xx, yy) && !reached[xx][yy])
				{
					reached[xx][yy] = true;
					neighbors[tail++] = Pos(xx, yy, d + 1);
					++count;
				}
			}
			++head;		// Pop
		} while (head != tail);
		depth = neighbors[tail - 1].depth;
		return count;
	}

	int run(int fear = 0)
	{
		for (int diff = 0; diff <= fear; ++diff)
			for (int dx = 0; dx <= diff; ++dx)
			{
				int dy = diff - dx;
				set_wall(enemy_x + dx, enemy_y + dy, true);
				set_wall(enemy_x - dx, enemy_y + dy, true);
				set_wall(enemy_x + dx, enemy_y - dy, true);
				set_wall(enemy_x - dx, enemy_y - dy, true);
			}
		wall[x][y] = 0;		// Clear self wall if any
#if 0
		for (int yy = 0; yy < height; ++yy)
		{
			for (int xx = 0; xx < width; ++xx)
				putc(wall[xx][yy] ? '+' : ' ', stderr);
			putc('\n', stderr);
		}
#endif
		return deepen();
	}

	int deepen(int depth = 0, std::string path = std::string("/"))
	{
		if (is_wall(x, y))		// If wall
			return -1;
		if (depth >= MAX_DEPTH)		// If reached max depth
			return depth;
		wall[x][y] = true;
#if 0
		fprintf(stderr, "%s\n", path.c_str());
		for (int yy = 0; yy < height; ++yy)
		{
			for (int xx = 0; xx < width; ++xx)
				putc(x == xx && y == yy ? '.' : wall[xx][yy] ? '+' : ' ', stderr);
			putc('\n', stderr);
		}
		fputs("-------------------------------\n", stderr);
#endif
		int max_score = -INFINITY;
		int max_score_neighbor = -1;
		// Pick and deepen only one neighbor based on visiting which neighbor leads into a bigger neighbor connected area
		for (int neighbor = 0; neighbor < 4; ++neighbor)
		{
			int xx = x + x_diff[neighbor];
			int yy = y + y_diff[neighbor];
			if (is_wall(xx, yy))
				continue;
			wall[xx][yy] = true;
			int max_neighbor_area_me = -INFINITY;
			int max_neighbor_me = -1;
			int min_flood_depth_me = INFINITY;
			int max_neighbor_area_enemy = -INFINITY;
			bool reached_enemy = false;
			for (int diff = 0; diff < 4; ++diff)
			{
				int flood_depth;
				bool reached;
				int this_neighbor_area = reachables(xx + x_diff[diff], yy + y_diff[diff], flood_depth, reached);
				reached_enemy = reached_enemy || reached;
				if (this_neighbor_area > max_neighbor_area_me)
					max_neighbor_area_me = this_neighbor_area;
				if (flood_depth && flood_depth < min_flood_depth_me)
					min_flood_depth_me = flood_depth;
				this_neighbor_area = reachables(enemy_x + x_diff[diff], enemy_y + y_diff[diff], flood_depth, reached);
				if (this_neighbor_area > max_neighbor_area_enemy)
					max_neighbor_area_enemy = this_neighbor_area;
			}
			int my_score = max_neighbor_area_me - max_neighbor_area_enemy;
			if (reached_enemy)
				my_score -= min_flood_depth_me;
			//fprintf(stderr, "%d %d %d %d %d %d\n", neighbor, max_neighbor_area_me, max_neighbor_area_enemy, reached_enemy, min_flood_depth_me, my_score);
			if (my_score > max_score)
			{
				max_score = my_score;
				max_score_neighbor = neighbor;
			}
			wall[xx][yy] = false;
		}
		if (max_score_neighbor < 0)		// If stuck
			return depth;
		x += x_diff[max_score_neighbor];
		y += y_diff[max_score_neighbor];
		int score = deepen(depth + 1, path + ("NESW"[max_score_neighbor]) + "/");
		max_neighbor = max_score_neighbor;
		return score;
	}

	bool wall[MAX][MAX];
	int width;
	int height;
	int x;
	int y;
	int enemy_x;
	int enemy_y;
	int max_neighbor;
};
	
int make_move(const Map& map)
{
	int scores[FEAR_BASE + 1];
	int moves[FEAR_BASE + 1];
	int scores_sum = 0;
	int scores_div = 0;
	for (int fear = 0; fear <= FEAR_BASE; ++fear)
	{
		LongestPath lp(map);
		scores[fear] = lp.run(fear);
		moves[fear] = lp.max_neighbor + 1;
		scores_sum = (fear + 1) * scores_sum * FEAR_MULT + scores[fear];
		scores_div = (fear + 1) * scores_div * FEAR_MULT + 1;
		//fprintf(stderr, "%d: %d %d\n", fear, scores[fear], moves[fear]);
	}
	int target_score = scores_sum / scores_div;
	//fprintf(stderr, "%d / %d = %d\n", scores_sum, scores_div, target_score);
	int min_value = INFINITY;
	int min_index = -1;
	for (int i = 0; i <= FEAR_BASE && scores[i] > 0; ++i)
	{
		int diff = scores[i] - target_score;
		if (diff < 0)
			diff = -diff;
		//fprintf(stderr, "%d: %d %d %d %d\n", i, scores[i], target_score, diff, min_value);
		if (diff <= min_value)
		{
			min_value = diff;
			min_index = i;
		}
	}
	return min_index < 0 ? NORTH : moves[min_index];
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
