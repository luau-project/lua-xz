local xz = require("lua-xz")

-- simulate content to be streamed to the encoder
local inputs = {"hello", " ", "world"}

-- table to hold the output of each `update' call
-- on both writer / reader streams
local outputs = {}

--[[ start of encoding ]]
-- create a writer stream
local writer_stream = xz.stream.writer(xz.PRESET_DEFAULT, xz.CHECK_CRC64)

-- push each element in the `inputs' table
-- onto the writer stream
for _, IN in ipairs(inputs) do
    table.insert(outputs, writer_stream:update(IN))
end
table.insert(outputs, writer_stream:finish())

-- close the writer stream to free resources
-- tip: it is automatically freed on garbage collection
writer_stream:close()

-- write each compressed chunk to the destination file
local output_file = assert(io.open("lua-xz.xz", "wb"), "failed to open the file lua-xz.xz for writing")
for _, OUT in ipairs(outputs) do
    output_file:write(OUT)
end
output_file:close()
--[[ end of encoding]]

--[[ start of decoding ]]
-- create a reader stream
local input_stream = xz.stream.reader(xz.MEMLIMIT_UNLIMITED, xz.CONCATENATED)

-- clear the outputs to fill it again
-- with the decoding output
for i = #outputs, 1, -1 do
    table.remove(outputs, i)
end

-- open the file created above
-- to feed the reader stream
-- with content to decode
local input_file = assert(io.open("lua-xz.xz", "rb"), "failed to open the file lua-xz.xz for reading")

-- read chunks of 8 bytes
local chunk_size = 8

-- read first chunk
local OUT = input_file:read(chunk_size)

-- keep reading the file while
-- the chunk is not nil
while (OUT ~= nil) do
    table.insert(outputs, input_stream:update(OUT))
    OUT = input_file:read(chunk_size)
end
table.insert(outputs, input_stream:finish())
input_file:close()

-- close the reader stream to free resources
-- tip: it is automatically freed on garbage collection
input_stream:close()
--[[ end of decoding]]

-- make sure that the decoded data
-- matches the initial inputs
assert(table.concat(inputs) == table.concat(outputs))