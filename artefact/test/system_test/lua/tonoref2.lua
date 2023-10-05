-- ===============
-- Name mapping
-- ---------------

--local _PATTERN_BEGIN_DNT = { }
--local _PATTERN_END_DNT = { }

--local _PATTERN_BEGIN_DNT_SPH = { }
--local _PATTERN_END_DNT_SPH = { }

--local _PATTERN_BEGIN_DNT_

--local _NAME_DNT = "DNT"
--local _NAME_DNT_SPH = "sph"
--local _NAME_DNT_CYL = "cyl"
--local _NAME_DNT_AXIS = "axis"

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
  printClassifierMessage("Running NIDEK TONOREF2 classifier...\n")
  local data = loadDataFromFile(filename)
  return _classify(data) 
end

function _checkCompanyAndModel(data)
  local _, _, company, model = string.find(data, "ID(%a*)/(%a*%d)")

  if (company ~= "NIDEK") or (model ~= "TONOREF2") then
    invalidData("Not NIDEK Tonoref 2 data.\n")
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

  print("CHECKSUM:", datachecksum, tonumber(checksum, 16))

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

  if _containsDRM(data) then
    printClassifierMessage("Refractometer class found.\n")
    addClass("tonoref2_refractometer")
    class_found = true
  end

  if _containsDKM(data) then
    printClassifierMessage("Keratometer class found.\n")
    addClass("tonoref2_keratometer")
    class_found = true
  end

  if _containsDNT(data) then
    printClassifierMessage("Tonometer class found.\n")
    addClass("tonoref2_tonometer")
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
 
