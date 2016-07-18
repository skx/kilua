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

#ifndef KILO_VERSION
#  define KILO_VERSION "unknown"
#endif

#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <getopt.h>

#ifdef _REGEXP
#include <regex.h>
#endif


/* Lua interface */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "kilo.h"




/* ======================= Low level terminal handling ====================== */

static struct termios orig_termios; /* In order to restore at exit.*/

/* Disable RAW mode - called at exit */
void disableRawMode(int fd)
{
    /* Don't even check the return value as it's too late. */
    if (E.rawmode)
    {
        tcsetattr(fd, TCSAFLUSH, &orig_termios);
        E.rawmode = 0;
    }
}

/* Called at exit to avoid remaining in raw mode, and clear the screen */
void editorAtExit(void)
{
#ifdef _UNDO
    /* kill our undo stack, to prevent valgrind leaks */
    us_clear(E.undo);
    free(E.undo);
#endif

    /* free all our rows */
    for (int i = 0; i < E.numrows; i++)
    {
        editorFreeRow(&E.row[i]);
    }

    free(E.row);

    /* close lua */
    lua_close(lua);

    if (E.filename)
    {
        free(E.filename);
        E.filename = NULL;
    }

    /* reset our input-mode and clear the screen. */
    disableRawMode(STDIN_FILENO);
    printf("\033[2J\033[1;1H");
}


/* Raw mode: 1960 magic shit. */
int enableRawMode(int fd)
{
    struct termios raw;

    if (E.rawmode) return 0; /* Already enabled. */

    if (!isatty(STDIN_FILENO)) goto fatal;

    atexit(editorAtExit);

    if (tcgetattr(fd, &orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - echoing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0) goto fatal;

    E.rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}


/* Read a key from raw-mode terminal, try to expand escape sequences. */
int editorReadKey(int fd)
{
    int nread;
    char c, seq[3];

    while ((nread = read(fd, &c, 1)) == 0);

    if (nread == -1) exit(1);

    while (1)
    {
        switch (c)
        {
        case ESC:    /* escape sequence */

            /* If this is just an ESC, we'll timeout here. */
            if (read(fd, seq, 1) == 0) return ESC;

            if (read(fd, seq + 1, 1) == 0) return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[')
            {
                if (seq[1] >= '0' && seq[1] <= '9')
                {
                    /* Extended escape, read additional byte. */
                    if (read(fd, seq + 2, 1) == 0) return ESC;

                    if (seq[2] == '~')
                    {
                        switch (seq[1])
                        {
                        case '3':
                            return DEL_KEY;

                        case '5':
                            return PAGE_UP;

                        case '6':
                            return PAGE_DOWN;
                        }
                    }
                }
                else
                {
                    switch (seq[1])
                    {
                    case 'A':
                        return ARROW_UP;

                    case 'B':
                        return ARROW_DOWN;

                    case 'C':
                        return ARROW_RIGHT;

                    case 'D':
                        return ARROW_LEFT;

                    case 'H':
                        return HOME_KEY;

                    case 'F':
                        return END_KEY;
                    }
                }
            }

            /* ESC O sequences. */
            else if (seq[0] == 'O')
            {
                switch (seq[1])
                {
                case 'H':
                    return HOME_KEY;

                case 'F':
                    return END_KEY;
                }
            }

            break;

        default:
            return c;
        }
    }
}


/* Record the size of our window */
void getWindowSize()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    E.screenrows = w.ws_row;
    E.screencols = w.ws_col;
}




/* ======================= Utility Functions ====================== */


/* Call a lua function which accepts a single string argument
 * and returns no result. */
void call_lua(char *function, char *arg)
{
    lua_getglobal(lua, function);

    if (lua_isnil(lua, -1))
    {
        editorSetStatusMessage("Failed to find function %s", function);
        return;
    }

    lua_pushstring(lua, arg);

    if (lua_pcall(lua, 1, 0, 0) != 0)
    {
        editorSetStatusMessage("%s failed %s", function, lua_tostring(lua, -1));
    }
}

/* Reverse a C-string, in-place */
void strrev(char *p)
{
    char *q = p;

    while (q && *q)
        ++q;

    for (--q; p < q; ++p, --q)
        *p = *p ^ *q, *q = *p ^ *q, *p = *p ^ *q;
}



/* Retrieve the single character at the current position. */
char at()
{
    int filerow = E.rowoff + E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    char tmp[2] = {'\n', '\0'};

    if (row)
    {
        if (E.cx < row->rsize)
            tmp[0] = row->render[E.cx];
    }

    return (tmp[0]);
}

/* Get the text which is currently selected - i.e. between mark & cursor */
char *get_selection()
{
    int saved_cx = E.cx, saved_cy = E.cy;
    int saved_coloff = E.coloff, saved_rowoff = E.rowoff;


    /*
     * Get the current X/Y
     */
    int x = E.coloff + E.cx;
    int y = E.rowoff + E.cy;

    /*
     * Append-buffer
     */
    struct abuf ab = ABUF_INIT;

    /* We'll create a string containing each byte-at-point and terminating
     * null character */
    char tmp[2] = { '\0', '\0' };

    /* Return value */
    char *result = NULL;

    /*
     * The cursor is above the mark.
     */
    if ((y > E.marky) || (x > E.markx && y == E.marky))
    {
        /*
         * Move left until we get there.
         */
        while (1)
        {
            /*
             * Get the character at the point.
             */
            tmp[0] = at();
            abAppend(&ab, tmp, 1);

            editorMoveCursor(ARROW_LEFT);

            if ((E.coloff + E.cx == E.markx) && (E.rowoff + E.cy == E.marky))
                break;
        }

        tmp[0] = at();
        abAppend(&ab, tmp, 1);

        /*
         * The append-buffer does not contain a trailing NULL.
         *
         * Allocate a buffer that is one byte bigger, ensure it gets
         * a null-terminated copy of the append-buffer's contents.
         */
        result = malloc(ab.len + 1);
        memset(result, '\0', ab.len + 1);
        memcpy(result, ab.b, ab.len);

        strrev(result);
    }
    else
    {
        /*
         * Move right until we get there.
         */
        while (1)
        {
            /*
             * Get the character at the point.
             */
            tmp[0] = at();
            abAppend(&ab, tmp, 1);

            editorMoveCursor(ARROW_RIGHT);

            if ((E.coloff + E.cx == E.markx) && (E.rowoff + E.cy == E.marky))
                break;
        }

        tmp[0] = at();
        abAppend(&ab, tmp, 1);

        /*
         * The append-buffer does not contain a trailing NULL.
         *
         * Allocate a buffer that is one byte bigger, ensure it gets
         * a null-terminated copy of the append-buffer's contents.
         */
        result = malloc(ab.len + 1);
        memset(result, '\0', ab.len + 1);
        memcpy(result, ab.b, ab.len);

    }

    E.cx = saved_cx;
    E.cy = saved_cy;
    E.coloff = saved_coloff;
    E.rowoff = saved_rowoff;

    abFree(&ab);
    return (result);
}

/* Load the specified program in the editor memory and returns 0 on success
 * or 1 on error. */
int editorOpen(char *filename)
{
    /*
     * Opened a file already?  Free the memory.
     */
    if (E.numrows)
    {
        for (int i = 0; i < E.numrows; i++)
        {
            erow *row = &E.row[i];
            free(row->chars);
            row->chars = NULL;
            free(row->render);
            row->render = NULL;
            free(row->hl);
            row->hl = NULL;
        }

        free(E.row);
        E.row = NULL;
    }

    FILE *fp;
    E.dirty = 0;
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.markx = -1;
    E.marky = -1;
    E.numrows = 0;
    free(E.filename);
    E.filename = strdup(filename);

#ifdef _UNDO
    /* kill our undo stack */
    us_clear(E.undo);
#endif

    fp = fopen(filename, "r");

    if (!fp)
    {
        if (errno != ENOENT)
        {
            perror("Opening file");
            exit(1);
        }

        /* invoke our lua callback function, even if opening failed.*/
        call_lua("on_loaded", E.filename);

        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, fp)) != -1)
    {
        if (linelen && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            line[--linelen] = '\0';

        editorInsertRow(E.numrows, line, linelen);
    }

    free(line);
    fclose(fp);
    E.dirty = 0;

    /* invoke our lua callback function */
    call_lua("on_loaded", E.filename);
    return 0;
}




