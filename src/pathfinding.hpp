#pragma once

struct movement {
  gint_t x : 2;
  gint_t y : 2;
};

struct grid;

/** A grid point containing a grid position and a movement to get to this from the
    'parent' of the grid point in the A* path. */
struct grid_path_closed_point {
  gint_t x, y;
  /** 0,0 if this is the starting point */
  movement from;
};

/** An 'open' point on the graph, containing the movement cost to get here plus
    a heuristic & movement (see closed point) */
struct grid_path_open_point {
  gint_t x, y;
  float cost;
  float heuristic;
  movement from;
  /** Calculate the f score for this */
  float f() const {
    return heuristic + cost;
  }
};

/** Iterate the 8 squares neighbouring the given point with the given lambda.
    Clockwise starting from top middle. Lambda is passed x, y, c, where c is a
    floating point cost for moving to this tile (either 1 or sqrt(2)).
*/
template <typename F>
void iterate_neighbours(gint_t x, gint_t y, F fn) {
  const float root2 = std::sqrt(2.0);
  fn(x, y-1, 1.0);
  fn(x+1, y-1, root2);
  fn(x+1, y, 1.0);
  fn(x+1, y+1, root2);
  fn(x, y+1, 1.0);
  fn(x-1, y+1, root2);
  fn(x-1, y, 1.0);
  fn(x-1, y-1, root2);
}

inline gint_t len_heuristic(gint_t px, gint_t py, gint_t qx, gint_t qy) {
  // Euclidean dis
  gint_t xdis = qx - px;
  gint_t ydis = qy - py;
  return std::sqrt(xdis * xdis + ydis * ydis);
}

struct grid_path {
  std::vector<movement> moves;
  /** Construct a path given a grid from a point p to a point q. If no path is
      available (because there is no route), no path is constructed. */
  grid_path(const grid& g, gint_t px, gint_t py, gint_t qx, gint_t qy) {
    std::vector<grid_path_open_point> open;
    std::vector<grid_path_closed_point> closed;
    closed.push_back({px, py, {0, 0}});

    /** Return a pointer (or nullptr) to the smallest open tile. Null if there
        are no more open tiles. */
    const auto get_smallest_open = [&open]() {
      float smallest_f = std::numeric_limits<float>::max();
      const grid_path_open_point* smallest = nullptr;
      for (const auto &tile : open) {
        if (tile.f() < smallest_f) {
          smallest = &tile;
          smallest_f = tile.heuristic;
        }
      }
      return smallest;
    };

    const auto find_closed = [&closed](gint_t x, gint_t y) -> grid_path_closed_point* {
      const auto res = std::find_if(closed.begin(), closed.end(), [x, y](auto &tile) {
          return tile.x == x && tile.y == y;
        });
      if (res == closed.end()) {
        return nullptr;
      } else {
        return &*res;
      }
    };

    const auto find_open = [&open](gint_t x, gint_t y) -> grid_path_open_point* {
      const auto res = std::find_if(open.begin(), open.end(), [x, y](auto &tile) {
          return tile.x == x && tile.y == y;
        });
      if (res == open.end()) {
        return nullptr;
      } else {
        return &*res;
      }
    };

    // Add all the immediate neighbours
    iterate_neighbours
      (px, py,
       [&g, &open, qx, qy, px, py] (gint_t x, gint_t y, float c) {
        if (g.find_solid_entity(x, y) == nullptr) {
          open.push_back({x, y, c, (float)len_heuristic(x, y, qx, qy), {(gint_t)(x - px), (gint_t)(y - py)}});
        }
      });

    std::cout << "Num open = " << open.size() << std::endl;
    std::cout << "Num closed = " << closed.size() << std::endl;

    bool goal_found = false;
    while(!goal_found) {
      const auto smallest_p = get_smallest_open();
      if (!smallest_p) { return; }
      // First take a copy on the stack so we can use it later
      const auto smallest = *smallest_p;
      // Then move to closed
      closed.push_back({smallest.x, smallest.y, smallest.from});
      open.erase(open.begin() + std::distance((const grid_path_open_point*) open.data(), smallest_p));
      // now walk the adjacent positions
      iterate_neighbours
        (smallest.x, smallest.y,
         [&](gint_t x, gint_t y, float c) {
          // First, check if tile is the goal, or if we've found the goal in a
          // previous iter. If so, set 'goal_found' to true, add the goal to closed, and ret.
          if (goal_found) { return; }
          if (x == qx && y == qy) {
            goal_found = true;
            closed.push_back({x, y, {(gint_t)(x - smallest.x), (gint_t)(y - smallest.y)}});
          }
          // If tile is closed, ignore
          if (find_closed(x, y)) { return; }
          const auto open_tile = find_open(x, y);
          const auto cost = smallest.cost + c;
          if (open_tile) {
            // If open, adjust heuristic
            const auto f = open_tile->heuristic + cost;
            if (f < open_tile->f()) {
              open_tile->cost = cost;
              open_tile->from.x = x - smallest.x;
              open_tile->from.y = y - smallest.y;
            }
          } else {
            // Not in any list, add to open
            open.push_back({x, y, cost, (float)len_heuristic(x, y, qx, qy),
                  {(gint_t)(x - smallest.x), (gint_t)(y - smallest.y)}});
          }
        });
    }

    std::cout << "Num open = " << open.size() << std::endl;
    std::cout << "Num closed = " << closed.size() << std::endl;

    // Build the final path from the back
    gint_t curr_x = qx, curr_y = qy;
    std::vector<grid_path_closed_point> path;
    while(true) {
      if (curr_x == px && curr_y == py) {
        // We reached the start - stop!
        break;
      }
      // Find the tile
      const auto res = std::find_if(closed.begin(), closed.end(), [curr_x, curr_y](auto &tile) {
          return tile.x == curr_x && tile.y == curr_y;
        });
      assert(res != closed.end());
      // Push the tile and update the pos
      path.push_back(*res);
      curr_x -= res->from.x;
      curr_y -= res->from.y;
    }

    // Trace backwards along path and insert the moves
    for (int ii = path.size()-1; ii >= 0; --ii) {
      moves.push_back(path[ii].from);
    }
  }
};
