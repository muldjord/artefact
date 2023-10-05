-- ===============
-- Name mapping
-- ---------------

LM600P_SOH = 1
LM600P_STX = 2
LM600P_ETB = 23
LM600P_EOT = 4
LM600P_CR  = 13

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
  printClassifierMessage("Running NIDEK LM600P classifier...\n")
  local data = loadDataFromFile(filename)
  return _classify(data) 
end

function _checkCompanyAndModel(data)
--  print("data", data)
  local _, _, company, model = string.find(data, "ID(%a*)/(%a%a%-%d%d%d%a)\023")

--  print("Company and model:", company, model);

  if (company ~= "NIDEK") or (model ~= "LM-600P") then
    invalidData("Not NIDEK LM600P data.\n")
    return false
  else
    printClassifierMessage("Company: " .. company .. ", model: " .. model .. "\n");
    return true
  end
end

function _checkChecksum(data)
  local i, _, checksum = string.find(data, "\004(%x%x%x%x)")

  -- Only use it if it exists
  if not checksum then
    return true 
  end

  checksumdata = string.sub(data, 0, i) -- Only look at the string up to the checksum
  checksumdata = string.gsub(checksumdata, "[\013\010]", ""); -- Remove \r\n

  local datachecksum = 0
  for i = 1,  string.len(checksumdata) do -- Include \004
    datachecksum = datachecksum + string.byte(checksumdata, i)
  end

--  print("CHECKSUM:", datachecksum, tonumber(checksum, 16))

  if datachecksum ~= tonumber(checksum, 16) then
    invalidData("Data does not match with checksum.\n")
    return false
  else
    return true
  end
  
  return true
end

function _addClasses(data)
  local class_found = false

  if _containsLensmeter(data) then
    printClassifierMessage("Lensmeter class found.\n")
    addClass("lm600p")
    class_found = true
  end
  
  if not class_found then
    invalidData("No classes found in data.\n")
    return false
  else
    return true
  end
end

function _classify(data)
 
  string.gsub(data, "\013", "") -- Remove all Carriage return if any

  if not _checkCompanyAndModel(data) then return false end

  if not _checkChecksum(data) then return false end 
  
  if not _addClasses(data) then return false end

  return true
