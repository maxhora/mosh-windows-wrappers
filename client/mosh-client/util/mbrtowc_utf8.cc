//
// Created by dimbo on 02.04.2019.
//

#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <cwchar>

#include <winnls.h>
#include <xlocinfo.h>

#include "mbrtowc_utf8.h"


// taken from the CRT sources of VS2017 CE v.14.16.27023


static int _Utf8_trailing_byte_count(unsigned long * partialCh, unsigned char ch)
{
    if ((ch & 0x80u) == 0x00u)
    {   // high bit unset, plain ASCII
        return 0;
    }

    if ((ch & 0xE0u) == 0xC0u)
    {   // first 3 bits are 110, 1 trailing byte
        *partialCh = ch & 0x1Fu;
        return 1;
    }

    if ((ch & 0xF0u) == 0xE0u)
    {   // first 4 bits are 1110, 2 trailing bytes
        *partialCh = ch & 0x0Fu;
        return 2;
    }

    if ((ch & 0xF8u) == 0xF0u)
    {   // first 5 bits are 11110, 3 trailing bytes
        *partialCh = ch & 0x07u;
        return 3;
    }

    return INT_MAX;
}

static int _Decode_utf8_trailing_byte(unsigned long * partialCh, unsigned char ch)
{
    if ((ch & 0xC0) == 0x80)
    {
        *partialCh <<= 6;
        *partialCh |= ch & 0x3Fu;
        return 0;
    }

    return 1;
}

/***
*int _Mbrtowc() - Convert multibyte char to wide character.
*
*Purpose:
*       Convert a multi-byte character into the equivalent wide character,
*       according to the specified LC_CTYPE category, or the current locale.
*       [ANSI].
*Entry:
*       wchar_t  *pwc = pointer to destination wide character
*       const char *s = pointer to multibyte character
*       size_t      n = maximum length of multibyte character to consider
*       mbstate_t *pst      = pointer to state
*       const _Cvtvec *     = pointer to locale info
*
*Exit:
*       Returns:  0 (if *s = null char)
*                -1 (if the next n or fewer bytes not valid mbc)
*                -2 (if partial conversion)
*                 number of bytes comprising converted mbc
*******************************************************************************/

int mbrtowc_utf8(wchar_t *pwc, const char *s, size_t n)
{
    if (n == 0)
    {   /* indicate do not have state-dependent encodings, handle zero length string */
        return 0;
    }

    if (!*s)
    {   /* handle NULL char, TRANSITION, VSO#654347 */
        *pwc = 0;
        return 0;
    }

    /* perform locale-dependent parse */
    unsigned char ch = (unsigned char)*s;
    unsigned long wch;
    int consumedCount;
    int trailingUtf8Units;

    trailingUtf8Units = _Utf8_trailing_byte_count(&wch, ch);
    if (trailingUtf8Units == 0)
    {   // optimistic ASCII passsthrough
        *pwc = (wchar_t)ch;
        return 1;
    }

    if (trailingUtf8Units > 2)
    {   // this would result in a UTF-16 surrogate pair, which we can't emit in our
        // singular output wchar_t, so fail
        // see N4750 [locale.codecvt.virtuals]/3
        errno = EILSEQ;
        return -1;
    }

    consumedCount = 1;
    for (; trailingUtf8Units; ++consumedCount, --trailingUtf8Units)
    {   // consume remaining trailing bytes
        if ((size_t)consumedCount >= n)
        {   // no byte to consume, return partial
            return -2;
        }

        if (_Decode_utf8_trailing_byte(&wch, (unsigned char)s[consumedCount]))
        {
            errno = EILSEQ;
            return -1;
        }
    }

    if (wch >= 0xD800u && wch <= 0xDFFFu)
    {   // tried to decode unpaired surrogate
        errno = EILSEQ;
        return -1;
    }

    *pwc = (wchar_t)wch;
    return consumedCount;
}

