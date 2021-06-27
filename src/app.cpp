// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "algorithms.hpp"
#include "app.hpp"

using namespace std;

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
EM_JS(bool, is_dark_scheme_preferred, (), {
  return window.matchMedia &&
         window.matchMedia('(prefers-color-scheme: dark)').matches;
});

App::App(): m_dark_scheme_preferred(is_dark_scheme_preferred()),
            m_status(EXIT_FAILURE) {
  if (!init_subsystems()) {
    return;
  }

  int win_width = 0, win_height = 0;
  const auto em_result =
      emscripten_get_canvas_element_size("canvas", &win_width, &win_height);
  if (em_result != EMSCRIPTEN_RESULT_SUCCESS) {
    cerr << "Couldn't get size of the canvas element" << endl;
    return;
  }

  m_win.reset(SDL_CreateWindow(emscripten_get_window_title(),
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      win_width, win_height, 0U));
  if (!m_win) {
    cerr << "Couldn't create window: " << SDL_GetError() << endl;
    return;
  }

  m_renderer = shared_ptr<SDL_Renderer>(
      // Passing -1 to initialize the first renderer.
      SDL_CreateRenderer(m_win.get(), -1, SDL_RENDERER_ACCELERATED),
      SDL_DestroyRenderer);
  if (!m_renderer) {
    cerr << "Couldn't create renderer: " << SDL_GetError() << endl;
    return;
  }

  m_visible_drawing_area = {0, 0, win_width, win_height};

  unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>
      win_surface(SDL_GetWindowSurface(m_win.get()), SDL_FreeSurface);
  if (!win_surface) {
    cerr << "Couldn't get window surface: " << SDL_GetError() << endl;
    return;
  }

  m_drawing_surface = shared_ptr<SDL_Surface>(SDL_CreateRGBSurface(
      // Passing no flags as their are unused.
      0U, win_width, win_height, win_surface->format->BitsPerPixel,
      win_surface->format->Rmask, win_surface->format->Gmask,
      win_surface->format->Bmask, win_surface->format->Amask), SDL_FreeSurface);
  if (!m_drawing_surface) {
    cerr << "Couldn't create the drawing surface: " << SDL_GetError() << endl;
    return;
  }

  m_drawing_texture.reset(
      SDL_CreateTextureFromSurface(m_renderer.get(), m_drawing_surface.get()));
  if (!m_drawing_texture) {
    cerr << "Couldn't create texture from the drawing surface: " <<
            SDL_GetError() << endl;
    return;
  }
  clear_drawing_surface();

  try {
    m_brush = make_unique<Brush>(
        m_dark_scheme_preferred ? Brush::Color::LIGHT : Brush::Color::DARK);
  } catch (const exception& e) {
    cerr << "Couldn't prepare brush: " << e.what() << endl;
    return;
  }

  if (!load_startup_msg()) {
    return;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
  emscripten_set_resize_callback(
      EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, call_resizer);
  m_status = EXIT_SUCCESS;
}

auto App::init_subsystems() -> bool {
  // Event handling is initialized along with video.
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    cerr << "Couldn't initialize the SDL library: " << SDL_GetError() << endl;
    return false;
  }
  if (atexit(SDL_Quit) != 0) {
    cerr << "Couldn't register the SDL cleanup function" << endl;
    return false;
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
    cerr << "Couldn't initialize the SDL_image library: " <<
            IMG_GetError() << endl;
    return false;
  }
  if (atexit(IMG_Quit) != 0) {
    cerr << "Couldn't register the SDL_image cleanup function" << endl;
    return false;
  }

  if (TTF_Init() != 0) {
    cerr << "Couldn't initialize the SDL_ttf library: " <<
            TTF_GetError() << endl;
    return false;
  }
  if (atexit(TTF_Quit) != 0) {
    cerr << "Couldn't register the SDL_ttf cleanup function" << endl;
    return false;
  }

  return true;
}

auto App::load_startup_msg() -> bool {
  constexpr auto PT_SIZE = 72;
  constexpr auto MESSAGE = "Just a canvas.";
  constexpr SDL_Color
      LIGHT_COLOR = BACKGROUND_LIGHT,
      DARK_COLOR{0x42U, 0x42U, 0x42U, SDL_ALPHA_OPAQUE};

  unique_ptr<TTF_Font, decltype(&TTF_CloseFont)>
      font{TTF_OpenFont("quicksand-light.ttf", PT_SIZE), TTF_CloseFont};
  if (!font) {
    cerr << "Couldn't load font of the startup message: " <<
            TTF_GetError() << endl;
    return false;
  }

  const auto color = m_dark_scheme_preferred ? LIGHT_COLOR : DARK_COLOR;
  unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface
      {TTF_RenderText_Blended(font.get(), MESSAGE, color), SDL_FreeSurface};
  if (!surface) {
    cerr << "Couldn't render the startup message: " << TTF_GetError() << endl;
    return false;
  }

  m_startup_msg.reset(
      SDL_CreateTextureFromSurface(m_renderer.get(), surface.get()));
  if (!m_startup_msg) {
    cerr << "Couldn't create texture of the startup message: " <<
            SDL_GetError() << endl;
    return false;
  }

  m_startup_msg_dest = {
    m_visible_drawing_area.w / 2 - surface->w / 2,
    m_visible_drawing_area.h / 2 - surface->h / 2,
    surface->w, surface->h
  };
  return true;
}

void App::hide_startup_msg() {
  // Is the startup message hidden?
  if (!m_startup_msg) {
    return;
  }

  Uint8 alpha = 0U;
  SDL_GetTextureAlphaMod(m_startup_msg.get(), &alpha);
  // Is the hide process not started?
  if (alpha == SDL_ALPHA_OPAQUE) {
    // Initiate the hide process.
    SDL_SetTextureAlphaMod(m_startup_msg.get(),
                           alpha - HIDE_STARTUP_MSG_ALPHA_STEP);
  }
}

