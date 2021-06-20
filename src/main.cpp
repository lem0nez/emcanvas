// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

#include <cstdlib>
#include <iostream>

#include "app.hpp"

// SDL requires arguments of the main function to
// make the application compatible with all platforms.
auto main(int /* argc */, char** /* argv */) -> int {
  App app;
  if (const auto status = app.get_status(); status != EXIT_SUCCESS) {
    std::cerr << "Failed to initialize the application" << std::endl;
    return status;
  }
  app.exec();
  return app.get_status();
}