/* ====================== Syntax highlight color scheme  ==================== */

int is_separator(int c)
{
    return c == '\0' || isspace(c) || strchr(":{},.()+-/*=~%[];<>|&", c) != NULL;
}

/* Return true if the specified row last char is part of a multi line comment
 * that starts at this row or at one before, and does not end at the end
 * of the row but spawns to the next row. */
int editorRowHasOpenComment(erow *row)
{
    /*
     * If the line is empty - then we have to check on the line before
     * that.
     */
    if (row->rsize == 0)
    {
        if (row->idx > 0)
            return (editorRowHasOpenComment(&E.row[row->idx - 1]));
        else
            return 0;
    }

    /*
     * OK we have some text.  Is the line ending in a MLCOMMENT
     * character-string?
     */
    if (row->hl && row->rsize && row->hl[row->rsize - 1] != HL_MLCOMMENT)
        return 0;

    /*
     * OK the line ends in a comment.  Are the closing characters
     * our closing tokens though?
     */
    int len = (int)strlen(E.syntax->multiline_comment_end);
    char *end = E.syntax->multiline_comment_end;

    if (len && strncmp(row->render + row->rsize - len, end, len) == 0)
        return 0;
    else
        return 1;


}

/* Set every byte of row->hl (that corresponds to every character in the line)
 * to the right syntax highlight type (HL_* defines). */
void editorUpdateSyntax(erow *row)
{
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);

    /* No syntax, everything is HL_NORMAL. */
    if (E.syntax == NULL)
        return;

    int i, prev_sep, in_string, in_comment;
    char *p;
    char **keywords = E.syntax->keywords;

    /* Point to the first non-space char. */
    p = row->render;
    i = 0; /* Current char offset */

    while (*p && isspace(*p))
    {
        p++;
        i++;
    }

    prev_sep = 1; /* Tell the parser if 'i' points to start of word. */
    in_string = 0; /* Are we inside "" or '' ? */
    in_comment = 0; /* Are we inside multi-line comment? */

    /* If the previous line has an open comment, this line starts
     * with an open comment state. */
    if (row->idx > 0 && editorRowHasOpenComment(&E.row[row->idx - 1]))
        in_comment = 1;

    while (*p)
    {

        /* Handle multi line comments. */
        if (in_comment)
        {
            row->hl[i] = HL_MLCOMMENT;

            if (strncmp(p, E.syntax->multiline_comment_end,
                        strlen(E.syntax->multiline_comment_end)) == 0)
            {

                for (int x = 0; x < (int)strlen(E.syntax->multiline_comment_end); x++)
                {
                    row->hl[i + x] = HL_MLCOMMENT;
                }

                p += strlen(E.syntax->multiline_comment_end) ;
                i += strlen(E.syntax->multiline_comment_end) ;
                in_comment = 0;
                prev_sep = 1;
                continue;
            }
            else
            {
                prev_sep = 0;
                p++;
                i++;
                continue;
            }
        }
        else if (strlen(E.syntax->multiline_comment_start) && strncmp(p, E.syntax->multiline_comment_start,
                 strlen(E.syntax->multiline_comment_start)) == 0)
        {

            for (int  x = 0; x < (int)strlen(E.syntax->multiline_comment_start) ; x++)
            {
                row->hl[i + x] = HL_MLCOMMENT;
            }

            p += (int)strlen(E.syntax->multiline_comment_start) ;
            i += (int)strlen(E.syntax->multiline_comment_start) ;
            in_comment = 1;
            prev_sep = 0;
            continue;
        }

        /* Handle // comments - colour the rest of the line and return. */
        if (prev_sep && strlen(E.syntax->singleline_comment_start) > 0 && (strncmp(p, E.syntax->singleline_comment_start, strlen(E.syntax->singleline_comment_start)) == 0))
        {
            /* From here to end is a comment */
            memset(row->hl + i, HL_COMMENT, row->rsize - i);
            return;
        }

        /* Handle "" and '' */
        if (in_string)
        {
            if (E.syntax->flags & HL_HIGHLIGHT_STRINGS)
                row->hl[i] = HL_STRING;

            if (*p == '\\')
            {
                if (E.syntax->flags & HL_HIGHLIGHT_STRINGS)
                    row->hl[i + 1] = HL_STRING;

                p += 2;
                i += 2;
                prev_sep = 0;
                continue;
            }

            if (*p == in_string)
                in_string = 0;

            p++;
            i++;
            continue;
        }
        else
        {
            if (*p == '"' || *p == '\'')
            {
                in_string = *p;

                if (E.syntax->flags & HL_HIGHLIGHT_STRINGS)
                    row->hl[i] = HL_STRING;

                p++;
                i++;
                prev_sep = 0;
                continue;
            }
        }

        /* Handle non printable chars. */
        if (!isprint(*p))
        {
            row->hl[i] = HL_NONPRINT;
            p++;
            i++;
            prev_sep = 0;
            continue;
        }

        /* Handle numbers */
        if ((isdigit(*p) && (prev_sep || row->hl[i - 1] == HL_NUMBER)) ||
                (*p == '.' && i > 0 && row->hl[i - 1] == HL_NUMBER))
        {
            if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS)
                row->hl[i] = HL_NUMBER;

            p++;
            i++;
            prev_sep = 0;
            continue;
        }

        if (is_separator(*p))
            row->hl[i] = HL_KEYWORD1;

        /* Handle keywords and lib calls */
        if (prev_sep)
        {
            int j;

            for (j = 0; keywords[j]; j++)
            {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen - 1] == '|';

                /*
                 * Take a copy of the keyword, so that we can
                 * remove the "|" from it, if we required
                 */
                char *tmp = strdup(keywords[j]);

                /*
                 * Compare the keyword as a regexp.
                 */
                if (kw2)
                    klen--;

#ifdef _REGEXP
                regex_t regex;

                /*
                 * Strip the trailing "|"
                 */
                if (kw2)
                    tmp[klen] = '\0';

                /*
                 * Can't compile?  Skip.
                 */
                if ( regcomp(&regex, tmp, 0) != 0 ) {
                    free(tmp);
                    continue;
                }

                regmatch_t result[1];
                int res = regexec(&regex, p, 1, result, 0);
                klen = (result[0]).rm_eo;
                free(tmp);

                if ((res == 0) && (is_separator(*(p + klen))))
                {
                    memset(row->hl + i, kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    p += klen;
                    i += klen;
                    break;
                }
#else
                if (!memcmp(p, keywords[j], klen) &&
                        is_separator(*(p + klen)))
                {
                    /* Keyword */
                    memset(row->hl + i, kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    p += klen;
                    i += klen;
                    free(tmp);
                    break;

                }
                free(tmp);
#endif
            }

            if (keywords[j] != NULL)
            {
                prev_sep = 0;
                continue; /* We had a keyword match */
            }

        }

        /* Not special chars */
        prev_sep = is_separator(*p);
        p++;
        i++;
    }

    /* Propagate syntax change to the next row if the open comment
     * state changed. This may recursively affect all the following rows
     * in the file. */
    int oc = editorRowHasOpenComment(row);

    if (row->hl_oc != oc && row->idx + 1 < E.numrows)
        editorUpdateSyntax(&E.row[row->idx + 1]);

    row->hl_oc = oc;
}

