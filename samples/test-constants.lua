local xz = require("lua-xz")

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

    local msgfmt
    if (t == 'string') then
        msgfmt = "%22s\t%22s\t(%s)"
    else
        msgfmt = "%22s\t%22d\t(%s)"
    end
    
    print(msgfmt:format(c.text, c.value, t))
end