#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fmt/base.h>
#include <random>
#include <raylib.h>
#include "raymath.h"
#include <fmt/format.h>
#include <vector>

#define WIDTH 800
#define HEIGHT 800

uint64_t get_time_ms() {
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             now.time_since_epoch())
      .count();
}


enum GridState {
  GRID_STATE_EMPTY = 0,
  GRID_STATE_FOOD,
};

class Snake {
public:
  std::vector<Vector2> body;
  bool input_consumed;

  Vector2 vel;

public:
  Snake() : body(), input_consumed(true), vel(Vector2Zero()) { body.push_back(Vector2Zero()); }
  ~Snake() = default;

  Vector2& head() { return body[0]; }

  void handle_input() {
    if (!input_consumed) { return; }

    if (IsKeyPressed(KEY_UP)) {
      if (vel.y != 0.f) { return; }
      vel.y = -1.f;
      vel.x = 0.f;
      input_consumed = false;
    } else if (IsKeyPressed(KEY_DOWN)) {
      if (vel.y != 0.f) { return; }
      vel.y = 1.f;
      vel.x = 0.f;
      input_consumed = false;
    }

    else if (IsKeyPressed(KEY_RIGHT)) {
      if (vel.x != 0.f) { return; }
      vel.x = 1.f;
      vel.y = 0.f;
      input_consumed = false;
    } else if (IsKeyPressed(KEY_LEFT)) {
      if (vel.x != 0.f) { return; }
      vel.x = -1.f;
      vel.y = 0.f;
      input_consumed = false;
    }
  }

  void update() {
    /**
     * Iterate from the tail up till the head (exclusively), shifting them ahead.
     */
    for (size_t i = body.size() - 1; i > 0; i--) {
      body[i] = body[i - 1];
    }

    head().x += vel.x;
    head().y += vel.y;

    if (!input_consumed) { input_consumed = true; }
  }

  void grow() {
    const Vector2 tail = body.back();
    body.push_back({ .x = tail.x, .y = tail.y });
  }
};


class UniformRandom {
private:
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<> rand;

public:
  UniformRandom() : rd(), gen(rd()), rand(0, 1) {}
  float random() { return rand(gen); }
};


class Grid {
public:
  size_t width;
  size_t height;
  size_t cell_size;

private:
  std::vector<GridState> grid;


public:
  Grid(size_t width, size_t height, size_t size)
      : width(width / size), height(height / size), cell_size(size) {
    this->grid.resize(this->width * this->height);
    std::fill(this->grid.begin(), this->grid.end(), GRID_STATE_EMPTY);
  }
  ~Grid() = default;

  GridState &at(size_t x, size_t y) { return grid[x + (y * this->width)]; }

  Vector2 cell_at(size_t x, size_t y) {
    return {.x = (float)x * cell_size, .y = (float)y * cell_size};
  }

  Color get_color(GridState state) {
    switch (state) {
      case GRID_STATE_EMPTY:
        return BLACK;
      case GRID_STATE_FOOD:
        return RED;
    }
    assert(0);
  }

  void draw() {
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++ ) {
        DrawRectangleV(
          cell_at(x, y),
          { .x = (float) cell_size, .y = (float) cell_size },
          get_color(at(x, y))
        );
      }
    }
  }
};


class Game {
private:
  Grid grid;
  Snake snake;

  uint64_t speed_ms;
  uint64_t last_updated_ms;

  UniformRandom rand;

public:
  bool is_running = true;

public:
  Game(size_t width, size_t height, size_t size, uint64_t initial_speed)
      : grid(width, height, size), speed_ms(initial_speed), last_updated_ms(0) {
    snake.head().x = grid.width / 2.f;
    snake.head().y = grid.height / 2.f;
  }
  ~Game() = default;

  void increase_speed() {
    speed_ms -= 10;
    speed_ms = std::clamp(speed_ms, 50UL, 1000UL);
  }

  void handle_input() { snake.handle_input(); }

  // TODO: handle going off board.
  void update() {
    uint64_t cur_time = get_time_ms();
    uint64_t dt_ms = cur_time - last_updated_ms;
    if (dt_ms < speed_ms) { return; }
    last_updated_ms = cur_time;


    snake.update();
    GridState& snake_head_grid = grid.at(snake.head().x, snake.head().y);

    switch (snake_head_grid) {
      case GRID_STATE_FOOD:
        snake_head_grid = GRID_STATE_EMPTY;

        snake.grow();
        spawn_food();
        increase_speed();
        break;

      case GRID_STATE_EMPTY:
      default:
        break;
    }

    if (snake.body.size() > 2) {
      for (size_t i = 1; i < snake.body.size(); i++) {
        const Vector2 &body_part = snake.body[i];
        if (snake.head().x == body_part.x && snake.head().y == body_part.y) {
          is_running = false;
        }
      }
    }
  }

  void draw() {
    /**
     * Draw the grid.
     */
    grid.draw();

    /**
     * Draw the snake.
     */
    for (Vector2& v : snake.body) {
      Vector2 snake_cell = grid.cell_at(v.x, v.y);
      DrawRectangleV(snake_cell, {(float)grid.cell_size, (float)grid.cell_size}, GREEN);
    }
    Vector2 snake_head_cell = grid.cell_at(snake.head().x, snake.head().y);
    DrawRectangleV(snake_head_cell, {(float)grid.cell_size, (float)grid.cell_size}, DARKGREEN);
  }

  void spawn_food() {
    const int x = rand.random() * grid.width;
    const int y = rand.random() * grid.height;

    fmt::println("spawn_food: ({}, {})", x, y);
    grid.at(x, y) = GRID_STATE_FOOD;
  }

  uint64_t get_score() { return snake.body.size(); }
  uint64_t get_speed() { return speed_ms; }
};


int main() {
  InitWindow(WIDTH, HEIGHT, "Snake");
  SetTargetFPS(60);

  const uint64_t initial_speed = 100;
  Game game(WIDTH, HEIGHT, 30, initial_speed);
  game.spawn_food();

  while (!WindowShouldClose()) {
    // IO
    {
      if (IsKeyPressed(KEY_Q)) {
        CloseWindow();
        break;
      }
      if (game.is_running) {
        game.handle_input();
      }
    }

    // UPDATE
    if (game.is_running) {
      game.update();
    }

    // DRAW
    {
      BeginDrawing();
      ClearBackground(BLACK);

      game.draw();
      if (!game.is_running) {
        DrawText(
          "GAME OVER",
          (WIDTH / 2) - 90,
          HEIGHT / 2,
          32,
          WHITE
        );
      }

      DrawFPS(WIDTH - 100.f, 10.f);
      DrawText(
        fmt::format("Score: {}", game.get_score()).c_str(),
        WIDTH - 100.f,
        40.f,
        18,
        GREEN
      );
      DrawText(
        fmt::format("Speed: {}", game.get_speed()).c_str(),
        WIDTH - 100.f,
        60.f,
        18,
        GREEN
      );
      EndDrawing();
    }
  }

  CloseWindow();
  return 0;
}
