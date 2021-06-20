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

  Brush() = default;
  explicit Brush(Color);

  void set_color(Color);
  void draw(std::shared_ptr<SDL_Surface> dest, const SDL_Point& pos);

private:
  std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>
      m_brushes{nullptr, SDL_FreeSurface};
  // Points to the selected brush in surface.
  SDL_Rect m_brush_rect{};
};
