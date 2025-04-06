package = "lua-xz"
local raw_version = "0.0.1"
version = raw_version .. "-1"

source = {
   url = "git+https://github.com/luau-project/lua-xz",
   tag = "v" .. raw_version
}

description = {
   homepage = "https://github.com/luau-project/lua-xz",
   license = "MIT",
   summary = [[Streaming interface to read/write .lzma and .xz files in Lua]],
   detailed = [=[lua-xz is a lightweight, native library for Lua providing a streaming interface to read/write .lzma and .xz files.

Visit the repository for more information.]=]
}

dependencies = {
   "lua >= 5.1"
}

external_dependencies = {
   LIBLZMA = {
      header = "lzma.h"
   }
}

build = {
   type = "builtin",
   modules = {
      ["lua-xz"] = {
         sources = { "src/lua-xz.c" },
         libraries = { "lzma" },
         defines = { "NDEBUG", "_NDEBUG", "LUA_XZ_BUILD_SHARED" },
         incdirs = { "src", "$(LIBLZMA_INCDIR)" },
         libdirs = { "$(LIBLZMA_LIBDIR)" }
      }
   }
}
