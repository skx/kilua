/* Kilo -- A very simple editor in around 1-kilo lines of code (as counted
 *         by "cloc"). Does not depend on libcurses, directly emits VT100
 *         escapes on the terminal.
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (C) 2016 Salvatore Sanfilippo <antirez at gmail dot com>
 *
 * Copyright (C) 2016 Steve Kemp https://steve.kemp.fi/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once

#include "undo_stack.h"


/* Syntax highlight types */
#define HL_NORMAL 0
#define HL_NONPRINT 1
#define HL_COMMENT 2   /* Single line comment. */
#define HL_MLCOMMENT 3 /* Multi-line comment. */
#define HL_KEYWORD1 4
#define HL_KEYWORD2 5
#define HL_STRING 6
#define HL_NUMBER 7
#define HL_MATCH 8      /* Search match. */
#define HL_SELECTION 9   /* area between point & mark */
#define HL_HIGHLIGHT_STRINGS (1<<1)
#define HL_HIGHLIGHT_NUMBERS (1<<2)

#define KILO_QUERY_LEN 256

/* Global lua handle */
lua_State * lua;

struct editorSyntax
{
    char **keywords;
    char singleline_comment_start[5];
    char multiline_comment_start[5];
    char multiline_comment_end[5];
    int flags;
};

/* This structure represents a single line of the file we are editing. */
typedef struct erow
{
    int idx;            /* Row index in the file, zero-based. */
    int size;           /* Size of the row, excluding the null term. */
    int rsize;          /* Size of the rendered row. */
    char *chars;        /* Row content. */
    char *render;       /* Row content "rendered" for screen (for TABs). */
    unsigned char *hl;  /* Syntax highlight type for each character in render.*/
    int hl_oc;          /* Row had open comment at end in last syntax highlight
                           check. */
} erow;

struct editorConfig
{
    int cx, cy; /* Cursor x and y position in characters */
    int markx, marky; /*  x and y position of mark in characters */
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int numrows;    /* Number of rows */
    int rawmode;    /* Is terminal raw mode enabled? */
    erow *row;      /* Rows */
    int dirty;      /* File modified but not saved. */
    char *filename; /* Currently open filename */
    char statusmsg[80];
    time_t statusmsg_time;
    struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    UndoStack *undo;

};

static struct editorConfig E;

enum KEY_ACTION
{
    CTRL_H = 8,         /* Ctrl-h */
    TAB = 9,            /* Tab */
    ENTER = 13,         /* Enter */
    ESC = 27,           /* Escape */
    BACKSPACE =  127,   /* Backspace */
    /* The following are just soft codes, not really reported by the
     * terminal directly. */
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};


/* We define a very simple "append buffer" structure, that is an heap
 * allocated string where we can append to. This is useful in order to
 * write all the escape sequences in a buffer and flush them to the standard
 * output in a single call, to avoid flickering effects. */
struct abuf
{
    char *b;
    int len;
};

#define ABUF_INIT {NULL,0}



/* prototypes */
void disableRawMode(int fd);
void editorAtExit(void);
int enableRawMode(int fd);
int editorReadKey(int fd);
int getCursorPosition(int ifd, int ofd, int *rows, int *cols);
void getWindowSize(int ifd, int ofd, int *rows, int *cols);
void call_lua(char *function, char *arg);
void strrev(char *p);
char at(void);
char *get_selection(void);
int editorOpen(char *filename);
int is_separator(int c);
int editorRowHasOpenComment(erow *row);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
char *get_input(char *prompt);
void editorUpdateRow(erow *row);
void editorInsertRow(int at, char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
char *editorRowsToString(int *buflen);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowAppendString(erow *row, char *s, size_t len);
void editorRowDelChar(erow *row, int at);
void editorInsertChar(int c);
void editorInsertNewline(void);
int dirty_lua(lua_State *L);
int get_line_lua(lua_State *L);
int kill_line_lua(lua_State *L);
int exit_lua(lua_State *L);
int insert_lua(lua_State *L);
int eol_lua(lua_State *L);
int sol_lua(lua_State *L);
int up_lua(lua_State *L);
int down_lua(lua_State *L);
int point_lua(lua_State *L);
int mark_lua(lua_State *L);
int selection_lua(lua_State *L);
int cut_selection_lua(lua_State *L);
int page_down_lua(lua_State *L);
int page_up_lua(lua_State *L);
int status_lua(lua_State *L);
int left_lua(lua_State *L);
int right_lua(lua_State *L);
int at_lua(lua_State *L);
int delete_lua(lua_State *L);
int prompt_lua(lua_State *L);
int save_lua(lua_State *L);
int set_syntax_keywords_lua(lua_State *L);
int syntax_highlight_numbers_lua(lua_State *L);
int syntax_highlight_strings_lua(lua_State *L);
int set_syntax_comments_lua(lua_State *L);
int find_lua(lua_State *L);
int eval_lua(lua_State *L);
int open_lua(lua_State *L);
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void editorRefreshScreen(void);
void editorSetStatusMessage(const char *fmt, ...);
void editorMoveCursor(int key);
int load_lua(char *filename);
void editorProcessKeypress(int fd);
void initEditor(void);
int main(int argc, char **argv);
