// MyTronBot.cc
//
// This is the code file that you will modify in order to create your entry.

#include <string>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <stdint.h>

#include <signal.h>
#include <sys/time.h>		// For setitimer

#include "Map.h"

#define MEMOIZE 1
#define TIMEOUT 990000		// usec
#define FIRST_TIMEOUT /*2*/990000		// usec
#define CACHE_SIZE (1<<19)

#if MEMOIZE
#define _BACKWARD_BACKWARD_WARNING_H 1		// Disable warning
#include <ext/hash_map>
#include <algorithm>
#include <queue>
#include <set>
#include <vector>
#include <deque>
#include <list>
#endif

static int x_diff[4] = { 0, 1, 0, -1 };
static int y_diff[4] = { -1, 0, 1, 0 };

static bool timed_out = false;

class AlphaBeta
{
public:
	enum { INFINITY = 1 << 30 };
#if MEMOIZE
	enum { START_DEPTH = 2 };
#else
	enum { START_DEPTH = 10 };
#endif
	enum { SCORE_LOSE = -INFINITY + 1 };
	enum { SCORE_DRAW = -7 };
	enum { SCORE_WIN = INFINITY - 1 };

#if MEMOIZE
	class GameState
	{
	public:
		GameState()
		{
		}

		GameState(const AlphaBeta& alphabeta)
		: x(alphabeta.x)
		, y(alphabeta.y)
		, enemy_x(alphabeta.enemy_x)
		, enemy_y(alphabeta.enemy_y)
		{
			for (int xx = 0; xx < MAX_SIDE; ++xx)
				map[xx] = 0;
			for (int xx = 0; xx < alphabeta.width; ++xx)
				for (int yy = 0; yy < alphabeta.height; ++yy)
					map[xx] |= alphabeta.wall[xx][yy] << yy;
		}

		size_t operator()(const GameState& value) const		// Hash function
		{
			size_t hash = 0;
			for (int xx = 0; xx < MAX_SIDE; ++xx)
				hash ^= value.map[xx];
			hash ^= value.x | (uint64_t(value.y) << 32);
			hash ^= value.enemy_x | (uint64_t(value.enemy_y) << 32);
			return hash;
		}

		size_t operator()(const GameState* const& value) const		// Hash function
		{
			size_t hash = 0;
			for (int xx = 0; xx < MAX_SIDE; ++xx)
				hash ^= value->map[xx];
			hash ^= value->x | (uint64_t(value->y) << 32);
			hash ^= value->enemy_x | (uint64_t(value->enemy_y) << 32);
			return hash;
		}

		bool operator()(const GameState& one, const GameState& two) const
		{
			if (one.x != two.x || one.y != two.y || one.enemy_x != two.enemy_x || one.enemy_y != two.enemy_y)
				return false;
			for (int xx = 0; xx < MAX_SIDE; ++xx)
				if (one.map[xx] != two.map[xx])
					return false;
			return true;
		}

		bool operator()(const GameState* const& one, const GameState* const& two) const
		{
			if (one->x != two->x || one->y != two->y || one->enemy_x != two->enemy_x || one->enemy_y != two->enemy_y)
				return false;
			for (int xx = 0; xx < MAX_SIDE; ++xx)
				if (one->map[xx] != two->map[xx])
					return false;
			return true;
		}

		uint64_t map[MAX_SIDE];
		uint32_t x, y, enemy_x, enemy_y;
	};

	typedef __gnu_cxx::hash_map<GameState, int, GameState, GameState> EvaluationCache;
	typedef std::deque<EvaluationCache::iterator> EvaluationCacheAge;

	struct Heuristic
	{
		Heuristic(int neighbor, int score = 0) : neighbor(neighbor), score(score) { }
		bool operator<(const Heuristic& other) const { return score > other.score; }
		int neighbor, score;
	};
	typedef std::multiset<Heuristic> Ordering;
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
			//fprintf(stderr, "depth: %d\n", depth);
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
			if (is_wall(x, y) || is_wall(enemy_x, enemy_y) || (x == enemy_x && y == enemy_y) || timed_out)		// If search in this branch ended
				return evaluate();
			else if (depth <= 0)
			{
				full_search = false;
				return evaluate();
			}
		}
		wall[x][y] = true;
		int my_max_score = -INFINITY;
		int my_max_neighbor = -1;
