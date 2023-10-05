--=============================================================================
-- Read the data from <filename> and determ which classes it contains.
--
-- The following functions are available in the classify function:
--    - addClass(classname)
--          Mark data content as classname.
--    - invalidData(errormsg)
--          Report that the data is invalid.
-------------------------------------------------------------------------------
function classify(filename)
  _loadUtils()
  printClassifierMessage("Running Humphrey LA 350 classifier...\n")
  local data = loadDataFromFile(filename)
  _classify(data)
end

function _checkCompany(data)
  if not data then return false end
  return string.find(data, ".*HUMPHREY SYSTEMS.*")
end

function _checkModel(data)
  if not data then return false end
  return string.find(data, ".*LA 350.*")
end

function _classify(data)
  if(_checkModel(data) and _checkCompany(data)) then 
    printClassifierMessage("Found Humphrey LA 350 data\n");
  else
    invalidData("Not Humphrey LA 350 data\n")
    return false
  end

  addClass("la350")
  return true 
end

-- function classify(filename)
--  printClassifierMessage("Running Humphrey LA 350 Classifier...\n")
--  local data = loadDataFromFile(filename)

  -- Validate
--  isHumphrey = string.find(data, ".*HUMPHREY SYSTEMS.*")
--  isLA350 = string.find(data, ".*LA 350.*")

--  if( isHumphrey and isLA350) then
--    printClassifierMessage("Found Humphrey LA 350 data\n")
--    addClass("lensmeter")
--  else 
--    invalidData("Not Humphrey LA 350 data\n")
--  end

--end

