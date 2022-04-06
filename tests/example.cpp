#include <catch2/catch.hpp>
TEST_CASE( "Example", "[example]" ) {
    REQUIRE( 1 == 1 );
    REQUIRE( 2 == 2 );
    REQUIRE( 3 == 6 );
    REQUIRE( 10 == 10 );
}
