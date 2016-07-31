/* util.h - character-set related utility-functions.
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



/**
 * A utility class for converting strings and characters.
 *
 * We assume that a single ASCII character can become a wide
 * character of no more than 6 bytes.
 */
class Util
{
public:
    static const int max_len = 6;


    /**
     * Convert the single wide-character specified to
     * an ASCII string.
     *
     * NOTE: The caller must `delete[]` the result.
     */
    static char * wchar2ascii(wchar_t ch)
    {
        std::wstring tmp;
        tmp += ch;

        char *str = new char[Util::max_len];
        sprintf(str, "%ls", tmp.c_str());

        return (str);
    }


    /**
     * Convert a normal string to a wide string.
     *
     * NOTE: The caller must `delete[]` the result.
     */
    static wchar_t * ascii2wide(const char *in)
    {
        size_t in_size  = strlen(in) + 1;
        size_t out_size = in_size * Util::max_len;

        wchar_t* result = new wchar_t[out_size];

        mbstowcs(result, in, out_size);
        return (result);
    };


    /**
     * Convert the wide-character string specified to
     * an ASCII string.
     *
     * NOTE: The caller must `delete[]` the result.
     */
    static char * wide2ascii(wchar_t *in)
    {
        std::wstring tmp;
        tmp += in;

        char *str = new char[(tmp.size() * Util::max_len) + 1];
        sprintf(str, "%ls", in);

        return (str);
    };

    static char * widestr2ascii(std::wstring in)
    {
        size_t len = in.size() * 5 + 1;
        char *str = new char[len + 1];
        sprintf(str, "%ls", in.c_str());
        return (str);
    };

};
