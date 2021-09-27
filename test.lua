local fastlz = require("fastlz")

local in_str = "any content for example"

local data1 = fastlz.compress(in_str)
local out_str1 = fastlz.decompress(data1)
print("1. compress/decompress level 1 result:", in_str == out_str1)

local data2 = fastlz.compress(2, in_str)
local out_str2 = fastlz.decompress(data2)
print("2. compress/decompress level 2 result:", in_str == out_str2)

local data, err_msg = fastlz.compress(nil)
print("3. error compress:", data, err_msg)

local data, err_msg = fastlz.decompress(in_str)
print("4. error decomperss:", data, err_msg)
