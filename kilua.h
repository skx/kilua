/* kilua.h - Header file.
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

#ifdef _REGEXP
#include <regex.h>
#endif

#ifdef _UNDO
#include "undo_stack.h"
#endif

/* Lua interface */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


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

/**
 * This structure holds the details of the syntax-highlighting
 * which is enabled for a particular file.
 */
struct editorSyntax
{
    /**
     * An array of keywords.
     */
    char **keywords;

    /**
     * The text-string that marks a single-line comment.
     */
    char singleline_comment_start[5];

    /**
     * The text-string that marks the opening of a multi-line comment.
     */
    char multiline_comment_start[5];

    /**
     * The text-string that marks the closing of a multi-line comment.
     */
    char multiline_comment_end[5];

    /**
     * Flags in-play for highlighting numbers/strings/etc.
     */
    int flags;
};



/**
 * This structure represents a single line of the file we are editing.
 */
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



/**
 * This structure represents the state of a file.
 *
 * It is distinct from the editorState because we expect we'll
 * allow multiple files in the future.
 */
struct fileState
{
    int cx, cy; /* Cursor x and y position in characters */
    int markx, marky; /*  x and y position of mark in characters */
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
    int numrows;    /* Number of rows */
    erow *row;      /* Rows */
    int dirty;      /* File modified but not saved. */
    int tab_size;   /* Width of tabs */
    char *filename; /* Currently open filename */
    struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
#ifdef _UNDO
    UndoStack *undo;
#endif
};


/**
 * This structure represents the global state of the editor.
 */
struct editorState
{
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    char statusmsg[KILO_QUERY_LEN + 1]; /* The status-message */

    /*
     * The file(s) we have open.
     */
    struct fileState **file;
    int current_file ;
    int max_files;
};

/**
 * The state of the editor.
 */
static struct editorState E;



/**
 * These are used for key-recognition.
 */
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


/**
 * We define a very simple "append buffer" structure, that is an heap
 * allocated string where we can append to. This is useful in order to
 * write all the escape sequences in a buffer and flush them to the standard
 * output in a single call, to avoid flickering effects.
 */
struct abuf
{
    char *b;
    int len;
};


/**
 * A new/empty append-buffer.
 */
#define ABUF_INIT {NULL,0}



/* prototypes */
char * status_bar();
char *editorRowsToString(int *buflen);
char *get_input(char *prompt);
char *get_selection(void);
char at(void);
int dirty();
int editorOpen(char *filename);
int editorReadKey(int fd);
int editorRowHasOpenComment(erow *row);
int editorSyntaxToColor(int hl);
int enableRawMode(int fd);
int is_separator(int c);
int load_lua(char *filename);
int main(int argc, char **argv);
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void call_lua(const char *func, const char *sig, ...);
void disableRawMode(int fd);
void editorAtExit(void);
void editorDelRow(int at);
void editorFreeRow(erow *row);
void editorInsertChar(int c);
void editorInsertNewline(void);
void editorInsertRow(int at, char *s, size_t len);
void editorMoveCursor(int key);
void editorProcessKeypress(int fd);
void editorRefreshScreen(void);
void editorRowAppendString(erow *row, char *s, size_t len);
void editorRowDelChar(erow *row, int at);
void editorRowInsertChar(erow *row, int at, int c);
void editorSetStatusMessage(int log, const char *fmt, ...);
void editorUpdateRow(erow *row);
void editorUpdateSyntax(erow *row);
void getWindowSize();
void initEditor(void);
void rerender();
void strrev(char *p);
void warp(int x, int y);


/* ========================= Lua Primitives  ======================== */

/* Accessors */
extern  int at_lua(lua_State *L);
extern  int dirty_lua(lua_State *L);
extern  int get_line_lua(lua_State *L);
extern  int text(lua_State *L);

/* Movement */
extern  int down_lua(lua_State *L);
extern  int eol_lua(lua_State *L);
extern  int left_lua(lua_State *L);
extern  int page_down_lua(lua_State *L);
extern  int page_up_lua(lua_State *L);
extern  int right_lua(lua_State *L);
extern  int sol_lua(lua_State *L);
extern  int up_lua(lua_State *L);

/* Selection */
extern  int cut_selection_lua(lua_State *L);
extern  int selection_lua(lua_State *L);

/* Removals */
extern  int delete_lua(lua_State *L);
extern  int kill_line_lua(lua_State *L);
extern  int key_lua(lua_State *L);
extern  int insert_lua(lua_State *L);

/* Markers */
extern  int mark_lua(lua_State *L);
extern  int point_lua(lua_State *L);

/* Core */
extern  int eval_lua(lua_State *L);
extern  int exit_lua(lua_State *L);
extern  int filename_lua(lua_State *L);
extern  int find_lua(lua_State *L);
extern  int height_lua(lua_State *L);
extern  int open_lua(lua_State *L);
extern  int prompt_lua(lua_State *L);
extern  int save_lua(lua_State *L);
extern  int search_lua(lua_State *L);
extern  int status_lua(lua_State *L);
extern  int undo_lua(lua_State *L);
extern  int width_lua(lua_State *L);

/* Syntax highlighting */
extern  int set_syntax_comments_lua(lua_State *L);
extern  int set_syntax_keywords_lua(lua_State *L);
extern  int syntax_highlight_numbers_lua(lua_State *L);
extern  int syntax_highlight_strings_lua(lua_State *L);
extern  int tabsize_lua(lua_State *L);

/* Buffers */
extern int choose_buffer_lua(lua_State *L);
extern int count_buffers_lua(lua_State *L);
extern int create_buffer_lua(lua_State *L);
extern int current_buffer_lua(lua_State *L);
extern int kill_buffer_lua(lua_State *L);
extern int next_buffer_lua(lua_State *L);
extern int prev_buffer_lua(lua_State *L);
extern int select_buffer_lua(lua_State *L);
