// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#pragma once

#include <memory>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

#include "brush.hpp"

class App {
public:
  // Maximum ratio between physical and logical pixel sizes.
  static constexpr auto MAX_PIXEL_RATIO = 2.0;

  App();

  // Passing 0 gives control over the frame rate to browser (that
  // makes animations smoother) and 1 to simulate infinity loop.
  inline void exec() { emscripten_set_main_loop_arg(call_loop, this, 0, 1); };
  void clear_drawing_surface();
  inline void set_brush_color(const Brush::Color color)
      { m_brush->set_color(color); }

  [[nodiscard]] inline auto get_status() const { return m_status; }
  [[nodiscard]] static auto get_pixel_ratio() -> double;

private:
  // Using wrapper around the main loop function
  // as Emscripten takes only static functions.
  static inline void call_loop(void* instance)
      { static_cast<App*>(instance)->loop(); }
  static inline void quit() { emscripten_cancel_main_loop(); }

  void loop();
  void handle_events();

  static auto call_resizer(
      int event_type, const EmscriptenUiEvent* event, void* app) -> EM_BOOL;
  void resize(int width, int height);

  void clear_surface(SDL_Surface&) const;

  static constexpr SDL_Color
      BACKGROUND_LIGHT{0xEEU, 0xEEU, 0xEEU, SDL_ALPHA_OPAQUE},
      BACKGROUND_DARK{0x21U, 0x21U, 0x21U, SDL_ALPHA_OPAQUE};

  const bool m_dark_scheme_preferred;
  int m_status;

  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>
      m_win{nullptr, SDL_DestroyWindow};
  std::shared_ptr<SDL_Renderer> m_renderer;

  SDL_Rect m_visible_drawing_area{};
  std::shared_ptr<SDL_Surface> m_drawing_surface;
  std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>
      m_drawing_texture{nullptr, SDL_DestroyTexture};

  SDL_Event m_last_event{};
  bool m_mouse_left_button_pressed{};

  // Using pointer, because the constructor call must be caught.
  std::unique_ptr<Brush> m_brush;
  // Used to connect two points as SDL's events handler
  // isn't so fast to detect mouse shift by every pixel.
  SDL_Point m_last_point{-1, -1};
};
