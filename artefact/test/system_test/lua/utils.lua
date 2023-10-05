function loadDataFromFile(filename)
  local file = assert(io.open(filename), "Could not open file.")
  local data = assert(file:read("*all"), "Error reading file.")
  file:close()
  return data;
end


function _cut(string, prefix, suffix)
  if not string or not prefix or not suffix then return false end
--  print("cut: ", string, prefix, suffix, string.len(prefix), string.len(suffix))
  i, j, cutted = string.find(string, prefix .. "(.-)" .. suffix)
--  print("cutted: ", i, j, cutted) 
  return cutted
end

function compareAndReturn(first_string, second_string, largest)
  if(first_string and second_string) then
    if(largest) then
      if(string.len(first_string) > string.len(second_string)) then
        return first_string
      else
        return second_string
    end
    else
      if string.len(first_string) < string.len(second_string) then
        return first_string
      else
        return second_string
      end
    end
  else
    if(first_string) then return first_string
    else return second_string end
  end

  return false
end

function cut(data, from_list, to_list, longest_string)
--  if not data or not from_list or not to_list then return false end

  local cutted_string = false 

  if(data == nil ) then 
    return "" 
  end

  if type(from_list) ~= "table" or type(to_list) ~= "table" then
    return ""
  end

  for from=1, #from_list do
    for to=1, #to_list do
      if(from_list[from] and to_list[to]) then
--        print("Cutting between: ", from_list[from], to_list[to])
        cutted = _cut(data, from_list[from], to_list[to]);
--        print("Cut:", string.len(cutted))
        cutted_string = compareAndReturn(cutted, cutted_string, longest_string)
      end
    end
  end
 
  if cutted_string then 
    return cutted_string
  else
    return false 
  end
end

function removePrefixZeros(data)
  local trimmed_data = nil
  local subs = nil
  trimmed_data, subs = string.gsub(data, "^0*(%d+)", "%1", 1)
 
  if(subs == 0) then
    return data
  end
  
  return trimmed_data;
end

------------------------------------------
--
-- Resume related functions 
----------------------------------------
--resume_text = ""
--depth = 0
--no_of_values = 0
--name_array = {}
--value_array = {}

function beginGroup(parent, name, do_not_add_resume)
  local group_node = addGroup(parent, name)

--  if not do_not_add_resume then
--  if depth == 0 then 
--    resume_text = resume_text .. "<table><th>" .. name .. "</th>"
--    resume_text = resume_text .. "<table><th>" .. name .. "</th>"
--    resume_text = resume_text .. "<b>" .. name .. ": </b><br/>" 
--  else
--    resume_text = resume_text .. "<tr><td><b>" .. name .. "</b></td></tr>"
--    resume_text = resume_text .. "<table><tr><td><b>" .. name .. ": " .. "</b></td>"
--  end
--  end

--  if depth == 0 then 
--    resume_text = resume_text .. "<table><th>" .. name .. "</th>"
--  else
--    resume_text = resume_text .. "<tr><th>" .. name .. ":&nbsp;"
--  end

--  depth = depth + 1

  return group_node;
end

function createGroup() 
--  print("Creating group HTML: " .. no_of_values);

--  i = 0
--  while(i < no_of_values) do
--    resume_text = resume_text .. "<td>&nbsp;</td><td align=center>" .. name_array[i] .. "</td>"
--    i = i + 1
--  end
  
--  resume_text = resume_text .. "<tr><td></td>"

--  i = 0 
--  while(i < no_of_values ) do
--    resume_text = resume_text .. "<td>&nbsp;&nbsp;&nbsp;</td><td>" .. value_array[i] .. "</td>"
--    i = i + 1
--  end
--  resume_text = resume_text .. "<tr>"

end

--create_group_depth = 1

function endGroup(do_not_add_resume)
 
--  print("Depth: " .. depth)

--  if not do_not_add_resume then
--  if(depth > create_group_depth) then createGroup() end
--  end

--  depth = depth - 1
--  
--  no_of_values = 0
--  value_array = {}
--  name_array = {}
--
--  if(depth < 0) then depth = 0 end
  
--  if not do_not_add_resume then
--  if depth == 0 then
--    resume_text = resume_text .. "</table><br/>"
--  else
--    resume_text = resume_text .. "</th></tr>"
----    resume_text = resume_text
--    resume_text = resume_text .. "</table>"
--  end
--  end
end

function _addValue(group_node, name, value)
  _addValue(group_node, name, value, true)
end