/* Maps syntax highlight token types to terminal colors. */
int editorSyntaxToColor(int hl)
{
    switch (hl)
    {
    case HL_COMMENT:
    case HL_MLCOMMENT:
        return 36;     /* cyan */

    case HL_KEYWORD1:
        return 33;    /* yellow */

    case HL_KEYWORD2:
        return 32;    /* green */

    case HL_STRING:
        return 35;      /* magenta */

    case HL_NUMBER:
        return 31;      /* red */

    case HL_MATCH:
        return 34;      /* blue */

    case HL_SELECTION:
        return 30;

    default:
        return 37;             /* white */
    }
}





/* ======================= Input functions           ======================= */

/* Show a prompt.  Read input until either `Esc` or `Return`. */
char *get_input(char *prompt)
{
    char query[KILO_QUERY_LEN + 1] = {0};
    int qlen = 0;

    /* Save the cursor position in order to restore it later. */
    int saved_cx = E.cx, saved_cy = E.cy;
    int saved_coloff = E.coloff, saved_rowoff = E.rowoff;

    while (1)
    {
        editorSetStatusMessage("%s%s", prompt, query);
        editorRefreshScreen();

        int c = editorReadKey(STDIN_FILENO);

        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE)
        {
            if (qlen != 0) query[--qlen] = '\0';
        }
        else if (c == ESC)
        {
            E.cx = saved_cx;
            E.cy = saved_cy;
            E.coloff = saved_coloff;
            E.rowoff = saved_rowoff;
            editorSetStatusMessage("");
            return NULL;
        }
        else if (c == ENTER)
        {
            E.cx = saved_cx;
            E.cy = saved_cy;
            E.coloff = saved_coloff;
            E.rowoff = saved_rowoff;
            editorSetStatusMessage("");
            return (strdup(query));
        }
        else if (isprint(c))
        {
            if (qlen < KILO_QUERY_LEN)
            {
                query[qlen++] = c;
                query[qlen] = '\0';
            }
        }
    }
}


/* ======================= Editor rows implementation ======================= */

/* Update the rendered version and the syntax highlight of a row. */
void editorUpdateRow(erow *row)
{
    int tabs = 0, nonprint = 0, j, idx;

    /* Create a version of the row we can directly print on the screen,
      * respecting tabs, substituting non printable characters with '?'. */
    free(row->render);

    for (j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;

    row->render = malloc(row->size + tabs * 8 + nonprint * 9 + 1);
    idx = 0;

    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == TAB)
        {
            row->render[idx++] = ' ';

            while ((idx + 1) % 8 != 0) row->render[idx++] = ' ';
        }
        else
        {
            row->render[idx++] = row->chars[j];
        }
    }

    row->rsize = idx;
    row->render[idx] = '\0';

    /* Update the syntax highlighting attributes of the row. */
    editorUpdateSyntax(row);
}

/* Insert a row at the specified position, shifting the other rows on the bottom
 * if required. */
void editorInsertRow(int at, char *s, size_t len)
{
    if (at > E.numrows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

    if (at != E.numrows)
    {
        memmove(E.row + at + 1, E.row + at, sizeof(E.row[0]) * (E.numrows - at));

        for (int j = at + 1; j <= E.numrows; j++) E.row[j].idx++;
    }

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len + 1);
    E.row[at].hl = NULL;
    E.row[at].hl_oc = 0;
    E.row[at].render = NULL;
    E.row[at].rsize = 0;
    E.row[at].idx = at;
    editorUpdateRow(E.row + at);
    E.numrows++;
    E.dirty++;
}

/* Free row's heap allocated stuff. */
void editorFreeRow(erow *row)
{
    free(row->render);
    free(row->chars);
    free(row->hl);

    /* Sanity-check - ensure we're not dereferenced / used */
    row->render = NULL;
    row->chars  = NULL;
    row->hl     = NULL;
    row->size   = 0;
    row->rsize  = 0;
}

/* Remove the row at the specified position, shifting the remaining on the
 * top. */
void editorDelRow(int at)
{
    erow *row;

    if (at >= E.numrows) return;

    row = E.row + at;
    editorFreeRow(row);
    memmove(E.row + at, E.row + at + 1, sizeof(E.row[0]) * (E.numrows - at - 1));

    for (int j = at; j < E.numrows - 1; j++)
        E.row[j].idx--;

    E.numrows--;
    E.dirty++;
}

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, excluding
 * the final nulterm. */
char *editorRowsToString(int *buflen)
{
    char *buf = NULL, *p;
    int totlen = 0;
    int j;

    /* Compute count of bytes */
    for (j = 0; j < E.numrows; j++)
        totlen += E.row[j].size + 1; /* +1 is for "\n" at end of every row */

    *buflen = totlen;
    totlen++; /* Also make space for nulterm */

    p = buf = malloc(totlen);

    for (j = 0; j < E.numrows; j++)
    {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }

    *p = '\0';
    return buf;
}

/* Insert a character at the specified position in a row, moving the remaining
 * chars on the right if needed. */