--=============================================================================
-- Interpret data in <filename> creating a tree structure of groups containing
-- data matching the requested type <requestedType>.
--
-- The following functions are available in the interpret function:
--    - addGroup(parent, groupname) 
--          Creates a new group with a groupname refering to a parent group.
--          A value of 0 is allowed the group does not have any parents 
--          (ie. it is root).
--          The function returns a group descriptor refering to the group.
--    - addValue(groupdescriptor, valuename, value)
--          Adds a value with key valuename to a group refered to by 
--          groupdescriptor.
-- interpret() must return the root of the created tree structure.
-------------------------------------------------------------------------------
function interpret(filename, requestedType)
  _loadUtils()
  printInterpreterMessage("Running Humphrey LA 350 Interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
  local data = loadDataFromFile(filename);
  _interpret(data, requestedType)
end

function _axis(data)
  if not data then return false end

  local axis = cut(data, {"X"}, {"LEFT", "$"})
  
  if not axis then return false end
  
  local _,_,value = string.find(axis, "(%d+)")

  if not value then return false end

  return value
end

function _cyl(data)
  if not data then return false end

  local cyl = cut(data, {"CYL"}, {"X", "$"})

  if not cyl then return false end

  local _,_,sign,value = string.find(cyl, "([%-%+]) (%d+%.%d+)")

  if not value then return false end

  if(sign == "-") then
    value = "-" .. value
  end

  return value 
end

function _sph(data)
  if not data then return false end

  local sph = cut(data, {"SPH"}, {"CYL", "$"})
  
  if not sph then return false end

  local _,_,sign,value = string.find(sph, "([%-%+]) (%d+%.%d+)")

  if not value then return false end
  
  if(sign == "-") then
    value = "-" .. value
  end

  return value
end

function _odxt(data)
  if not data then return false end

  local odxt_block = cut(data, {"RIGHT"}, {"LEFT", "$"})

  if not odxt_block then return false end

  local odxt = {}
  odxt["sph"] = _sph(odxt_block)
  odxt["cyl"] = _cyl(odxt_block)
  odxt["axis"] = _axis(odxt_block)
 
  return odxt
end

function _osin(data)
  if not data then return false end
  
  local osin_block = cut(data, {"LEFT"}, {"$"})
  
  if not osin_block then return false end
  
  local osin = {}
  osin["sph"] = _sph(osin_block)
  osin["cyl"] = _cyl(osin_block)
  osin["axis"] = _axis(osin_block)

  return osin
end

function _lensmeterAddGroup(group, group_name, parent)
  
  if not group or not group_name or not parent then return false end

  local group_node = addGroup(parent, group_name)
  if group["sph"] then
    addValue(group_node, "sph", group["sph"])
  end
  if group["cyl"] then
    addValue(group_node, "cyl", group["cyl"])
  end
  if group["axis"] then
    addValue(group_node, "axis", group["axis"])
  end

  return true
end

function _interpretLensmeter(data)
    
    local odxt = _odxt(data)
    local osin = _osin(data)
    
 
    if not odxt and not osin then return false end

    local root = addGroup(0, "lensmeter")

    -- odxt
    if odxt then
      _lensmeterAddGroup(odxt, "odxt", root)
    end

    -- osin
    if osin then
      _lensmeterAddGroup(osin, "osin", root)
    end

    return root
end

function _interpret(data, requestedType)
  if requestedType == "la350" then
    return interpretLensmeter(data)  
  else 
    return false
  end
end

--==============================================
-- Helper functions
--==============================================
function _loadUtils()
  local lua_base_dir = luaBaseDir()
  local utils = lua_base_dir .. "/" .. "utils.lua"
  dofile(utils)
end


--=============================================================================
--=============================================================================
--=============================================================================
--
-- Test function
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

-- TEST_LA350

function test()
  dofile 'test.lua'
  dofile 'utils.lua'
  -- Testing classifier
  TEST_TRUE(_checkModel("1234LA 350\nJHIKLA"), "Testing that correct model is found.\n")
  TEST_FALSE(_checkModel("1234LA 450\nJHIKLA"), "Testing if _checkModel fails on invalid model.\n")
 
  TEST_TRUE(_checkCompany("1234\nJHIKLA\nHUMPHREY SYSTEMS"), "Testing that correct company is found.\n")
  TEST_FALSE(_checkCompany("1234LA 450\nJHIKLA\nHUMFREY SYSTEMS"), "Testing if _checkCompany fails on invalid company.\n")

  testReset()
  _classify("AAA____\n\Ý\nHUMPHREY SYSTEMSRTELA 350mm")
  TEST_EQUAL_INT(1, testAddClassCount_, "Testing addition of classes based on input data.\n")
 
  testReset()
  _classify("AALA")
  TEST_NOTEQUAL_INT(1, testAddClassCount_, "Testing failure on invalid input.\n")

  -- Testing interpreter
  -- TODO: Add more negative tests
  TEST_EQUAL_STR("180.9", _cyl("ASDRGRDRG\nCYL: + 180.9"), "Testing cyl value.\n")
  TEST_EQUAL_STR("-10.0", _cyl("@#REWEWCYL: - 10.0"), "Testing cyl negative value.\n")

  TEST_EQUAL_STR("70", _axis("X    70 asd"), "Testing axis value.\n")
  TEST_FALSE(_axis("ZZZ  1 a"), "Testing invalid axis value - 1.\n")
  TEST_FALSE(_axis("ZZX  AA a"), "Testing invalid axis value - 2.\n")

  TEST_EQUAL_STR("44.0", _sph("SDSDASDASSPH: + 44.0"), "Testing sph value.\n")
  TEST_EQUAL_STR("-3.0", _sph("CYL: + 44.0 SPH: - 3.0\n"), "Testing negative sph value.\n")

  local data = "ADASDSARIGHT:   SPH: + 44.0\n CYL: - 18.0\n X 70\nLEFT:  SPH: - 39.4\n CYL: + 22.0 X 44\n"
  local odxt = _odxt(data)
  TEST_TRUE(_odxt, "Testing parsing of lensmeter odxt data.\n")
  if odxt then
    TEST_EQUAL_STR("44.0", odxt["sph"], "Testing parsing of lensmeter odxt sph value.\n")
    TEST_EQUAL_STR("-18.0", odxt["cyl"], "Testing parsing of lensmeter odxt cyl value.\n")
    TEST_EQUAL_STR("70", odxt["axis"], "Testing parsing of lensmeter odxt axis value.\n")
  end

  local osin = _osin(data)
  TEST_TRUE(_osin, "Testing parsing of lensmeter osin data.\n")
  if osin then
    TEST_EQUAL_STR("-39.4", osin["sph"], "Testing parsing of lensmeter osin sph value.\n")
    TEST_EQUAL_STR("22.0", osin["cyl"], "Testing parsing of lensmeter osin cyl value.\n")
    TEST_EQUAL_STR("44", osin["axis"], "Testing parsing of lensmeter osin axis value.\n")
  end

  local root = addGroup(0, "lensmeter")
  local odxt = {}
  odxt["sph"] = "44.0"
  odxt["cyl"] = "-18.4"
  odxt["axis"] = "79"
  testReset()
  TEST_TRUE(_lensmeterAddGroup(odxt, "odxt", root), "Testing creation of lensmeter group tree.\n")
  -- TODO: This validation should be extented to look at the content of the tree. 
  TEST_EQUAL_INT(3, testAddValueCount_, "Validating lensmeter group tree.\n") 

  testReset()
  TEST_TRUE(_interpretLensmeter(data), "Testing parsing of lensmeter data.\n")
  -- TODO: This validation should be extented to look at the content of the tree. 
  TEST_EQUAL_INT(6, testAddValueCount_, "Validating lensmeter addValue.\n")
  TEST_EQUAL_INT(3, testAddGroupCount_, "Validating lensmeter addGroup.\n")

  return 0 
end
--_axis("@#REWEWCYL: -  10.0")
--  _classify("RTELA 350")
--  root = addGroup(0, "lensmeter")
--  odxt = {}
--  odxt["sph"] = "44.0"
--  odxt["cyl"] = "-18.4"
--  odxt["axis"] = "79"

--  print ("hel: ", _interpret_lensmeter_create_group(odxt, "odxt", root))

--interpret("AAA", "lensmeter")
--classify("la350.lua")

--function _parseClassLA350(data)
--  values = {}
  -- Parse data
--  _, _, 
--  values["odxt"], 
--  values["odxt_sph_sign"], 
--  values["odxt_sph"], 
--  values["odxt_cyl_sign"], 
--  values["odxt_cyl"], 
--  values["odxt_axis"] = string.find(data, "(RIGHT) +SPH: +([-%+]) (%d+%.%d+)." 
--                            .. "\n      CYL: +([-%+]) (%d+%.%d+) *X* *(%d+)")

--  _, _, 
--  values["osin"], 
--  values["osin_sph_sign"], 
--  values["osin_sph"], 
--  values["osin_cyl_sign"], 
--  values["osin_cyl"], 
--  values["osin_axis"] = string.find(data, "(LEFT) +SPH: +([-%+]) (%d+%.%d+)."
--                            .. "\n      CYL: +([-%+]) (%d+%.%d+) *X* *(%d+)")

--  return values
--end
--
--function _interpret(data, requestedType)
 
--  odxt = "(RIGHT)+.-"
--  odxt_sph_sign = ".-SPH: +([-%+])+"
--  odxt_sph = ".-SPH"

--  if(requestedType == "la350") then
--    la350 = _parseClassLA350(data);
--  end
  
--  if(la350) then
--    root = addGroup(0, "la350")
    -- Build data tree
--    if(reye) then
--      right = addGroup(root, "odxt")
--      if(rsphsign and rsph) then
--        if( (rsphsign == "-") and (rsph ~= "0.00") ) then
--          rsph = "-" .. rsph 
--        end
--        addValue(right, "sph", rsph)
--      end
--      if(rcylsign and rcyl and raxis) then
--        if( (rcylsign == "-") and (rcyl ~= "0.00") ) then
--          rcyl = "-" .. rcyl 
--        end
--        addValue(right, "cyl",  rcyl)
--        addValue(right, "axis", raxis)
--      end
--    end
--    if(leye) then
--      left = addGroup(root, "osin")
--      if(lsphsign and lsph) then
--        if( (lsphsign == "-") and (lsph ~= "0.00") ) then
--          lsph = "-" .. lsph 
--        end
--        addValue(left, "sph", lsph)
--      end
--      if(lcylsign and lcyl and laxis) then
--        if( (lcylsign == "-") and (lcyl ~= "0.00") ) then
--          lcyl = "-" .. lcyl 
--        end
--        addValue(left, "cyl",  lcyl)
--        addValue(left, "axis", laxis)
--      end
--    end
--  end
--  return root
--end

