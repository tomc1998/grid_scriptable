#pragma once

#include <entt/entt.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <lua.hpp>
#include <png.h>

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <deque>

#include "input.hpp"
#include "load_image.hpp"
#include "script_test.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include "grid.hpp"
#include "pathfinding.hpp"
#include "components.hpp"
#include "shader.hpp"
#include "renderer/renderer.hpp"
#include "setup_context.hpp"
#include "grid_ecs.hpp"

#include "tests/test_runner.hpp"

void update(GLFWwindow* window, entt::DefaultRegistry &registry, grid& g) {
  registry.view<cpos, cvel>().each([](auto entity, cpos &pos, cvel &vel) {
      if (pos.vec.x > 800.0 || pos.vec.x < 0.0) { vel.vec.x = -vel.vec.x; }
      if (pos.vec.y > 600.0 || pos.vec.y < 0.0) { vel.vec.y = -vel.vec.y; }
      pos.vec.x += vel.vec.x;
      pos.vec.y += vel.vec.y;
    });
  const auto& iman = input_manager::get();
  const auto& cstate = iman.cstate;
  if (cstate.move.down) {
    registry.view<cpos, cplayer_controlled>()
      .each([&registry, &iman, &g](auto entity, const auto &pos, const auto &player_controlled) {
          const auto ge = g.find_entity(entity);
          assert(ge);
          gint_t gx = world_to_grid((float)iman.mouse_x);
          gint_t gy = world_to_grid((float)iman.mouse_y);
          if (registry.has<cpathfinder>(entity)) {
            // Set the new position
            auto& pf = registry.get<cpathfinder>(entity);
            pf.path = grid_path(g, ge->x, ge->y, gx, gy);
          } else {
            registry.assign<cpathfinder>(entity, grid_path(g, ge->x, ge->y, gx, gy));
          }
        });
  }
  registry.view<cpathfinder>().each([&g, &registry](auto entity, auto &pf) {
      const auto ge = g.find_entity(entity);
      assert(ge);
      if (ge->is_moving) { return; }
      if (pf.path.moves.size() <= 0) {
        registry.remove<cpathfinder>(entity);
        return;
      }
      g.move(entity, pf.path.moves[0].x, pf.path.moves[0].y);
      pf.path.moves.erase(pf.path.moves.begin());
    });
}

int main(int argc, char** argv) {
  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    return tests::run_all_tests();
  }

  // Create state
  entt::DefaultRegistry registry;
  grid g;

  for(auto ii = 0; ii < 4; ++ii) {
    auto e = registry.create();
    unsigned char color[4] {5, 5, (unsigned char)((rand() % 30) + 20), 255};
    float x = 800.0 * (float)rand() / (float)RAND_MAX;
    float y = 600.0 * (float)rand() / (float)RAND_MAX;
    registry.assign<cpos>(e, vec2 { x, y });
    registry.assign<cdebug_draw>(e, vec2 { 8.0f, 8.0f }, color);
    if (e == 0) {
      registry.assign<cplayer_controlled>(e);
    }
    g.insert(e, ii, ii, true);
  }

  grid_snap(g, registry);

  for(auto ii = 0; ii < 4; ++ii) {
    g.move(ii, 1, 1);
  }

  // Create window & renderer
  auto window = setup_context();
  renderer game_renderer(registry);

  // Setup input
  input_manager& iman = input_manager::create_instance(window);

  do_script_test();

  glClearColor(0.0, 0.0, 0.0, 1.0);

  while(!glfwWindowShouldClose(window)) {
    grid_step(g, registry);
    update(window, registry, g);
    game_renderer.render();
    iman.poll();
    glfwSwapBuffers(window);
  }
}
