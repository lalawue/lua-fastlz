/*
 * Copyright (c) 2021 lalawue
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "fastlz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* check types in lua stack */
static int
_check_type(lua_State *L, int *types, int count) {
	int i = 0;
	for (i=0; i<count; i++) {
		if (lua_type(L, i+1) != types[i]) {
			return 0;
		}
	}
	return 1;
}

/* .compress(string) or .compress(level, string), store original size in the very beginning
 */
static int
_lcompress(lua_State *L) {
	int index = 1;
	int level = 1;

	if (lua_type(L, index) == LUA_TNUMBER) {
		level = lua_tonumber(L, index);
		index += 1;
	}

	if (lua_type(L, index) != LUA_TSTRING) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid input params");
		return 2;
	}

	size_t in_sz = 0, out_sz = 0;
	const uint8_t *in_str = (const uint8_t*)lua_tolstring(L, index, &in_sz);
	if (in_sz < 80) {
		out_sz = 128;
	} else {
		out_sz = (size_t)((double)in_sz * 1.06) + 4;
	}

	uint8_t *out_str = (uint8_t *)malloc(out_sz);
	if (out_str == NULL) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "failed to malloc");
		return 2;
	}

	int ret = 0;
	if (level == 1 || level == 2) {
		ret = fastlz_compress_level(level, in_str, (int)in_sz, out_str + 4);
	} else {
		ret = fastlz_compress(in_str, (int)in_sz, out_str + 4);
	}

	out_str[0] = (in_sz >> 24) & 0xff;
	out_str[1] = (in_sz >> 16) & 0xff;
	out_str[2] = (in_sz >> 8) & 0xff;
	out_str[3] = in_sz & 0xff;

	lua_pushlstring(L, (const char *)out_str, ret + 4);
	free(out_str);
	return 1;
}

/* decompress string
 */
static int
_ldecompress(lua_State *L) {
	if (lua_type(L, 1) != LUA_TSTRING) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid input params");
		return 2;
	}

	size_t in_sz = 0, out_sz = 0;
	const uint8_t *in_str = (const uint8_t *)lua_tolstring(L, 1, &in_sz);
	uint8_t *out_str = NULL;

	out_sz = ((in_str[0] << 24) | (in_str[1] << 16) | (in_str[2] << 8) | in_str[3]) + 8;
	out_str = (uint8_t *)malloc(out_sz);
	if (out_str == NULL) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "failed to malloc decompress space");
		return 2;
	}

	int out_len = fastlz_decompress(in_str + 4, (int)in_sz - 4, out_str, (int)out_sz);
	int ret = 0;
	if (out_len > 0) {
		lua_pushlstring(L, (const char *)out_str, out_len);
		ret = 1;
	} else {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "failed to decompress");
		ret = 2;
	}
	free(out_str);
	return ret;
}

static const luaL_Reg fastlz_lib[] = {
	{ "compress", _lcompress},
	{ "decompress", _ldecompress},
	{ NULL, NULL }
};

LUALIB_API int
luaopen_fastlz(lua_State *L) {
	luaL_newlib(L, fastlz_lib);
	return 1;
}