// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#include <stdexcept>
#include <SDL2/SDL_image.h>

#include "app.hpp"
#include "brush.hpp"

using namespace std;

Brush::Brush(const Color t_color):
    m_size(static_cast<int>(BASE_BRUSH_SIZE * App::get_pixel_ratio())) {

  m_brushes.reset(IMG_Load("brushes.png"));
  if (!m_brushes) {
    throw runtime_error(IMG_GetError());
  }
  // Make drawing of transparent pixels faster.
  SDL_SetSurfaceRLE(m_brushes.get(), 1);
  set_color(t_color);
}

void Brush::set_color(const Color t_color) {
  m_brush_rect = {
    static_cast<int>(t_color) * m_brushes->h, 0,
    m_brushes->h, m_brushes->h
  };
}

void Brush::draw(shared_ptr<SDL_Surface> t_dest, const SDL_Point& t_pos) {
  SDL_Rect dest_rect{
    t_pos.x - m_size / 2, t_pos.y - m_size / 2,
    m_size, m_size
  };
  SDL_BlitScaled(m_brushes.get(), &m_brush_rect, t_dest.get(), &dest_rect);
}
