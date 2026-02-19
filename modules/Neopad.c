#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include <locale.h> // Добавьте это

// Для работы с широкими символами в ncurses
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#define NCURSES_STATIC

#ifdef _WIN32
    #include <ncursesw/ncurses.h> // Используем ncursesw
#else
    #include <ncurses.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

// Пытаемся подключить ncurses правильно для разных систем



/*-----------------------Neopad 0.0.1 MIT License-----------------------*/


ssize_t my_getline(char **lineptr, size_t *n, FILE *stream) {
    if (!lineptr || !n || !stream) return -1;
    if (*lineptr == NULL) {
        *n = 128;
        *lineptr = malloc(*n);
        if (!*lineptr) return -1;
    }
    char *ptr = *lineptr;
    int c;
    size_t pos = 0;
    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            *n *= 2;
            char *new_ptr = realloc(*lineptr, *n);
            if (!new_ptr) return -1;
            *lineptr = new_ptr;
            ptr = *lineptr + pos;
        }
        *ptr++ = c;
        pos++;
        if (c == '\n') break;
    }
    *ptr = '\0';
    return (pos == 0 && c == EOF) ? -1 : (ssize_t)pos;
}

/*** data ***/
typedef struct erow {
    int size;
    char *chars;
} erow;

struct editorConfig {
    int cx, cy;
    int rowoff;
    int numrows;
    erow *row;
    char *filename;
    char statusmsg[80];
};


struct editorConfig E;

void editorInit() {
    setlocale(LC_ALL, ".UTF8");
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.filename = NULL;
}

void editorFree() {
    if (E.row != NULL) {
        for (int i = 0; i < E.numrows; i++) {
            if (E.row[i].chars != NULL) free(E.row[i].chars);
        }
        free(E.row);
        E.row = NULL; // Обнуляем, чтобы избежать double free
    }
    if (E.filename != NULL) {
        free(E.filename);
        E.filename = NULL;
    }
}
void editorMoveCursor(int key);
void editorRefreshScreen();

/*** row operations ***/
void editorInsertRow(int at, char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numrows++;
}
void insertNewLine(){
    if (E.cx == 0) {
        editorInsertRow(E.cy, "", 0);
    } else {
        erow *row = &E.row[E.cy];
        editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row->size = E.cx;
        row->chars[row->size] = '\0';
    }
    E.cy++;
    E.cx = 0;
}
void editorDelRow(int at) {
    if (at < 0 || at >= E.numrows) return;
    free(E.row[at].chars);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    E.numrows--;
}

void editorRowAppendString(erow *row, char *s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
}

void editorRowInsertChar(erow *row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
}

void editorRowDelChar(erow *row, int at) {
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
}

// Вставляет массив байтов UTF-8 символа
void editorRowInsertUTF8(erow *row, int at, const char *utf8, int utf8len) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + utf8len + 1);
    memmove(&row->chars[at + utf8len], &row->chars[at], row->size - at + 1);
    memcpy(&row->chars[at], utf8, utf8len);
    row->size += utf8len;
    row->chars[row->size] = '\0';
}

// Вставляет UTF-8 символ (все байты), курсор двигает на 1 символ
void editorInsertUTF8(const char *utf8, int utf8len) {
    if (E.cy == E.numrows) editorInsertRow(E.numrows, "", 0);
    editorRowInsertUTF8(&E.row[E.cy], E.cx, utf8, utf8len);
    E.cx++;
}

