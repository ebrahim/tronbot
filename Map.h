// Map.h
//
// Handles the Tron map. Also handles communicating with the Tron game engine.
// You don't need to change anything in this file.

#include <string>
#include <vector>

#define MAX 256

enum { NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4 };

class Map
{
public:
	// Constructs a Map by reading an ASCII representation from the console
	// (stdin).
	Map();

	// Returns whether or not the given cell is a wall or not. TRUE means it's
	// a wall, FALSE means it's not a wall, and is passable. Any spaces that are
	// not on the board are deemed to be walls.
	bool is_wall(int x, int y) const;
	bool is_wall(int move) const;

	// Get my X and Y position. These are zero-based.
	int my_x() const;
	int my_y() const;

	// Get the opponent's X and Y position. These are zero-based.
	int opponent_x() const;
	int opponent_y() const;

	// Sends your move to the contest engine. The four possible moves are
	//   * 1 -- North. Negative Y direction.
	//   * 2 -- East. Positive X direction.
	//   * 3 -- South. Positive X direction.
	//   * 4 -- West. Negative X direction.
	static void make_move(int move);

	// Am I stuck?
	bool stuck() const;

	// Map dimensions.
	int width, height;

private:
	// Load a board from an open file handle. To read from the console, pass
	// stdin, which is actually a (FILE*).
	//   file_handle -- an open file handle from which to read.
	//
	// If there is a problem, the function returns NULL. Otherwise, a valid
	// Board structure is returned.
	//
	// The file should be an ascii file. The first line contains the width and
	// height of the board, separated by a space. subsequent lines contain visual
	// representations of the rows of the board, using '#' and space characters.
	// The starting positions of the two players are indicated by '1' and '2'
	// characters. There must be exactly one '1' character and one '2' character
	// on the board. For example:
	// 6 4
	// ######
	// #1# 2#
	// #   ##
	// ######
	void read_from_file(FILE *file_handle);

private:
	// Indicates whether or not each cell in the board is passable.
	bool map[MAX][MAX];

	// The locations of both players.
	int player_one_x, player_one_y;
	int player_two_x, player_two_y;
};
