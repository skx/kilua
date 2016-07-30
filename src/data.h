/*
 * An editor has : buffers.
 *
 * A buffer has rows.
 *
 * A row has characters.
 *
 * --
 *
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
    erow()
    {
        chars = new std::vector<std::wstring>;
        cols  = new std::vector<int>;
    };
    ~erow()
    {
        delete(chars);
        delete(cols);
    };

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
    Buffer(const char *bname)
    {
        cx       = 0;
        cy       = 0;
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

    ~Buffer()
    {
        /*
         * Remove the rows
         */
        for (std::vector<erow *>::iterator it = rows.begin(); it != rows.end(); ++it)
        {
            erow *row = (*it);
            delete(row);
        }

        rows.clear();

        if (m_name)
            free(m_name);
    };

    /**
     * Remove all text from the buffer.
     */
    void empty_buffer()
    {
        for (std::vector<erow *>::iterator it = rows.begin(); it != rows.end(); ++it)
        {
            erow *row = (*it);
            delete(row);
        }


        rows.clear();
        cx      = 0;
        cy      = 0;
        rowoff  = 0;
        coloff  = 0;

        /*
         * The buffer will have one (empty) row.
         */
        erow *row = new erow();
        rows.push_back(row);
    }


    /**
     * Is this buffer dirty?
     */
    bool dirty()
    {
        // buffers are never dirty, only files.
        if (m_name && (m_name[0] == '*'))
            return false;

        return (m_dirty);
    };


    /**
     * Mark the buffer as dirty.
     */
    void set_dirty(bool state)
    {
        m_dirty = state;
    }

    /**
     * Get the name of the buffer.
     */
    const char *get_name()
    {
        return (m_name);
    };


    /**
     * Set the name of the buffer.
     */
    void set_name(const char *name)
    {
        if (m_name)
            free(m_name);

        m_name = strdup(name);
    }

public:
    /* Cursor x and y position in characters */
    int cx, cy;

    /* Offset of row/col displayed. */
    int rowoff, coloff;

    /* the actual rows */
    std::vector<erow *> rows;

    /* Syntax-mode which is in-use. */
    std::string m_syntax;

private:
    /* Is this buffer dirty? */
    bool m_dirty;

    /* The name of this buffer */
    char *m_name;

};


/**
 * This structure represents the global state of the editor.
 */
class editorState
{
public:
    /*
     *  Number of rows that we can show.
     */
    int screenrows;

    /*
     * Number of cols that we can show
     */
    int screencols;

    /*
     * The status-message
     */
    char statusmsg[255];

    /*
     * The buffers we have.
     */
    std::vector<Buffer *> buffers;

    /*
     * The currently selected buffer.
     */
    int current_buffer ;

}
;
