#pragma once

#define GRID_SIZE 16.0f

/** The type of a grid index. */
using gint_t = int16_t;

inline float grid_to_world(gint_t v) {
  return v * GRID_SIZE;
}

/** Snaps a world position to the grid - truncates. Asserts that the magnitude
    of truncated v can fit into a gint_t. */
inline gint_t world_to_grid(float v) {
  float ret = v / GRID_SIZE;
  assert(ret < (float)std::numeric_limits<gint_t>::max()
         && "World position magnitude too large for grid.");
  assert(ret > -(float)std::numeric_limits<gint_t>::max()
         && "World position magnitude too large for grid.");
  return (gint_t)ret;
}

struct grid_entity {
  /** The entity */
  int e;
  float transition_speed = 2.0;
  gint_t x;
  gint_t y;
  bool is_solid : 1;
  bool is_moving : 1;
};

/**
   A grid.

   This manages a list of entities, applying transforms and responding to
   'collisions', limiting them to a grid (where each entity takes up 1 square on
   the grid).

   Once an entity is part of the grid, the grid 'assumes control' of the
   entities position, and also handles slowly transposing entities between grid
   squares.

   An entity can be solid or non-solid. Only 1 solid entity can be in a square
   at any one point.

   Most of the functions will return false (or nullopt) if a grid square is
   full.

   Entities on the grid shouldn't have a velocity, or any other system
   controlling their movement - the grid acts on entity positions, and so
   can snap an entity back to the grid at any moment.

   See grid_ecs.hpp for functions which integrate this with the ECS.
 */
struct grid {
  /** Just model the grid as a linear list for now. */
  std::deque<grid_entity> entity_list;
  /** Holds a list of pointers into the entity list, which are the entities that
  are currently moving. These should be processed every step to slowly
  transition an entity to its target grid square.*/
  std::vector<grid_entity*> moving_list;

  grid() : entity_list() {};

  /** Find a solid entity at a given point, or nullptr if not available. */
  const grid_entity* find_solid_entity(gint_t x, gint_t y) const {
    for (const auto& e : entity_list) {
      if (e.x == x && e.y == y && e.is_solid) {
        return &e;
      }
    }
    return nullptr;
  }

  const grid_entity* find_entity(int e) const {
    for (const auto& ge : entity_list) {
      if (ge.e == e) {
        return &ge;
      }
    }
    return nullptr;
  }


  /** Insert an entity. Return false if insertion is impossible. */
  bool insert(int e, gint_t x, gint_t y, bool is_solid) {
    if (is_solid && find_solid_entity(x, y) != nullptr) {
      return false;
    }
    entity_list.push_back({e, 2.0, x, y, is_solid});
    return true;
  }

  /** Insert an entity. Return false if insertion is impossible, or if alreayd
      moving. Asserts if entity is not in the grid.

      The API present here is merely due to ease of use - entities are still
      constrained to moving 1 square at a time in the cardinal directions.

      NOTE To support more than 4 directions, we need to fix step() - see step's 'todo'.
  */
  bool move(int e, gint_t x, gint_t y) {
    assert (!(x == 0 && y == 0) && "Both x and y for a movement are 0.");
    assert((x == 0 || x == 1 || x == -1) && "x can only be 0, 1, or -1");
    assert((y == 0 || y == 1 || y == -1) && "y can only be 0, 1, or -1");

    // Find the entity
    grid_entity *grid_entity = nullptr;
    for (auto& ge : entity_list) {
      if (ge.e == e) {
        grid_entity = &ge;
        break;
      }
    }
    assert(grid_entity != nullptr);

    // Check we're not already moving
    if (grid_entity->is_moving) {
      return false;
    }

    // Check the area we're moving to is empty
    if (grid_entity->is_solid && find_solid_entity(grid_entity->x + x, grid_entity->y + y) != nullptr) {
      return false;
    }

    // Otherwise, move the entity
    if (!grid_entity->is_moving) {
      moving_list.push_back(grid_entity);
      grid_entity->is_moving = true;
    }
    grid_entity->x += x;
    grid_entity->y += y;
    return true;
  }
};
