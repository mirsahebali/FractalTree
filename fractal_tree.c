#include "raylib.h"
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SQR(x) ((x) * (x))
#define TO_RADIAN(deg) (deg * (PI / 180))
#define CMP_VEC2(vec1, vec2) ((vec1).x == (vec2).x && (vec1).y == (vec2).y)

#define RECT(x, y, w, h) ((Rectangle){(x), (y), (w), (h)})
#define WITHIN_RECT(rect, pos)                                                 \
  (((pos).x > (rect).x && (pos).x < (rect).x + (rect).width) &&                \
   ((pos).y > (rect).y && (pos).y < (rect).y + (rect).height))

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

typedef struct {
  Vector2 start;
  Vector2 end;
} Line;

#if defined(PLATFORM_WEB)
int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
#else
int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;
#endif

int font_size = 20;
Vector2 start_point;
Vector2 end_point;
Line seed_line;
float scale_factor = 0.9;
float angle = 35.0;
float initial_len = 50.0;
float depth = 5.0f;
int grabbed = 0;
Camera2D camera = {0};

inline int line_length(Line line) {
  return sqrt(SQR(line.end.x - line.start.x) + SQR(line.end.y - line.start.y));
}

static void DrawFractalTree(Vector2 point, float len, int direction,
                            int depth) {

  Vector2 new_point = {};
  new_point.x = point.x + (len * (cos(TO_RADIAN(direction))));
  new_point.y = point.y - (len * (sin(TO_RADIAN(direction))));
  DrawLineEx(point, new_point, 2.0, BLACK);

  if (depth > 0) {
    DrawFractalTree(new_point, len * scale_factor, direction - angle,
                    depth - 1);
    DrawFractalTree(new_point, len * scale_factor, direction + angle,
                    depth - 1);
  }
}

static void GameLoop(void) {
  start_point = (Vector2){SCREEN_WIDTH / 2.0, SCREEN_HEIGHT};
  end_point = (Vector2){.x = start_point.x, .y = start_point.y - initial_len};
  seed_line = (Line){start_point, end_point};

  float dt = GetFrameTime();
  Vector2 mouse_pos = GetMousePosition();
#if defined(PLATFORM_WEB)
  float container_width = SCREEN_WIDTH / 3.5;
  ;
#else
  float container_width = SCREEN_WIDTH / 2.8;

#endif /* if defined(PLATFORM_WEB) */

  Rectangle controls_container = {10, 10, container_width, 170};

  if (IsKeyDown(KEY_H)) {
    camera.offset.x -= 100 * dt;
  }

  if (IsKeyDown(KEY_L)) {
    camera.offset.x += 100 * dt;
  }
  if (IsKeyDown(KEY_J)) {
    camera.offset.y += 100 * dt;
  }

  if (IsKeyDown(KEY_K)) {
    camera.offset.y -= 100 * dt;
  }

  if (IsKeyDown(KEY_UP)) {
    scale_factor += 0.5 * dt;
  }
  if (IsKeyDown(KEY_DOWN)) {
    scale_factor -= 0.5 * dt;
  }

  if (IsKeyDown(KEY_LEFT)) {
    angle += 90 * dt;
  }
  if (IsKeyDown(KEY_RIGHT)) {
    angle -= 90 * dt;
  }

  if (IsKeyPressed(KEY_A)) {
    depth += 1;
  }
  if (IsKeyPressed(KEY_D)) {
    if (depth > 0)
      depth -= 1;
  }

  if (IsKeyDown(KEY_W)) {
    initial_len += 10 * dt;
  }

  if (IsKeyDown(KEY_S)) {
    initial_len -= 10 * dt;
  }

  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
      !WITHIN_RECT(controls_container, mouse_pos)) {
    grabbed = 1;
  }

  if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
    grabbed = 0;
  }

  Vector2 mouse_delta = GetMouseDelta();
  if (grabbed) {
    camera.offset.x += mouse_delta.x;
    camera.offset.y += mouse_delta.y;
  }
  camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove() * 0.1f));

  BeginDrawing();
  ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

  BeginMode2D(camera);
  DrawFractalTree(start_point, initial_len, 90, depth);
  EndMode2D();

  DrawRectangleRec(controls_container, Fade(SKYBLUE, 0.5f));
  DrawRectangleLinesEx(controls_container, 3, BLUE);

  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x686868ff);
  GuiSetStyle(DEFAULT, TEXT_SIZE, 12);

  GuiGroupBox(RECT(40, 20, 150, 60), "Angle");
  GuiSlider(RECT(40 + 20, 20 + 20, 100, 20), "0", "360", &angle, 0, 360);

  GuiGroupBox(RECT(40 + 150 + 20, 20, 150, 60), "Depth");
  GuiSlider(RECT(40 + 150 + 20 + 20, 20 + 20, 100, 20), "0", "15", &depth, 0,
            15);

  GuiGroupBox(RECT(40, 90, 150, 60), "Scale Factor");
  GuiSlider(RECT(40 + 20, 110, 100, 20), "0.1", "4.0", &scale_factor, 0.1, 4.0);

  GuiGroupBox(RECT(40 + 150 + 20, 90, 150, 60), "Trunk Length");
  GuiSlider(RECT(40 + 150 + 20 + 20, 110, 100, 20), "0", "100", &initial_len, 0,
            100);

  controls_container.y += controls_container.height + 10;
  controls_container.height += 30;
  DrawRectangleRec(controls_container, Fade(SKYBLUE, 0.5f));
  DrawRectangleLinesEx(controls_container, 3, BLUE);
  controls_container.x += 10;
  DrawText("Controls: ", controls_container.x, controls_container.y + 10,
           font_size, BLACK);
  DrawText("- Right/Left: angle of rotation", controls_container.x,
           controls_container.y + 50, font_size, DARKGRAY);
  DrawText("- Up/Down: scaling factor of branch size", controls_container.x,
           controls_container.y + 70, font_size, DARKGRAY);
  DrawText("- A / S: depth", controls_container.x, controls_container.y + 90,
           font_size, DARKGRAY);
  DrawText("- W / D: initial length of the branch", controls_container.x,
           controls_container.y + 110, font_size, DARKGRAY);

  DrawText("- HJKL to move camera(VIM motions)", controls_container.x,
           controls_container.y + 130, font_size, DARKGRAY);

  DrawText("- Left Mouse Click and drag move camera", controls_container.x,
           controls_container.y + 150, font_size, DARKGRAY);

  DrawText("- Mouse scroll: adjust zoom", controls_container.x,
           controls_container.y + 170, font_size, DARKGRAY);

  EndDrawing();
}

int main(int argc, char *argv[]) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fractal Tree");
  SCREEN_WIDTH = GetScreenWidth();
  SCREEN_HEIGHT = GetScreenHeight();

  camera.zoom = 1.0f;
  camera.target = (Vector2){0, 0};
  camera.offset = (Vector2){0, -SCREEN_HEIGHT / 2.0};
  camera.rotation = 0.0f;

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(GameLoop, 60, 1);
#else

  while (!WindowShouldClose()) {
    GameLoop();
  }
#endif
  CloseWindow();

  return 0;
}