void editorRowInsertChar(erow *row, int at, int c)
{
    if (at > row->size)
    {
        /* Pad the string with spaces if the insert location is outside the
         * current length by more than a single character. */
        int padlen = at - row->size;
        /* In the next line +2 means: new char and null term. */
        row->chars = realloc(row->chars, row->size + padlen + 2);
        memset(row->chars + row->size, ' ', padlen);
        row->chars[row->size + padlen + 1] = '\0';
        row->size += padlen + 1;
    }
    else
    {
        /* If we are in the middle of the string just make space for 1 new
         * char plus the (already existing) null term. */
        row->chars = realloc(row->chars, row->size + 2);
        memmove(row->chars + at + 1, row->chars + at, row->size - at + 1);
        row->size++;
    }

    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

/* Append the string 's' at the end of a row */
void editorRowAppendString(erow *row, char *s, size_t len)
{
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(row->chars + row->size, s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

/* Delete the character at offset 'at' from the specified row. */
void editorRowDelChar(erow *row, int at)
{
    if (row->size <= at)
        return;

    if (at < 0)
        return;

    /*
     * Record the character we're deleting - and where we were
     * before we deleted it.
     */
#ifdef _UNDO
    int x = E.coloff + E.cx;
    int y = E.rowoff + E.cy;


    if (row->rsize >= at)
        add_undo(E.undo, INSERT, row->render[at], x, y);

#endif

    memmove(row->chars + at, row->chars + at + 1, row->size - at);
    editorUpdateRow(row);
    row->size--;
    E.dirty++;
}

/* Insert the specified char at the current prompt position. */
void editorInsertChar(int c)
{
    if (c == '\n')
    {
        editorInsertNewline();
        return;
    }

    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    /* If the row where the cursor is currently located does not exist in our
     * logical representation of the file, add enough empty rows as needed. */
    if (!row)
    {
        while (E.numrows <= filerow)
            editorInsertRow(E.numrows, "", 0);
    }

    row = &E.row[filerow];
    editorRowInsertChar(row, filecol, c);

    if (E.cx == E.screencols - 1)
        E.coloff++;
    else
        E.cx++;

    E.dirty++;
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
void editorInsertNewline(void)
{
    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row)
    {
        if (filerow == E.numrows)
        {
            editorInsertRow(filerow, "", 0);
            goto fixcursor;
        }

        return;
    }

    /* If the cursor is over the current line size, we want to conceptually
     * think it's just over the last character. */
    if (filecol >= row->size) filecol = row->size;

    if (filecol == 0)
    {
        editorInsertRow(filerow, "", 0);
    }
    else
    {
        /* We are in the middle of a line. Split it between two rows. */
        editorInsertRow(filerow + 1, row->chars + filecol, row->size - filecol);
        row = &E.row[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
        editorUpdateRow(row);
    }

fixcursor:

    if (E.cy == E.screenrows - 1)
    {
        E.rowoff++;
    }
    else
    {
        E.cy++;
    }

    E.cx = 0;
    E.coloff = 0;
}



/* ======================= Lua Functions ======================= */


/* is the buffer dirty? */
int dirty_lua(lua_State *L)
{
    if (E.dirty != 0)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

/* return the contents of the line from the point-onwards */
int get_line_lua(lua_State *L)
{
    int filerow = E.rowoff + E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (row)
    {
        lua_pushstring(L, row->chars + E.cx);
        return 1;
    }

    return 0;
}

/* read a single key */
int key_lua(lua_State *L)
{
    char buf[2] = { '\0', '\0' };
    buf[0] = editorReadKey(STDIN_FILENO);
    lua_pushstring(L, buf);
    return 1;
}

/* Remove the current line. */
int kill_line_lua(lua_State *L)
{
    (void)L;

    /* move to end of line. */
    eol_lua(L);

    /* number of deletes we need. */
    int len = 0;

    /* count the characters */
    int filerow = E.rowoff + E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (row)
        len = row->rsize;

    while (len > 0)
    {
        delete_lua(L);
        len -= 1;
    }

    /*
     * Remove the line itself.
     */
    right_lua(L);
    delete_lua(L);

    return 0;
}

/* exit the editor */
int exit_lua(lua_State *L)
{
    (void)L;
    exit(0);
    return 0;
}

/* insert a character */
int insert_lua(lua_State *L)
{
    const char *str = lua_tostring(L, -1);

    if (str != NULL)
    {
        size_t len = strlen(str);

        for (unsigned int i = 0; i < len; i++)
        {
            editorInsertChar(str[i]);
#ifdef _UNDO
            /*
             * Add the undo-record.
             */
            int x = E.coloff + E.cx;
            int y = E.rowoff + E.cy;
            add_undo(E.undo, DELETE, ' ', x, y);
#endif
        }
    }

    return 0;
}

/* move to end of line */
int eol_lua(lua_State *L)
{
    (void)L;

    int filerow = E.rowoff + E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (row)
    {
        /*
         * Row width.
         */
        int size = row->rsize;
        int x = E.coloff + E.cx;

        while (x < size)
        {
            editorMoveCursor(ARROW_RIGHT);
            x += 1;
        }
    }

    return 0;
}

/* Move to start of line */
int sol_lua(lua_State *L)
{
    (void)L;

    int x = E.coloff + E.cx;

    while (x)
    {
        editorMoveCursor(ARROW_LEFT);
        x -= 1;
    }

    return 0;
}

int undo_lua(lua_State *L)
{
    (void)L;
#ifdef _UNDO
    UndoAction *action = us_pop(E.undo);

    if (action == NULL)
    {
        editorSetStatusMessage("Undo stack is empty!");
        return 0;
    }

    if (action->type == DELETE)
    {
        warp(action->x, action->y);
        delete_lua(L);
    }
    else if (action->type == INSERT)
    {
        warp(action->x, action->y);

        char str[2] = { '\0', '\0' };
        editorSetStatusMessage("Inserting '%c'", str[0]);
        str[0] = action->data;
        lua_pushstring(L, str);
        insert_lua(L);
    }

    /*
     * Performing the action to undo the user's previous
     * thing will add a new action to the undo-stack.
     *
     * So we need to explicitly remove that faux addition here.
     */
    us_pop(E.undo);
#else
    editorSetStatusMessage("undo-support is not compiled in");
#endif
    return 0;
}

/* move cursor one character up */
int up_lua(lua_State *L)
{
    (void)L;
    editorMoveCursor(ARROW_UP);
    return 0;
}

/* move cursor one character down */
int down_lua(lua_State *L)
{
    (void)L;
    editorMoveCursor(ARROW_DOWN);
    return 0;
}

void warp(int x, int y)
{
    if (y < 0)
        y = 0;

    if (x < 0)
        x = 0;

    /*
     * Move to top-left
     */
    E.cx = E.coloff = E.cy = E.rowoff = 0;

    /*
     * Move down first - that should always work.
     */
    while (y--)
        editorMoveCursor(ARROW_DOWN);

    /*
     * Now move right.
     */
    while (x--)
        editorMoveCursor(ARROW_RIGHT);

}

/* Get/Set X,Y position of the cursor. */
int point_lua(lua_State *L)
{
    if (lua_isnumber(L, -2) &&
            lua_isnumber(L, -1))
    {
        int y = lua_tonumber(L, -1) - 1;
        int x = lua_tonumber(L, -2) - 1;

        warp(x, y);
    }

    lua_pushnumber(L, E.cx + E.coloff);
    lua_pushnumber(L, E.cy + E.rowoff);
    return 2;
}


/* Get/Set X,Y position of the mark. */
int mark_lua(lua_State *L)
{
    if (lua_isnumber(L, -2) &&
            lua_isnumber(L, -1))
    {
        int y = lua_tonumber(L, -1);
        int x = lua_tonumber(L, -2);

        if ((x >= 0 || x == -1) &&
                (y >= 0 || y == -1))
        {
            E.markx = x ;
            E.marky = y ;
        }

    }

    lua_pushnumber(L, E.markx);
    lua_pushnumber(L, E.marky);
    return 2;
}


/* Get the text between the point and the mark */
int selection_lua(lua_State *L)
{
    int x = E.coloff + E.cx;
    int y = E.rowoff + E.cy;

    /*
     * No selection - either because the mark is not set, and
     * the cursor is in the mark.
     */
    if ((E.markx == -1) && (E.marky == -1))
    {
        lua_pushnil(L);
        return 1;
    }

    if ((E.markx == x) && (E.marky == y))
    {
        lua_pushnil(L);
        return 1;
    }

    char *t = get_selection();
    lua_pushstring(L, t);
    free(t);
    return 1;

}


/* Delete the text between the point and the mark */
int cut_selection_lua(lua_State *L)
{
    int x = E.coloff + E.cx;
    int y = E.rowoff + E.cy;

    /* There is no mark. */
    if ((E.markx == -1) && (E.marky == -1))
        return 0;

    /* The cursor is at the mark. */
    if ((E.markx == x) && (E.marky == y))
        return 0;

    int left = 0;

    if ((y > E.marky) || (x > E.markx && y == E.marky))
        left = 1;

    /* Get the selection text */
    char *sel = get_selection();

    /* Delete the number of characters that the selection includes. */
    int max = (int)strlen(sel);

    if (left == 0)
    {
        for (int i = 0; i < max ; i++)
        {
            editorMoveCursor(ARROW_RIGHT);
            delete_lua(L);
        }
    }
    else
    {
        /* we want to delete the character under the cursor too - so
           we move one place right before we remove the characters. */
        editorMoveCursor(ARROW_RIGHT);

        for (int i = 0; i < max ; i++)
            delete_lua(L);
    }

    /* Cleanup & remove the mark. */
    free(sel);
    E.markx = -1;
    E.marky = -1;
    return 0;
}

/* page down */
int page_down_lua(lua_State *L)
{
    (void)L;

    int times = E.screenrows - 1;

    while (times--)
        down_lua(L);

    return 0;
}

/* page up */
int page_up_lua(lua_State *L)
{
    (void)L;
    int times = E.screenrows - 1;

    while (times--)
        up_lua(L);

    return 0;
}

/* set the status-bar text */
int status_lua(lua_State *L)
{
    const char *str = lua_tostring(L, -1);
    editorSetStatusMessage(str);
    return 0;
}

/* move the cursor one character left */
int left_lua(lua_State *L)
{
    (void)L;
    editorMoveCursor(ARROW_LEFT);
    return 0;
}

/* move the cursor one character right */
int right_lua(lua_State *L)
{
    (void)L;
    editorMoveCursor(ARROW_RIGHT);
    return 0;
}

/* get the character that the current position */
int at_lua(lua_State *L)
{
    char tmp[2] = {'\n', '\0'};
    tmp[0] = at();
    lua_pushstring(L, tmp);
    return 1;

}

/* Delete the char at the current position. */
int delete_lua(lua_State *L)
{
    (void)L;

    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;

    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row || (filecol == 0 && filerow == 0))
        return 0;


    if (filecol == 0)
    {
#ifdef _UNDO
        int x = E.coloff + E.cx;
        int y = E.rowoff + E.cy;
        add_undo(E.undo, INSERT, '\n', x, y);
#endif

        /* Handle the case of column 0, we need to move the current line
         * on the right of the previous one. */
        filecol = E.row[filerow - 1].size;
        editorRowAppendString(&E.row[filerow - 1], row->chars, row->size);
        editorDelRow(filerow);
        row = NULL;

        if (E.cy == 0)
            E.rowoff--;
        else
            E.cy--;

        E.cx = filecol;

        if (E.cx >= E.screencols)
        {
            int shift = (E.screencols - E.cx) + 1;
            E.cx -= shift;
            E.coloff += shift;
        }
    }
    else
    {
        editorRowDelChar(row, filecol - 1);

        if (E.cx == 0 && E.coloff)
            E.coloff--;
        else
            E.cx--;
    }

    /*
     * BUGFIX: Ensure we don't walk off the screen, and draw gibberish.
     *
     * This seems to happen when `shift` becomes negative above.
     */
    if (E.coloff < 0)
        E.coloff = 0;

    if (E.rowoff < 0)
        E.rowoff = 0;

    if (row) editorUpdateRow(row);

    E.dirty++;
    return 0;
}


/* prompt for input */
int prompt_lua(lua_State *L)
{
    char *prompt = (char *)lua_tostring(L, -1);
    char *buf = get_input(prompt);

    if (buf)
    {
        lua_pushstring(lua, buf);
        free(buf);
        return 1;
    }

    return 0;
}

/* Save the current file on disk. Return 0 on success, 1 on error. */
int save_lua(lua_State *L)
{

    /*
     * If we were given a filename then use that.
     */
    char *path = (char *)lua_tostring(L, -1);

    if (path != NULL)
    {
        free(E.filename);
        E.filename = strdup(path);
    }

    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);

    if (fd == -1) goto writeerr;

    /* Use truncate + a single write(2) call in order to make saving
     * a bit safer, under the limits of what we can do in a small editor. */
    if (ftruncate(fd, len) == -1) goto writeerr;

    if (write(fd, buf, len) != len) goto writeerr;

    close(fd);
    free(buf);
    E.dirty = 0;

    editorSetStatusMessage("%d bytes written to %s", len, E.filename);

    /* invoke our lua callback function */
    call_lua("on_saved", E.filename);

#ifdef _UNDO
    /* since we've saved - kill our undo stack */
    us_clear(E.undo);
#endif

    return 0;

writeerr:
    free(buf);

    if (fd != -1) close(fd);

    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
    return 1;
}

/* set the syntax keywords. */
int set_syntax_keywords_lua(lua_State *L)
{
    if (! lua_istable(L, 1))
        return 0;

    if (E.syntax == NULL)
    {
        struct editorSyntax *s = (struct editorSyntax*)malloc(sizeof(struct editorSyntax));
        s->keywords                    = NULL;
        s->singleline_comment_start[0] = '\0';
        s->multiline_comment_start[0]  = '\0';
        s->multiline_comment_end[0]    = '\0';
        s->flags                       =  HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS;
        E.syntax = s;
    }

    size_t len = lua_rawlen(L, 1);
    E.syntax->keywords = malloc((1 + len) * sizeof(char*));

    int i = 0;
    lua_pushnil(L);

    while (lua_next(L, -2) != 0)
    {
        const char *str = lua_tostring(L, -1);
        E.syntax->keywords[i] = strdup(str);
        lua_pop(L, 1);
        i += 1;
    }

    E.syntax->keywords[i] = NULL;

    /*
     * Force re-render.
     */
    for (int i = 0; i < E.numrows; i++)
        editorUpdateRow(E.row + i);

    return 0;
}

/* Enable highlighting of numbers. */
int syntax_highlight_numbers_lua(lua_State *L)
{
    int cond = lua_tonumber(L, -1);

    if (E.syntax)
    {
        if (cond == 1)
            E.syntax->flags |= HL_HIGHLIGHT_NUMBERS;
        else
            E.syntax->flags &= ~HL_HIGHLIGHT_NUMBERS;

        /*
         * Force re-render.
         */
        for (int i = 0; i < E.numrows; i++)
            editorUpdateRow(E.row + i);
    }

    return 0;
}

/* Enable highlighting of strings. */
int syntax_highlight_strings_lua(lua_State *L)
{
    int cond = lua_tonumber(L, -1);

    if (E.syntax)
    {
        if (cond == 1)
            E.syntax->flags |= HL_HIGHLIGHT_STRINGS;
        else
            E.syntax->flags &= ~HL_HIGHLIGHT_STRINGS;

        /*
         * Force re-render.
         */
        for (int i = 0; i < E.numrows; i++)
            editorUpdateRow(E.row + i);
    }

    return 0;
}

/* Set comment handling. */
int set_syntax_comments_lua(lua_State *L)
{
    char *single = (char *)lua_tostring(L, -3);
    char *multi_open = (char *)lua_tostring(L, -2);
    char *multi_end = (char *)lua_tostring(L, -1);

    if ((single == NULL) ||
            (multi_open == NULL) ||
            (multi_end == NULL))
        return 0;

    /*
     * If we don't have syntax that's a bug.
     */
    if (E.syntax == NULL)
        return 0;

    strcpy(E.syntax->singleline_comment_start, single);
    strcpy(E.syntax->multiline_comment_start, multi_open);
    strcpy(E.syntax->multiline_comment_end, multi_end);

    /*
     * Force re-render.
     */
    for (int i = 0; i < E.numrows; i++)
        editorUpdateRow(E.row + i);

    return 0;
}

/* Switch to find-mode */
int find_lua(lua_State *L)
{
    (void)L;
    char query[KILO_QUERY_LEN + 1] = {0};
    int qlen = 0;
    int last_match = -1; /* Last line where a match was found. -1 for none. */
    int find_next = 0; /* if 1 search next, if -1 search prev. */
    int saved_hl_line = -1;  /* No saved HL */
    char *saved_hl = NULL;

#define FIND_RESTORE_HL do { \
    if (saved_hl) { \
        memcpy(E.row[saved_hl_line].hl,saved_hl, E.row[saved_hl_line].rsize); \
        saved_hl = NULL; \
    } \
} while (0)

    /* Save the cursor position in order to restore it later. */
    int saved_cx = E.cx, saved_cy = E.cy;
    int saved_coloff = E.coloff, saved_rowoff = E.rowoff;

    while (1)
    {
        editorSetStatusMessage(
            "Search: %s (Use ESC/Arrows/Enter)", query);
        editorRefreshScreen();

        int c = editorReadKey(STDIN_FILENO);

        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE)
        {
            if (qlen != 0) query[--qlen] = '\0';

            last_match = -1;
        }
        else if (c == ESC || c == ENTER)
        {
            if (c == ESC)
            {
                E.cx = saved_cx;
                E.cy = saved_cy;
                E.coloff = saved_coloff;
                E.rowoff = saved_rowoff;
            }

            FIND_RESTORE_HL;
            editorSetStatusMessage("");
            return 0;
        }
        else if (c == ARROW_RIGHT || c == ARROW_DOWN)
        {
            find_next = 1;
        }
        else if (c == ARROW_LEFT || c == ARROW_UP)
        {
            find_next = -1;
        }
        else if (isprint(c))
        {
            if (qlen < KILO_QUERY_LEN)
            {
                query[qlen++] = c;
                query[qlen] = '\0';
                last_match = -1;
            }
        }

        /* Search occurrence. */
        if (last_match == -1) find_next = 1;

        if (find_next)
        {
            char *match = NULL;
            int match_offset = 0;
            int i, current = last_match;

            for (i = 0; i < E.numrows; i++)
            {
                current += find_next;

                if (current == -1) current = E.numrows - 1;
                else if (current == E.numrows) current = 0;

                match = strstr(E.row[current].render, query);

                if (match)
                {
                    match_offset = match - E.row[current].render;
                    break;
                }
            }

            find_next = 0;

            /* Highlight */
            FIND_RESTORE_HL;

            if (match)
            {
                erow *row = &E.row[current];
                last_match = current;

                if (row->hl)
                {
                    saved_hl_line = current;
                    saved_hl = malloc(row->rsize);
                    memcpy(saved_hl, row->hl, row->rsize);
                    memset(row->hl + match_offset, HL_MATCH, qlen);
                }

                /*
                 * NOTE: This breaks our undo, by warping to a new
                 * position that isn't tracked by our stack.
                 */
                E.cy = 0;
                E.cx = match_offset;
                E.rowoff = current;
                E.coloff = 0;

                /* Scroll horizontally as needed. */
                if (E.cx > E.screencols)
                {
                    int diff = E.cx - E.screencols;
                    E.cx -= diff;
                    E.coloff += diff;
                }
            }
        }
    }
}

/* prompt for a string, and evaluate that as lua. */
int eval_lua(lua_State *L)
{
    (void)L;

    char *txt = get_input("Eval: ");

    if (txt)
    {
        int res =  luaL_loadstring(lua, txt);

        if (res == 0)
        {
            res = lua_pcall(lua, 0, LUA_MULTRET, 0);
        }
        else
        {
            const char *er = lua_tostring(lua, -1);

            if (er)
                editorSetStatusMessage(er);
        }

        free(txt);
    }

    return 0;
}

/* Prompt for a filename and open it. */
int open_lua(lua_State *L)
{
    (void)L;
    /*
     * If we got a string then open it as a filename.
     */
    char *path = (char *)lua_tostring(L, -1);

    if (path != NULL)
    {
        editorOpen(path);
        return 0;
    }

    /*
     * Prompt for a path.
     */
    path = get_input("Open: ");

    if (path)
    {
        editorOpen(path);
        free(path);
    }

    return 0;
}





/* ============================= Append Buffer ============================ */

void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;

    memcpy(new + ab->len, s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab)
{
    free(ab->b);
}



/* ============================= Terminal update ============================ */

/* This function writes the whole screen using VT100 escape characters
 * starting from the logical state of the editor in the global state 'E'. */
void editorRefreshScreen(void)
{
    int y;
    erow *r;
    char buf[32];
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); /* Hide cursor. */
    abAppend(&ab, "\x1b[H", 3); /* Go home. */

    int drawn = 0;

    for (y = 0; y < E.screenrows; y++)
    {
        int filerow = E.rowoff + y;

        if (filerow >= E.numrows)
        {
            if (E.numrows == 0 && ( y == ( E.screenrows / 3 ) + drawn) &&
                ( drawn < 4 ) )
            {

                const char * array[] = {
                    "Kilo editor -- version " KILO_VERSION "\x1b[0K\r\n",
                    "\r\n",
#ifdef _REGEXP
                    "Regular expression support enabled.\r\n",
#else
                    "\r\n",
#endif
#ifdef _UNDO
                    "Undo-support enabled.\r\n",
#else
                    "\r\n",
#endif
                };

                int padding = (E.screencols - strlen(array[drawn])) / 2;

                if (padding)
                {
                    abAppend(&ab, "~", 1);
                    padding--;
                }
                while (padding--) abAppend(&ab, " ", 1);

                abAppend(&ab, array[drawn], strlen(array[drawn]));
                drawn += 1;

            }
            else
            {
                abAppend(&ab, "~\x1b[0K\r\n", 7);
            }

            continue;
        }

        r = &E.row[filerow];

        int len = r->rsize - E.coloff;
        int current_color = -1;

        if (len > 0)
        {
            if (len > E.screencols) len = E.screencols;

            char *c = r->render + E.coloff;
            unsigned char *hl = r->hl + E.coloff;
            int j;

            for (j = 0; j < len; j++)
            {

                int color = hl[j];

                /*
                 * HACK - draw the selection over
                 *
                 * filerow = y;
                 *  j      = x;
                 */
                if ((E.markx != -1) && (E.marky != -1))
                {
                    int mx = E.markx;
                    int my = E.marky;

                    int cx = E.coloff + E.cx;
                    int cy = E.rowoff + E.cy;

                    /* is the cursor above the mark? */
                    if ((cy > my) || (cx > mx && cy == my))
                    {
                        if (cy == my)
                        {
                            /*
                             * Point and mark on same line
                             */
                            if ((cy == filerow) && (j >= mx && j < cx))
                                color = HL_SELECTION;
                        }
                        else
                        {
                            /*
                             * Point + mark on different lines.
                             *
                             * mark is before point
                             */

                            /*
                             * Cover the line containing the mark.
                             */
                            if ((filerow == my) && (j >= mx))
                                color = HL_SELECTION;

                            /*
                             * Cover the line containing the cursor.
                             */
                            if ((filerow == cy) && (j < cx))
                                color = HL_SELECTION;

                            /*
                             * Cover the lines in between.
                             */
                            if (filerow > my && filerow < cy)
                                color = HL_SELECTION;
                        }
                    }
                    else
                    {
                        /*
                         * cursor is below the mark.
                         */
                        if (cy == my)
                        {
                            /*
                             * Point and mark on same line
                             */
                            if ((cy == filerow) && (j <= mx && j >= cx))
                                color = HL_SELECTION;
                        }
                        else
                        {
                            /*
                             * Point + mark on different lines.
                             *
                             * mark is after point
                             */

                            /*
                             * Cover the line containing the mark.
                             */
                            if ((filerow == my) && (j <= mx))
                                color = HL_SELECTION;

                            /*
                             * Cover the line containing the cursor.
                             */
                            if ((filerow == cy) && (j > cx))
                                color = HL_SELECTION;

                            /*
                             * Cover the lines in between.
                             */
                            if (filerow > cy && filerow < my)
                                color = HL_SELECTION;
                        }
                    }
                }


                if (color == HL_NONPRINT)
                {
                    char sym;
                    abAppend(&ab, "\x1b[7m", 4);

                    if (c[j] <= 26)
                        sym = '@' + c[j];
                    else
                        sym = '?';

                    abAppend(&ab, &sym, 1);
                    abAppend(&ab, "\x1b[0m", 4);
                }
                else if (color == HL_NORMAL)
                {
                    if (current_color != -1)
                    {
                        abAppend(&ab, "\x1b[39m", 5);
                        current_color = -1;
                    }

                    abAppend(&ab, c + j, 1);
                }
                else
                {
                    if (color == HL_SELECTION)
                    {
                        /*
                         * Show the mark in inverse white.
                         */
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[47m");
                        abAppend(&ab, buf, clen);
                        abAppend(&ab, c + j, 1);
                        clen = snprintf(buf, sizeof(buf), "\x1b[49m");
                        abAppend(&ab, buf, clen);
                    }
                    else
                    {
                        color = editorSyntaxToColor(color);

                        if (color != current_color)
                        {
                            char buf[16];
                            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
                            current_color = color;
                            abAppend(&ab, buf, clen);
                        }

                        abAppend(&ab, c + j, 1);
                    }

                }
            }
        }

        abAppend(&ab, "\x1b[39m", 5);
        abAppend(&ab, "\x1b[0K", 4);
        abAppend(&ab, "\r\n", 2);
    }

    /* Create a two rows status. First row: */
    abAppend(&ab, "\x1b[0K", 4);
    abAppend(&ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                       E.filename, E.numrows, E.dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus),
                        "Col:%d Row:%d/%d", E.coloff + E.cx + 1, E.rowoff + E.cy + 1, E.numrows);

    if (len > E.screencols) len = E.screencols;

    abAppend(&ab, status, len);

    while (len < E.screencols)
    {
        if (E.screencols - len == rlen)
        {
            abAppend(&ab, rstatus, rlen);
            break;
        }
        else
        {
            abAppend(&ab, " ", 1);
            len++;
        }
    }

    abAppend(&ab, "\x1b[0m\r\n", 6);

    /* Second row depends on E.statusmsg and the status message update time. */
    abAppend(&ab, "\x1b[0K", 4);
    int msglen = strlen(E.statusmsg);

    if (msglen && time(NULL) - E.statusmsg_time < 5)
        abAppend(&ab, E.statusmsg, msglen <= E.screencols ? msglen : E.screencols);

    /* Put cursor at its current position. Note that the horizontal position
     * at which the cursor is displayed may be different compared to 'E.cx'
     * because of TABs. */
    int j;
    int cx = 1;
    int filerow = E.rowoff + E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (row)
    {
        for (j = E.coloff; j < (E.cx + E.coloff); j++)
        {
            if (j < row->size && row->chars[j] == TAB) cx += 7 - ((cx) % 8);

            cx++;
        }
    }

    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, cx);
    abAppend(&ab, buf, strlen(buf));
    abAppend(&ab, "\x1b[?25h", 6); /* Show cursor. */
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

