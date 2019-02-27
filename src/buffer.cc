/* buffer.cc - Our buffer/row implementation.
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


#include <string.h>
#include "buffer.h"

/**
 * Constructor.
 */
erow::erow()
{
    chars = new std::vector<std::wstring>;
    cols  = new std::vector<int>;
}


/**
 * Destructor.
 */
erow::~erow()
{
    delete (chars);
    delete (cols);
}

/**
 * The text of the row, from the given character offset.
 */
std::wstring erow::text(int offset)
{
    int max = chars->size();

    std::wstring ret;

    while (offset < max)
    {
        ret += chars->at(offset);
        offset++;
    }

    return (ret);
}

/**
 * Constructor.
 */
Buffer::Buffer(const char *bname)
{
    cx       = 0;
    cy       = 0;
    markx    = -1;
    marky    = -1;
    rowoff   = 0;
    coloff   = 0;
    m_name   = strdup(bname);
    m_dirty  = false;
    m_syntax = "";

    /*
     * The buffer will have one (empty) row.
     */
    erow *row = new erow();
    rows.push_back(row);
};


/**
 * Destructor
 */
Buffer::~Buffer()
{
    /*
     * Remove the rows
     */
    for (std::vector<erow *>::iterator it = rows.begin(); it != rows.end(); ++it)
    {
        erow *row = (*it);
        delete (row);
    }

    rows.clear();

    if (m_name)
        free(m_name);
}


/**
 * Remove all text from the buffer.
 */
void Buffer::empty_buffer()
{
    for (std::vector<erow *>::iterator it = rows.begin(); it != rows.end(); ++it)
    {
        erow *row = (*it);
        delete (row);
    }

    rows.clear();
    cx         = 0;
    cy         = 0;
    markx      = -1;
    marky      = -1;
    rowoff     = 0;
    coloff     = 0;

    /*
     * The buffer will have one (empty) row.
     */
    erow *row = new erow();
    rows.push_back(row);
}


/**
 * Get the character offset of the given X,Y coordinate in our
 * buffer.
 */
int Buffer::pos2offset(int w_x, int w_y)
{
    int nrows = rows.size();
    int count = 0;

    for (int y = 0; y < nrows; y++)
    {
        erow *row    = rows.at(y);
        int row_size = row->chars->size();

        /*
         * NOTE: We add one character to the row
         * to cope with the trailing newline.
         */
        for (int x = 0; x < row_size + 1; x++)
        {
            if (x == w_x && y == w_y)
                return count;

            count += 1;
        }
    }

    return -1;
}

/**
 * Is this buffer dirty?
 */
bool Buffer::dirty()
{
    // buffers are never dirty, only files.
    if (m_name && (m_name[0] == '*'))
        return false;

    return (m_dirty);
}


/**
 * Mark the buffer as dirty.
 */
void Buffer::set_dirty(bool state)
{
    m_dirty = state;
}

/**
 * Get the name of the buffer.
 */
const char *Buffer::get_name()
{
    return (m_name);
}


/**
 * Set the name of the buffer.
 */
void Buffer::set_name(const char *name)
{
    if (m_name)
        free(m_name);

    m_name = strdup(name);
}


/**
 * Get the buffer contents, as text.
 *
 * NOTE: NOT wide-text.
 */
std::string Buffer::text()
{
    std::string text;

    int row_count = rows.size();

    for (int y = 0; y < row_count; y++)
    {
        int chars = rows.at(y)->chars->size();

        for (int x = 0; x < chars; x++)
        {
            /*
             * We append the character at the row,col position.
             *
             * NOTE This might be an N-byte character string, we
             * deliberately only use the first because LPEG doesn't
             * even handle UTF-8, so it doesn't matter.
             *
             * We could have said this instead:
             *
             *   if (cur->rows.at(y)->chars->at(x)->size() > 1 )
             *      text += "?";
             *   else
             *      text += cur->rows.at(y)->chars->at(x);
             *
             */
            text += rows.at(y)->chars->at(x)[0];
        }

        text += '\n';
    }

    return (text);
}


/**
 * Update the colours of the current buffer, via the
 * result of the lua callback.
 */
void Buffer::update_syntax(const char *colours, size_t len)
{
    /*
     * Free the current colour, if any.
     */
    int row_count = rows.size();

    for (int y = 0; y < row_count; y++)
        rows.at(y)->cols->clear();

    /*
     * Now we'll update the colour of each character.
     */
    int done = 0;

    for (int y = 0; y < row_count; y++)
    {
        /*
         * The current row.
         */
        erow *crow = rows.at(y);

        /*
         * For each character in the row, set the colour
         * to be the return value.
         */
        for (int x = 0; x < (int)crow->chars->size(); x++)
        {
            if (done < (int)len)
                crow->cols->push_back(colours[done]);
            else
                crow->cols->push_back(7) ; /* white */

            done += 1;
        }

        /*
         * those damn newlines.
         */
        done += 1;
    }

}


/**
 * Get per-buffer data.
 */
std::string Buffer::get_data(std::string key)
{
    return (m_data[key]);
}


/**
 * Set per-buffer data.
 */
void Buffer::set_data(std::string key, std::string value)
{
    m_data[key] = value;
}
