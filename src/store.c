#include "cvector/cvector.h"

#include "input.h"
#include "state.h"
#include "store.h"
#include "tui.h"
#include "utils.h"

static float total_bill = 0;
static struct Store* store;

static struct WrapLine*
showStoreCategory(struct WrapLine* lines, const struct StoreCategory* categories, byte index)
{
  char text[16] = { '1' + index, '.', ' ' };
  strcpy(text + 3, categories[index].name);
  char text2[8];
  sprintf(text2, "$%.2f", categories[index].spent);
  return justifyLineWL(lines, text, text2, DIALOG_CONTENT_WIDTH);
}

static void
showStoreCategoryMenu(struct StoreCategory* category)
{
  struct StoreItem* item = category->items;
  const struct StoreItem* items_end = &category->items[category->items_count];
  category->spent = 0;

  const char question_start[] = "\n\nHow many " CONTROL_CHAR_STR;
  const char question_end[] = "? " CONTROL_CHAR_STR;
  const size_t len1 = strlen(category->desciption), len2 = strlen(item->name);
  char* text = malloc(len1 + len2 + sizeof(question_start) + sizeof(question_end));
  assert(text);

  text[0] = '\n';
  size_t pos = 1;
  memcpy(text + pos, category->desciption, len1);
  pos += len1;
  memcpy(text + pos, question_start, sizeof(question_start) - 1);
  pos += sizeof(question_start) - 1;
  memcpy(text + pos, item->name, len2);
  pos += len2;
  memcpy(text + pos, question_end, sizeof(question_end));

  Coord captures[2] = { { 0 }, { 0 } };
  struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, &(struct WrapLineOptions){ .captures = &captures[0] });
  free(text);

  lines = addNewline(lines);
  lines = textToLinesWL(lines, category->image);
  lines = addNewline(lines);
  char text2[24];
  sprintf(text2, "Bill so far: $%.2f", total_bill);
  lines = addLine(lines, text2, WRAPLINEKIND_CENTER);

  clearStdout();
  drawBoxWL(
      lines, DIALOG_WIDTH, BORDER_DOUBLE,
      &(struct BoxOptions){ .title = store->name, .color = store->color_category, .paddingX = DIALOG_PADDING_X }
  );
  setCursorPos(captures[1].x + DIALOG_PADDING_X + 1, captures[1].y + DIALOG_PADDING_Y);
  fflush(stdout);

  goto get_input;

  while (++item < items_end)
  {
    strcpy(text2, item->name);
    strcat(text2, question_end);
    struct WrapLine* lines = wrapText(text2, DIALOG_CONTENT_WIDTH, &(struct WrapLineOptions){ .captures = &captures[1] });
    putBlockWL(lines, captures[0].x + DIALOG_PADDING_X + 1, captures[0].y + DIALOG_PADDING_Y, 0);
    setCursorPos(captures[1].x + captures[0].x + DIALOG_PADDING_X + 1, captures[1].y + captures[0].y + DIALOG_PADDING_Y);
    fflush(stdout);

  get_input:
    item->amount = getNumberInput(item->min, item->max, 1, NULL);
    if (item->amount == (unsigned)-1) return;
    category->spent += item->price * item->amount;
  }
}

static void
drawChoiceStore(byte index, bool selected)
{
  setCursorPos(5, 5 + index);
  if (selected) putsn(ANSI_SELECTED);
  putchar('1' + index);
  putsn(". ");
  putsn(store->categories[index].name);
  if (selected) putsn(ANSI_COLOR_RESET);
  fflush(stdout);
}

void
showStoreAlert(char* text)
{
  struct WrapLine* lines = wrapText(text, DIALOG_CONTENT_WIDTH, NULL);

  lines = addLine(lines, "Press SPACE BAR to continue", WRAPLINEKIND_CENTER);

  putsn(ANSI_CURSOR_SAVE);
  putBlockWLFill((byte)cvector_size(lines), lines, 5, 8 + store->categories_count, DIALOG_CONTENT_WIDTH);
  cvector_free(lines);
  putsn(ANSI_CURSOR_HIDE);
  fflush(stdout);
  waitForKey(' ');
  if (HALT) return;

  lines = NULL;
  cvector_init(lines, 4, NULL);
  char buffer[32];
  sprintf(buffer, "Amount you have: $%.2f", state.money);
  lines = addLine(lines, buffer, WRAPLINEKIND_RTL);
  lines = addNewline(lines);

  byte added_lines_count = 0;
  lines = wrapText(
      "Which item would you like to buy?\n", DIALOG_CONTENT_WIDTH - INDENT_SIZE,
      &(struct WrapLineOptions){ .lines = lines, .added_count = &added_lines_count }
  );
  indentLines(cvector_end(lines) - added_lines_count, cvector_end(lines), INDENT_SIZE);
  lines = addLine(lines, "Press SPACE BAR to leave store", WRAPLINEKIND_CENTER);
  putBlockWLFill((byte)cvector_size(lines), lines, 5, 8 + store->categories_count, DIALOG_CONTENT_WIDTH);
  putsn(ANSI_CURSOR_SHOW ANSI_CURSOR_RESTORE);
  fflush(stdout);
  cvector_free(lines);
}