end

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
  printInterpreterMessage("Running NIDEK LM600P interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
 
  local data = loadDataFromFile(filename)
  _interpret(data, requestedType)
end

function _lensmeterOdxt(data)
  
  if not data then return data end

  local _,_,sph_sign,sph,cyl_sign,cyl,axis = string.find(data, " R([%+%-]?)(%d%d%.%d%d)([%+%-]?)(%d%d%.%d%d)(%d%d%d)")

  if not sph_sign or not sph or not cyl_sign or not cyl or not axis then return false end

  sph = removePrefixZeros(sph)
  cyl = removePrefixZeros(cyl)
  axis = removePrefixZeros(axis)

  if sph_sign then
    sph = sph_sign .. sph
  end

  if cyl_sign == "-" then
    cyl = "-" .. cyl
  end

  local odxt = {}
  odxt["sph"] = sph
  odxt["cyl"] = cyl
  odxt["axis"] = axis

  return odxt 
end

function _lensmeterOsin(data)
  
  if not data then return data end

  local _,_,sph_sign,sph,cyl_sign,cyl,axis = string.find(data, " L([%+%-]?)(%d%d%.%d%d)([%+%-]?)(%d%d%.%d%d)(%d%d%d)")

  if not sph_sign or not sph or not cyl_sign or not cyl or not axis then return false end

  sph = removePrefixZeros(sph)
  cyl = removePrefixZeros(cyl)
  axis = removePrefixZeros(axis)

  if sph_sign  then
    sph = sph_sign .. sph
  end

  if cyl_sign == "-" then
    cyl = "-" .. cyl
  end

  local osin = {}
  osin["sph"] = sph 
  osin["cyl"] = cyl
  osin["axis"] = axis

  return osin 
end

function _lensmeterAddGroup(group, group_name, parent)
  if not group or not group_name or not parent then return false end

  local group_node = beginGroup(parent, group_name)

  if group["sph"] then
    _addValue(group_node, "sph", group["sph"])
  end
  if group["cyl"] then
    _addValue(group_node, "cyl", group["cyl"])
  end
  if group["axis"] then
    _addValue(group_node, "axis", group["axis"])
  end

  endGroup()

  return true
end


function _interpretLensmeter(data)

  if not data then return false end

  if not _containsLensmeter(data) then return false end

  local odxt = _lensmeterOdxt(data)
  local osin = _lensmeterOsin(data)

-- How do we deal with a single value (ie. "  ...."?
  
  if not odxt and not osin then return false end

  local root = beginGroup(0, "lm600p")

  if odxt then
    _lensmeterAddGroup(odxt, "odxt", root)
  end

  if osin then 
    _lensmeterAddGroup(osin, "osin", root)
  end

  endGroup()
end

function _interpret(data, requestedType)
  data = string.gsub(data, "\013", "")

  if requestedType == "lm600p" then
    _interpretLensmeter(data)
  else
    printInterpreterMessage("Invalid requested class.")
    return false
  end

end


--=============================================================================
--
-- Local Functions
-------------------------------------------------------------------------------
function _loadUtils()
  local lua_base_dir = luaBaseDir()
  local utils = lua_base_dir .. "/" .. "utils.lua"
  dofile(utils)
end

function _containsLensmeter(data)
  if not data then return false end
  
  return string.find(data, " R") or string.find(data, " L") or string.find(data, "  ")
end

--=============================================================================
--
-- Test function
--
-------------------------------------------------------------------------------

function test()
  dofile 'test.lua'
  dofile 'utils.lua'

  -- Testing classifier
  local data = "IDNIDEK/LM-600PD\023"
  TEST_FALSE(_checkCompanyAndModel(data), "Testing _companyAndModel on data not containing lensmeter data.\n")
  
  testReset()
  local data = "IDNIDEK/LM-600P\023"
  TEST_TRUE(_checkCompanyAndModel(data), "Testing _companyAndModel on data containing lensmeter data.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "0", "Testing that invalidData are not called on good data in _checkCompanyAndModel.\n")
 
 -- Testing interpreter

  local data = " L+22.22-23.23123"
  local osin = _lensmeterOsin(data);
  TEST_TRUE(osin, "Testing _lensmeterOsin return value on good data");
  TEST_TRUE(osin["sph"], "Testing that osin[sph] is true");
  TEST_TRUE(osin["cyl"], "Testing that osin[cyl] is true");
  TEST_TRUE(osin["axis"], "Testing that osin[axis] is true");
  TEST_EQUAL_STR("+22.22", osin["sph"], "Testing osin[sph] has correct value.\n")
  TEST_EQUAL_STR("-23.23", osin["cyl"], "Testing osin[cyl] has correct value.\n")
  TEST_EQUAL_STR("123", osin["axis"], "Testing osin[axis] has correct value.\n")

  local data = " R+22.22-23.23123"
  local odxt = _lensmeterOdxt(data);
  TEST_TRUE(odxt, "Testing _lensmeterOdxt return value on good data");
  TEST_TRUE(odxt["sph"], "Testing that odxt[sph] is true");
  TEST_TRUE(odxt["cyl"], "Testing that odxt[cyl] is true");
  TEST_TRUE(odxt["axis"], "Testing that odxt[axis] is true");
  TEST_EQUAL_STR("+22.22", odxt["sph"], "Testing odxt[sph] has correct value.\n")
  TEST_EQUAL_STR("-23.23", odxt["cyl"], "Testing odxt[cyl] has correct value.\n")
  TEST_EQUAL_STR("123", odxt["axis"], "Testing odxt[axis] has correct value.\n")

end
