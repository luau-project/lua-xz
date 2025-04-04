local xz = require("lua-xz")

local function format_integer(val, n)
    local s = tostring(val)
    return (" "):rep(n - #s) .. s
end

local constants = {
    {text = "liblzma version:", value = xz._VERSION, type = "string"},
    {text = "binding version:", value = xz.version, type = "string"},
    {text = "MEMLIMIT_UNLIMITED:", value = xz.MEMLIMIT_UNLIMITED, type = "number"},
    {text = "CONCATENATED:", value = xz.CONCATENATED, type = "number"},
    {text = "PRESET_DEFAULT:", value = xz.PRESET_DEFAULT, type = "number"},
    {text = "CHECK_NONE:", value = xz.CHECK_NONE, type = "number"},
    {text = "CHECK_CRC32:", value = xz.CHECK_CRC32, type = "number"},
    {text = "CHECK_CRC64:", value = xz.CHECK_CRC64, type = "number"},
    {text = "CHECK_SHA256:", value = xz.CHECK_SHA256, type = "number"}
}

for _, c in ipairs(constants) do
    local t = type(c.value)
    if (c.type ~= t) then
        local errfmt = "type mismatch (%s). Expected: (%s), Got: (%s)"
        error(errfmt:format(c.text, c.type, t))
    end

    if (t == 'string') then
        local msgfmt = "%22s\t%22s\t(%s)"
        print(msgfmt:format(c.text, c.value, t))
    else
        local msgfmt = "%22s\t%s\t(%s)"
        print(msgfmt:format(c.text, format_integer(c.value, 22), t))
    end
end