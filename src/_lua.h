#ifndef _LUA_H
#define _LUA_H

#include <string>

static const std::string LUA_FLUSH = "local keys = redis.call('keys', ARGV[1]) \n for i=1,#keys,5000 do \n redis.call('del', unpack(keys, i, math.min(i+4999, #keys))) \n end \n return keys";

static const std::string LUA_REDIS_KEY = "local count  = {}\n local results = redis.call('keys', ARGV[1]) \n for i=1,#results do \n local key = unpack(results, i, math.min(i, #results)) \n table.insert( count,key ) \n end \n return count";

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
"return results\n";
#endif // _LUA_H
