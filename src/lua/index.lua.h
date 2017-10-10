#ifndef _INDEX_LUA_H
#define _INDEX_LUA_H

#include <string>

static const std::string LUA_INDEX = "local matches = redis.call('KEYS', 'fs:*:list')\n" \
"local results = {};\n" \
"for _,key in ipairs(matches) do\n" \
"\n" \
"    redis.call('SORT', key, 'BY', 'fs:*:node->clean_string', 'ALPHA', 'ASC', 'STORE', key .. ':sort:alpha:asc')\n" \
"    redis.call('SORT', key, 'BY', 'fs:*:node->clean_string', 'ALPHA', 'DESC', 'STORE', key .. ':sort:alpha:desc')\n" \
"    redis.call('SORT', key, 'BY', 'fs:*:node->timestamp', 'ALPHA', 'ASC', 'STORE', key .. ':sort:timestamp:asc')\n" \
"    redis.call('SORT', key, 'BY', 'fs:*:node->timestamp', 'ALPHA', 'DESC', 'STORE', key .. ':sort:timestamp:desc')\n" \
"    redis.call('SADD', key .. ':sort:list', 'default', 'timestamp', 'alpha' )\n" \
"\n" \
"    local _parent_key = string.sub(key, 0, string.find( key, ':list')-1)\n" \
"    local _parent_cls = redis.call('HGET', _parent_key, 'cls' )\n" \
"    if _parent_key == 'fs:album' or _parent_key == 'fs:movie' then\n" \
"        redis.call('SORT', key, 'BY', 'fs:*:node->year', 'ALPHA', 'ASC', 'STORE', key .. ':sort:year:asc')\n" \
"        redis.call('SORT', key, 'BY', 'fs:*->year', 'ALPHA', 'DESC', 'STORE', key .. ':sort:year:desc')\n" \
"        redis.call('SADD', key .. ':sort:list', 'year' )\n" \
"    end\n" \
"    if _parent_cls == 'album' then\n" \
"    elseif _parent_cls == 'movie' then\n" \
"    end\n" \
"end\n" \
"return 0\n";
#endif //_INDEX_LUA
