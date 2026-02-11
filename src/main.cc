#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fmt/base.h>
#include <raylib.h>
#include "raymath.h"
#include <fmt/format.h>
#include <vector>

#define WIDTH 800
#define HEIGHT 800

enum GridState {
  GRID_STATE_EMPTY = 0,
  GRID_STATE_FOOD,
  GRID_STATE_BODY,
};

class Snake {
public:
  Vector2 pos;
  Vector2 vel;

  uint64_t last_updated_ms;


public:
  Snake() : pos(Vector2Zero()), vel(Vector2Zero()), last_updated_ms(0) {}
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

  uint64_t get_time_ms() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch())
        .count();
  }

  void update(uint64_t speed_ms) {
    uint64_t cur_time = get_time_ms();
    uint64_t dt_ms = cur_time - last_updated_ms;
    if (dt_ms < speed_ms) { return; }

    last_updated_ms = cur_time;

    pos.x += vel.x;
    pos.y += vel.y;
  }

};


class Grid {
private:
  std::vector<GridState> grid;
  size_t cell_size;
  Snake snake;

public:
  Grid(size_t width, size_t height, size_t size) {
    this->width = width / size;
    this->height = height / size;
    this->cell_size = size;

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
      case GRID_STATE_BODY:
        return GREEN;
    }
    assert(0);
  }

  Snake& get_snake() { return this->snake; }

  void update() {}

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

public:
  size_t width;
  size_t height;
};



class Food {};


int main() {
  InitWindow(WIDTH, HEIGHT, "Snake");
  SetTargetFPS(60);

  Grid grid(WIDTH, HEIGHT, 20);
  for (size_t y = 0; y < grid.height; y++) {
    for (size_t x = 0; x < grid.width; x++) {
      grid.at(x, y) = (x+y) % 2 == 0 ? GRID_STATE_FOOD : GRID_STATE_EMPTY;
    }
  }

  Snake& snake = grid.get_snake();

  while (!WindowShouldClose()) {
    // IO
    {
      if (IsKeyPressed(KEY_Q)) {
        CloseWindow();
        break;
      }
      snake.handle_input();
    }

    // UPDATE
    {
      snake.update(500);
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
