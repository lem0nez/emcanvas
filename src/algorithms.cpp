// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#include <cmath>
#include <utility>

#include "algorithms.hpp"

using namespace std;

// Source: https://stackoverflow.com/a/11683720.
auto Algorithms::get_line_points(
    const SDL_Point& t_first_point, const SDL_Point& t_last_point,
    const bool t_exclude_passed) -> deque<SDL_Point> {

  using coord_t = decltype(SDL_Point::x);
  const coord_t
      w = t_last_point.x - t_first_point.x,
      h = t_last_point.y - t_first_point.y,
      dx1 = (w < 0) ? -1 : ((w > 0) ? 1 : 0),
      dy1 = (h < 0) ? -1 : ((h > 0) ? 1 : 0);

  coord_t
      dx2 = dx1,
      dy2 = 0,
      longest = abs(w),
      shortest = abs(h);

  if (longest <= shortest) {
    swap(longest, shortest);

    dx2 = 0;
    if (h != 0) {
      dy2 = (h < 0) ? -1 : 1;
    }
  }

  deque<SDL_Point> points;
  coord_t
      x = t_first_point.x,
      y = t_first_point.y,
      numerator = longest >> 1U;

  for (coord_t i = 0; i <= longest; ++i) {
    points.push_back({x, y});
    numerator += shortest;

    if (numerator < longest) {
      x += dx2;
      y += dy2;
    } else {
      numerator -= longest;
      x += dx1;
      y += dy1;
    }
  }

  // Following modifies have constant complexity for deque.
  if (t_exclude_passed) {
    // First point always exists.
    points.pop_front();
    if (!points.empty()) {
      points.pop_back();
    }
  }
  return points;
}
