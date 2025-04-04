/*
** The MIT License (MIT)
** 
** Copyright (c) 2025 luau-project [https://github.com/luau-project/lua-xz](https://github.com/luau-project/lua-xz)
** 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** 
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
*/

#include "lua-xz.h"

#include <lauxlib.h>
#include <lualib.h>
#include <lzma.h>
#include <string.h>

/*
** Define LUA_MAXINTEGER and LUA_MININTEGER:
** 
** these values are not defined on Lua 5.3 or older
*/
#if !(defined(LUA_MAXINTEGER) && defined(LUA_MININTEGER))
#if LUA_VERSION_NUM < 503
/* On Lua 5.1 and Lua 5.2, lua_Integer is a ptrdiff_t */
#define LUA_MAXINTEGER PTRDIFF_MAX
#define LUA_MININTEGER PTRDIFF_MIN
#elif LUA_VERSION_NUM == 503
#if LUA_INT_TYPE == LUA_INT_INT
#define LUA_MAXINTEGER INT_MAX
#define LUA_MININTEGER INT_MIN
#elif LUA_INT_TYPE == LUA_INT_LONG
#define LUA_MAXINTEGER LONG_MAX
#define LUA_MININTEGER LONG_MIN
#elif LUA_INT_TYPE == LUA_INT_LONGLONG
#if defined(LLONG_MAX)
#define LUA_MAXINTEGER LLONG_MAX
#define LUA_MININTEGER LLONG_MIN
#elif defined(LUA_USE_WINDOWS)
#define LUA_MAXINTEGER _I64_MAX
#define LUA_MININTEGER _I64_MIN
#else
#error "Compiler does not support 'long long'"
#endif
#endif
#endif
#endif

#if defined(LUA_MAXINTEGER) && defined(LUA_MININTEGER) && (LUA_MAXINTEGER-20 < 32760 || LUA_MAXINTEGER-20 < 2147483640L)
#error "Lua integers must be able to store at least 64-bits"
#endif

/* start of auxiliary functions */
#if LUA_VERSION_NUM < 503
static int lua_xz_aux_isinteger(lua_State *L, int idx)
{
    /*
    ** the body of this function
    ** came from `luaL_checkinteger'
    ** at https://www.lua.org/source/5.1/lauxlib.c.html
    */
    lua_Integer d = lua_tointeger(L, idx);
    return !(d == 0 && !lua_isnumber(L, idx));
}
#else
#define lua_xz_aux_isinteger lua_isinteger
#endif
/* end of auxiliary functions */

/* start of lua_xz_aux_buffers */
#define LUA_XZ_AUX_BUFFERS_METATABLE "lua_xz_aux_buffers_metatable"

typedef struct taglua_xz_aux_buffers
{
    /* size of the input buffer */
    size_t input_buffer_size;

    /* input buffer */
    uint8_t *input_buffer;

    /* size of the output buffer*/
    size_t output_buffer_size;

    /*
    ** note:
    **   output_buffer member must be the last
    **   field of the lua_xz_aux_buffers.
    **   The reason is that we follow
    **   the same idea employed by
    **   Roberto Ierusalimschy in the
    **   double array example on PIL.
    **   In short, we allocate
    **   a struct with size
    **   sizeof(lua_xz_aux_buffers) + (output_buffer_size - 1) * sizeof(uint8_t)
    **   at userdata creation. 
    */
    uint8_t output_buffer[1];
} lua_xz_aux_buffers;

/* resizes the input buffer */
static void *lua_xz_aux_buffers_resize_input_buffer(lua_State *L, int index, size_t new_size)
{
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    lua_xz_aux_buffers *b = (lua_xz_aux_buffers *)lua_touserdata(L, index);
    void *temp = allocf(ud, b->input_buffer, b->input_buffer_size, new_size);
    if (temp == NULL && new_size > 0)
    {
        luaL_error(L, "Failed to allocate memory to resize the auxiliary buffer");
    }
    b->input_buffer = (uint8_t *)temp;
    b->input_buffer_size = new_size;
    return temp;
}

static int lua_xz_aux_buffers_gc(lua_State *L)
{
    lua_xz_aux_buffers_resize_input_buffer(L, 1, 0);
    return 0;
}

static const luaL_Reg lua_xz_aux_buffers_functions[] = {
    { "__gc", lua_xz_aux_buffers_gc },
    { NULL, NULL }
};

