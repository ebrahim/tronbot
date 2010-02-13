// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include "Map.h"
#include <string>
#include <list>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <signal.h>
#include <sys/time.h>		// For setitimer

#define TIMEOUT 990000		// usec

static int x_diff[4] = { 0, 1, 0, -1 };
static int y_diff[4] = { -1, 0, 1, 0 };

static bool timed_out = false;

class AlphaBeta
{
public:
	enum { INFINITY = MAX_SIDE * MAX_SIDE };
	enum { START_DEPTH = 12 };
	enum { SCORE_LOSE = -INFINITY + 1 };
	enum { SCORE_DRAW = -32 };
	enum { SCORE_WIN = INFINITY - 1 };

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
	, full_search(false)
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

	int run()
	{
#if 0
		alphabeta(18);
		return max_neighbor;
#endif
		int max_score = -INFINITY;
		int best_neighbor = -1;
		full_search = false;
		for (int depth = START_DEPTH; !timed_out && !full_search; depth += 2)
		{
			full_search = true;		// Assume full search, until game tree is cut
			int alpha = alphabeta(depth);
			if (alpha > max_score)
			{
				max_score = alpha;
				best_neighbor = max_neighbor;
			}
		}
		return best_neighbor;
	}

	int alphabeta(int depth, int alpha = -INFINITY, int beta = INFINITY)
	{
		//fprintf(stderr, "Depth: %d, Alpha: %d, Beta: %d\n", depth, alpha, beta);
		if (depth % 2 == 0)
		{
			if (is_wall(x, y) || is_wall(enemy_x, enemy_y) || x == enemy_x && y == enemy_y || timed_out)		// If search in this branch ended
				return evaluate();
			else if (depth <= 0)
			{
				full_search = false;
				return evaluate();
			}
		}
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
			//fprintf(stderr, "depth: %d, my_max_score: %d, alpha: %d, beta: %d\n", depth, my_max_score, alpha, beta);
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

	int floodfill(int xx, int yy, int& depth, int& enemy_distance)
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
		enemy_distance = INFINITY;
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
				if (xx == enemy_x && yy == enemy_y && d < enemy_distance)
					enemy_distance = d;
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

	int distance(int x1, int y1, int x2, int y2)
	{
		return abs(x1 - x2) + abs(y1 - y2);
	}

	int evaluate()
	{
#if 0
		fputs("-------------------------------\n", stderr);
		fprintf(stderr, "(x1,y1): (%d,%d)\n(x2,y2): (%d,%d)\n", x, y, enemy_x, enemy_y);
		for (int yy = 0; yy < height; ++yy)
		{
			for (int xx = 0; xx < width; ++xx)
				putc(xx == x && yy == y ? '1' : xx == enemy_x && yy == enemy_y ? '2' : wall[xx][yy] ? '#' : ' ', stderr);
			putc('\n', stderr);
		}
		//fputs("-------------------------------\n", stderr);
#endif
		if (is_wall(x, y))		// If hit wall
		{
			if (is_wall(enemy_x, enemy_y))		// If enemy also hit wall
				return SCORE_DRAW;
			else
				return SCORE_LOSE;
		}
		else if (is_wall(enemy_x, enemy_y))		// If enemy hit wall
			return SCORE_WIN;
		if (x == enemy_x && y == enemy_y)		// If collided
			return SCORE_DRAW;
		int max_neighbor_area_me = -INFINITY;
		int min_flood_depth_me = INFINITY;
		int max_neighbor_area_enemy = -INFINITY;
		int min_flood_depth_enemy = INFINITY;
		int enemy_distance = INFINITY;
		for (int diff = 0; diff < 4; ++diff)
		{
			int flood_depth;
			int distance;
			int this_neighbor_area = floodfill(x + x_diff[diff], y + y_diff[diff], flood_depth, distance);
			if (distance < enemy_distance)
				enemy_distance = distance;
			if (this_neighbor_area > max_neighbor_area_me)
				max_neighbor_area_me = this_neighbor_area;
			if (flood_depth && flood_depth < min_flood_depth_me)
				min_flood_depth_me = flood_depth;
			this_neighbor_area = floodfill(enemy_x + x_diff[diff], enemy_y + y_diff[diff], flood_depth, distance);
			if (this_neighbor_area > max_neighbor_area_enemy)
				max_neighbor_area_enemy = this_neighbor_area;
			if (flood_depth && flood_depth < min_flood_depth_enemy)
				min_flood_depth_enemy = flood_depth;
		}
		int score = max_neighbor_area_me - max_neighbor_area_enemy;
		if (enemy_distance == INFINITY)		// If separated
			score *= -SCORE_DRAW - 1;
		else		// If in the same area
		{
			score += width + height;		// Prevent preferring collision
			score -= 2 * enemy_distance + distance(x, y, enemy_x, enemy_y);		// Prefer near enemy
			score -= min_flood_depth_me / 2;		// Prefer center
			score += min_flood_depth_enemy / 2;		// Prefer enemy at corners
		}
		//fprintf(stderr, "%d %d %d %d %d\n", max_neighbor_area_me, max_neighbor_area_enemy, enemy_distance, min_flood_depth_me, score);
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
	bool full_search;
};

void timeout_handler(int /*sig*/)
{
	timed_out = true;
}

int main()
{
	signal(SIGALRM, timeout_handler);
	for (;;)
	{
		Map map;
		timed_out = false;
		// Setup timer
		itimerval timeout = { { 0, 0 }, { 0, TIMEOUT } };
		setitimer(ITIMER_REAL, &timeout, NULL);
		AlphaBeta alphabeta(map);
		int move = alphabeta.run() + 1;
		// Disable timer
		timeout.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &timeout, NULL);
		Map::make_move(move);
	}
	return 0;
}
