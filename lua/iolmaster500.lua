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
  printClassifierMessage("Running ZEISS IOLMaster 500 classifier...\n")
  local data = loadDataFromFile(filename)
  return _classify(data) 
end

function _checkBeginAndEndCmd(data)
  local first_cmd = string.sub(data, 0, 2);
  if not first_cmd[0] == 'E' and first_cmd[1] == 'D' then
    invalidData("Not valid IOL Master 500 data")
    return false
  end
  
  local last_cmd = string.sub(data, -4, -2)
  if not last_cmd[0] == 'E' and last_cmd[1] == 'E' then
    invalidData("Not valid IOL Master 500 data")
    return false
  end
 
  print("Valid IOL Master 500 data")
  return true
end

function _addClasses(data)
  local class_found = false

  if _containsPatient(data) then
    printClassifierMessage("Patient class found.\n")
    addClass("iolmaster500_patient")
    class_found = true
  end

  if _containsKeratometer(data) then
    printClassifierMessage("Keratometer class found.\n")
    addClass("iolmaster500_keratometer")
    class_found = true
  end

  if _containsAnteriorChamberDepth(data) then
    printClassifierMessage("Anterior Chamber Depth class found.\n")
    addClass("iolmaster500_anteriorchamberdepth")
    class_found = true
  end

  if _containsIOL(data) then
    printClassifierMessage("IOL class found.\n")
    addClass("iolmaster500_iol")
    class_found = true
  end

  if _containsAxialLength(data) then
    printClassifierMessage("Axial Length class found.\n")
    addClass("iolmaster500_axiallength")
    class_found = true
  end

  if _containsIOLCalculationParameter(data) then
    printClassifierMessage("IOL Calculation Parameter class found.\n")
    addClass("iolmaster500_iolcalculationparameter")
    class_found = true
  end

  if _containsIOLCalculationResult(data) then
    printClassifierMessage("IOL Calculation Result class found.\n")
    addClass("iolmaster500_iolcalculationresult")
    class_found = true
  end

  if _containsWTW(data) then
    printClassifierMessage("WTW class found.\n")
    addClass("iolmaster500_wtw")
    class_found = true
  end

  return class_found
end