void App::manage_startup_msg() {
  Uint8 alpha = 0U;
  SDL_GetTextureAlphaMod(m_startup_msg.get(), &alpha);

  // Nothing to manage?
  if (alpha == SDL_ALPHA_OPAQUE) {
    return;
  }

  // Is it end of the hide process?
  if (alpha < HIDE_STARTUP_MSG_ALPHA_STEP) {
    // Completely remove the startup message.
    m_startup_msg.reset();
  } else {
    // Increase transprence more.
    SDL_SetTextureAlphaMod(m_startup_msg.get(),
                           alpha - HIDE_STARTUP_MSG_ALPHA_STEP);
  }
}

void App::loop() {
  SDL_RenderClear(m_renderer.get());

  handle_events();

  SDL_RenderCopy(m_renderer.get(), m_drawing_texture.get(),
                 &m_visible_drawing_area, nullptr);
  if (m_startup_msg) {
    manage_startup_msg();
    SDL_RenderCopy(m_renderer.get(), m_startup_msg.get(),
                   nullptr, &m_startup_msg_dest);
  }
  SDL_RenderPresent(m_renderer.get());
}

void App::handle_events() {
  // Poll events until queue ends.
  while (SDL_PollEvent(&m_last_event) != 0) {
    SDL_Point mouse_pos{-1, -1};

    switch (m_last_event.type) {
      // Mouse events will also work with touchscreens.
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN: {
        m_mouse_left_button_pressed =
            (m_last_event.button.button == SDL_BUTTON_LEFT) &&
            (m_last_event.button.state == SDL_PRESSED);

        if (m_mouse_left_button_pressed) {
          mouse_pos.x = m_last_event.button.x;
          mouse_pos.y = m_last_event.button.y;
          // Don't connect new point with previous as user starts new drawing.
          m_last_point = {-1, -1};
        }
        [[fallthrough]];
      }
      case SDL_MOUSEMOTION: {
        if (!m_mouse_left_button_pressed) {
          break;
        }
        hide_startup_msg();

        // Did motion initiated the event?
        if (mouse_pos.x == -1) {
          mouse_pos.x = m_last_event.motion.x;
          mouse_pos.y = m_last_event.motion.y;
        }
        m_brush->draw(*m_drawing_surface, mouse_pos);

        // Should be the current point connected with previous?
        if (m_last_point.x != -1) {
          const auto points_between =
              Algorithms::get_line_points(m_last_point, mouse_pos, true);
          for (const auto& p : points_between) {
            m_brush->draw(*m_drawing_surface, p);
          }
        }
        m_last_point = mouse_pos;

        SDL_UpdateTexture(m_drawing_texture.get(), nullptr,
            m_drawing_surface->pixels, m_drawing_surface->pitch);
        break;
      }

      case SDL_QUIT: {
        quit();
        return;
      }
    }
  }
}

auto App::call_resizer(const int /* event_type */,
    const EmscriptenUiEvent* /* event */, void* t_app) -> EM_BOOL {
  static_cast<App*>(t_app)->resize(
      // Don't use members of the event object,
      // because they are provide incorrect values.
      EM_ASM_INT({ return document.documentElement.clientWidth; }), // NOLINT
      EM_ASM_INT({ return document.documentElement.clientHeight; })); // NOLINT
  return EM_TRUE;
}

void App::resize(const int t_width, const int t_height) {
  if (t_width == m_visible_drawing_area.w &&
      t_height == m_visible_drawing_area.h) {
    return;
  }
  hide_startup_msg();

  emscripten_set_canvas_element_size("canvas", t_width, t_height);
  SDL_SetWindowSize(m_win.get(), t_width, t_height);
  m_visible_drawing_area = {0, 0, t_width, t_height};

  // The drawing surface can only be made larger (to preserve all drawn items).
  if (t_width < m_drawing_surface->w && t_height < m_drawing_surface->h) {
    return;
  }

  const auto
      surface_width = max(t_width, m_drawing_surface->w),
      surface_height = max(t_height, m_drawing_surface->h);

  auto* new_surface = SDL_CreateRGBSurface(0U,
      surface_width, surface_height, m_drawing_surface->format->BitsPerPixel,
      m_drawing_surface->format->Rmask, m_drawing_surface->format->Gmask,
      m_drawing_surface->format->Bmask, m_drawing_surface->format->Amask);
  clear_surface(*new_surface);
  SDL_BlitSurface(m_drawing_surface.get(), nullptr, new_surface, nullptr);

  m_drawing_surface.reset(new_surface, SDL_FreeSurface);
  m_drawing_texture.reset(SDL_CreateTextureFromSurface(
      m_renderer.get(), m_drawing_surface.get()));
}

void App::clear_drawing_surface() {
  clear_surface(*m_drawing_surface);
  SDL_UpdateTexture(m_drawing_texture.get(), nullptr,
      m_drawing_surface->pixels, m_drawing_surface->pitch);
}

void App::clear_surface(SDL_Surface& t_surface) const {
  const auto background =
      m_dark_scheme_preferred ? BACKGROUND_DARK : BACKGROUND_LIGHT;
  const auto mapped_color = SDL_MapRGBA(t_surface.format,
      background.r, background.g, background.b, background.a);
  SDL_FillRect(&t_surface, nullptr, mapped_color);
}

auto App::get_pixel_ratio() -> double {
  static const auto ratio =
      min(emscripten_get_device_pixel_ratio(), MAX_PIXEL_RATIO);
  return ratio;
}
