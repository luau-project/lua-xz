package = "lua-xz"
local raw_version = "dev"
version = raw_version .. "-1"

source = {
   url = "git+https://github.com/luau-project/lua-xz"
}

description = {
   homepage = "https://github.com/luau-project/lua-xz",
   license = "MIT",
   summary = [[Streaming interface to read/write xz files in Lua]],
   detailed = [=[lua-xz is a lightweight, native library for Lua providing a streaming interface to read/write xz files.

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
         defines = { "LUA_XZ_BUILD_SHARED" },
         incdirs = { "src", "$(LIBLZMA_INCDIR)" },
         libdirs = { "$(LIBLZMA_LIBDIR)" }
      }
   }
}
