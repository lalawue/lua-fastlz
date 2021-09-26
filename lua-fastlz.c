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

static int
_lcompress(lua_State *L) {
	if (lua_type(L, 1) != LUA_TSTRING) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid input params");
		return 2;
	}

	size_t in_sz = 0, out_sz = 0;
	const char *in_str = lua_tolstring(L, 1, &in_sz);
	if (in_sz < 80) {
		out_sz = 128;
	} else {
		out_sz = (size_t)((double)in_sz * 1.06);
	}

	char *out_str = (char *)calloc(1, out_sz);
	if (out_str == NULL) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "failed to malloc");
		return 2;
	}

	int ret = fastlz_compress(in_str, (int)in_sz, out_str);
	lua_pushlstring(L, (const char *)out_str, ret);
	free(out_str);
	return 1;
}

static int
_ldecompress(lua_State *L) {
	if (lua_type(L, 1) != LUA_TSTRING) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid input params");
		return 2;
	}

	size_t in_sz = 0, out_sz = 0;
	const char *in_str = lua_tolstring(L, 1, &in_sz);
	char *out_str = NULL;
	int out_len = 0;
	int ratio = 1;
	do {
		ratio *= 2;
		out_sz = (size_t)(in_sz * ratio);
		out_str = (char *)realloc(out_str, out_sz);
		if (out_str == NULL) {
			break;
		}
		out_len = fastlz_decompress(in_str, (int)in_sz, out_str, (int)out_sz);
	} while (out_len <= 0 && ratio <= 8);

	int ret = 0;
	if (out_len > 0) {
		lua_pushlstring(L, out_str, out_len);
		ret = 1;
	} else {
		char err_msg[32];
		memset(err_msg, 0, 32);
		snprintf(err_msg, 32, "failed to decompress with ratio: %d", ratio);
		lua_pushboolean(L, 0);
		lua_pushstring(L, err_msg);
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