/* Set an editor status message for the second line of the status, at the
 * end of the screen. */
void editorSetStatusMessage(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}


/* ========================= Editor events handling  ======================== */

/* Handle cursor position change because arrow keys were pressed. */
void editorMoveCursor(int key)
{
    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;
    int rowlen;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    switch (key)
    {
    case ARROW_LEFT:
        if (E.cx == 0)
        {
            if (E.coloff)
            {
                E.coloff--;
            }
            else
            {
                if (filerow > 0)
                {
                    E.cy--;
                    E.cx = E.row[filerow - 1].size;

                    if (E.cx > E.screencols - 1)
                    {
                        E.coloff = E.cx - E.screencols + 1;
                        E.cx = E.screencols - 1;
                    }
                }
            }
        }
        else
        {
            E.cx -= 1;
        }

        break;

    case ARROW_RIGHT:
        if (row && filecol < row->size)
        {
            if (E.cx == E.screencols - 1)
            {
                E.coloff++;
            }
            else
            {
                E.cx += 1;
            }
        }
        else if (row && filecol == row->size)
        {
            if (E.cy == E.screenrows - 1)
            {
                E.cx = 0;
                E.coloff = 0;

                E.rowoff++;
            }
            else
            {
                if (filerow < (E.numrows - 1))
                {
                    E.cx = 0;
                    E.coloff = 0;

                    E.cy += 1;
                }
            }
        }

        break;

    case ARROW_UP:
        if (E.cy == 0)
        {
            if (E.rowoff) E.rowoff--;
        }
        else
        {
            E.cy -= 1;
        }

        break;

    case ARROW_DOWN:
        if (filerow < (E.numrows - 1))
        {
            if (E.cy == E.screenrows - 1)
            {
                E.rowoff++;
            }
            else
            {
                E.cy += 1;
            }
        }

        break;
    }

    /* Fix cx if the current line has not enough chars. */
    filerow = E.rowoff + E.cy;
    filecol = E.coloff + E.cx;
    row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    rowlen = row ? row->size : 0;

    if (filecol > rowlen)
    {
        E.cx -= filecol - rowlen;

        if (E.cx < 0)
        {
            E.coloff += E.cx;
            E.cx = 0;
        }
    }
}

