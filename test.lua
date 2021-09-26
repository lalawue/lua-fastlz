local fastlz = require("fastlz")
local in_str = "any content for example"
local data = fastlz.compress(in_str)
local out_str = fastlz.decompress(data)
print("1. compress/decompress result:", in_str == out_str)

local data, err_msg = fastlz.compress(nil)
print("2. error compress:", data, err_msg)

local data, err_msg = fastlz.decompress(in_str)
print("3. error decomperss:", data, err_msg)
