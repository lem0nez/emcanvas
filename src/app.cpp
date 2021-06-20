// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#include <cstdlib>
#include <emscripten/html5.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>

#include "algorithms.hpp"
#include "app.hpp"

using namespace std;

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
EM_JS(bool, is_dark_scheme_preferred, (), {
  return window.matchMedia &&
         window.matchMedia("(prefers-color-scheme: dark)").matches;
});

App::App(): m_dark_scheme_preferred(is_dark_scheme_preferred()),
            m_status(EXIT_FAILURE) {
  // Event handling is initialized along with video.
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    cerr << "Couldn't initialize the SDL library: " << SDL_GetError() << endl;
    return;
  }
  if (atexit(SDL_Quit) != 0) {
    cerr << "Couldn't register the SDL cleanup function" << endl;
    return;
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
    cerr << "Couldn't initialize the SDL_image library: " <<
            IMG_GetError() << endl;
    return;
  }
  if (atexit(IMG_Quit) != 0) {
    cerr << "Couldn't register the SDL_image cleanup function" << endl;
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
    m_brush = Brush(m_dark_scheme_preferred ?
                    Brush::Color::LIGHT : Brush::Color::DARK);
  } catch (const exception& e) {
    cerr << "Couldn't prepare brush: " << e.what() << endl;
    return;
  }

  m_status = EXIT_SUCCESS;
}

void App::loop() {
  SDL_RenderClear(m_renderer.get());

  handle_events();

  SDL_RenderCopy(m_renderer.get(), m_drawing_texture.get(), nullptr, nullptr);
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

        // Did motion initiated the event?
        if (mouse_pos.x == -1) {
          mouse_pos.x = m_last_event.motion.x;
          mouse_pos.y = m_last_event.motion.y;
        }
        m_brush.draw(m_drawing_surface, mouse_pos);

        // Should be the current point connected with previous?
        if (m_last_point.x != -1) {
          const auto points_between =
              Algorithms::get_line_points(m_last_point, mouse_pos, true);
          for (const auto& p : points_between) {
            m_brush.draw(m_drawing_surface, p);
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

void App::clear_drawing_surface() {
  const auto background =
      m_dark_scheme_preferred ? BACKGROUND_DARK : BACKGROUND_LIGHT;
  const auto mapped_color = SDL_MapRGBA(m_drawing_surface->format,
      background.r, background.g, background.b, background.a);

  SDL_FillRect(m_drawing_surface.get(), nullptr, mapped_color);
  SDL_UpdateTexture(m_drawing_texture.get(), nullptr,
      m_drawing_surface->pixels, m_drawing_surface->pitch);
}
