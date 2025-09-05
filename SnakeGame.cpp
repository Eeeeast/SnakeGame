#include <chrono>
#include <conio.h>
#include <deque>
#include <iostream>
#include <thread>
#include <vector>

#define KEY_ESCAPE 27
#define KEY_BACKSPACE 8
#define KEY_ENTER 13
#define KEY_F10 324
#define KEY_W 119
#define KEY_A 97
#define KEY_S 115
#define KEY_D 100
#define KEY_UP 328
#define KEY_LEFT 331
#define KEY_DOWN 336
#define KEY_RIGHT 333

int GetKey() {
  int c = _getch();
  if (c == 0 || c == 224)
    c = 256 + _getch(); /* If extended key (like F10), add 256. */
  return c;
}

struct GameInput {
  std::pair<int, int> dir{0, 0};
  bool is_game_paused{true};
  bool is_game_on{false};
};

static void handle_input(GameInput &input) {
  while (_kbhit()) {
    int key = GetKey();
    switch (key) {
    case KEY_ESCAPE:
      input.is_game_paused = true;
      break;

    case KEY_UP:
    case KEY_W:
      input.is_game_paused = false;
      input.dir = {-1, 0};
      break;

    case KEY_LEFT:
    case KEY_A:
      input.is_game_paused = false;
      input.dir = {0, -1};
      break;

    case KEY_DOWN:
    case KEY_S:
      input.is_game_paused = false;
      input.dir = {1, 0};
      break;

    case KEY_RIGHT:
    case KEY_D:
      input.is_game_paused = false;
      input.dir = {0, 1};
      break;
    }
  }
}

static void to_begin_of_screen() {
  std::cout << std::endl;
  printf("\033[H\033[3J");
  fflush(stdout);
}

static void print_board(const std::vector<std::vector<char>> &board,
                        const std::pair<uint8_t, uint8_t> board_size,
                        int score) {
  std::cout << std::string(board_size.second + 2, '-') << '\n';
  for (auto &t : board) {
    std::cout << '|';
    for (auto c : t)
      std::cout << c;
    std::cout << '|' << '\n';
  }
  std::cout << std::string(board_size.second + 2, '-') << '\n';
  std::cout << "Score: " << score << '\n';
  std::cout << std::endl;
}

static void print_game_over(int score) {
  std::cout << std::endl
            << "Game Over!\n"
            << "Final Score: " << score << '\n'
            << std::endl;
}

static std::pair<int, int>
generate_food(const std::vector<std::vector<char>> &board,
              const std::pair<uint8_t, uint8_t> board_size) {
  int i{board_size.first * board_size.second};
  std::pair<int, int> pos{0, 0};
  do {
    // Generate random x and y values within the map
    pos.first = rand() % (board_size.first);
    pos.second = rand() % (board_size.second);

    // If no space, return special pos
    if (--i == 0)
      return {-1, -1};
    // If location is not free try again
  } while (board[pos.first][pos.second] != ' ');

  // Return food position
  return pos;
}

static std::vector<std::vector<char>>
fill_board(const std::pair<uint8_t, uint8_t> board_size,
           const std::deque<std::pair<int, int>> &snake_body,
           const std::pair<int, int> food_pos) {
  std::vector<std::vector<char>> board{
      board_size.first, std::vector<char>(board_size.second, ' ')};

  for (auto &s : snake_body)
    board[s.first][s.second] = '*';

  if (food_pos.first >= 0 && food_pos.first < board_size.first &&
      food_pos.second >= 0 && food_pos.second < board_size.second)
    board[food_pos.first][food_pos.second] = '@';

  return board;
}

bool update_snake(std::deque<std::pair<int, int>> &snake_body,
                  const std::pair<int, int> dir,
                  const std::pair<uint8_t, uint8_t> board_size,
                  const std::vector<std::vector<char>> &board,
                  std::pair<int, int> &food_pos, int &score) {
  std::pair<int, int> new_head = snake_body.front();
  new_head.first += dir.first;
  new_head.second += dir.second;

  if (new_head.first < 0 || new_head.first >= board_size.first ||
      new_head.second < 0 || new_head.second >= board_size.second) {
    return false;
  }

  switch (board[new_head.first][new_head.second]) {
  case '@':
    score += 1;
    food_pos = generate_food(board, board_size);
    break;
  case ' ':
    snake_body.pop_back();
    break;
  default:
    return false;
  }

  snake_body.push_front(new_head);

  return true;
}

int main() {
  std::srand(std::time({}));

  GameInput game_input;
  game_input.is_game_on = true;
  std::pair<uint8_t, uint8_t> board_size{7, 11};
  int score{0};

  std::cout << std::string(board_size.second + 3, ' ')
            << "Hit a key -- ESC key pause\n"
            << std::string(board_size.second + 3, ' ') << "Move -- WASD\n"
            << std::endl;

  std::deque<std::pair<int, int>> snake_body{
      {rand() % (board_size.first), rand() % (board_size.second)}};
  std::vector<std::vector<char>> board =
      fill_board(board_size, snake_body, {-1, -1});
  std::pair<int, int> food_pos = generate_food(board, board_size);
  board = fill_board(board_size, snake_body, food_pos);

  to_begin_of_screen();
  print_board(board, board_size, score);

  while (game_input.is_game_on) {
    handle_input(game_input);
    if (game_input.is_game_paused)
      continue;

    if (!update_snake(snake_body, game_input.dir, board_size, board, food_pos,
                      score)) {
      game_input.is_game_on = false;
      print_game_over(score);
      return 0;
    }

    to_begin_of_screen();
    board = fill_board(board_size, snake_body, food_pos);
    print_board(board, board_size, score);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(std::max(200, 400 - score * 5)));
  }

  return 0;
}