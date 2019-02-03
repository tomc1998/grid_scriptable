#pragma once

void test_insert() {
  grid g;
  assert_eq(g.entity_list.size(), 0UL);
  assert_true(g.insert(1, 1, 1, true));
  assert_eq(g.entity_list.size(), 1UL);
  assert_true(g.insert(2, 2, 2, true));
  assert_eq(g.entity_list.size(), 2UL);

  // test solid collides with non-solid
  assert_false(g.insert(3, 2, 2, true));
  assert_eq(g.entity_list.size(), 2UL);

  // Test non solid on top of solid
  assert_true(g.insert(4, 2, 2, false));
  assert_eq(g.entity_list.size(), 3UL);

  // test solid on top of non solid
  assert_true(g.insert(5, 3, 3, false));
  assert_eq(g.entity_list.size(), 4UL);
  assert_true(g.insert(6, 3, 3, true));
  assert_eq(g.entity_list.size(), 5UL);
  assert_false(g.insert(7, 3, 3, true));
  assert_eq(g.entity_list.size(), 5UL);
}

void test_lookup() {
  grid g;
  g.insert(1, 1, 1, true);
  g.insert(2, 2, 2, true);
  g.insert(3, 3, 3, false);
  assert_eq(g.find_solid_entity(1, 0), nullptr);
  assert_ne(g.find_solid_entity(1, 1), nullptr);
  assert_eq(g.find_solid_entity(1, 1)->e, 1);
  assert_ne(g.find_solid_entity(2, 2), nullptr);
  assert_eq(g.find_solid_entity(2, 2)->e, 2);
  assert_eq(g.find_solid_entity(3, 3), nullptr);

  assert_ne(g.find_entity(1), nullptr);
  assert_eq(g.find_entity(4), nullptr);
}

void test_move() {
  grid g;
  g.insert(1, 1, 1, true);
  g.insert(2, 2, 1, true);
  g.insert(3, 3, 1, false);
  g.insert(4, 2, 2, true);
  g.insert(5, 3, 2, false);

  // Check solid blocking
  assert_false(g.move(1, 1, 0));
  assert_false(g.move(2, -1, 0));

  // Check we can't move if we're already moving
  assert_true(g.move(1, 0, 1));
  assert_false(g.move(1, 0, 1));
  assert_eq(g.find_solid_entity(1, 3), nullptr);

  // Check we can move solid onto non-solid
  assert_true(g.move(2, 1, 0));
  // And non-solid onto solit
  assert_true(g.move(5, -1, 0));

  // Check the moving list is the right length
  assert_eq(g.moving_list.size(), 3UL);

  // Now make sure all the final positions are correct
  assert_eq(g.find_entity(1)->x, (gint_t)1);
  assert_eq(g.find_entity(1)->y, (gint_t)2);
  assert_eq(g.find_entity(2)->x, (gint_t)3);
  assert_eq(g.find_entity(2)->y, (gint_t)1);
  assert_eq(g.find_entity(3)->x, (gint_t)3);
  assert_eq(g.find_entity(3)->y, (gint_t)1);
  assert_eq(g.find_entity(4)->x, (gint_t)2);
  assert_eq(g.find_entity(4)->y, (gint_t)2);
  assert_eq(g.find_entity(5)->x, (gint_t)2);
  assert_eq(g.find_entity(5)->y, (gint_t)2);
}

std::vector<test> test_grid_all_tests() {
  return {
    { "grid_insert", test_insert },
    { "grid_lookup", test_lookup },
    { "grid_move", test_move },
  };
}
