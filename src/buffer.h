/* buffer.h - Our buffer/row definitions.
 *
 * -----------------------------------------------------------------------
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

#include <vector>
#include <unordered_map>
#include <string>


/**
 * This structure represents a single line of text.
 */
class erow
{
public:
    /**
     * Constructor.
     */
    erow();

    /**
     * Destructor.
     */
    ~erow();

public:
    /**
     * The text of the row, from the given character offset.
     */
    std::wstring text(int offset);

    /*
     * The character at each position in this row.
     */
    std::vector<std::wstring> *chars;

    /*
     * The colour to draw at each position.
     */
    std::vector<int> *cols;
};



/**
 * This class represents a buffer.
 *
 * The editor allows multiple buffers to be retrieved.
 *
 * Each buffer has:
 *
 *   A name.
 *   An "is modified" flag
 *   A number of rows of content.
 *
 */
class Buffer
{
public:
    /**
     * Constructor
     */
    Buffer(const char *bname);

    /**
     * Destructor
     */
    ~Buffer();

public:

    /**
     * Remove all text from the buffer.
     */
    void empty_buffer();

    /**
     * Get the character offset of the given X,Y coordinate in our buffer.
     */
    int pos2offset(int w_x, int w_y);

    /**
     * Is this buffer dirty?
     */
    bool dirty();

    /**
     * Mark the buffer as dirty.
     */
    void set_dirty(bool state);

    /**
     * Get the name of the buffer.
     */
    const char *get_name();

    /**
     * Set the name of the buffer.
     */
    void set_name(const char *name);

    /**
     * Return the modification-number of the buffer.
     */
    int updated();

    /**
     * Bump the modification number.
     */
    void touch();

    /**
     * Get the buffer contents, as text.
     *
     * NOTE: NOT wide-text.
     */
    std::string text();

    /**
     * Update the colours of the current buffer, via the
     * result of the lua callback.
     */
    void update_syntax(const char *colours);

public:

    /* Cursor x and y position in characters */
    int cx, cy;

    /* Offset of row/col displayed. */
    int rowoff, coloff;

    /* the actual rows */
    std::vector<erow *> rows;

    /* Syntax-mode which is in-use. */
    std::string m_syntax;

    /* mark */
    int markx;
    int marky;

private:
    /* Is this buffer dirty? */
    bool m_dirty;

    /* The name of this buffer */
    char *m_name;

    /* Modifiction-marker for the buffer. */
    int m_modified;
};