#if MEMOIZE
		Ordering ordering;
		if (depth % 2 == 0)
		{
			for (int neighbor = 0; neighbor < 4; ++neighbor)
			{
				x += x_diff[neighbor];
				y += y_diff[neighbor];
				ordering.insert(Heuristic(neighbor, evaluate(true)));
				x -= x_diff[neighbor];
				y -= y_diff[neighbor];
			}
			//ordering.sort();
			//std::sort(ordering.begin(), ordering.end());
		}
		else
			for (int neighbor = 0; neighbor < 4; ++neighbor)
				ordering.insert(Heuristic(neighbor));
		for (Ordering::iterator it = ordering.begin(); it != ordering.end(); ++it)
		{
			int neighbor = it->neighbor;
#else
		for (int neighbor = 0; neighbor < 4; ++neighbor)
		{
#endif
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
			Pos(int x, int y) : x(x), y(y) { }
			int x, y;
		};

		static int reached[MAX_SIDE][MAX_SIDE];
		static Pos neighbors[MAX_SIDE * MAX_SIDE];

		depth = 0;
		enemy_distance = INFINITY;
		if (is_wall(xx, yy))
			return 0;
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
				reached[x][y] = 0;
		int tail = 0;
		int head = 0;
		reached[xx][yy] = 1;
		neighbors[tail++] = Pos(xx, yy);
		int density = 1;		// Neighborhood density score
		do
		{
			int x = neighbors[head].x;
			int y = neighbors[head].y;
			int d = reached[x][y];
			for (int diff = 0; diff < 4; ++diff)
			{
				int xx = x + x_diff[diff];
				int yy = y + y_diff[diff];
				if (wall[xx][yy])
					continue;
				++density;
				if (reached[xx][yy])
					continue;
				++density;
				depth += d;
				reached[xx][yy] = d + 1;
				neighbors[tail++] = Pos(xx, yy);
			}
			++head;		// Pop
		} while (head != tail);
		for (int diff = 0; diff < 4; ++diff)
		{
			int xx = enemy_x + x_diff[diff];
			int yy = enemy_y + y_diff[diff];
			int d = reached[xx][yy];
			if (d && d < enemy_distance)
				enemy_distance = d;
		}
		return density;
	}

	int distance(int x1, int y1, int x2, int y2)
	{
		int dx = x1 - x2;
		int dy = y1 - y2;
		return dx * dx + dy * dy;
	}

#if MEMOIZE
	int evaluate(bool cheap = false)
#else
	int evaluate(bool /*cheap*/ = false)
#endif
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
		fputs("-------------------------------\n", stderr);
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

#if MEMOIZE
		// Memoize
		static EvaluationCache cache(CACHE_SIZE);
		static EvaluationCacheAge cache_age(CACHE_SIZE);
		static uint64_t cache_counter = 0;

		GameState game_state(*this);
		EvaluationCache::iterator cached = cache.find(game_state);
		if (cached != cache.end())		// If already calculated
			return cached->second;		// Return cached value
		if (cheap)
			return 0;
#endif

		int max_neighbor_area_me = -INFINITY;
		int flood_depth_me = 0;
		int max_neighbor_area_enemy = -INFINITY;
		int flood_depth_enemy = 0;
		int enemy_distance = INFINITY;
		wall[x][y] = true;
		wall[enemy_x][enemy_y] = true;
		for (int diff = 0; diff < 4; ++diff)
		{
			int flood_depth;
			int distance;
			int this_neighbor_area = floodfill(x + x_diff[diff], y + y_diff[diff], flood_depth, distance);
			if (distance < enemy_distance)
				enemy_distance = distance;
			if (this_neighbor_area > max_neighbor_area_me)
			{
				max_neighbor_area_me = this_neighbor_area;
				flood_depth_me = flood_depth;
			}
			this_neighbor_area = floodfill(enemy_x + x_diff[diff], enemy_y + y_diff[diff], flood_depth, distance);
			if (this_neighbor_area > max_neighbor_area_enemy)
			{
				max_neighbor_area_enemy = this_neighbor_area;
				flood_depth_enemy = flood_depth;
			}
		}
		wall[x][y] = false;
		wall[enemy_x][enemy_y] = false;
		int score = 0;
		score += max_neighbor_area_me;		// Prefer larger neighborhood for myself
		score -= max_neighbor_area_enemy;		// Prefer smaller neighborhood for enemy
		if (enemy_distance == INFINITY)		// If separated
			score *= -SCORE_DRAW + 1;		// If have got less room, prefer collision
		else		// If in the same area
		{
			score *= 4;
			score -= 8 * enemy_distance;		// Prefer near enemy
			score -= distance(x, y, enemy_x, enemy_y);		// Prefer near enemy again!
			score -= flood_depth_me;		// Prefer myself at center
			score += flood_depth_enemy;		// Prefer enemy at corners
		}
		//fprintf(stderr, "%d %d %d %d %d %d\n", max_neighbor_area_me, max_neighbor_area_enemy, enemy_distance, flood_depth_me, flood_depth_enemy, score);
#if MEMOIZE
		std::pair<EvaluationCache::iterator, bool> res = cache.insert(EvaluationCache::value_type(game_state, score));
		cache_age.push_back(res.first);
		if (++cache_counter > CACHE_SIZE)
		{
			cache.erase(cache_age.front());
			cache_age.pop_front();
			--cache_counter;
		}
#endif
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
	for (long timeout = FIRST_TIMEOUT; ; timeout = TIMEOUT)
	{
		Map map;
		timed_out = false;
		// Setup timer
		itimerval timer = { { 0, 0 }, { 0, timeout } };
		setitimer(ITIMER_REAL, &timer, NULL);
		AlphaBeta alphabeta(map);
		int move = alphabeta.run() + 1;
		// Disable timer
		timer.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, NULL);
		Map::make_move(move);
	}
	return 0;
}
