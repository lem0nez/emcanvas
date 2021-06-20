// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#pragma once

#include <deque>
#include <SDL2/SDL_rect.h>

class Algorithms {
public:
  // Bresenham's line algorithm.
  static auto get_line_points(
      const SDL_Point& first_point, const SDL_Point& last_point,
      bool exclude_passed = false) -> std::deque<SDL_Point>;
};