/*** editor operations ***/
void editorInsertChar(int c) {
    if (E.cy == E.numrows) editorInsertRow(E.numrows, "", 0);
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void editorDelChar() {
    if (E.cy == E.numrows || (E.cx == 0 && E.cy == 0)) return;
    erow *row = &E.row[E.cy];
    
    if (E.cx > 0) {
        // Проверяем, является ли символ перед курсором частью UTF-8
        // В UTF-8 последующие байты символа всегда начинаются с битов 10xxxxxx (0x80 - 0xBF)
        int bytes_to_delete = 1;
        while (E.cx - bytes_to_delete > 0 && 
               (row->chars[E.cx - bytes_to_delete] & 0xC0) == 0x80) {
            bytes_to_delete++;
        }
        
        // Если нашли старший байт UTF-8 (11xxxxxx)
        if (E.cx - bytes_to_delete >= 0 && 
            (row->chars[E.cx - bytes_to_delete] & 0xC0) == 0xC0) {
            // Удаляем весь многобайтовый символ
        } else {
            bytes_to_delete = 1; // Обычный ASCII
        }

        for (int i = 0; i < bytes_to_delete; i++) {
            editorRowDelChar(row, E.cx - 1);
            E.cx--;
        }
    } else {
        // Логика склеивания строк остается прежней
        E.cx = E.row[E.cy - 1].size;
        editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}

/*** file i/o ***/
void editorOpen(char *filename) {
    if (E.filename) free(E.filename);
    E.filename = strdup(filename);
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        
        FILE *fp = fopen(filename, "w");// if file does exist create it
        
    }
    
    if (!fp) return;
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    // Используем нашу функцию my_getline
    while ((linelen = my_getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        editorInsertRow(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
}

void editorSave() {
    if (E.filename == NULL) return;
    FILE *fp = fopen(E.filename, "w");
    if (fp) {
        for (int i = 0; i < E.numrows; i++) {
            fprintf(fp, "%s\n", E.row[i].chars);
        }
        fclose(fp);
    }
}

/*** output ***/
void editorRefreshScreen() {
    int screenrows, screencols;
    getmaxyx(stdscr, screenrows, screencols);
    if (E.cy < E.rowoff) E.rowoff = E.cy;
    if (E.cy >= E.rowoff + screenrows - 2) E.rowoff = E.cy - screenrows + 3;
    erase();
    for (int y = 0; y < screenrows - 2; y++) {
        int filerow = y + E.rowoff;
        if (filerow < E.numrows) {
            int len = E.row[filerow].size;
            if (len > screencols) len = screencols;
            mvaddnstr(y, 0, E.row[filerow].chars, len);
        } else {
            mvaddch(y, 0, '~');
        }
    }
    attron(A_REVERSE);
    mvprintw(screenrows - 2, 0, " %s - %d lines | Ctrl+C: Quit | Ctrl+S: Save ", E.filename ? E.filename : "[No Name]", E.numrows);
    for (int i = getcurx(stdscr); i < screencols; i++) addch(' ');
    attroff(A_REVERSE);
    move(E.cy - E.rowoff, E.cx);
    refresh();
}

/*** input ***/
void editorMoveCursor(int key) {
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    switch (key) {
        case KEY_LEFT: if (E.cx > 0) E.cx--; break;
        case KEY_RIGHT: if (row && E.cx < row->size) E.cx++; break;
        case KEY_UP: if (E.cy > 0) E.cy--; break;
        case KEY_DOWN: if (E.cy < E.numrows) E.cy++; break;
    }
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) E.cx = rowlen;
}
void editorClose() {
    // 1. Очищаем память строк редактора
    editorFree(); 
    // 2. Завершаем работу ncurses
    if (!isendwin()) {
        endwin();
    }
    return;
}
void editorProcessKeypress() {
    wint_t c;
    int res = get_wch(&c); // Получаем широкий символ

    if (res == KEY_CODE_YES) {
        // Обработка функциональных клавиш (стрелки, F1 и т.д.)
        switch (c) {
            case KEY_UP: case KEY_DOWN: case KEY_LEFT: case KEY_RIGHT: 
                editorMoveCursor(c); break;
            case KEY_BACKSPACE: case 127: case 8: 
                editorDelChar(); break;
        }
    } else {
        // Обработка обычных и многобайтовых символов
        switch (c) {
            case 3: editorClose(); break; // Ctrl+C
            case 19: editorSave(); break;  // Ctrl+S
            case '\n': case '\r': 
                editorInsertRow(E.cy + 1, "", 0); 
                E.cy++; E.cx = 0; break;
            default:
                if (c >= 32) {
                    // Конвертируем широкий символ обратно в UTF-8 байты для хранения
                    char utf8[MB_CUR_MAX];
                    int len = wctomb(utf8, c);
                    if (len > 0) {
                        editorInsertUTF8(utf8, len);
                    }
                }
                break;
        }
    }
}


// Основная точка входа в ваш модуль
void run_neopad(char *filename) {
    editorInit();
    editorOpen(filename);
    
    // Инициализация ncurses специально для этого модуля
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    int running = 1;
    while (running) {
        editorRefreshScreen();
        editorProcessKeypress();
        // Для выхода используйте Ctrl+C (обработка есть в editorProcessKeypress)
    }

    editorClose(); 
    // Теперь управление вернется в ProggShell
}
