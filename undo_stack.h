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



#include <stdio.h>
#include <stdlib.h>


/*
 * Each undo operations we support.
 *
 * Moving the cursor - undo for moving in opposite direction.
 * Inserting a character - undo for `delete`.
 * Deleting a character - undor for inserting a character.
 */
typedef enum {INSERT, DELETE} undo_type;



/*
 * A single action which can be undone.
 */
typedef struct UndoAction
{
    /*
     * The type of operation this is.
     */
    undo_type type;

    /*
     * For the case of insertion this is the character we'll insert.
     */
    char data;

    /*
     * The position of the insert/delete.
     */
    int x, y;

} UndoAction;



/*
 * Our actual undo-stack.
 */
typedef struct UndoStack
{
    /*
     * Size of stack.
     */
    int size;

    /*
     * The action.
     */
    struct UndoAction **elements;

} UndoStack;



/*
 * Create a new stack.
 */
UndoStack * us_create()
{
    UndoStack *S = (UndoStack *)malloc(sizeof(UndoStack));
    S->elements = NULL;
    S->size = 0;
    return S;
}

/*
 * Pop an operation from our stack.
 *
 * If the stack is empty then we'll return NULL.
 */
UndoAction *us_pop(UndoStack *S)
{
    if (S->size == 0)
        return NULL;

    UndoAction *top = S->elements[S->size - 1];
    S->size -= 1;
    return top;
}


/*
 * Store an entry on the stack.
 */
void us_push(UndoStack *S, UndoAction *action)
{
    int size = S->size;
    S->elements = realloc(S->elements, sizeof(UndoAction) * size + 1);
    S->elements[size] = action;
    S->size = size + 1;
}


/*
 * Clear the stack.
 */
void us_clear(UndoStack *S)
{
    /*
     * Free each action.
     */
    for (int i = 0 ; i < S->size; i++)
    {
        UndoAction *a = S->elements[i];
        free(a);
        S->elements[i] = NULL;
    }

    /*
     * Now free the space to hold them, and reset the size.
     */
    free(S->elements);
    S->elements = NULL;
    S->size = 0;
}

/*
 * Add an undo-operation, taking care of the allocation.
 */
void add_undo(UndoStack *S, undo_type type, char data, int x, int y)
{
    UndoAction *u = (UndoAction *)malloc(sizeof(UndoAction));
    u->type = type;
    u->data = data;
    u->x    = x;
    u->y    = y;

    us_push(S, u);
}
