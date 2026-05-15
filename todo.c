#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_TODOS 100
#define MAX_LEN 256

typedef struct {
  int done;
  char text[MAX_LEN];
} Todo;

Todo todos[MAX_TODOS];
int todo_count = 0;
int selected = 0;
char *filepath;

void load_todos() {
  FILE *f = fopen(filepath, "r");
  if (!f)
    return;
  todo_count = 0;
  char line[MAX_LEN + 4];
  while (fgets(line, sizeof(line), f) && todo_count < MAX_TODOS) {
    if (line[0] == '[') {
      todos[todo_count].done = (line[1] == 'x');
      strcpy(todos[todo_count].text, line + 4);
      int len = strlen(todos[todo_count].text);
      if (len > 0 && todos[todo_count].text[len - 1] == '\n') {
        todos[todo_count].text[len - 1] = '\0';
      }
      todo_count++;
    }
  }
  fclose(f);
}

void save_todos() {
  FILE *f = fopen(filepath, "w");
  if (!f)
    return;
  for (int i = 0; i < todo_count; i++) {
    fprintf(f, "[%c] %s\n", todos[i].done ? 'x' : ' ', todos[i].text);
  }
  fclose(f);
}

void render() {
  clear();
  mvprintw(0, 0, "=== TODO LIST ===");
  mvprintw(1, 0,
           "Controls: j/k or arrows: move | space: toggle | a: add | d: delete "
           "| q: quit");

  for (int i = 0; i < todo_count; i++) {
    char *prefix = todos[i].done ? "[x]" : "[ ]";
    if (i == selected) {
      attrset(A_REVERSE);
      mvprintw(3 + i, 0, " %s %s", prefix, todos[i].text);
      attroff(A_REVERSE);
    } else {
      mvprintw(3 + i, 0, " %s %s", prefix, todos[i].text);
    }
  }

  if (todo_count == 0) {
    mvprintw(3, 0, " (no todos yet, press 'a' to add)");
  }

  refresh();
}

void add_todo() {
  if (todo_count >= MAX_TODOS)
    return;

  echo();
  curs_set(1);
  mvprintw(todo_count + 4, 0, "Enter todo: ");
  char buf[MAX_LEN];
  getnstr(buf, MAX_LEN - 1);
  noecho();
  curs_set(0);

  if (strlen(buf) > 0) {
    todos[todo_count].done = 0;
    strncpy(todos[todo_count].text, buf, MAX_LEN - 1);
    todos[todo_count].text[MAX_LEN - 1] = '\0';
    todo_count++;
    save_todos();
  }
  render();
}

void delete_todo() {
  if (todo_count == 0 || selected >= todo_count)
    return;
  for (int i = selected; i < todo_count - 1; i++) {
    todos[i] = todos[i + 1];
  }
  todo_count--;
  if (selected >= todo_count && selected > 0)
    selected--;
  save_todos();
  render();
}

void toggle_todo() {
  if (todo_count == 0 || selected >= todo_count)
    return;
  todos[selected].done = !todos[selected].done;
  save_todos();
  render();
}

int main() {
  filepath = getenv("HOME");
  if (!filepath)
    filepath = ".";
  strcat(filepath, "/.todo.txt");

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  load_todos();
  render();

  int running = 1;
  while (running) {
    int ch = getch();
    switch (ch) {
    case 'k':
    case KEY_UP:
      if (selected > 0)
        selected--;
      break;
    case 'j':
    case KEY_DOWN:
      if (selected < todo_count - 1)
        selected++;
      break;
    case ' ':
      toggle_todo();
      break;
    case 'a':
      add_todo();
      break;
    case 'd':
      delete_todo();
      break;
    case 'q':
      running = 0;
      break;
    }
    render();
  }

  endwin();
  return 0;
}
