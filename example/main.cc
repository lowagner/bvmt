#include "test.h"

#include "raylib.h"

using namespace bvmt;

int main() {
    #ifndef NDEBUG
    run_tests();
    #endif

    window *Window = window::get();

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
        EndDrawing();
    }

    return 0;
}