function _addValue(group_node, name, value, not_add_resume)
  addValue(group_node, name, UTF8Encode(value))
--  if(tonumber(value) >= 0) then value = "+" .. value end

--  resume_text = resume_text .. "<td>" .. name .. ": " .. "</td><td>" .. value .. "</td><td>&nbsp;</td>"
--  resume_text = resume_text .. "<tr><td>" .. name .. ": " .. "</td><td>" .. value .. "</td></tr>"
--  if not not_add_resume then
--    name_array[no_of_values] = name
--    value_array[no_of_values] = UTF8Encode(value)
--    no_of_values = no_of_values + 1
--    print("Added: " .. name)
--  end
--  resume_text = resume_text .. "<td>" .. name .. "</td>" 
end

------------------------------------------
--
-- TESTING
----------------------------------------

function test()
  data = "12345678910111213141516181920\n"

  cutted = cut(data, nil, nil)
  TEST_EQUAL_STR(cutted, "", "Testing empty input.")

  cutted = cut(data, nil, {"a"})
  TEST_EQUAL_STR(cutted, "", "Testing empty input in first argument.")

   cutted = cut(data, {"a"}, nil)
  TEST_EQUAL_STR(cutted, "", "Testing empty input in second argument.") 

  cutted = cut(data, "g", "g")
  TEST_EQUAL_STR(cutted, "", "Testing non-table as input.")

  cutted = cut(data, "g", nil)
  TEST_EQUAL_STR(cutted, "", "Testing non-table as input as first argument.")

  cutted = cut(data, nil, "g")
  TEST_EQUAL_STR(cutted, "", "Testing non-table as input as second argument.")

  cutted = cut(data, {"1"}, {"19"})
  TEST_EQUAL_STR(cutted, string.sub(data, 2, -6), "Testing a match.")
  
  cutted = cut(data, {"A"}, {"19"})
  TEST_FALSE(cutted, "Testing that no match return false.")

  cutted = cut(data, {"1", "2"}, {"18", "19"})
  TEST_EQUAL_STR(cutted, string.sub(data, 3, -8), "Testing multiple arguments (2 and 2).")

  cutted = cut(data, {"1", "2", "3"}, {"18", "19"})
  TEST_EQUAL_STR(cutted, string.sub(data, 4, -8), "Testing multiple arguments (3 and 2).")

  cutted = cut(data, {"1", "2", "3"}, {"18", "$"}, true)
  TEST_EQUAL_STR(cutted, string.sub(data, 2, -1), "Testing longest match.")

  data = "\048\049abcd\050\051"
  cutted = cut(data, {"\048\049"}, {"\050\051"})
  TEST_EQUAL_STR(cutted, "abcd", "Testing character escapes.")

--  TODO: Fix '\0' accepted as string termination in lua implementation
--  print(cutted) 
--  print("LOOK: ", string.find("\002\000ab\000cd\003\004", "\002(ab)\000cd"))
--  if "\000" == nil then 
--    print("HELLO WORLD") 
--  else 
--    print "nothing" 
--  end
  data = "\000\002abcd\003\004"
  cutted = cut(data, {"%z\002"}, {"\003\004"})
 
  TEST_EQUAL_INT(string.len(data), 8, "Testing string length.")
  TEST_EQUAL_STR(cutted, "abcd", "Testing control character escapes.")


  data = "000.00"
  TEST_EQUAL_STR("0.00", removePrefixZeros(data), "Testing removePrefixZeros - 1");

  data = "100.0101"
  TEST_EQUAL_STR("100.0101", removePrefixZeros(data), "Testing removePrefixZeros - 2");

  data = "0"
  TEST_EQUAL_STR("0", removePrefixZeros(data), "Testing removePrefixZeros - 3");

  data = "00"
  TEST_EQUAL_STR("0", removePrefixZeros(data), "Testing removePrefixZeros - 4");

end






















--function cutSmallest(data, prefix, suffix)
--  block = {}
--  for i=1, #suffix do
--    cutted_string = cut(data, prefix, suffix);
--    
--    if(cutted_string) 
--    if(block.firstb and block.lastb) then
--      if(match.first) then
--        block_length = block.lastb - block.firstb
--        match_length = match.last - match.first
--        if( block_length < match_length ) then
--          match.first = block.firstb
--          match.last = block.lastb
--        end
--      else
--        match.first = block.firstb
--        match.last = block.lastb
--      end
--    end
--  end
--  
--  if(match.first) then
--    return string.sub(data, match.first, match.last)
--  else
--    return false; 
--  end
--end
--
