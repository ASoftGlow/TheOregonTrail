#include "input.h"
#include "screens.h"
#include "state.h"
#include "tui.h"
#include "utils.h"

#include "map_data.c"

#define MAP_VIEWPORT_WIDTH SCREEN_WIDTH

struct MapMark* findMark(byte path_id, byte pos);
void drawMap(Coord focus);
void drawMark(struct MapMark* mark, Coord offset);

extern byte MAP_VIEWPORT_HEIGHT;
Coord last_offset;

void
drawMap(Coord focus)
{
  bool overflow_up = 0, overflow_down = 0, overflow_left = 0, overflow_right = 0;

  Coord offset = { 0 };
  byte height, width;
  if (MAP_HEIGHT > MAP_VIEWPORT_HEIGHT)
  {
    overflow_up = focus.y > MAP_VIEWPORT_HEIGHT / 2;
    overflow_down = focus.y < MAP_HEIGHT - MAP_VIEWPORT_HEIGHT / 2 - 1;
    height = MAP_VIEWPORT_HEIGHT;
    if (overflow_up)
    {
      if (focus.y > MAP_HEIGHT - MAP_VIEWPORT_HEIGHT / 2)
      {
        offset.y = MAP_HEIGHT - MAP_VIEWPORT_HEIGHT;
      }
      else
      {
        offset.y = focus.y - MAP_VIEWPORT_HEIGHT / 2;
      }
    }
  }
  else
  {
    height = MAP_HEIGHT;
  }
  if (MAP_WIDTH > MAP_VIEWPORT_WIDTH)
  {
    overflow_left = focus.x > MAP_VIEWPORT_WIDTH / 2;
    overflow_right = focus.x < MAP_WIDTH - MAP_VIEWPORT_WIDTH / 2;
    width = MAP_VIEWPORT_WIDTH;
    if (overflow_left)
    {
      if (focus.x > MAP_WIDTH - MAP_VIEWPORT_WIDTH / 2)
      {
        offset.x = MAP_WIDTH - MAP_VIEWPORT_WIDTH;
      }
      else
      {
        offset.x = focus.x - MAP_VIEWPORT_WIDTH / 2;
      }
    }
  }
  else
  {
    width = MAP_WIDTH;
  }
  if (last_offset.x == offset.x && last_offset.y == offset.y)
  {
    setCursorPos(focus.x - offset.x, focus.y - offset.y + 1);
    return;
  }
  last_offset = offset;
  if (overflow_down) --height;
  if (overflow_right) --width;

  putsn(ANSI_CURSOR_HIDE ANSI_CURSOR_ZERO ANSI_CURSOR_POS("1", "2"));

  byte h = 0, w;
  if (overflow_up)
  {
    putsn(INDICATOR_COLOR);
    putsn(overflow_left ? INDICATOR_SLANT_LEFT : INDICATOR_UP);
    byte i = MAP_VIEWPORT_WIDTH - 2;
    while (i--) putsn(INDICATOR_UP);
    puts(overflow_right ? INDICATOR_SLANT_RIGHT : INDICATOR_UP);
    putsn(ANSI_COLOR_RESET);
    ++h;
  }

  while (h < height)
  {
    w = 0;
    if (overflow_left)
    {
      putsn(INDICATOR_LEFT);
      ++w;
    }

    while (w < width)
    {
      putchar(MAP[h + offset.y][w + offset.x]);
      ++w;
    }
    if (overflow_right)
    {
      putsn(INDICATOR_RIGHT);
    }
    putchar('\n');
    ++h;
  }

  if (overflow_down)
  {
    putsn(INDICATOR_COLOR);
    putsn(overflow_left ? INDICATOR_SLANT_RIGHT : INDICATOR_DOWN);
    byte i = MAP_VIEWPORT_WIDTH - 2;
    while (i--) putsn(INDICATOR_DOWN);
    puts(overflow_right ? INDICATOR_SLANT_LEFT : INDICATOR_DOWN);
    putsn(ANSI_COLOR_RESET);
    --h;
  }

  // marks
  putsn(ANSI_COLOR_MAGENTA);
  for (byte i = 0; i < state.map_marks_count; i++)
  {
    drawMark(&state.map_marks[i], offset);
  }
  putsn(ANSI_COLOR_RESET);

  setCursorPos(focus.x - offset.x, focus.y - offset.y + 1);
}

void
drawMark(struct MapMark* mark, Coord offset)
{
  const Coord pos = MAP_PATHS[map_paths_order[mark->path_index]][mark->pos];
  if ((MAP_WIDTH > MAP_VIEWPORT_WIDTH && (pos.x > offset.x + MAP_VIEWPORT_WIDTH - 2 || pos.x < offset.x + 1))
      || (MAP_HEIGHT > MAP_VIEWPORT_HEIGHT && (pos.y > offset.y + MAP_VIEWPORT_HEIGHT - 2 || pos.y < offset.y + 1)))
    return;
  setCursorPos(pos.x - offset.x, pos.y - offset.y + 1);
  putsn(DENSITY_INDICATORS[mark->density]);
}

