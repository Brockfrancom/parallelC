#include <iostream>
#include <vector>
#include <numeric>
/*
1. Make a game of life.   Create a 'world' of 1024 by 1024 cells, without edge connections (i.e., the world is 'flat' with 'edges' that organisms can 'fall off' of).

The rules for the game of life are:
        every organism that has 0 or 1 neighbors dies of lonelines.
        every organism that has 2 or 3 neighbors survives to the next day.
        every organism that has 4 or more neighbors dies of overcrowding.
        every cell that has exactly three neighbors gives birth to a new organism.

Test to see if your program is working by creating a 'glider gun' at one edge of the world and tracking it as it crosses the problem space.

After you are sure that your program is working correctly, seed the entire space with organisms so that there is a 1-in-5 probability that any one cell initially contains an organism.

Be sure to correctly implement pseudo-random number generators over your processor set.

That being accomplished, make your program run as quickly as possible.   Provide timing information on a 'per day' basis, and chart your program versus the number of processors used.
*/

using Row   = std::vector<int>;
using Cells = std::vector<Row>;
// ------------------------------------------------------------------
Cells board = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

int numRows = 10;
int numCols = 20;
// ------------------------------------------------------------------
int getNeighbor(int row, int col, Cells& board) {
  // use modulus to get wrapping effect at board edges
  return board.at((row + numRows) % numRows).at((col + numCols) % numCols);
}

int getCount(int row, int col, Cells& board) {
  int count = 0;
  std::vector<int> deltas {-1, 0, 1};
  for (int dc : deltas) {
    for (int dr : deltas) {
      if (dr || dc) {
        count += getNeighbor(row + dr, col + dc, board);
      }
    }
  }
  return count;
}

void showCell(int cell) {
  std::cout << (cell ? "*" : " ");
}

void showRow(const Row& row) {
  std::cout << "|";
  for (int cell : row) {showCell(cell);}
  std::cout << "|\n";
}

void showCells(Cells board) {
  for (const Row& row : board) { showRow(row); }
}

int tick(Cells& board, int row, int col) {
  int count = getCount(row, col, board);
  bool birth = !board.at(row).at(col) && count == 3;
  bool survive = board.at(row).at(col) && (count == 2 || count == 3);
  return birth || survive;
}

void updateCells(Cells& board) {
  Cells original = board;
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      board.at(row).at(col) = tick(original, row, col);
    }
  }
}

int main () {
  for (int gen = 0; gen < 20; gen++) {
    std::cout << "\ngeneration " << gen << ":\n";
    showCells(board);
    updateCells(board);
  }
}
