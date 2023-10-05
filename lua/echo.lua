function classify(filename)
   addClass('echo')
end

function interpret(filename, requestedType)
  local file = assert(io.open(filename), "Could not open file: " .. filename)
  local data = assert(file:read("*all"), "Error reading file.")
  file:close()

  root = addGroup(0, 'echo')
  addValue(root, 'value', data)
end