static lua_xz_aux_buffers *lua_xz_aux_buffers_new(lua_State *L, size_t output_buffer_size)
{
    lua_xz_aux_buffers *b;
    void *ud = lua_newuserdata(L, sizeof(lua_xz_aux_buffers) + (output_buffer_size - 1) * sizeof(uint8_t));
    if (ud == NULL)
    {
        luaL_error(L, "Failed to allocate memory for the auxiliary buffer");
    }

    if (luaL_newmetatable(L, LUA_XZ_AUX_BUFFERS_METATABLE) > 0)
    {
#if LUA_VERSION_NUM < 502
        luaL_register(L, NULL, lua_xz_aux_buffers_functions);
#else
        luaL_setfuncs(L, lua_xz_aux_buffers_functions, 0);
#endif        
    }
    lua_setmetatable(L, -2);

    b = (lua_xz_aux_buffers *)ud;
    b->input_buffer = NULL;
    b->input_buffer_size = 0;
    b->output_buffer_size = output_buffer_size;

    return b;
}
/* end of lua_xz_aux_buffers */

/* start of lua_xz */
#define LUA_XZ_METATABLE "lua_xz_metatable"

static int lua_xz_newindex(lua_State *L)
{
    return luaL_error(L, "Read-only object");
}
/* end of lua_xz */

/* start of lua_xz_stream */
typedef struct taglua_xz_stream {

    lzma_stream strm;
    int is_writer;
    int executed;
    int is_closed;

} lua_xz_stream;

#define LUA_XZ_STREAM_METATABLE "lua_xz_stream_metatable"

static lua_xz_stream *lua_xz_check_stream(lua_State *L, int index)
{
    void *ud = luaL_checkudata(L, index, LUA_XZ_STREAM_METATABLE);
    luaL_argcheck(L, ud != NULL, index, "lua_xz_stream expected");
    return (lua_xz_stream *)ud;
}

static lua_xz_stream *lua_xz_check_active_stream(lua_State *L, int index)
{
    lua_xz_stream *stream = lua_xz_check_stream(L, index);
    luaL_argcheck(L, !stream->executed, 1, "lua_xz_stream cannot be used after it was executed");
    luaL_argcheck(L, !stream->is_closed, 1, "lua_xz_stream cannot be used after it was closed");
    return stream;
}

