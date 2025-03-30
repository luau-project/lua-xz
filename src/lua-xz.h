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

#ifndef LUA_XZ_H
#define LUA_XZ_H

#include <lua.h>

#define LUA_XZ_BINDING_VERSION_MAJOR "0"
#define LUA_XZ_BINDING_VERSION_MINOR "0"
#define LUA_XZ_BINDING_VERSION_PATCH "1"
#define LUA_XZ_BINDING_VERSION LUA_XZ_BINDING_VERSION_MAJOR "." LUA_XZ_BINDING_VERSION_MINOR "." LUA_XZ_BINDING_VERSION_PATCH

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

#ifndef LUA_XZ_EXPORT /* { */
#ifdef LUA_XZ_BUILD_STATIC /* { */
#define LUA_XZ_EXPORT
#else /* }{ */
#ifdef LUA_XZ_BUILD_SHARED /* { */
#if defined(_WIN32) /* { */
#if defined(__GNUC__) || defined(__MINGW32__) /* { */
#define LUA_XZ_EXPORT __attribute__((dllexport))
#else /* }{ */
#define LUA_XZ_EXPORT __declspec(dllexport)
#endif /* } */
#else /* }{ */
#define LUA_XZ_EXPORT __attribute__((visibility("default")))
#endif /* } */
#else /* }{ */
#if defined(_WIN32) /* { */
#if defined(__GNUC__) || defined(__MINGW32__) /* { */
#define LUA_XZ_EXPORT __attribute__((dllimport))
#else /* }{ */
#define LUA_XZ_EXPORT __declspec(dllimport)
#endif /* } */
#else /* }{ */
#define LUA_XZ_EXPORT
#endif /* } */
#endif /* } */
#endif /* } */
#endif /* } */

#ifdef __cplusplus
extern "C" {
#endif

LUA_XZ_EXPORT int luaopen_xz(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif