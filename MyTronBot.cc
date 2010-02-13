// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <list>
#include <cstdlib>
#include <ctime>
#include <cstdio>

static int x_diff[4] = { 0, 1, 0, -1 };
static int y_diff[4] = { -1, 0, 1, 0 };

class AlphaBeta
{
public:
	enum { INFINITY = MAX_SIDE * MAX_SIDE };
	enum { MAX_DEPTH = 8 };
	enum { SCORE_LOSE = -256 };
	enum { SCORE_COLLIDE = -128 };
	enum { SCORE_WIN = 256 };

#if 0
	struct Order
	{
		Order(int neighbor, int score) : neighbor(neighbor), score(score) { }
		bool operator<(const Order& other) { return score > other.score; }
		int neighbor, score;
	};
	typedef std::list<Order> Ordering;
#endif

	AlphaBeta(const Map& map)
	: width(map.width)
	, height(map.height)
	, x(map.my_x())
	, y(map.my_y())
	, enemy_x(map.opponent_x())
	, enemy_y(map.opponent_y())
	, max_neighbor(0)
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

	void swap_roles()
	{
		int tmp = x;
		x = enemy_x;
		enemy_x = tmp;
		tmp = y;
		y = enemy_y;
		enemy_y = tmp;
	}

	int alphabeta(int depth = MAX_DEPTH, int alpha = -INFINITY, int beta = INFINITY)
	{
		//fprintf(stderr, "Depth: %d, Alpha: %d, Beta: %d\n", depth, alpha, beta);
		if (!depth || is_wall(x, y) || is_wall(enemy_x, enemy_y) || x == enemy_x && y == enemy_y)		// If search in this branch ended
			return evaluate();
		wall[x][y] = true;
#if 0
		Ordering ordering;
		for (int neighbor = 0; neighbor < 4; ++neighbor)
		{
			x += x_diff[neighbor];
			y += y_diff[neighbor];
			ordering.push_back(Order(neighbor, evaluate()));
			//fprintf(stderr, "%d\n", ordering.back().score);
			x -= x_diff[neighbor];
			y -= y_diff[neighbor];
		}
		ordering.sort();
		for (Ordering::iterator it = ordering.begin(); it != ordering.end(); ++it)
		{
			int neighbor = it->neighbor;
		}
#endif
		int my_max_score = -INFINITY;
		int my_max_neighbor = -1;
		for (int neighbor = 0; neighbor < 4; ++neighbor)
		{
			x += x_diff[neighbor];
			y += y_diff[neighbor];
			swap_roles();
			alpha = std::max(alpha, -alphabeta(depth - 1, -beta, -alpha));
			swap_roles();
			x -= x_diff[neighbor];
			y -= y_diff[neighbor];
			if (alpha > my_max_score)
			{
				my_max_score = alpha;
				my_max_neighbor = neighbor;
			}
			if (beta <= alpha)		// Beta cut-off
				break;
		}
		max_neighbor = my_max_neighbor;
		wall[x][y] = false;
		return alpha;
	}

	int floodfill(int xx, int yy, int& depth, bool& reached_enemy)
	{
		struct Pos
		{
			Pos() { }
			Pos(int x, int y, int depth) : x(x), y(y), depth(depth) { }
			int x, y, depth;
		};

		static bool reached[MAX_SIDE][MAX_SIDE];
		static Pos neighbors[MAX_SIDE * MAX_SIDE];
		depth = 0;
		reached_enemy = false;
		if (is_wall(xx, yy))
			return 0;
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
				reached[x][y] = false;
		int tail = 0;
		int head = 0;
		reached[xx][yy] = true;
		neighbors[tail++] = Pos(xx, yy, 0);
		int density = 1;		// Neighborhood density score
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
				if (is_wall(xx, yy))
					continue;
				++density;
				if (reached[xx][yy])
					continue;
				++density;
				reached[xx][yy] = true;
				neighbors[tail++] = Pos(xx, yy, d + 1);
			}
			++head;		// Pop
		} while (head != tail);
		depth = neighbors[tail - 1].depth;
		return density;
	}

	int evaluate()
	{
#if 0
		fputs("-------------------------------\n", stderr);
		fprintf(stderr, "(x1,y1): (%d,%d)\n(x2,y2): (%d,%d)\n", x, y, enemy_x, enemy_y);
		for (int yy = 0; yy < height; ++yy)
		{
			for (int xx = 0; xx < width; ++xx)
				putc(x == xx && y == yy ? '.' : wall[xx][yy] ? '+' : ' ', stderr);
			putc('\n', stderr);
		}
		fputs("-------------------------------\n", stderr);
#endif
		if (is_wall(x, y))		// If hit wall
			return SCORE_LOSE;
		else if (is_wall(enemy_x, enemy_y))		// If enemy hit wall
			return SCORE_WIN;
		else if (x == enemy_x && y == enemy_y)		// If collided
			return SCORE_COLLIDE;
		int max_neighbor_area_me = -INFINITY;
		int min_flood_depth_me = INFINITY;
		int max_neighbor_area_enemy = -INFINITY;
		bool reached_enemy = false;
		for (int diff = 0; diff < 4; ++diff)
		{
			int flood_depth;
			bool reached;
			int this_neighbor_area = floodfill(x + x_diff[diff], y + y_diff[diff], flood_depth, reached);
			reached_enemy = reached_enemy || reached;
			if (this_neighbor_area > max_neighbor_area_me)
				max_neighbor_area_me = this_neighbor_area;
			if (flood_depth && flood_depth < min_flood_depth_me)
				min_flood_depth_me = flood_depth;
			this_neighbor_area = floodfill(enemy_x + x_diff[diff], enemy_y + y_diff[diff], flood_depth, reached);
			if (this_neighbor_area > max_neighbor_area_enemy)
				max_neighbor_area_enemy = this_neighbor_area;
		}
		int score = max_neighbor_area_me - max_neighbor_area_enemy;
		if (reached_enemy)
			score -= min_flood_depth_me;		// Prefer center if in the same region as enemy
		//fprintf(stderr, "%d %d %d %d %d\n", max_neighbor_area_me, max_neighbor_area_enemy, reached_enemy, min_flood_depth_me, score);
		return score;
	}

	bool wall[MAX_SIDE][MAX_SIDE];
	int width;
	int height;
	int x;
	int y;
	int enemy_x;
	int enemy_y;
	int max_neighbor;
};
	
// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main()
{
	for (;;)
	{
		Map map;
		AlphaBeta alphabeta(map);
		alphabeta.alphabeta();
		Map::make_move(alphabeta.max_neighbor + 1);
	}
	return 0;
}