static int lua_xz_stream_new(lua_State *L, int is_writer)
{
    /* writer variables and args */
    lua_Integer arg_preset;
    size_t arg_preset_str_len;
    const char *arg_preset_str;
    int first_preset_char;
    lua_Integer arg_check;

    uint32_t preset;
    lzma_check check;

    /* reader variables and args */
    lua_Integer arg_memlimit;
    uint64_t memlimit;
    lua_Integer arg_flags;
    uint32_t flags;

    /* variables for both */
    lua_xz_stream *stream;
    lzma_ret ret;
    lua_Integer arg_buffer_size;
    size_t buffer_size;
    void *ud;

    ud = lua_newuserdata(L, sizeof(lua_xz_stream));
    if (ud == NULL)
    {
        return luaL_error(L, "Failed to create lua_xz_stream userdata");
    }

    luaL_getmetatable(L, LUA_XZ_STREAM_METATABLE);
    lua_setmetatable(L, -2);

    stream = (lua_xz_stream *)ud;
    memset(&stream->strm, 0, sizeof(lzma_stream));
    stream->is_writer = is_writer;
    stream->executed = 0;
    stream->is_closed = 0;

    if (is_writer)
    {
        preset = LZMA_PRESET_DEFAULT;
        if (lua_xz_aux_isinteger(L, 1))
        {
            arg_preset = lua_tointeger(L, 1);
            luaL_argcheck(L, 0 <= arg_preset && arg_preset <= 9, 1, "preset must be an integer in the interval [0, 9]");
            preset = (uint32_t)arg_preset;
        }
        else if (lua_isstring(L, 1))
        {
            arg_preset_str = lua_tolstring(L, 1, &arg_preset_str_len);
            luaL_argcheck(L, 1 <= arg_preset_str_len && arg_preset_str_len <= 2, 1, "preset must be a string with length 1 or 2");
            first_preset_char = arg_preset_str[0] - '0';
            luaL_argcheck(L, 0 <= first_preset_char && first_preset_char <= 9, 1, "first char of preset must be a digit between 0 and 9");
            preset = (uint32_t)first_preset_char;
            if (arg_preset_str_len == 2)
            {
                luaL_argcheck(L, arg_preset_str[1] == 'e', 1, "when specified, the second char of preset must be e");
                preset |= LZMA_PRESET_EXTREME;
            }
        }
        else
        {
            return luaL_error(L, "Invalid preset type");
        }

        arg_check = luaL_checkinteger(L, 2);
        check = (lzma_check)arg_check;

        ret = lzma_easy_encoder(
            &stream->strm,
            preset,
            check);

        if (ret != LZMA_OK)
        {
            switch (ret)
            {
            case LZMA_MEM_ERROR:
                return luaL_error(L, "Memory allocation failed");
            case LZMA_OPTIONS_ERROR:
                return luaL_error(L, "The given compression preset is not supported by this build of liblzma");
            case LZMA_UNSUPPORTED_CHECK:
                return luaL_error(L, "The given check type is not supported by this build of liblzma");
            case LZMA_PROG_ERROR:
                return luaL_error(L, "One or more of the parameters have values that will never be valid");
            default:
                return luaL_error(L, "Failed to create lzma_easy_encoder");
            }
        }
    }
    else
    {
        arg_memlimit = luaL_checkinteger(L, 1);
        luaL_argcheck(L, arg_memlimit == LUA_MININTEGER || arg_memlimit >= 0, 1, "memlimit must be an integer greater than or equal to 0");
        
        arg_flags = luaL_checkinteger(L, 2);
        luaL_argcheck(L, arg_flags >= 0, 2, "flags must be an integer greater than or equal to 0");

        if (arg_memlimit == LUA_MININTEGER)
        {
            memlimit = UINT64_MAX;
        }
        else if (arg_memlimit == 0)
        {
            memlimit = 1;
        }
        else
        {
            memlimit = (uint64_t)arg_memlimit;
        }

        flags = (uint32_t)arg_flags;

        ret = lzma_stream_decoder(
            &stream->strm,
            memlimit,
            flags);

        if (ret != LZMA_OK)
        {
            switch (ret)
            {
            case LZMA_MEM_ERROR:
                return luaL_error(L, "Memory allocation failed");
            case LZMA_OPTIONS_ERROR:
                return luaL_error(L, "Unsupported decompressor flags");
            default:
                return luaL_error(L, "Failed to create lzma_stream_decoder");
            }
        }
    }

    return 1;
}

