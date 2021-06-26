// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#pragma once

#include <memory>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>

class Brush {
public:
  // Items order must be equal to brushes order in image.
  enum class Color {
    DARK,
    LIGHT
  };

  explicit Brush(Color);
  void set_color(Color);
  void draw(SDL_Surface& dest, const SDL_Point& pos);

private:
  static constexpr auto BASE_BRUSH_SIZE = 10;

  const int m_size;
  std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>
      m_brushes{nullptr, SDL_FreeSurface};
  // Points to the selected brush in surface.
  SDL_Rect m_brush_rect{};
};