function _classify(data)
  if not _checkBeginAndEndCmd(data) then return false end

  if not _addClasses(data) then return false end
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
  printInterpreterMessage("Running ZEISS IOLMaster 500 interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
  
  local data = loadDataFromFile(filename)
  _interpret(data, requestedType)
end

function getValuesOfBlock(data)
  values = { }

  if not data then return values end

  local begin = 1
  local number_of_values = 1; 
  for i=1, string.len(data) do
    if string.sub(data,i,i) == "\0" then
      value = string.sub(data, begin, i-1)
      values[number_of_values] = value
      begin = i+1
      number_of_values = number_of_values + 1
    end
  end

  return values;
end

function _interpretB1(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end

  values = getValuesOfBlock(data)

  if values[1] then _addValue(grp_node, "name", values[1], true) end
  if values[2] then _addValue(grp_node, "vorname", values[2], true) end
  if values[3] then _addValue(grp_node, "gebdat", values[3], true) end

  return true
end

function _interpretB2(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end

  local values = getValuesOfBlock(data)

  if values[1] then _addValue(grp_node, "id", values[1], true) end
  if values[2] then _addValue(grp_node, "note", values[2], true) end
  if values[3] then _addValue(grp_node, "od_sph", values[3], true) end
  if values[4] then _addValue(grp_node, "od_cyl", values[4], true) end
  if values[5] then _addValue(grp_node, "od_ax", values[5], true) end
  if values[6] then _addValue(grp_node, "od_vis", values[6], true) end 

  return true
end

function _interpretB3(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  if values[1] then _addValue(grp_node, "os_sph", values[1], true) end
  if values[2] then _addValue(grp_node, "os_cyl", values[2], true) end
  if values[3] then _addValue(grp_node, "os_ax", values[3], true) end
  if values[4] then _addValue(grp_node, "os_vis", values[4], true) end
  if values[5] then _addValue(grp_node, "phakicr", values[5], true) end
  if values[6] then _addValue(grp_node, "phakicl", values[6], true) end

  return true
end

function _interpretB4(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end

  local values = getValuesOfBlock(data)
  
  if values[1] then _addValue(grp_node, "examdat", values[1], true) end

  return true
end


function _interpretPatient(data)
  if not _containsPatient(data) then return false end
  
  local root = beginGroup(0, "iolmaster500_patient", true)

  local b1 = cut(data, { "\r\nB1" }, { "\r\n"})
  if b1 then
    _interpretB1(root, b1)
  end
  
  local b2 = cut(data, { "\r\nB2" }, { "\r\n"})
  if b2 then
    _interpretB2(root, b2)
  end
 
  local b3 = cut(data, { "\r\nB3" }, { "\r\n"})
  if b3 then
    _interpretB3(root, b3)
  end

  local b4 = cut(data, { "\r\nB4" }, { "\r\n"})
  if b4 then
    _interpretB4(root, b4)
  end

  endGroup(true)
end

function _interpretB5(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  local odxt_node = beginGroup(grp_node, "odxt", true)
  if values[1] then _addValue(odxt_node, "isempty1", values[1], true) end
  if values[3] then _addValue(odxt_node, "r11", values[3], true) end
  if values[4] then _addValue(odxt_node, "r21", values[4], true) end
  if values[5] then _addValue(odxt_node, "axis11", values[5], true) end
  if values[9] then _addValue(odxt_node, "isempty2", values[9], true) end
  if values[11] then _addValue(odxt_node, "r12", values[11], true) end
  if values[12] then _addValue(odxt_node, "r22", values[12], true) end
  if values[13] then _addValue(odxt_node, "axis12", values[13], true) end
  if values[17] then _addValue(odxt_node, "isempty3", values[17], true) end
  if values[19] then _addValue(odxt_node, "r13", values[19], true) end
  if values[20] then _addValue(odxt_node, "r23", values[20], true) end
  if values[21] then _addValue(odxt_node, "axis13", values[21], true) end
  if values[25] then _addValue(odxt_node, "keridx", values[25], true) end
  endGroup(true)

  local osin_node = beginGroup(grp_node, "osin", true)
  if values[2] then _addValue(osin_node, "isempty1", values[2], true) end
  if values[6] then _addValue(osin_node, "r11", values[6], true) end
  if values[7] then _addValue(osin_node, "r21", values[7], true) end
  if values[8] then _addValue(osin_node, "axis11", values[8], true) end
  if values[10] then _addValue(osin_node, "isempty2", values[10], true) end
  if values[14] then _addValue(osin_node, "r12", values[14], true) end
  if values[15] then _addValue(osin_node, "r22", values[15], true) end
  if values[16] then _addValue(osin_node, "axis12", values[16], true) end
  if values[18] then _addValue(osin_node, "isempty3", values[18], true) end
  if values[22] then _addValue(osin_node, "r13", values[22], true) end
  if values[23] then _addValue(osin_node, "r23", values[23], true) end
  if values[24] then _addValue(osin_node, "axis13", values[24], true) end
  if values[26] then _addValue(osin_node, "keridx", values[26], true) end
  endGroup(true)

  return true
end


function _interpretKeratometer(data)
  if not _containsKeratometer(data) then return false end
 
  local root = beginGroup(0, "iolmaster500_keratometer", true)

  local b5 = cut(data, { "\r\nB5" }, { "\r\n"})
  if b5 then
    _interpretB5(root, b5)
  end

  endGroup(true)
end

function _interpretB6(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  local odxt_node = beginGroup(grp_node, "odxt", true)
  if values[1] then _addValue(odxt_node, "isempty", values[1], true) end
  if values[3] then _addValue(odxt_node, "acd", values[3], true) end
  endGroup(true)

  local osin_node = beginGroup(grp_node, "osin", true)
  if values[2] then _addValue(osin_node, "isempty", values[2], true) end
  if values[4] then _addValue(osin_node, "acd", values[4], true) end
  endGroup(true)

  return true
end


function _interpretAnteriorChamberDepth(data)
  if not _containsAnteriorChamberDepth(data) then return false end
 
  local root = beginGroup(0, "iolmaster500_anteriorchamberdepth", true)

  local b6 = cut(data, { "\r\nB6" }, { "\r\n"})
  if b6 then
    _interpretB6(root, b6)
  end

  endGroup(true)
end

function _interpretB7(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  local odxt_node = beginGroup(grp_node, "odxt", true)
  
  if values[1] then _addValue(odxt_node, "m_nnomess", values[1], true) end
  if values[3] then _addValue(odxt_node, "m_pat.szoperator", values[3], true) end
  if values[4] then _addValue(odxt_node, "dai", values[4], true) end
  if values[5] then _addValue(odxt_node, "ichgai", values[5], true) end
  if values[6] then _addValue(odxt_node, "dr1", values[6], true) end
  if values[7] then _addValue(odxt_node, "dr2", values[7], true) end
  if values[8] then _addValue(odxt_node, "nchgr", values[8], true) end
  if values[9] then _addValue(odxt_node, "dvkt", values[9], true) end
  if values[10] then _addValue(odxt_node, "lchgvkt", values[10], true) end

  endGroup(true)

  local osin_node = beginGroup(grp_node, "osin", true)

  if values[2] then _addValue(osin_node, "m_nnomess", values[2], true) end
  if values[3] then _addValue(osin_node, "m_pat.szoperator", values[3], true) end
  if values[11] then _addValue(osin_node, "dai", values[11], true) end
  if values[12] then _addValue(osin_node, "ichgai", values[12], true) end
  if values[13] then _addValue(osin_node, "dr1", values[13], true) end
  if values[14] then _addValue(osin_node, "dr2", values[14], true) end
  if values[15] then _addValue(osin_node, "nchgr", values[15], true) end
  if values[16] then _addValue(osin_node, "dvkt", values[16], true) end
  if values[17] then _addValue(osin_node, "lchgvkt", values[17], true) end

  endGroup(true)

  return true
end

function _interpretB8(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  if values[1] then _addValue(grp_node, "lvalidcalc", values[1], true) end
  if values[2] then _addValue(grp_node, "nseite", values[2], true) end
  if values[3] then _addValue(grp_node, "szlinse", values[3], true) end
  if values[4] then _addValue(grp_node, "szformal", values[4], true) end
  if values[5] then _addValue(grp_node, "szkonst", values[5], true) end
  if values[6] then _addValue(grp_node, "szzielref", values[6], true) end
  if values[7] then _addValue(grp_node, "sziold", values[7], true) end
  if values[8] then _addValue(grp_node, "szrefd", values[8], true) end
  
  return true
end

function _interpretIOL(data)
  if not _containsIOL(data) then return false end
 
  local root = beginGroup(0, "iolmaster500_iol", true)

  local b7 = cut(data, { "\r\nB7" }, { "\r\n"})
  if b7 then
    _interpretB7(root, b7)
  end

--  local b8 = cut(data, { "\r\nB8" }, { "\r\n"})
--  if b8 then
--    _interpretB8(root, b8)
--  end

  endGroup(true)
end

function _interpretB9(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  bin_len = { values[2], values[3] } 
  return bin_len, values[1], values 
end

--function _interpretBA(grp_node, data, bin_len)
--  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end
--  if not data then return false end
  
--  if not bin_len then return false end
  
--  local values = {}
--  values[1] = string.sub(data, 1, 4)
--  values[2] = string.sub(data, 5, 5 + bin_len - 1)

--  if values[1] then _addValue(grp_node, "value1", b64enc(values[1]), true) end
--  if values[2] then _addValue(grp_node, "date", b64enc(values[2]), true) end

--  return true
--end

--function _interpretBB(grp_node, data, bin_len)
--  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end
--  if not data then return false end
  
--  if not bin_len then return false end
  
--  values = {}
--  values[1] = string.sub(data, 1, 8)
--  values[2] = string.sub(data, 9, 16)
--  values[3] = string.sub(data, 17, 17+bin_len - 1)

--  if values[1] then _addValue(grp_node, "max_value", b64enc(values[1]), true) end
--  if values[2] then _addValue(grp_node, "min_value", b64enc(values[2]), true) end
--  if values[3] then _addValue(grp_node, "data", b64enc(values[3]), true) end
  
--  return true
--end

--function _interpretBC(grp_node, data)
--  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end
  
--  if not data then return false end
  
--  local values = {}

--  values[1] = string.sub(data, 1, 4)
--  values[2] = string.sub(data, 5, 9)
--  values[3] = string.sub(data, 10, 12)
--  values[4] = string.sub(data, 13, 17)

--  if values[1] then _addValue(grp_node, "mdwstartmessung", b64enc(values[1]), true) end
--  if values[2] then _addValue(grp_node, "m_dwnullimpuls", b64enc(values[2]), true) end
--  if values[3] then _addValue(grp_node, "wnullctc", b64enc(values[3]), true) end
--  if values[4] then _addValue(grp_node, "m_dmaxidx", b64enc(values[4]), true) end

--  return true
--end

function _interpretAxialLength(data)
  if not _containsAxialLength(data) then return false end
 

  -- todo: calculate binary length and only read that length

  local exists = string.find(data, "\r\nB9", 0);
  
  if not exists then return false end

  local root = beginGroup(0, "iolmaster500_axiallength", true)

  local odxt = {}
  local osin = {}

  local counter = 1
  local remaining_data = data; 
  while true do
    local b9 = cut(remaining_data, { "\r\nB9" }, { "\r\n"})
    if b9 then
      local bin_len = nil
      local info = nil
      local values = nil
      bin_len, info, values = _interpretB9(root, b9)
      if info == "L" then
        table.insert(osin, values)
      elseif info == "R" then 
        table.insert(odxt, values) 
      end
      counter = counter + 1
    end

    local start_pos, end_pos = string.find(remaining_data, "\r\nB9", 0);
    if not end_pos then break end;
    remaining_data = string.sub(remaining_data, end_pos);
  end

--  local n = 0;
--  if #odxt > #osin then
--    n = #odxt
--  else
--    n = #osin
--  end

  local odxt_exists = false
  local n = #odxt
  for i = 1, n do
    if(values[2] or
       values[3] or
       values[4] or
       values[5] or
       values[6] or
       values[7] ) then
         odxt_exists = true
       end
  end
    
  if odxt_exists then
    local odxt_node = beginGroup(root, "odxt", true)
    for i = 1, n do
      local no = tostring(i)
      local values = odxt[i]
      if values[2] then _addValue(odxt_node, "m_nvmessanz" .. no, values[2], true) end
      if values[3] then _addValue(odxt_node, "m_nhkanz" .. no, values[3], true) end
      if values[4] then _addValue(odxt_node, "m_dal" .. no, values[4], true) end
      if values[5] then _addValue(odxt_node, "m_dsnr" .. no, values[5], true) end
      if values[6] then _addValue(odxt_node, "m_lmodi" .. no, values[6], true) end
      if values[7] then _addValue(odxt_node, "m_m_dn" .. no, values[7], true) end
    end
    endGroup(true)
  end

  local osin_exists = false
  local n = #osin
  for i = 1, n do
    if(values[2] or
       values[3] or
       values[4] or
       values[5] or
       values[6] or
       values[7] ) then
         osin_exists = true
       end
  end

  if osin_exists then
    local osin_node = beginGroup(root, "osin", true)
    for i = 1, n do
      local no = tostring(i)
      local values = osin[i]
      if values[2] then _addValue(osin_node, "m_nvmessanz" .. no, values[2], true) end
      if values[3] then _addValue(osin_node, "m_nhkanz" .. no, values[3], true) end
      if values[4] then _addValue(osin_node, "m_dal" .. no, values[4], true) end
      if values[5] then _addValue(osin_node, "m_dsnr" .. no, values[5], true) end
      if values[6] then _addValue(osin_node, "m_lmodi" .. no, values[6], true) end
      if values[7] then _addValue(osin_node, "m_m_dn" .. no, values[7], true) end
    end
    endGroup(true)
  end
  
end

function _interpretBD(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
--  local node = beginGroup(grp_node, "BD", true)

  local values = getValuesOfBlock(data)

  if values[1] then _addValue(grp_node, "lens1", values[1], true) end
  if values[2] then _addValue(grp_node, "const1", values[2], true) end
  if values[3] then _addValue(grp_node, "lens2", values[3], true) end
  if values[4] then _addValue(grp_node, "const2", values[4], true) end
  if values[5] then _addValue(grp_node, "lens3", values[5], true) end
  if values[6] then _addValue(grp_node, "const3", values[6], true) end
  if values[7] then _addValue(grp_node, "lens4", values[7], true) end
  if values[8] then _addValue(grp_node, "const4", values[8], true) end
  if values[9] then _addValue(grp_node, "targetref", values[9], true) end

--  endGroup(true)

  return true
end


function _interpretIOLCalculationParameter(data)
  return false
--  if not _containsIOLCalculationParameter(data) then return false end
 
--  local root = beginGroup(0, "iolmaster500_iolcalculationparameter", true)

--  local bd = cut(data, { "\r\nBD" }, { "\r\n"})
--  if bd then
--    _interpretBD(root, bd)
--  end

--  endGroup(true)
end

function _interpretBE(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

  if values[1] then _addValue(grp_node, "formula", values[1], true) end
  if values[2] then _addValue(grp_node, "calculated", values[2], true) end

  for i=1,72 do
    if values[i+2] then _addValue(grp_node, "iolvalue" .. i, values[i+2], true) end
    if values[i+3] then _addValue(grp_node, "refvalue" .. i, values[i+3], true) end
  end

  return true
end

function _interpretIOLCalculationResult(data)
  return false
--  if not _containsIOLCalculationResult(data) then return false end

--  local root = beginGroup(0, "iolmaster500_iolcalculationresult", true)

--  local remaining_data = data; 
--  while true do
--    local be = cut(remaining_data, { "\r\nBE" }, { "\r\n"})
--    if be then
--      _interpretBE(root, be)
--    end

--    start_pos, end_pos = string.find(remaining_data, "\r\rBE", 0);
--    if not end_pos then break end;
--    remaining_data = string.sub(remaining_data, end_pos);
--  end

--  endGroup(true)
end

function _interpretBF(grp_node, data)
  if string.sub(data, 1, 1) == "\0" then data = string.sub(data, 2, string.len(data)) end

  if not data then return false end
  
  local values = getValuesOfBlock(data)

--  local odxt_node = beginGroup(grp_node, "odxt", true)
--  for i=1,3 do
--    if values[i] then _addValue(odxt_node, UTF8Encode("empty" .. i), values[i]) end
--    if values[i+2] then _addValue(odxt_node, UTF8Encode("factor" .. i), values[i+2]) end
--    if values[i+3] then _addValue(odxt_node, UTF8Encode("wtw" .. i .. "0"), values[i+3]) end
--    if values[i+4] then _addValue(odxt_node, UTF8Encode("wtw" .. i .. "1"), values[i+4]) end
--    if values[i+5] then _addValue(odxt_node, UTF8Encode("wtw" .. i .. "2"), values[i+5]) end
--    if values[i+6] then _addValue(odxt_node, UTF8Encode("fixp" .. i .. "0"), values[i+6]) end
--    if values[i+7] then _addValue(odxt_node, UTF8Encode("fixp" .. i .. "1"), values[i+7]) end
--    if values[i+8] then _addValue(odxt_node, UTF8Encode("error" .. i), values[i+8]) end
--  end
--  if values[49] then _addValue(odxt_node, "wzwidx", values[49], true) end
--  endGroup(true)

--  local osin_node = beginGroup(grp_node, "osin", true)
--  for c=1,3 do
--    local i = 1;
--    if values[i+1] then _addValue(osin_node, "empty" .. i, values[i+1]) end
--    if values[i+2] then _addValue(osin_node, "factor" .. i, values[i+2]) end
--    if values[i+9] then _addValue(osin_node, "wtw" .. i .. "0", values[i+9]) end
--    if values[i+10] then _addValue(osin_node, "wtw" .. i .. "1", values[i+10]) end
--    if values[i+11] then _addValue(osin_node, "wtw" .. i .. "2", values[i+11]) end
--    if values[i+12] then _addValue(osin_node, "fixp" .. i .. "0", values[i+12]) end
--    if values[i+13] then _addValue(osin_node, "fixp" .. i .. "1", values[i+13]) end
--    if values[i+14] then _addValue(osin_node, "wtw" .. i .. "1", values[i+14]) end
--    if values[i+15] then _addValue(osin_node, "error" .. i, values[i+15]) end
--  end
--  if values[50] then _addValue(osin_node, "wzwidx", values[50]) end
--  endGroup(true)

  local odxt_node = beginGroup(grp_node, "odxt", true)
  local i = 1 
  if values[i] then _addValue(odxt_node, UTF8Encode("empty1"), values[i]) end
  if values[i+2] then _addValue(odxt_node, UTF8Encode("factor1"), values[i+2]) end
  if values[i+3] then _addValue(odxt_node, UTF8Encode("wtw10"), values[i+3]) end
  if values[i+4] then _addValue(odxt_node, UTF8Encode("wtw11"), values[i+4]) end
  if values[i+5] then _addValue(odxt_node, UTF8Encode("wtw12"), values[i+5]) end
  if values[i+6] then _addValue(odxt_node, UTF8Encode("fixp10"), values[i+6]) end
  if values[i+7] then _addValue(odxt_node, UTF8Encode("fixp11"), values[i+7]) end
  if values[i+8] then _addValue(odxt_node, UTF8Encode("error1"), values[i+8]) end
  local i = 16
  if values[i] then _addValue(odxt_node, UTF8Encode("empty2"), values[i]) end
  if values[i+2] then _addValue(odxt_node, UTF8Encode("factor2"), values[i+2]) end
  if values[i+3] then _addValue(odxt_node, UTF8Encode("wtw20"), values[i+3]) end
  if values[i+4] then _addValue(odxt_node, UTF8Encode("wtw21"), values[i+4]) end
  if values[i+5] then _addValue(odxt_node, UTF8Encode("wtw22"), values[i+5]) end
  if values[i+6] then _addValue(odxt_node, UTF8Encode("fixp20"), values[i+6]) end
  if values[i+7] then _addValue(odxt_node, UTF8Encode("fixp21"), values[i+7]) end
  if values[i+8] then _addValue(odxt_node, UTF8Encode("error2"), values[i+8]) end
  local i = 31
  if values[i] then _addValue(odxt_node, UTF8Encode("empty3"), values[i]) end
  if values[i+2] then _addValue(odxt_node, UTF8Encode("factor3"), values[i+2]) end
  if values[i+3] then _addValue(odxt_node, UTF8Encode("wtw30"), values[i+3]) end
  if values[i+4] then _addValue(odxt_node, UTF8Encode("wtw31"), values[i+4]) end
  if values[i+5] then _addValue(odxt_node, UTF8Encode("wtw32"), values[i+5]) end
  if values[i+6] then _addValue(odxt_node, UTF8Encode("fixp30"), values[i+6]) end
  if values[i+7] then _addValue(odxt_node, UTF8Encode("fixp31"), values[i+7]) end
  if values[i+8] then _addValue(odxt_node, UTF8Encode("error3"), values[i+8]) end
  if values[49] then _addValue(odxt_node, "wzwidx", values[49], true) end
  endGroup(true)

  local osin_node = beginGroup(grp_node, "osin", true)
  local i = 1
  if values[i+1] then _addValue(osin_node, "empty1", values[i+1]) end
  if values[i+2] then _addValue(osin_node, "factor1", values[i+2]) end
  if values[i+9] then _addValue(osin_node, "wtw10", values[i+9]) end
  if values[i+10] then _addValue(osin_node, "wtw11", values[i+10]) end
  if values[i+11] then _addValue(osin_node, "wtw12", values[i+11]) end
  if values[i+12] then _addValue(osin_node, "fixp10", values[i+12]) end
  if values[i+13] then _addValue(osin_node, "fixp11", values[i+13]) end
  if values[i+15] then _addValue(osin_node, "error1", values[i+15]) end
  local i = 16
  if values[i+1] then _addValue(osin_node, "empty2", values[i+1]) end
  if values[i+2] then _addValue(osin_node, "factor2", values[i+2]) end
  if values[i+9] then _addValue(osin_node, "wtw20", values[i+9]) end
  if values[i+10] then _addValue(osin_node, "wtw21", values[i+10]) end
  if values[i+11] then _addValue(osin_node, "wtw22", values[i+11]) end
  if values[i+12] then _addValue(osin_node, "fixp20", values[i+12]) end
  if values[i+13] then _addValue(osin_node, "fixp21", values[i+13]) end
  if values[i+15] then _addValue(osin_node, "error2", values[i+15]) end
  local i = 31
  if values[i+1] then _addValue(osin_node, "empty3", values[i+1]) end
  if values[i+2] then _addValue(osin_node, "factor3", values[i+2]) end
  if values[i+9] then _addValue(osin_node, "wtw30", values[i+9]) end
  if values[i+10] then _addValue(osin_node, "wtw31", values[i+10]) end
  if values[i+11] then _addValue(osin_node, "wtw32", values[i+11]) end
  if values[i+12] then _addValue(osin_node, "fixp30", values[i+12]) end
  if values[i+13] then _addValue(osin_node, "fixp31", values[i+13]) end
  if values[i+15] then _addValue(osin_node, "error3", values[i+15]) end
  if values[50] then _addValue(osin_node, "wzwidx", values[50]) end
  endGroup(true)

  return true
end

function _interpretWTW(data)
  if not _containsWTW(data) then return false end

  local root = beginGroup(0, "iolmaster500_wtw", true)

  local bf = cut(data, { "\r\nBF" }, { "\r\n"})
  if bf then
    _interpretBF(root, bf)
  end

  endGroup(true)
end

function _dummy(data)
  local root = beginGroup(0, "", true)
  _addValue("akselaengde_osin", "22.00", true)
  _addValue("akselaengde_odxt", "26.00", true)
  endGroup(true)
  return true
end

function _interpret(data, requestedType)
  printInterpreterMessage("Requesting type " .. requestedType)

  if requestedType == "iolmaster500_patient" then
    _interpretPatient(data)
  elseif requestedType == "iolmaster500_keratometer" then
    _interpretKeratometer(data)
  elseif requestedType == "iolmaster500_anteriorchamberdepth" then
    _interpretAnteriorChamberDepth(data)
  elseif requestedType == "iolmaster500_iol" then
    _interpretIOL(data)
  elseif requestedType == "iolmaster500_axiallength" then
    _interpretAxialLength(data)
  elseif requestedType == "iolmaster500_iolcalculationparameter" then
    _interpretIOLCalculationParameter(data)
  elseif requestedType == "iolmaster500_iolcalculationresult" then
    _interpretIOLCalculationResult(data)
  elseif requestedType == "iolmaster500_wtw" then
    _interpretWTW(data)
  else
    printInterpreterMessage("Invalid requested class.\n")
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
  local base64 = lua_base_dir .. "/" .. "base64.lua"
  dofile(base64)

--  create_group_depth = 0
end

function _containsBlock(data, begin_str, end_str)
  if not data then print("Empty data") return false end
  local block = cut(data, { begin_str }, { end_str } )
  if not block then print("Block not found") return false end
  return true
end

function _containsPatient(data)
  return _containsBlock(data, "\r\nB1", "\r\n") or
         _containsBlock(data, "\r\nB2", "\r\n") or
         _containsBlock(data, "\r\nB3", "\r\n") or
         _containsBlock(data, "\r\nB4", "\r\n")
end

function  _containsKeratometer(data)
  return _containsBlock(data, "\r\nB5", "\r\n")
end

function _containsAnteriorChamberDepth(data)
  return _containsBlock(data, "\r\nB6", "\r\n")
end

function _containsIOL(data)
  return _containsBlock(data, "\r\nB7", "\r\n") or
         _containsBlock(data, "\r\nB8", "\r\n")
end

function _containsAxialLength(data)
  return _containsBlock(data, "\r\nB9", "\r\n")
end

function _containsIOLCalculationParameter(data)
  return _containsBlock(data, "\r\nBD", "\r\n")
end

function _containsIOLCalculationResult(data)
  return _containsBlock(data, "\r\nBE", "\r\n")
end

function _containsWTW(data)
  return _containsBlock(data, "\r\nBF", "\r\n")
end

--=============================================================================
--
-- Test function
--
-------------------------------------------------------------------------------

function test()
  dofile 'test.lua'
  dofile 'utils.lua'
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

