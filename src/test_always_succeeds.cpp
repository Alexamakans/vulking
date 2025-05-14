#include <catch2/catch_all.hpp>

// ./test "[tag_used_for_filtering]"
// ./test "~[tag_used_for_filtering]"
TEST_CASE("Test 1", "[tag_used_for_filtering]") { REQUIRE(true); }
TEST_CASE("Test 2", "[never_run]") { REQUIRE(false); }