/* Process events arriving from the standard input, which is, the user
 * is typing stuff on the terminal. */
void editorProcessKeypress(int fd)
{
    char tmp[2] = {'\0', '\0'};
    tmp[0] = editorReadKey(fd);
    call_lua("on_key", tmp);
}

/* Load and evaluate a Lua file - if it exists */
int load_lua(char *filename)
{
    if (access(filename, 0) == 0)
    {
        int erred = luaL_dofile(lua, filename);

        if (erred)
        {
            if (lua_isstring(lua, -1))
                fprintf(stderr, "%s\n", lua_tostring(lua, -1));

            fprintf(stderr, "Failed to load %s - aborting\n", filename);
            exit(1);
        }

        return 1;
    }

    return 0;
}

void initEditor(void)
{
    E.markx = -1;
    E.marky = -1;
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.syntax = NULL;

    getWindowSize();
    E.screenrows -= 2; /* Get room for status bar. */

#ifdef _UNDO
    E.undo = us_create();
#endif

    /*
     * Setup lua.
     */
    lua = luaL_newstate();
    luaopen_base(lua);
    luaL_openlibs(lua);

    /*
     * Lua bindings.
     */
    lua_register(lua, "at", at_lua);
    lua_register(lua, "delete", delete_lua);
    lua_register(lua, "dirty", dirty_lua);
    lua_register(lua, "down", down_lua);
    lua_register(lua, "eol", eol_lua);
    lua_register(lua, "eval", eval_lua);
    lua_register(lua, "exit", exit_lua);
    lua_register(lua, "find", find_lua);
    lua_register(lua, "get_line", get_line_lua);
    lua_register(lua, "kill", kill_line_lua);
    lua_register(lua, "key", key_lua);
    lua_register(lua, "insert", insert_lua);
    lua_register(lua, "left", left_lua);
    lua_register(lua, "right", right_lua);
    lua_register(lua, "mark", mark_lua);
    lua_register(lua, "point", point_lua);
    lua_register(lua, "page_down", page_down_lua);
    lua_register(lua, "page_up", page_up_lua);
    lua_register(lua, "open", open_lua);
    lua_register(lua, "prompt", prompt_lua);
    lua_register(lua, "save", save_lua);
    lua_register(lua, "selection", selection_lua);
    lua_register(lua, "cut_selection", cut_selection_lua);
    lua_register(lua, "set_syntax_keywords", set_syntax_keywords_lua);
    lua_register(lua, "set_syntax_comments", set_syntax_comments_lua);
    lua_register(lua, "syntax_highlight_numbers", syntax_highlight_numbers_lua);
    lua_register(lua, "syntax_highlight_strings", syntax_highlight_strings_lua);
    lua_register(lua, "status", status_lua);
    lua_register(lua, "sol", sol_lua);
    lua_register(lua, "undo", undo_lua);
    lua_register(lua, "up", up_lua);
}

