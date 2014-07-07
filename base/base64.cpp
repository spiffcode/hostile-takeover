/*
 * FILE:   base64.c
 * AUTHOR: Colin Perkins
 *
 * MIME base64 encoder/decoder described in rfc1521. This code is derived
 * from version 2.7 of the Bellcore metamail package.
 *
 * Copyright (c) 1998-2000 University College London
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Computer Science
 *      Department at University College London
 * 4. Neither the name of the University nor of the Department may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)
 *
 * Permission to use, copy, modify, and distribute this material
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies, and that the name of Bellcore not be
 * used in advertising or publicity pertaining to this
 * material without the specific, prior written permission
 * of an authorized representative of Bellcore.  BELLCORE
 * MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
 * OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
 *
 */

#include "base/base64.h"

namespace base {

static unsigned char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64encode(const unsigned char *input, int input_length,
        unsigned char *output, int output_length)
{
    int i = 0, j = 0;
    int pad;

    if (output_length < (input_length * 4 / 3)) {
        return -1;
    }

    while (i < input_length) {
        pad = 3 - (input_length - i);
        if (pad == 2) {
            output[j  ] = basis_64[input[i]>>2];
            output[j+1] = basis_64[(input[i] & 0x03) << 4];
            output[j+2] = '=';
            output[j+3] = '=';
        } else if (pad == 1) {
            output[j  ] = basis_64[input[i]>>2];
            output[j+1] = basis_64[((input[i] & 0x03) << 4) |
                    ((input[i+1] & 0xf0) >> 4)];
            output[j+2] = basis_64[(input[i+1] & 0x0f) << 2];
            output[j+3] = '=';
        } else{
            output[j  ] = basis_64[input[i]>>2];
            output[j+1] = basis_64[((input[i] & 0x03) << 4) |
                    ((input[i+1] & 0xf0) >> 4)];
            output[j+2] = basis_64[((input[i+1] & 0x0f) << 2) |
                    ((input[i+2] & 0xc0) >> 6)];
            output[j+3] = basis_64[input[i+2] & 0x3f];
        }
        i += 3;
        j += 4;
    }
    return j;
}

/* This assumes that an unsigned char is exactly 8 bits. Not portable code! :-) */
static unsigned char index_64[128] = {
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff
};

#define char64(c)  ((c > 127) ? 0xff : index_64[(c)])

int base64decode(const unsigned char *input, int input_length, unsigned char *output, int output_length)
{
    int i = 0, j = 0, pad;
    unsigned char c[4];

    if (output_length < (input_length * 3 / 4)) {
        return -1;
    }
    if ((input_length % 4) != 0) {
        return -1;
    }
    while ((i + 3) < input_length) {
        pad  = 0;
        c[0] = char64(input[i  ]); pad += (c[0] == 0xff);
        c[1] = char64(input[i+1]); pad += (c[1] == 0xff);
        c[2] = char64(input[i+2]); pad += (c[2] == 0xff);
        c[3] = char64(input[i+3]); pad += (c[3] == 0xff);
        if (pad == 2) {
            output[j++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
            output[j]   = (c[1] & 0x0f) << 4;
        } else if (pad == 1) {
            output[j++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
            output[j++] = ((c[1] & 0x0f) << 4) | ((c[2] & 0x3c) >> 2);
            output[j]   = (c[2] & 0x03) << 6;
        } else {
            output[j++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
            output[j++] = ((c[1] & 0x0f) << 4) | ((c[2] & 0x3c) >> 2);
            output[j++] = ((c[2] & 0x03) << 6) | (c[3] & 0x3f);
        }
        i += 4;
    }
    return j;
}

} // namespace base
