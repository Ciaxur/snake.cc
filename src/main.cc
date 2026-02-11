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
  Vector2 pos;
  Vector2 vel;

public:
  Snake() : pos(Vector2Zero()), vel(Vector2Zero()) {}
  ~Snake() = default;

  void handle_input() {
    if (IsKeyPressed(KEY_UP)) {
      if (vel.y != 0.f) { return; }
      vel.y = -1.f;
      vel.x = 0.f;
    } else if (IsKeyPressed(KEY_DOWN)) {
      if (vel.y != 0.f) { return; }
      vel.y = 1.f;
      vel.x = 0.f;
    }

    else if (IsKeyPressed(KEY_RIGHT)) {
      if (vel.x != 0.f) { return; }
      vel.x = 1.f;
      vel.y = 0.f;
    } else if (IsKeyPressed(KEY_LEFT)) {
      if (vel.x != 0.f) { return; }
      vel.x = -1.f;
      vel.y = 0.f;
    }
  }

  void update() {
    pos.x += vel.x;
    pos.y += vel.y;
  }

  void grow() {
    fmt::println("TODO: implement grow()");
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

private:
  std::vector<GridState> grid;
  size_t cell_size;
  Snake snake;
  uint64_t speed_ms;

  UniformRandom rand;
  uint64_t last_updated_ms = 0;

public:
  Grid(size_t width, size_t height, size_t size, uint64_t initial_speed)
      : width(width / size), height(height / size), cell_size(size),
        speed_ms(initial_speed) {
    snake.pos.x = this->width / 2.f;
    snake.pos.y = this->height / 2.f;

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

  void spawn_food() {
    const int x = rand.random() * this->width;
    const int y = rand.random() * this->height;
    fmt::println("spawn_food: ({}, {})", x, y);

    fmt::println("TODO: spawn_food handle collisions");
    at(x, y) = GRID_STATE_FOOD;
  }

  void handle_input() {
    snake.handle_input();
  }

  void update() {
    uint64_t cur_time = get_time_ms();
    uint64_t dt_ms = cur_time - last_updated_ms;
    if (dt_ms < speed_ms) { return; }
    last_updated_ms = cur_time;


    snake.update();
    GridState& snake_head_grid = at(snake.pos.x, snake.pos.y);

    switch (snake_head_grid) {
      case GRID_STATE_FOOD:
        snake.grow();
        snake_head_grid = GRID_STATE_EMPTY;
        spawn_food();
        speed_ms *= 0.9f;
        break;

      case GRID_STATE_EMPTY:
      default:
        break;
    }
  }

  void draw() {
    /**
     * Draw the grid.
     */
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++ ) {
        DrawRectangleV(
          cell_at(x, y),
          { .x = (float) cell_size, .y = (float) cell_size },
          get_color(at(x, y))
        );
      }
    }

    /**
     * Draw the snake.
     */
    Vector2 snake_cell = cell_at(snake.pos.x, snake.pos.y);
    DrawRectangleV(snake_cell, {(float)cell_size, (float)cell_size}, GREEN);
  }
};


int main() {
  InitWindow(WIDTH, HEIGHT, "Snake");
  SetTargetFPS(60);

  const uint64_t initial_speed = 500;
  Grid grid(WIDTH, HEIGHT, 20, initial_speed);
  grid.spawn_food();

  while (!WindowShouldClose()) {
    // IO
    {
      if (IsKeyPressed(KEY_Q)) {
        CloseWindow();
        break;
      }
      grid.handle_input();
    }

    // UPDATE
    {
      grid.update();
    }

    // DRAW
    {
      BeginDrawing();
      ClearBackground(BLACK);

      grid.draw();

      DrawFPS(WIDTH - 100.f, 10.f);
      EndDrawing();
    }
  }

  CloseWindow();
  return 0;
}
