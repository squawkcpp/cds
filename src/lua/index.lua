local matches = redis.call('KEYS', 'fs:*:list')
local results = {};
for _,key in ipairs(matches) do

    redis.call('SORT', key, 'BY', 'fs:*->clean_string', 'ALPHA', 'ASC', 'STORE', key .. ':alpha:asc')
    redis.call('SORT', key, 'BY', 'fs:*->clean_string', 'ALPHA', 'DESC', 'STORE', key .. ':alpha:desc')
    redis.call('SORT', key, 'BY', 'fs:*->timestamp', 'ALPHA', 'ASC', 'STORE', key .. ':timestamp:asc')
    redis.call('SORT', key, 'BY', 'fs:*->timestamp', 'ALPHA', 'DESC', 'STORE', key .. ':timestamp:desc')
    redis.call('SADD', key .. ':sort', 'default', 'timestamp', 'alpha' )

    local _parent_key = string.sub(key, 0, string.find( key, ':list')-1)
    local _parent_cls = redis.call('HGET', _parent_key, 'cls' )
    if _parent_key == 'fs:album' or _parent_key == 'fs:movie' then
        redis.call('SORT', key, 'BY', 'fs:*->year', 'ALPHA', 'ASC', 'STORE', key .. ':year:asc')
        redis.call('SORT', key, 'BY', 'fs:*->year', 'ALPHA', 'DESC', 'STORE', key .. ':year:desc')
        redis.call('SADD', key .. ':sort', 'year' )
    end
    if _parent_cls == 'album' then
    elseif _parent_cls == 'movie' then
        table.insert(results, _parent_cls)
    end
end
return results;
