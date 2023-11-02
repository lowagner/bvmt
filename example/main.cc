#include "../library/window.h"

#include "raylib.h"

using namespace bvmt;

int main() {
    window *Window = window::get();

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
        EndDrawing();
    }

    return 0;
}