void
screen_map(void)
{
  if (!state.map_viewed)
  {
    showInfoDialog(
        "Map Tips", "Use arrow keys to pan map\nPress m key to enter mark mode\n  Use up/right and down/left arrows to "
                    "traverse path\n  Press enter key to make a mark\n  Press m key again or escape key to exit mark mode"
    );
    if (HALT) return;
    state.map_viewed = 1;
  }

  byte path_pos, path_id, path_index;
  Coord pan_pos = { MAP_VIEWPORT_WIDTH / 2, MAP_VIEWPORT_HEIGHT / 2 };
  bool marking = 0;
  last_offset.x = -1;

  if (state.map_marks_count)
  {
    struct MapMark* last_mark = &state.map_marks[state.map_marks_count - 1];
    path_index = last_mark->path_index;
    path_id = map_paths_order[path_index];
    path_pos = last_mark->pos;
  }
  else
  {
    path_pos = 0;
    path_index = 0;
    path_id = map_paths_order[0];
  }

  clearStdout();
  putsn("Map of Oregon Trail");
  drawMap(pan_pos);
  setCursorPos(0, MAP_VIEWPORT_HEIGHT + 1);
  putsn("Press SPACE to exit" ANSI_CURSOR_SAVE);
  putsn(ANSI_CURSOR_STYLE_BLOCK);
  fflush(stdout);

  while (1)
  {
    int key = getKeyInput();

    if (KEY_IS_TERMINATING(key)) return;
    if (KEY_IS_ARROW(key) && marking)
    {
      switch (key)
      {
      case KEY_ARROW_UP:
      case KEY_ARROW_RIGHT:
        if (path_pos + 1 < MAP_PATH_LENGTHS[path_id])
        {
          ++path_pos;
        }
        else if (path_index < countof(map_paths_order) - 1)
        {
          path_id = map_paths_order[++path_index];
          path_pos = 0;
        }
        else continue;
        break;

      case KEY_ARROW_DOWN:
      case KEY_ARROW_LEFT:
        if (path_pos)
        {
          --path_pos;
        }
        else if (path_index)
        {
          path_id = map_paths_order[--path_index];
          path_pos = MAP_PATH_LENGTHS[path_id] - 1;
        }
        else continue;
        break;

      case KEY_PAGE_UP:
        path_id = countof(MAP_PATHS) - 1;
        path_pos = MAP_PATH_LENGTHS[path_id] - 1;
        break;

      case KEY_PAGE_DOWN:
        path_id = 0;
        path_pos = 0;
        break;
      }
      drawMap(MAP_PATHS[path_id][path_pos]);
      putsn(ANSI_CURSOR_SHOW);
      fflush(stdout);
      continue;
    }

    switch (key)
    {
    case 'm':
      if (!marking)
      {
        marking = 1;
        drawMap(MAP_PATHS[path_id][path_pos]);
        putsn(ANSI_CURSOR_SHOW);
        fflush(stdout);
        break;
      }

    case ESC_CHAR:
      if (marking)
      {
        marking = 0;
        escape_combo = 0;
        putsn(ANSI_CURSOR_HIDE);
        fflush(stdout);
      }
      break;

    case ETR_CHAR:
      if (!marking) break;
      struct MapMark* mark = findMark(path_index, path_pos);
      if (mark == NULL)
      {
        // reached max marks TODO: show alert
        if (state.map_marks_count == countof(state.map_marks)) break;

        mark = &state.map_marks[state.map_marks_count++];
        mark->path_index = path_index;
        mark->pos = path_pos;
      }
      else
      {
        if (mark->density < countof(DENSITY_INDICATORS) - 1) ++mark->density;
        else break;
      }
      putsn(ANSI_COLOR_MAGENTA);
      drawMark(mark, last_offset);
      putsn("\b" ANSI_COLOR_RESET);
      fflush(stdout);
      break;

    case ' ': putsn(ANSI_CURSOR_STYLE_UNDERLINE ANSI_CURSOR_SHOW); return;

    case KEY_ARROW_DOWN:
      if (pan_pos.y >= MAP_HEIGHT - MAP_VIEWPORT_HEIGHT / 2 - 1) break;
      ++pan_pos.y;
      goto map;

    case KEY_ARROW_UP:
      if (pan_pos.y <= MAP_VIEWPORT_HEIGHT / 2) break;
      --pan_pos.y;
      goto map;

    case KEY_ARROW_RIGHT:
      if (pan_pos.x >= MAP_WIDTH - MAP_VIEWPORT_WIDTH / 2) break;
      ++pan_pos.x;
      goto map;

    case KEY_ARROW_LEFT:
      if (pan_pos.x <= MAP_VIEWPORT_WIDTH / 2) break;
      --pan_pos.x;
      goto map;
    }
    continue;
  map:
    drawMap(pan_pos);
    fflush(stdout);
  }
}

struct MapMark*
findMark(byte path_index, byte path_pos)
{
  for (byte i = 0; i < state.map_marks_count; i++)
  {
    if (state.map_marks[i].path_index == path_index && state.map_marks[i].pos == path_pos) return &state.map_marks[i];
  }
  return 0;
}
