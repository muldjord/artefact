testAddClassCount_ = 0;
testInvalidDataCount_ = 0;
testAddGroupCount_ = 0
testAddValueCount_ = 0

--testGroupList_ = {}
testValueList_ = {}

function addClass(class) 
  testAddClassCount_ = testAddClassCount_ + 1
end

function invalidData(msg)
  testInvalidDataCount_ = testInvalidDataCount_ + 1
end

function printClassifierMessage(msg)
end

function addGroup(parent, group)
  testAddGroupCount_ = testAddGroupCount_ + 1 
--  testGroupList_[parent] = { group }
 
  return 0 -- For now, return anything but false
end

function addValue(parent, name, value)
  testAddValueCount_ = testAddValueCount_ + 1

  if not testValueList_[parent] then testValueList_[parent] = {} end
  
  testValueList_[parent][name] = value 
--  test = testValueList_[parent][name]

--  print("TableValueList:")
--  for k,v in pairs(testValueList_) do print(k,v) end
  
--  print("parent = " .. parent)
--  print("name = " .. name)
--  print("value = " .. test)


--  t = testValueList_["alcontest"]
--  if t then
--    print("Table: ")
--    for k,v in pairs(t) do print(k,v) end
--    if t["vendor"] then
--      print("t.vendor = " .. t["vendor"])
--    end
--  end

end

function printInterpreterMessage(msg)
end

function setResume(resume)
end

function UTF8Encode(text)
  return text
end

function testReset()
  testAddClassCount_ = 0
  testInvalidDataCount_ = 0
  testAddGroupCount_ = 0
  testAddValueCount_ = 0
  testGroupList = {}
  testValueList_ = {}
end

