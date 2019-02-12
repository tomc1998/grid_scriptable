/**
   Handles the tracking of the focused grid square
 */

#pragma once

struct grid_focus {
  gint_t x;
  gint_t y;

  float world_x() {
    return world_to_grid(x);
  }
  float world_y() {
    return world_to_grid(y);
  }

} grid_focus_state;

void update_grid_focus() {
  const auto& iman = input_manager::get();
  grid_focus_state.x = world_to_grid((float)iman.mouse_x);
  grid_focus_state.y = world_to_grid((float)iman.mouse_y);
}
