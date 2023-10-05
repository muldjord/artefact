-- Lua 5.1+ base64 v3.0 (c) 2009 by Alex Kloss <alexthkloss@web.de>
-- Modified Fri Apr 13 10:43:43 CEST 2012 by Jonas Suhr Christensen <jsc@umbraculum.org>
-- Licensed under the terms of the LGPL2

local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

-- encoding
function b64enc(data)
    return ((data:gsub('.', function(x) 
        local r,b='',x:byte()
        for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
        return r;
    end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
        if (#x < 6) then return '' end
        local c=0
        for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
        return b:sub(c+1,c+1)
    end)..({ '', '==', '=' })[#data%3+1])
end

-- decoding
function b64dec(data)
    data = string.gsub(data, '[^'..b..'=]', '')
    return string.sub((data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(b:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
        if (#x ~= 8) then return '' end
        local c=0
        for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
        return string.char(c)
    end)), 0, -1)
end

function test()
  data = "12345678910111213141516181920"
  b64data = b64enc(data)
  TEST_EQUAL_STR("MTIzNDU2Nzg5MTAxMTEyMTMxNDE1MTYxODE5MjA=", b64data, "Testing encoding")
  TEST_EQUAL_STR(data, b64dec(b64data), "Testing encoding and devocinf")
end
