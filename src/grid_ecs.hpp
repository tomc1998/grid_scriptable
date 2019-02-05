/** Contains code to tie the grid to the ecs. The grid is partially split this
way to work around forward declarations. */
#pragma once

/** Step the moving entities */
void grid_step(grid& g, entt::DefaultRegistry& r) {
  for (unsigned ii = 0; ii < g.moving_list.size(); ++ii) {
    // Find the (world) position we SHOULD be at
    auto &e = *g.moving_list[ii];
    float world_x = grid_to_world(e.x);
    float world_y = grid_to_world(e.y);

    // Lookup the entity's position
    auto& pos = r.get<cpos>(e.e);

    // Move the entity at the given transition speed.
    // TODO We're assuming that we're moving in one of the cardinal
    // directions, and therefore are using the manhattan distance rather than
    // the euclidean distance. This needs to be changed if we want movement in
    // more than 4 directions.
    float dis = std::abs(world_x - pos.vec.x) + std::abs(world_y - pos.vec.y);
    if (dis < e.transition_speed) {
      // Stop moving this entity
      pos.vec.x = world_x;
      pos.vec.y = world_y;
      e.is_moving = false;
      g.moving_list.erase(g.moving_list.begin() + ii);
      ii --;
    } else {
      // Step
      pos.vec.x += e.transition_speed * (world_x - pos.vec.x) / dis;
      pos.vec.y += e.transition_speed * (world_y - pos.vec.y) / dis;
    }
  }
}

/** Snap all the entities to the grid. */
void grid_snap(const grid& g, entt::DefaultRegistry& r) {
  for (const auto &e : g.entity_list) {
    auto& pos = r.get<cpos>(e.e);
    pos.vec.x = grid_to_world(e.x);
    pos.vec.y = grid_to_world(e.y);
  }
}