int main(int argc, char **argv)
{
    /*
     * Initialize our editor.  We do this first so that
     * we have lua enabled which is required if we want
     * to load a configuration file specified by the command-line
     * arguments.
     */
    initEditor();

    /*
     * Count of init-files loaded.
     */
    int loaded = 0;

    /*
     * Load our default configuration files: ~/.kilo.lua + ./kilo.lua.
     */
    char init_buf[1024] = {'\0'};
    snprintf(init_buf, sizeof(init_buf) - 1, "%s%s",
             getenv("HOME"), "/.kilo.lua");
    loaded += load_lua(init_buf);
    loaded += load_lua("kilo.lua");

    /*
     * An init-function to call, if any.
     */
    char *eval = NULL;

    /*
     * Parse command-line options.
     */
    while (1)
    {
        static struct option long_options[] =
        {
            {"config", required_argument, 0, 'c'},
            {"eval", required_argument, 0, 'e'},
            {"version", no_argument, 0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        char c = getopt_long(argc, argv, "e:c:v", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 'v':
            fprintf(stderr, "kilo v%s\n", KILO_VERSION);
            exit(0);
            break;

        case 'c':
            loaded += load_lua(optarg);
            break;

        case 'e':
            eval = strdup(optarg);
            break;
        }
    }

    /*
     * If we've not loaded at least one configuration file then
     * we will have no `on_key` defined, which means the editor
     * will be broken.
     *
     * On that basis we immediately exit.
     */
    if (loaded == 0)
    {
        fprintf(stderr, "Neither ./kilo.lua nor ~/.kilo.lua could be loaded\n");
        exit(1);
    }

    /*
     * Ensure we have at least one command-line parameter left, which
     * is the file to edit.
     */
    if (optind >= argc)
    {
        fprintf(stderr, "Usage: kilo [options] filename\n");
        exit(1);
    }

    /*
     * Open the file, setup the mode, and the status-message.
     */
    editorOpen(argv[optind]);
    enableRawMode(STDIN_FILENO);
    editorSetStatusMessage(
        "HELP: ^o = open | ^s = save | ^q = quit | ^f = find | ^l = eval");

    /*
     * Run our event loop.
     */
    while (1)
    {
        /*
         * If we have a function to evaluate, post-load, do that.
         */
        if (eval != NULL)
        {
            call_lua(eval, "");
            free(eval);
            eval = NULL;
        }

        editorRefreshScreen();
        editorProcessKeypress(STDIN_FILENO);
    }

    return 0;
}
