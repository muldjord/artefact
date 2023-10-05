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
  printClassifierMessage("Running NIDEK NT-3000 classifier...\n")
  local data = loadDataFromFile(filename)
  return _classify(data) 
end

function _checkCompanyAndModel(data)
  local _, _, company, model = string.find(data, "ID(%a*)/(%a*-%d%d%d%d)")

  if (company ~= "NIDEK") or (model ~= "NT-3000") then
    invalidData("Not NIDEK NT-3000 data.\n")
    return false
  else
    printClassifierMessage("Company: " .. company .. ", model: " .. model .. "\n");
    return true
  end
end

function _addClasses(data)
  local class_found = false

  if _containsDNT(data) then
    printClassifierMessage("Tonometer class found.\n")
    addClass("nt3000")
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
  if not _checkCompanyAndModel(data) then return false end

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
  printInterpreterMessage("Running NIDEK NT-3000 interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
  
  local data = loadDataFromFile(filename)
  _interpret(data, requestedType)
end

function _dntMmhg(data)
  if not data then return false end

--  local _,_,value = string.find(data, "03.-(%d%d%.%d)")
  
--  if not value then
--    _,_,value = string.find(data, "03.-(%d%.%d)")
--  end

  local value = string.sub(data, -4)

  if not value then return false end

  value = value:match'^%s*(.*)' 
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
 
  local root = beginGroup(0, "nt3000")

  if dnt_odxt then
    _dntAddGroup(dnt_odxt, "odxt", root)
  end

  if dnt_osin then
    _dntAddGroup(dnt_osin, "osin", root)
  end

  endGroup()
end

function _interpret(data, requestedType)
  data = string.gsub(data, "\013", "")

  if requestedType == "nt3000" then
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

function _containsDNT(data)
  if not data then 
    return false 
  end

  dnt_block = cut(data, { "DNT" }, { "$"} )
 
  if not dnt_block then return false end

  return string.find(dnt_block, " R.-03") or string.find(dnt_block, " L.-03")

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
  TEST_TRUE(_containsDNT(data), "Testing _containsDNT on data containing a DNT block.\n")
  
  local data = "\001DRM\02#@$#$@%5sdsf23\001DKM\0024ASDADWDASDAWDADNT\004"
  TEST_FALSE(_containsDNT(data), "Testing _containsDNT on data not containing a DNT block.\n")

  -- Testing classifier
  testReset()
  local data = "DRM  21314VI as as IDNIDEK/NT-3000"
  TEST_TRUE(_checkCompanyAndModel(data), "Testing _checkCompanyAndModel on good nt3000 data.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "0", "Testing that invalidData are not called on good data in _checkCompanyAndModel.\n")
  testReset()
  local data = "DRM 23244IDNIDEL/TONOREF"
  TEST_FALSE(_checkCompanyAndModel(data), "Testing _checkCompanyAndModel on bad nt3000 data.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "1", "Testing if invalidData is called on bad data in _checkCompanyAndModel.\n")

  testReset()
  local data = "\001DRM\002OL  \023\001DKM\002 R1113123\023\001DNT\002 R03 \004"
  TEST_TRUE(_addClasses(data), "Testing _addClasses on good data.")
  TEST_EQUAL_INT(testAddClassCount_, "1", "Testing that three classes where added on data containing three classes.\n")
  TEST_EQUAL_INT(testInvalidDataCount_,"0", "Testing that invalidData where not called on data containing classes.\n")
  testReset()
  local data = "ASDADSASD"
  TEST_FALSE(_addClasses(data), "Testing that _addClasses returns false on data not containing any classes.\n")
  TEST_EQUAL_INT(testInvalidDataCount_, "1", "Testing that invalidData was reported on data not containing any classes.\n")
  TEST_EQUAL_INT(testAddClassCount_, "0", "Testing that no class where added on data not containing any classes.\n")

  local data = "DRM\001\004#@$#$@%5sdsf23\001\004DKM\001\004ASDADWDASDAWDADNT\001\001"
  TEST_FALSE(_classify(data), "Testing _classify on bad data.\n")

  -- Testing interpreter

  -- DNT --
  local data = " R03    10   22.0    12   1.0"
  TEST_EQUAL_STR("1.0", _dntMmhg(data), "Testing _dntMmhg return value on good data.\n")
  

  local data = " R0310   22.0    12   11.0\023 L03    10   12  1.6  11.2\023\004"

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