/* 
** this function follows the pattern
** https://github.com/tukaani-project/xz/raw/c3cb1e53a114ac944f559fe7cac45dbf48cca156/doc/examples/01_compress_easy.c
** to compress data, and
** https://github.com/tukaani-project/xz/raw/c3cb1e53a114ac944f559fe7cac45dbf48cca156/doc/examples/02_decompress.c
** to decompress data
** 
** on top of that,
** we added a producer / consumer
** pattern to the lzma_stream
*/
static int lua_xz_stream_exec(lua_State *L)
{
    lua_xz_stream *stream = lua_xz_check_active_stream(L, 1);

    lzma_ret ret;
    size_t write_size;
    lzma_stream *s;
    size_t produced_data_size = 0;
    const char *produced_data;

    /* 
    ** input buffer allocated on stack, but
    ** switch to a dynamically allocated
    ** input buffer by `lua_xz_aux_buffers *b'
    ** to be able to hold a copy to the
    ** last produced data
    */
    uint8_t input_buffer[LUA_XZ_BUFFER_SIZE];

    /* 
    ** dynamically allocated
    ** (input and output) buffers to hold
    ** data for the lzma_stream
    */
    lua_xz_aux_buffers *b;

    /*
    ** use LZMA_RUN until the producer function
    ** returns nil or none, because
    ** a produced nil or none signals
    ** that it is time to finish the stream
    ** by setting LZMA_FINISH
    */
    lzma_action action = LZMA_RUN;

    lua_Integer arg_output_buffer_size;
    size_t output_buffer_size;

    /* prevent exec from running again */
    stream->executed = 1;

    /*
    ** validate buffer size to be able
    ** to create the output buffer
    ** of lua_xz_aux_buffers with correct size
    */
    if (lua_xz_aux_isinteger(L, 4))
    {
        arg_output_buffer_size = lua_tointeger(L, 4);
    }
    else
    {
        arg_output_buffer_size = LUA_XZ_BUFFER_SIZE;
    }

    if (arg_output_buffer_size <= 0)
    {
        return luaL_error(L, "Buffer size must be a positive integer");
    }

    output_buffer_size = (size_t)arg_output_buffer_size;

    /* assert that producer is a function */
    luaL_checktype(L, 2, LUA_TFUNCTION);

    /* assert that consumer is a function */
    luaL_checktype(L, 3, LUA_TFUNCTION);

    /* create the aux buffers */
    b = lua_xz_aux_buffers_new(L, output_buffer_size);

    s = &stream->strm;

    s->next_in = NULL;
    s->avail_in = 0;
    s->next_out = b->output_buffer;
    s->avail_out = b->output_buffer_size;

    while (1)
    {
        if (s->avail_in == 0 && action != LZMA_FINISH)
        {
            /* push the producer function */
            lua_pushvalue(L, 2);
            
            if (lua_pcall(L, 0, 1, 0) == 0)
            {
                if (lua_isnoneornil(L, -1))
                {
                    s->next_in = NULL;
                    s->avail_in = 0;
                    action = LZMA_FINISH;
                }
                else
                {
                    produced_data = luaL_checklstring(L, -1, &produced_data_size);

                    if (produced_data_size > sizeof(input_buffer))
                    {
                        /* input buffer on stack is not enough, fallback to the dynamic buffer */

                        if (b->input_buffer == NULL || produced_data_size > b->input_buffer_size)
                        {
                            /* dynamic input buffer is small, grow it and copy data */

                            lua_xz_aux_buffers_resize_input_buffer(L, -2, produced_data_size);
                            s->next_in = b->input_buffer;
                            s->avail_in = produced_data_size;
                            memcpy((void *)b->input_buffer, (const void *)produced_data, produced_data_size);
                        }
                        else
                        {
                            /* dynamic input buffer is enough, copy data to it */

                            s->next_in = b->input_buffer;
                            s->avail_in = produced_data_size;
                            memcpy((void *)b->input_buffer, (const void *)produced_data, produced_data_size);
                        }
                    }
                    else
                    {
                        /* input buffer on stack is enough, copy data to it */

                        s->next_in = (uint8_t *)produced_data;
                        s->avail_in = produced_data_size;
                        memcpy((void *)input_buffer, (const void *)produced_data, produced_data_size);
                    }

                    /* remove the produced data */
                    lua_pop(L, 1);
                }
            }
            else
            {
                return luaL_error(L, lua_tostring(L, -1));
            }
        }

        /* do the encoding / decoding */
        ret = lzma_code(s, action);

        /* output buffer is full or compression finished successfully */
        if (s->avail_out == 0 || ret == LZMA_STREAM_END)
        {
            write_size = b->output_buffer_size - s->avail_out;

            /* push the consumer function */
            lua_pushvalue(L, 3);

            /* push the arg of the consumer function */
            lua_pushlstring(L, (const char *)b->output_buffer, write_size);

            /* call the consumer function */
            if (lua_pcall(L, 1, 0, 0) == 0)
            {
                s->next_out = b->output_buffer;
                s->avail_out = b->output_buffer_size;
            }
            else
            {
                return luaL_error(L, lua_tostring(L, -1));
            }
        }

        if (ret != LZMA_OK)
        {
            if (ret == LZMA_STREAM_END)
            {
                return 0;
            }

            if (stream->is_writer)
            {
                switch (ret)
                {
                case LZMA_MEM_ERROR:
                    return luaL_error(L, "Memory allocation failed in the writer stream");
                case LZMA_DATA_ERROR:
                    return luaL_error(L, "File size limits exceeded");
                default:
                    return luaL_error(L, "Unknown error, possibly a bug in the writer stream");
                }
            }
            else
            {
                switch (ret)
                {
                case LZMA_MEM_ERROR:
                    return luaL_error(L, "Memory allocation failed in the reader stream");
                case LZMA_FORMAT_ERROR:
                    return luaL_error(L, "The input is not in the .xz format");
                case LZMA_OPTIONS_ERROR:
                    return luaL_error(L, "Unsupported compression options");
                case LZMA_DATA_ERROR:
                    return luaL_error(L, "Compressed file is corrupt");
                case LZMA_BUF_ERROR:
                    return luaL_error(L, "Compressed file is truncated or otherwise corrupt");
                default:
                    return luaL_error(L, "Unknown error, possibly a bug in the reader stream");
                }
            }
        }
    }

    return 0;
}