--  string.gsub(data, "\013", "") -- Remove all Carriage return if any

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
  printInterpreterMessage("Running NIDEK TONOREF2 interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
  
  local data = loadDataFromFile(filename)
  _interpret(data, requestedType)
end

--function _dkmCyl(data)
--  if not data then return false end
--
--  local cyl = string.sub(data, 19, 24)
-- 
--  if not cyl then return false end
--
--  local _,_,sign,value = string.find(cyl, "([%-%+]?)(%d%d%.%d%d)")
--
--  if sign == "-" then
--    value = "-" .. value
--  end
--
--  return value
--end

function _dntMmhg(data)
  if not data then return false end

  local _,_,value = string.find(data, "AV.-(%d%d%.%d)")

  if not value then
    _,_,value = string.find(data, "AV.-(%d%.%d)")
  end

  if not value then return false end

  value = removePrefixZeros(value)

  return value
end


function _dntOdxt(data)
  if not data then return false end

  local odxt_block = cut(data, {" R"}, {" L", "\023", "$"})

  if not odxt_block then return false end

  local odxt = {}

  odxt["mmhg"] = _dntMmhg(odxt_block)

  if not odxt["mmhg"] then return false end

  return odxt
end

function _dntOsin(data)
  if not data then return false end

  local osin_block = cut(data, {" L"}, {"\023", "$"})

  if not osin_block then return false end

  local osin = {}

  osin["mmhg"] = _dntMmhg(osin_block)

  if not osin["mmhg"] then return false end

  return osin 
end

function _dntAddGroup(group, group_name, parent)
  if not group or not group_name or not parent then return false end

  local group_node = beginGroup(parent, group_name)

  if group["mmhg"] then
    _addValue(group_node, "mmhg", group["mmhg"])
  end

  endGroup()

  return true
end


function _interpretDNT(data)
  if not data then return false end
  
  if not _containsDNT(data) then return false end


  local dnt_block = cut(data, {"DNT"}, {"$"})

  local dnt_odxt = _dntOdxt(dnt_block)
  local dnt_osin = _dntOsin(dnt_block)

  if not dnt_odxt and not dnt_osin then return false end
  
  local root = beginGroup(0, "tonoref2_tonometer")

  if dnt_odxt then
    _dntAddGroup(dnt_odxt, "odxt", root)
  end

  if dnt_osin then
    _dntAddGroup(dnt_osin, "osin", root)
  end

  endGroup()
end

function _dkmAxis(data)
  if not data then return false end

  local axis = string.sub(data, 11, 13)
  
  if not axis then return false end

  local _,_,value = string.find(axis, "(%d%d%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  return value
end

function _dkmR2(data)
  if not data then return false end

  local r2 = string.sub(data, 6, 10)
  
  if not r2 then return false end

  local _,_,value = string.find(r2, "(%d%d%.%d%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  return value
end

function _dkmR1(data)
  if not data then return false end

  local r1 = string.sub(data, 1, 5)
  
  if not r1 then return false end

  local _,_,value = string.find(r1, "(%d%d%.%d%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  return value
end

function _dkmPs(data)
  if not data then return false end
 
  local _,_,value = string.find(data, "(%d%d%.%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  return value;
end

function _dkmOdxt(data)
  if not data then return false end

  local dkm_block = cut(data, {" R"}, {"DR", "\023", " R", "SL", "SR", "$"}) 

  if not dkm_block then return false end

  local odxt = {}

  odxt["r1"] = _dkmR1(dkm_block)
  odxt["r2"] = _dkmR2(dkm_block)
  odxt["axis"] = _dkmAxis(dkm_block)

  local ps_block = cut(data, {"PR"}, {"\023", "DNT", "$"})

  odxt["ps"] = _dkmPs(ps_block)

  return odxt 
end

function _dkmOsin(data)
  if not data then return false end

  local dkm_block = cut(data, {" L"}, {"DL", "\023", " L", " R", "DR", "SL", "SR", "$"})

  if not dkm_block then return false end

  local osin = {}

  osin["r1"] = _dkmR1(dkm_block)
  osin["r2"] = _dkmR2(dkm_block)
  osin["axis"] = _dkmAxis(dkm_block)

  local ps_block = cut(data, {"PL"}, {"PR", "\023", "DNT", "$"})

  osin["ps"] = _dkmPs(ps_block)
--  odxt["cyl"] = _dkmCyl(odxt_block);

  return osin
end

function _dkmAddGroup(group, group_name, parent)
  if not group or not group_name or not parent then return false end

  local group_node = beginGroup(parent, group_name)

  if group["r1"] then
    _addValue(group_node, "r1", group["r1"])
  end

  if group["r2"] then
    _addValue(group_node, "r2", group["r2"])
  end

  if group["axis"] then
    _addValue(group_node, "axis", group["axis"])
  end
  
  if group["ps"] then
    _addValue(group_node, "ps", group["ps"])
  end

  endGroup()

  return true
end

function _interpretDKM(data)
  if not data then return false end
  
  if not _containsDKM(data) then return false end


  local dkm_block = cut(data, {"DKM"}, {"DNT", "DNT", "$"})

  local dkm_odxt = _dkmOdxt(dkm_block)
  local dkm_osin = _dkmOsin(dkm_block)

  if not dkm_odxt and not dkm_osin then return false end
  
  local root = beginGroup(0, "tonoref2_keratometer")

  if dkm_odxt then
    _dkmAddGroup(dkm_odxt, "odxt", root)
  end

  if dkm_osin then
    _dkmAddGroup(dkm_osin, "osin", root)
  end

  endGroup()
end

function _drmSph(data)
  if not data then return false end

  local sph = string.sub(data, 1, 6)
  
  if not sph then return false end

  local _,_,sign,value = string.find(sph, "([%-%+]?)(%d%d%.%d%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  if sign then
    value = sign .. value
  end

  return value
end

function _drmCyl(data)
  if not data then return false end
  local cyl = string.sub(data, 7, 12)
  
  if not cyl then return false end

  local _,_,sign,value = string.find(cyl, "([%+%-]?)(%d%d%.%d%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  if sign == "-" then
    value = "-" .. value
  end

  return value
end

function _drmAxis(data)
  if not data then return false end
--  print("DATA: ", data) 
  local axis = string.sub(data, 13, 15)
--  print("AXIS: ", axis) 
  if not axis then return false end

  local _,_,value = string.find(axis, "(%d%d%d)")

  if not value then return false end

  value = removePrefixZeros(value)

  return value
end

function _drmOdxt(data)
  if not data then return false end

  local odxt_block = cut(data, {"\023OR"}, {"\023OR", "OL", "\023", "$"})
--  print("ODXT_BLOCK: ", odxt_block) 
  if not odxt_block then return false end

  local odxt = {}
  odxt["sph"] = _drmSph(odxt_block)
  odxt["cyl"] = _drmCyl(odxt_block)
  odxt["axis"] = _drmAxis(odxt_block)

--  print(odxt["sph"], odxt["cyl"], odxt["axis"])

  return odxt
end

-- Returning the median value
function _drmOsin(data)
--  print("DATA: ", data)

  if not data then return data end

  local osin_block = cut(data, {"\023OL"}, {"OR", "OL", "\023", "$"})

  if not osin_block then return false end

  local osin = {}
  osin["sph"] = _drmSph(osin_block)
  osin["cyl"] = _drmCyl(osin_block)
  osin["axis"] = _drmAxis(osin_block)

  return osin 
end

function _drmAddGroup(group, group_name, parent)
--  print(group, group_name, parent)
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
--  print("addGroup 2")
  return true
end

function _interpretDRM(data)
  if not data then return false end

  if not _containsDRM(data) then return false end


  local drm_block = cut(data, {"DRM"}, {"DKM", "DNT", "$"})

  local drm_odxt = _drmOdxt(drm_block)
  local drm_osin = _drmOsin(drm_block)

  if not drm_odxt and not drm_osin then return false end
  
  local root = beginGroup(0, "tonoref2_refractometer")

  if drm_odxt then
    _drmAddGroup(drm_odxt, "odxt", root)
  end

  if drm_osin then 
    _drmAddGroup(drm_osin, "osin", root)
  end

  endGroup()
end

function _interpret(data, requestedType)
  data = string.gsub(data, "\013", "")

  if requestedType == "tonoref2_refractometer" then
    _interpretDRM(data)
  elseif requestedType == "tonoref2_keratometer" then
    _interpretDKM(data)
  elseif requestedType == "tonoref2_tonometer" then
    _interpretDNT(data)
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

function _containsDRM(data)
  if not data then return false end
  
  drm_block = cut(data, { "DRM" }, { "DKM", "DNT", "$" })

  if not drm_block then return false end

--  odxt = cut(drm_block, { "OL" }, { "$" })
--  osin = cut(drm_block, { "OR" }, { "$" })

--  if not odxt and osin then return false end

  return string.find(drm_block, "\023OR") or string.find(drm_block, "OL")
end

function _containsDKM(data)
  if not data then return false end

  dkm_block = cut(data, { "DKM" }, { "DNT", "$" })

  if not dkm_block then return false end

  return string.find(dkm_block, " R") or string.find(dkm_block, " L")
end

function _containsDNT(data)
  if not data then return false end
  dnt_block = cut(data, { "DNT" }, { "$"} )
  
  if not dnt_block then return false end

  return string.find(dnt_block, " R.-%d/%d.-AV") or string.find(dnt_block, " L.-%d/%d.-AV")

--  return string.find(data, "\001DNT\002") 
end


--=============================================================================
--
-- Test function
--
-------------------------------------------------------------------------------

function test()
  dofile 'test.lua'
  dofile 'utils.lua'

  -- Testing local functions
  local data = "\001DRM\002OL-05.25-00.751099*\001DKM\002 L07.9507.7117607.83\001DNT\002 R03     10/1.3*1  AV11.0/1.43\004"
  TEST_TRUE(_containsDRM(data), "Testing _containsDRM on data containing a DRM block.\n")
  TEST_TRUE(_containsDKM(data), "Testing _containsDKM on data containing a DKM block.\n")
  TEST_TRUE(_containsDNT(data), "Testing _containsDNT on data containing a DNT block.\n")
  
  local data = "\001DRM\02#@$#$@%5sdsf23\001DKM\0024ASDADWDASDAWDADNT\004"
  TEST_FALSE(_containsDRM(data), "Testing _containsDRM on data not containing a DRM block.\n")
  TEST_FALSE(_containsDKM(data), "Testing _containsDKM on data not containing a DKM block.\n")
  TEST_FALSE(_containsDNT(data), "Testing _containsDNT on data not containing a DNT block.\n")

  -- Testing classifier
  testReset()
  local data = "DRM  21314VI as as IDNIDEK/TONOREF2"
  TEST_TRUE(_checkCompanyAndModel(data), "Testing _checkCompanyAndModel on good tonoref2 data.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "0", "Testing that invalidData are not called on good data in _checkCompanyAndModel.\n")
  testReset()
  local data = "DRM 23244IDNIDEL/TONOREF"
  TEST_FALSE(_checkCompanyAndModel(data), "Testing _checkCompanyAndModel on bad tonoref2 data.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "1", "Testing if invalidData is called on bad data in _checkCompanyAndModel.\n")

  local data = "0123456789\0040211" 
  TEST_TRUE(_checkChecksum(data), "Testing _checkChecksum on good data.\n")
  local data = "0\r\n1\r\n2\r\n3\r\n4\r\n5\r\n6\r\n7\r\n8\r\n9\r\n\0040211\r\n" 
  TEST_TRUE(_checkChecksum(data), "Testing _checkChecksum on good data with line delimiter.\n")
  testReset()
  local data = "0123456789\023" 
  TEST_TRUE(_checkChecksum(data), "Testing _checkChecksum on data without checksum.\n")
  local data = "0123456789\0040212"
  testReset()
  TEST_FALSE(_checkChecksum(data), "Testing _checkChecksum on data with wrong checksum.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "1", "Testing that _checkChecksum on data with wrong checksum calls invalidData.\n")

  testReset()
  local data = "\001DRM\002OL  \023\001DKM\002 R1113123\023\001DNT\002 R  AV\004"
  TEST_TRUE(_addClasses(data), "Testing _addClasses on good data.")
  TEST_EQUAL_INT(testAddClassCount_, "3", "Testing that three classes where added on data containing three classes.\n")
  TEST_EQUAL_INT(testInvalidDataCount_,"0", "Testing that invalidData where not called on data containing classes.\n")
  testReset()
  local data = "ASDADSASD"
  TEST_FALSE(_addClasses(data), "Testing that _addClasses returns false on data not containing any classes.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "1", "Testing that invalidData was reported on data not containing any classes.\n")
  TEST_EQUAL_INT(testAddClassCount_, "0", "Testing that no class where added on data not containing any classes.\n")

  local data = "DRM\001\004#@$#$@%5sdsf23\001\004DKM\001\004ASDADWDASDAWDADNT\001\001"
  TEST_FALSE(_classify(data), "Testing _classify on bad data.\n")

  -- Testing interpreter

  -- DRM --
  local data = "-05.25-00.751099*\023"
  TEST_EQUAL_STR("-5.25", _drmSph(data), "Testing _drmSph return value - 1.\n")
  TEST_EQUAL_STR("-0.75", _drmCyl(data), "Testing _drmCyl return value - 1.\n")
  TEST_EQUAL_STR("109", _drmAxis(data), "Testing _drmAxis return value.\n")
  
  local data = "+05.25+00.751099*\023"
  TEST_EQUAL_STR("5.25", _drmSph(data), "Testing _drmSph return value - 2.\n")
  TEST_EQUAL_STR("0.75", _drmCyl(data), "Testing _drmCyl return value - 2.\n")

  local data = " 05.25 00.751099*\023"
  TEST_EQUAL_STR("5.25", _drmSph(data), "Testing _drmSph return value - 3.\n")
  TEST_EQUAL_STR("0.75", _drmCyl(data), "Testing _drmCyl return value - 3.\n")
  

  local data = "\001DRM\002BLABLALBALBLALBLA13.23ASDASD\023ASDSAD+99.99\023" ..
               "OL-99.09 20.001799\023OR 00.99-15.020442*\023PL10.4\023\004"
  local drm_odxt = _drmOdxt(data)
  TEST_TRUE(drm_odxt, "Testing parsing of DRM odxt data.\n")
  if drm_odxt then
    TEST_EQUAL_STR("0.99", drm_odxt["sph"], "Testing _drmOdxt sph value.\n")
    TEST_EQUAL_STR("-15.02", drm_odxt["cyl"], "Testing _drmOdxt cyl value.\n")
    TEST_EQUAL_STR("44", drm_odxt["axis"], "Testing _drmOdxt axis value.\n")
  end
  local drm_osin = _drmOsin(data)
  TEST_TRUE(drm_osin, "Testing parsing of DRM osin data.\n")
  if drm_osin then
    TEST_EQUAL_STR("-99.09", drm_osin["sph"], "Testing _drmOdxt sph value.\n")
    TEST_EQUAL_STR("20.00", drm_osin["cyl"], "Testing _drmOdxt cyl value.\n")
    TEST_EQUAL_STR("179", drm_osin["axis"], "Testing _drmOdxt axis value.\n")
  end

  testReset()
  _interpret( data, "tonoref2_refractometer")

  testReset()
  local root = addGroup(0, "autorefraktion")
  local odxt = {}
  odxt["sph"] = "44.0"
  odxt["cyl"] = "-18.4"
  odxt["axis"] = "79"
  TEST_TRUE(_drmAddGroup(odxt, "odxt", root), "Testing creation of drm group tree.\n")
  TEST_EQUAL_INT(3, testAddValueCount_, "Validating drm group tree.\n")

  -- DKM --
  local data = "05.2519.7510907.83\023"
  TEST_EQUAL_STR("109", _dkmAxis(data), "Testing _dkmAxis return value on good data.\n")
  TEST_EQUAL_STR("5.25", _dkmR1(data), "Testing _dkmR1 return value on good data.\n")
  TEST_EQUAL_STR("19.75", _dkmR2(data), "Testing _dkmR2 return value on good data.\n")
 
  local data = "05325195751-907.83\023"
  TEST_FALSE(_dkmAxis(data), "Testing _dkmAxis return value on bad data.\n")
  TEST_FALSE(_dkmR1(data), "Testing _dkmR1 return value on bad data.\n")
  TEST_FALSE(_dkmR2(data), "Testing _dkmR2 return value on bad data.\n")
 
  TEST_EQUAL_STR("7.8", _dkmPs(data), "Testing _dkmPs return value on good data.\n")
  
  local data = "\001DRM\002BLaBLABLALBLASLDSK\023BLABLAL12.56-00.55\023OL-99.09 20.001799\023\001DKM\002 L07.9507.7117607.83\023DL42.4543.7717643.11-01.32\023 R07.8607.5317507.70\023 R17.5511.3313455.81\023\001DNT\002blablalba\023\004"
  local dkm_odxt = _dkmOdxt(data)
  TEST_TRUE(dkm_odxt, "Testing parsing of DKM odxt data.\n")
  if dkm_odxt then
    TEST_EQUAL_STR("7.86", dkm_odxt["r1"], "Testing _dkmOdxt r1 value.\n")
    TEST_EQUAL_STR("7.53", dkm_odxt["r2"], "Testing _dkmOdxt r2 value.\n")
    TEST_EQUAL_STR("175", dkm_odxt["axis"], "Testing _dkmOdxt axis value.\n")
  end
  local dkm_osin = _dkmOsin(data)
  TEST_TRUE(dkm_odxt, "Testing parsing of DKM odxt data.\n")
  if dkm_osin then
    TEST_EQUAL_STR("7.95", dkm_osin["r1"], "Testing _dkmOdxt r1 value.\n")
    TEST_EQUAL_STR("7.71", dkm_osin["r2"], "Testing _dkmOdxt r2 value.\n")
    TEST_EQUAL_STR("176", dkm_osin["axis"], "Testing _dkmOdxt axis value.\n")
  end
  
  testReset()
  local root = addGroup(0, "keratometri")
  local odxt = {}
  odxt["r1"] = "14.0"
  odxt["r2"] = "18.4"
  odxt["axis"] = "79"
  TEST_TRUE(_dkmAddGroup(odxt, "odxt", root), "Testing creation of dkm group tree.\n")
  TEST_EQUAL_INT(3, testAddValueCount_, "Validating dkm group tree.\n")

  -- DNT --
  local data = " R03    10/1.3*1   22.0/1.4*2    12/1.6   AV11.0/1.43"
  TEST_EQUAL_STR("11.0", _dntMmhg(data), "Testing _dntMmhg return value on good data.\n")
  
  local bad_data = " R03    10/1.3*1   22.0/1.4*2    12/1.6   A11.0/1.43"
  TEST_FALSE(_dntMmhg(bad_data), "Testing _dntMmhg return value on bad data.\n")


  local data = " R03    10/1.3*1   22.0/1.4*2    12/1.6   AV11.0/1.43\023 L03    10/1.3*1   12/2.3*2   12/1.6  AV11.2/1.43\023\004"

  local dnt_odxt = _dntOdxt(data)
  TEST_TRUE(dnt_odxt, "Testing parsing of DNT odxt data.\n")
  if dnt_odxt then
    TEST_EQUAL_STR("11.0", dnt_odxt["mmhg"], "Testing _dntOdxt mmhg value.\n")
  end
  
  local dnt_osin = _dntOsin(data)
  TEST_TRUE(dnt_osin, "Testing parsing of DNT osin data.\n")
  if dnt_osin then
    TEST_EQUAL_STR("11.2", dnt_osin["mmhg"], "Testing _dntOsin mmhg value.\n")
  end

  testReset()
  local root = addGroup(0, "tonometri")
  local odxt = {}
  odxt["mmhg"] = "11.0"
  TEST_TRUE(_dntAddGroup(odxt, "odxt", root), "Testing creation of dnt group tree.\n")
  TEST_EQUAL_INT(1, testAddValueCount_, "Validating dnt group tree addValue.\n")
  TEST_EQUAL_INT(0, testInvalidDataCount_, "Validating dnt group tree invalidData.\n")

  testReset()
--  testfile = testGetResource(1)
  
end
--function classify(filename)
--  printClassifierMessage("Running NIDEK TONOREF2 classifier...\n")
--  local data = loadDataFromFile(filename);
--  
--  -- Check model number
--  --
--  local _, _, company, model = string.find(data, "ID(%a*)/(%a*%d)")
--
----  printClassifierMessage("Company: " .. company .. ", model: " .. model);
--
--  if(company ~= "NIDEK" or model ~= "TONOREF2") then
--    invalidData("Not NIDEK Tonoref 2 data")
--    return
--  end
--
--  printClassifierMessage("Found NIDEK Tonoref 2 data\n")
--  -- Match data with checksum
--  --
--  local i, _, checksum = string.find(data, "\4(%x%x%x%x)")
--  if(checksum) then
--    checksumdata = string.sub(data, 0, i) -- Only look at the string up to the checksum
--    checksumdata = string.gsub(checksumdata, "[\13\10]", ""); -- Remove \r and \n
--    local datavalue = 0
--    for i = 1,  string.len(checksumdata) do
--      datavalue = datavalue + string.byte(checksumdata, i)
--    end
--
----    printClassifierMessage("datavalue: " .. datavalue)
----    printClassifierMessage("checksum: " .. tonumber(checksum,16))
--
--    if(datavalue ~= tonumber(checksum, 16)) then
--      invalidData("Data does not match with checksum")
--      return
--    end
--  else
--    printClassifierMessage("Checksum not found\n")
--    invalidData("Checksum not found")
--    return
--  end
--
--  printClassifierMessage("Checksum correct\n");
--
--  local parsedData = parse(data)
--
--  -- Find classes contained in the data
--  --
--  if includesDRM(parsedData) then
--      printClassifierMessage("DRM class found\n")
--      addClass("DRM")
--  else
--    printClassifierMessage("DRM not found\n")
--  end
--
--  if includesDKM(parsedData) then
--      printClassifierMessage("DKM class found\n")
--      addClass("DKM")
--  else
--    printClassifierMessage("DKM not found\n")
--  end
--
--  if includesDNT(parsedData) then
--      printClassifierMessage("DNT class found\n")
--      addClass("DNT")
--  else
--    printClassifierMessage("DNT not found\n")
--  end
--end
--

--function old_interpret(filename, requestedType)
--  -- Get structured data
--  local data = loadDataFromFile(filename)
--  local parsedData = parse(data)
--
--  -- Create tree structure
--  root = addGroup(0, "tonoref2")
--
--  -- DRM data (autorefraktion)
--  if ( (requestedType == "DRM") and includesDRM(parsedData)) then
--    local leftobj = parsedData.DRM.leftobj;
--    local rightobj = parsedData.DRM.rightobj;
--  
--    printInterpreterMessage("Adding DRM group\n")
--    DRM = addGroup(root, "DRM")
--    
--    if(leftobj) then
--      printInterpreterMessage("Adding left eye\n")
--      local left = addGroup(DRM, "left")
--      addValue(left, "sphere", leftobj[1].sph)
--      addValue(left, "cyl", leftobj[1].sph)
--      addValue(left, "axis", leftobj[1].sph)
--    end
--    
--    if(rightobj) then
--      printInterpreterMessage("Adding right eye\n")
--      local right = addGroup(DRM, "right")
--      addValue(right, "sphere", rightobj[1].sph)
--      addValue(right, "cyl", rightobj[1].sph)
--      addValue(right, "axis", rightobj[1].sph)
--    end
--
--  end
--
--  -- DKM data (keratometer)
--  if ( (requestedType == "DKM")  and includesDKM() ) then
--    printInterpreterMessage("Adding DKM group")
--    DKM = addGroup(root, "DKM")
--    addGroup(root, "DKM")
--  end
--  
--  -- DNT data (tonometer)
--  if ( (requestedType == "DNT") and includesDNT(parsedData)) then
--    local leftntavmmhg = parsedData.DNT.leftntavmmhg;
--    local rightntavmmhg = parsedData.DNT.rightntavmmhg;
--    
--    printInterpreterMessage("Adding DNT group\n")
--    local DNT = addGroup(root, "DNT")
--
--    if(leftntavmmhg) then
--      printInterpreterMessage("Adding left eye\n")
--      local left = addGroup(DNT, "left")
--      addValue(left, "mmhg", parsedData.DNT.leftntavmmhg)
--    end
--
--    if(rightntavmmhg) then
--      printInterpreterMessage("Adding right eye\n")
--      local right = addGroup(DNT, "right")
--      addValue(right, "mmhg", parsedData.DNT.rightntavmmhg)
--    end
--  end
--
--  return root
--end
--=============================================================================
-- Class identifier code
-------------------------------------------------------------------------------
--function includesDRM(parsedData)
--  if(parsedData and parsedData.DRM and 
--      ((table.getn(parsedData.DRM.leftobj) > 0) or 
--      (table.getn(parsedData.DRM.rightobj) > 0))
--    ) then
--        return true
--   else
--     return false
--   end
--end
--
--function includesDKM(parsedData)
--  if(parsedData and parsedData.DKM and 
--      ((table.getn(parsedData.DKM.leftmm) > 0) or
--      (table.getn(parsedData.DKM.leftd) > 0) or
--      (table.getn(parsedData.DKM.rightmm) > 0) or
--      (table.getn(parsedData.DKM.rightd) > 0))
--    ) then
--        return true
--   else
--     return false
--   end
--end
--
--function includesDNT(parsedData)
--  if(parsedData and parsedData.DNT and
--      ((table.getn(parsedData.DNT.rightnt) > 0) or
--      (table.getn(parsedData.DNT.leftnt) > 0))
--    ) then
--        return true
--  else
--    return false
--  end
--end

--=============================================================================
-- Parse functions
-------------------------------------------------------------------------------

--=============================================================================
-- Parse data
-------------------------------------------------------------------------------
--function parse(data)
--  local i_DRM, j_DRM = string.find(data, "DRM.-[\1\4]") 
--  local i_DKM, j_DKM = string.find(data, "DKM.-[\1\4]") 
--  local i_DNT, j_DNT = string.find(data, "DNT.-[\1\4]") 
--
--  -- Parse DRM block
--  if(i_DRM and j_DRM) then 
--    DRM = parse_DRM(string.sub(data, i_DRM+4, j_DRM))
--  end
--  
--  -- Parse DKM block
--  if(i_DKM and j_DKM) then
--    DKM = parse_DKM(string.sub(data, i_DKM+4, j_DKM))
--  end
--
--  if(i_DNT and j_DNT) then 
--    DNT = parse_DNT(string.sub(data, i_DNT+4, j_DNT))
--  end
--
--  return { DRM = DRM, DKM = DKM, DNT = DNT }
--
--end
--
--function splitIntoLines(data)
--  local lines = {}
--  for l in string.gfind(data, ".-[\13\10]") do
--    l = string.gsub(l, "[\13\10\23].-", "")
--    table.insert(lines, l)
--  end
--
--  return lines
--end
--
--function parse_DRM(data)
--  -- Split data into lines
--  local lines = splitIntoLines(data) 
--
--  -- Local variables and types
--  local company
--  local model
--  local patient
--  local year, month, date, hour, minute, meridiem
--  local vertexdist
--  local workingdist;
--  local leftobj = {}
--  local rightobj = {}
--  local leftcl = {}
--  local rightcl = {}
--  local lefttl = {}
--  local righttl = {};
--  local pd; 
--  local rpd;
--  local lpd;
--  local npd;
--  
--  -- Parse
--  for i,l in ipairs( lines ) do
--    local dt = string.sub( l, 1, 2 );
--    if( dt == "ID" ) then
--      _, _, company, model = string.find( l, "ID(%a*)/(%a*%d*)" );
--    elseif( dt == "NO" ) then 
--      _, _, patient = string.find( l, "NO(%d*)" );
--    elseif( dt == "DA" ) then
--      -- YEAR.MONTH.DATE.HOUR:MINUTEMERIDIEM
--      found, _, year, month, date, hour, minute, meridiem = string.find(l, "DA(%d%d%d%d)%.(%d%d).(%d%d)%.(%d%d):(%d%d)(%a%a)")
--      if(found == nil) then
--        -- YEAR.MONTH.DATE.HOUR:MINUTE
--        found, _, year, month, date, hour, minute = string.find(l, "DA(%d%d%d%d)%.(%d%d)%.(%d%d)%.(%d%d):(%d%d)")
--      end
--      if(found == nil) then
--        -- MONTH/DATE/YEAR.HOUR:MINUTEMERIDIEM
--        found, _, month, date, year, hour, minute, meridiem = string.find(l, "DA(%a%a%a)/(%d%d)/(%d%d%d%d)%.(%d%d):(%d%d)(%a%a)")
--      end
--      if(found == nil) then
--        -- MONTH/DATA/YEAR.HOUR:MINUTE
--        found, _, month, date, year, hour, minute = string.find(l, "DA(%a%a%a)/(%d%d)/(%d%d%d%d)%.(%d%d):(%d%d)")
--      end
--      if(found == nil) then
--        -- DATE/MONTH/YEAR.HOUR:MINUTEMERIDIEM
--        found, _, date, month, year, hour, minute, meridiem = string.find(l, "DA(%d%d)/(%a%a%a)/(%d%d%d%d)%.(%d%d):(%d%d)(%a%a)")
--      end
--      if(found == nil) then
--        -- DATE/MONTH/YEAR.HOUR:MINUTE
--        found, _, date, month, year, hour, minute = string.find(l, "DA(%d%d)/(%a%a%a)/(%d%d%d%d)%.(%d%d):(%d%d)")
--      end
--    elseif( dt == "VD" ) then
--      vertexdist = string.sub(l, 3, 7)
--    elseif( dt == "WD" ) then
--      workingdist = string.sub(l, 3,4)
--    elseif( dt == "OL" ) then
--      table.insert( leftobj, { 
--                        sph = string.sub(l, 3, 8),
--                        cyl = string.sub(l, 9, 14),
--                        axis = string.sub(l, 15, 17),
--                        conf = string.sub(l, 18, 18),
--                        cat = string.sub(l, 19,19 ),
--                        err = nil
--                             } );
--    elseif( dt == "OR" ) then
--      table.insert( rightobj, { 
--                        sph = string.sub(l, 3, 8),
--                        cyl = string.sub(l, 9, 14),
--                        axis = string.sub(l, 15, 17),
--                        conf = string.sub(l, 18, 18),
--                        cat = string.sub(l, 19,19 ),
--                        err = nil 
--                              } );
--    elseif( dt == "ER" ) then
--      table.insert( rightobj, {
--                        sph = nil,
--                        cyl = nil,
--                        axis = nil,
--                        conf = nil,
--                        cat = nil,
--                        err = string.sub(l, 3,4)
--                              } );
--    elseif( dt == "EL" ) then
--      table.insert( leftobj, {
--                        sph = nil,
--                        cyl = nil,
--                        axis = nil,
--                        conf = nil,
--                        cat = nil,
--                        err = string.sub(l, 3,4)
--                             } );
--    elseif( dt == "CL" ) then
--      leftcl = { sph = string.sub(l, 3,8),
--                 cyl = string.sub(l, 9, 14),
--                 axis = string.sub(l, -3, -1)
--               };
--    elseif( dt == "CR" ) then
--      rightcl = { sph = string.sub(l, 3, 8),
--                  cyl = string.sub(l, 9, 14),
--                  axis = string.sub(l, -3, -1)
--                };
--    elseif( dt == "TL" ) then
--      lefttl = { sph = string.sub(l, 3, 8),
--                 cyl = string.sub(l, 9, 14),
--                 axis = string.sub(l, -3, -1)
--               };
--    elseif( dt == "TR" ) then
--      righttl = { sph = string.sub(l, 3, 8),
--                  cyl = string.sub(l, 9, 14),
--                  axis = string.sub(l, -3, -1)
--                };
--    elseif( dt == "PD" ) then
--      pd = string.sub(l,3,4);
--      rpd = string.sub(l,5,6);
--      lpd = string.sub(l,7,8);
--      npd = string.sub(l,9,10);
--    else
----      print("Unknown data type in DRM block:", l)
--      printInterpreterMessage("Unknown data type in DRM block" .. l .. "\n")
--      error("Unknown data type in DRM block:" .. l)
--    end
--  end
--  return { company = company, model = model, patient = patient, year = year, month = month, date = date, hour = hour, minute = minute, meridiem = meridiem, vertexdist = vertexdist, workingdist = workingdist, leftobj = leftobj, rightobj = rightobj, leftcl = leftcl, rightcl = rightcl, lefttl = lefttl, righttl = righttl, pd = pd, rpd = rpd, lpd = lpd, npd = npd }
--end
--
--function parse_DKM(data) 
--  local lines = splitIntoLines(data) 
--
--  local leftmm = {} 
--  local leftd = {} 
--  local rightmm = {} 
--  local rightd = {} 
--  local leftcs
--  local rightcs
--  local leftps = {}
--  local rightps = {}
--
--
--  for i,l in ipairs(lines) do
--    local dt = string.sub(l,1,2);
--    if( dt == " L" ) then
--      table.insert(leftmm, { r1 = string.sub(l, 3, 7),
--                         r2 = string.sub(l, 8, 12),
--                         axis = string.sub(l, 13, 15),
--                         ave = string.sub(l, 16, 20)
--                           } );
--    elseif( dt == "DL") then
--      table.insert(leftd, { r1 = string.sub(l, 3, 7),
--                          r2 = string.sub(l, 8, 12),
--                          axis = string.sub(l, 13, 15),
--                          ave = string.sub(l, 16, 20),
--                          cyl = string.sub(l, 21, 26)
--                          } );
--    elseif( dt == " R" ) then
--      table.insert(rightmm,  { r1 = string.sub(l, 3, 7),
--                           r2 = string.sub(l, 8, 12),
--                           axis = string.sub(l, 13, 15),
--                           ave = string.sub(l, 16, 20)
--                             } );
--    elseif( dt == "DR") then
--      table.insert(rightd, { r1 = string.sub(l, 3, 7),
--                           r2 = string.sub(l, 8, 12),
--                           axis = string.sub(l, 13, 15),
--                           ave = string.sub(l, 16, 20),
--                           cyl = string.sub(l, 21, 26)
--                           } );
--    elseif( dt == "SL" ) then
--      leftcs = string.sub(l, 3, 6)
--    elseif( dt == "SR" ) then
--      rightcs = string.sub(l, 3, 6)
--    elseif( dt == "PL" ) then
--      leftps = { ps = string.sub(l, 3, 6),
--                 tlamp = string.sub(l, 7, 7)
--               };
--    elseif( dt == "PR" ) then
--      rightps = { ps = string.sub(l, 3, 6),
--                  tlamp = string.sub(l, 7, 7)
--                };
--    else
--      printInterpreterMessage("Unknown data type in DKM block:" .. l .. "\n")
--      error("Unknown data type in DKM block:" .. l)
--    end
--  end
--
--  return {leftmm = leftmm, leftd = leftd, rightmm = rightmm, rightd = rightd, leftcs = leftcs, rightcs = rightcs, leftps = leftps, rightps = rightps}
--end
--
--function parse_DNT(data)
--  local lines = splitIntoLines(data) 
--
--  local rightnt = {} 
--  local rightntavmmhg
--  local rightntavkpa
--  local rightntno
--  local rightnterr
--  local leftnt = {} 
--  local leftntavmmhg
--  local leftntavkpa
--  local leftntno
--  local leftnterr
--  local checksum
--
--  
--  for i,l in ipairs(lines) do -- There are only two entries, one for right and one for left eye data.
--    local dt = string.sub(l,1,2);
--    if( dt == " R" ) then
--      _, _, rightntavmmhg, rightntavkpa = string.find(l, "AV(%d%d%.%d)/(%d%.%d%d)")
--      _, _, rightnterr = string.find(l, "AV(%A%A%A)/(%A%A%A )")
--      rightntno = string.sub(l,3,4);
--      l = string.sub(l, 5, -1)
--
--      if( rightnterr == nil ) then
--        for l in string.gfind(l, "[%d]+.%d.- ") do
----          print(":" .. l .. ".")
--          if(string.sub(l, 3,3) == "/" ) then
--            table.insert(rightnt, { mmhg = string.sub(l, 1, 2),
--                                    kpa = string.sub(l, 4, 6),
--                                    conf = string.sub(l, 7, 8)
--                                  } );
--          else
--            table.insert(rightnt, { mmhg = string.sub(l, 1, 3),
--                                    kpa = string.sub(l, 5, 7),
--                                    conf = string.sub(l, 8, 9)
--                                  } );
--          end
--        end
--      end
--    elseif( dt == " L" ) then
--      _, _, leftntavmmhg, leftntavkpa = string.find(l, "AV(%d%d%.%d)/(%d%.%d%d)")
--      _, _, leftnterr = string.find(l, "AV(%A%A%A)/(%A%A%A )")
--      leftntno = string.sub(l,3,4);
--      l = string.sub(l, 6, -1)
--
--      if( leftnterr == nil ) then
--        for l in string.gfind(l, "[%d]+.%d.- ") do
----          print(":" .. l .. ".")
--          if(string.sub(l, 3,3) == "/" ) then
--            table.insert(leftnt, { mmhg = string.sub(l, 1, 2),
--                                    kpa = string.sub(l, 4, 6),
--                                    conf = string.sub(l, 7, 8)
--                                  } );
--          else
--            table.insert(leftnt, { mmhg = string.sub(l, 1, 3),
--                                    kpa = string.sub(l, 5, 7),
--                                    conf = string.sub(l, 8, 9)
--                                  } );
--          end
--        end
--      end
--    else
--      printInterpreterMessage("Unknown data type in DNT block:" .. l .. "\n")
--      error("Unknown data type in DNT block:" .. l)
--    end
--  end
--
----  print(rightnt[0], leftnt[1].mmhg, rightnt[2].mmhg, rightnt[3].mmhg, rightnt[4], rightnt[5])
----  print(rightnt[0], leftnt[1].kpa, rightnt[2].kpa, rightnt[3].kpa, rightnt[4], rightnt[5])
----  print(rightnt[0], leftnt[1].conf, rightnt[2].conf, rightnt[3].conf, rightnt[4], rightnt[5])
--
--  return { rightnt = rightnt, rightntavmmhg = rightntavmmhg, rightntavkpa = rightntavkpa, rightntno = rightntno, leftnt = leftnt, leftntavmmhg = leftntavmmhg, leftntavkpa = leftntavkpa, leftntno = leftntno, rightntno = rightntno }
--end