static enum QKeyCallbackReturn
storeInputCallback(int key, va_list args)
{
  unsigned* cur_pos = va_arg(args, unsigned*);
  Coord end = va_arg(args, Coord);

  switch (key)
  {
  case KEY_ARROW_DOWN:
    if (*cur_pos == store->categories_count - 1) break;
    if (*cur_pos != -1u) drawChoiceStore(*cur_pos, 0);
    else putsn(ANSI_CURSOR_HIDE);

    drawChoiceStore(++*cur_pos, 1);
    break;

  case KEY_ARROW_UP:
    if (*cur_pos == 0) break;
    if (*cur_pos != -1u) drawChoiceStore(*cur_pos, 0);
    else
    {
      *cur_pos = store->categories_count;
      putsn(ANSI_CURSOR_HIDE);
    }
    drawChoiceStore(--*cur_pos, 1);
    break;

  case KEY_PAGE_DOWN:
    if (*cur_pos == -1u || *cur_pos == store->categories_count - 1) break;
    drawChoiceStore(*cur_pos, 0);
    *cur_pos = store->categories_count - 1;
    drawChoiceStore(*cur_pos, 1);
    break;

  case KEY_PAGE_UP:
    if (*cur_pos == 0 || *cur_pos == -1u) break;
    drawChoiceStore(*cur_pos, 0);
    *cur_pos = 0;
    drawChoiceStore(*cur_pos, 1);
    break;

  case ' ':
    if (*cur_pos != -1u)
    {
      drawChoiceStore(*cur_pos, 0);
      *cur_pos = -1;
    }
    if (state.money < total_bill)
    {
      char text[116];
      sprintf(
          text, "Okay, that comes to a total of $%.2f, but I see you only have $%.2f. We'd better go over the list again.\n",
          total_bill, state.money
      );
      showStoreAlert(text);
      if (HALT) return QKEY_CALLBACK_RETURN_END;
      return QKEY_CALLBACK_RETURN_IGNORE;
    }

    if (store->callback_leave && store->callback_leave(store))
    {
      if (HALT) return QKEY_CALLBACK_RETURN_END;
      return QKEY_CALLBACK_RETURN_IGNORE;
    }
    state.money -= total_bill;
    return QKEY_CALLBACK_RETURN_END;

  case ETR_CHAR:
    if (*cur_pos != -1u)
    {
      putsn(ANSI_CURSOR_SHOW);
      showStoreCategoryMenu(&store->categories[*cur_pos]);
      return QKEY_CALLBACK_RETURN_END;
    }
    break;

  case ESC_CHAR:
    if (*cur_pos != -1u)
    {
      drawChoiceStore(*cur_pos, 0);
      *cur_pos = -1u;
      setCursorPos(end.x, end.y);
      putsn(ANSI_CURSOR_SHOW);
      fflush(stdout);
      escape_combo = 0;
    }
    break;
  }
  return *cur_pos == -1u ? QKEY_CALLBACK_RETURN_NORMAL : QKEY_CALLBACK_RETURN_IGNORE;
}

static Coord
drawStore(void)
{
  struct WrapLine* lines = NULL;
  cvector_init(lines, 12, NULL);
  lines = addLine(lines, &state.location[0], WRAPLINEKIND_CENTER);
  lines = addNewline(lines);
  char date[16];
  sprintf(date, "%s, %i, 1868", MONTHS[state.month], state.day);
  lines = addLine(lines, date, WRAPLINEKIND_RTL);
  lines = addBar(lines);

  for (byte i = 0; i < store->categories_count; i++)
  {
    total_bill += store->categories[i].spent;
    lines = showStoreCategory(lines, store->categories, i);
  }
  lines = addBar(lines);
  char text[32];
  sprintf(text, "Total bill: $%.2f", total_bill);
  lines = addLine(lines, text, WRAPLINEKIND_RTL);
  lines = addNewline(lines);
  sprintf(text, "Amount you have: $%.2f", state.money);
  lines = addLine(lines, text, WRAPLINEKIND_RTL);
  lines = addNewline(lines);

  Coord capture = { 0 };
  byte added_lines_count = 0;
  lines = wrapText(
      "Which item would you like to buy? " CONTROL_CHAR_STR "\n", DIALOG_CONTENT_WIDTH - INDENT_SIZE,
      &(struct WrapLineOptions){
          .captures = &capture,
          .lines = lines,
          .added_count = &added_lines_count,
      }
  );
  indentLines(cvector_end(lines) - added_lines_count, cvector_end(lines), INDENT_SIZE);
  lines = addLine(lines, "Press SPACE BAR to leave store", WRAPLINEKIND_CENTER);

  clearStdout();
  drawBoxWL(
      lines, DIALOG_WIDTH, BORDER_DOUBLE,
      &(struct BoxOptions){
          .title = store->name,
          .paddingX = DIALOG_PADDING_X,
          .color = COLOR_CYAN,
      }
  );

  capture.x += 1 + DIALOG_PADDING_X + INDENT_SIZE;
  capture.y++;

  return capture;
}

void
showStore(struct Store* store_in)
{
  store = store_in;
  total_bill = 0.f;

  do {
    unsigned cur_pos = -1u;
    Coord capture = drawStore();
    setCursorPos(capture.x, capture.y);
    putsn(ANSI_CURSOR_SHOW);
    fflush(stdout);
    const int choice = getNumberInput(1, store->categories_count, 1, &storeInputCallback, &cur_pos, capture) - 1;
    if (choice < 0) return;
    showStoreCategoryMenu(&store->categories[choice]);
  } while (!HALT);
}