static int lua_xz_stream_writer(lua_State *L)
{
    return lua_xz_stream_new(L, 1);
}

static int lua_xz_stream_reader(lua_State *L)
{
    return lua_xz_stream_new(L, 0);
}

static int lua_xz_stream_close(lua_State *L)
{
    lua_xz_stream *stream = lua_xz_check_stream(L, 1);
    if (!stream->is_closed)
    {
        /* free the lzma_stream */
        lzma_end(&stream->strm);

        /* if input_buffer was allocated, free it */
        // if (stream->input_buffer != NULL)
        // {
        //     lua_xz_aux_free(L, (void *)stream->input_buffer);
        // }

        /* prevent it from being called again */
        stream->is_closed = 1;
    }
    return 0;
}

static int lua_xz_stream_newindex(lua_State *L)
{
    return luaL_error(L, "Read-only object");
}

static const luaL_Reg lua_xz_stream_functions[] = {
    {"close", lua_xz_stream_close},
    {"exec", lua_xz_stream_exec},
    {"reader", lua_xz_stream_reader},
    {"writer", lua_xz_stream_writer},
    {"__gc", lua_xz_stream_close},
    {NULL, NULL}
};
/* end of lua_xz_stream */

/* exporting the library */
LUA_XZ_EXPORT int luaopen_xz(lua_State *L)
{
    lua_createtable(L, 0, 0);
    luaL_newmetatable(L, LUA_XZ_METATABLE);

    /* start of lua_xz constants */
    lua_pushstring(L, "version");
    lua_pushstring(L, LUA_XZ_BINDING_VERSION);
    lua_settable(L, -3);

    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, lzma_version_string());
    lua_settable(L, -3);

    lua_pushstring(L, "MEMLIMIT_UNLIMITED");
    lua_pushinteger(L, LUA_MININTEGER);
    lua_settable(L, -3);

    lua_pushstring(L, "CONCATENATED");
    lua_pushinteger(L, LZMA_CONCATENATED);
    lua_settable(L, -3);

    lua_pushstring(L, "PRESET_DEFAULT");
    lua_pushinteger(L, LZMA_PRESET_DEFAULT);
    lua_settable(L, -3);

    lua_pushstring(L, "CHECK_NONE");
    lua_pushinteger(L, LZMA_CHECK_NONE);
    lua_settable(L, -3);

    lua_pushstring(L, "CHECK_CRC32");
    lua_pushinteger(L, LZMA_CHECK_CRC32);
    lua_settable(L, -3);

    lua_pushstring(L, "CHECK_CRC64");
    lua_pushinteger(L, LZMA_CHECK_CRC64);
    lua_settable(L, -3);

    lua_pushstring(L, "CHECK_SHA256");
    lua_pushinteger(L, LZMA_CHECK_SHA256);
    lua_settable(L, -3);
    /* start of lua_xz constants */

    /* start of lua_xz_stream */
    lua_pushstring(L, "stream");

    lua_createtable(L, 0, 0);
    luaL_newmetatable(L, LUA_XZ_STREAM_METATABLE);

#if LUA_VERSION_NUM < 502
    luaL_register(L, NULL, lua_xz_stream_functions);
#else
    luaL_setfuncs(L, lua_xz_stream_functions, 0);
#endif

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    lua_pushstring(L, "__metatable");
    lua_pushboolean(L, 0);
    lua_settable(L, -3);

    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_xz_stream_newindex);
    lua_settable(L, -3);

    lua_setmetatable(L, -2); /* setmetatable(lua_xz_stream, LUA_XZ_STREAM_METATABLE) */

    lua_settable(L, -3); /* lua_xz.stream = lua_xz_stream */
    /* end of lua_xz_stream */

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    lua_pushstring(L, "__metatable");
    lua_pushboolean(L, 0);
    lua_settable(L, -3);

    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_xz_newindex);
    lua_settable(L, -3);

    lua_setmetatable(L, -2); /* setmetatable(lua_xz, LUA_XZ_METATABLE) */

    return 1;
}