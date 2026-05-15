-- Pandoc Lua filter: set proportional column widths on known tables so
-- short-header columns don't steal width from the long Description column.
-- Targets the three table shapes used throughout the manual:
--   * Inputs / Outputs:  Port | Label | Type | Description
--   * Parameters:        Parameter | Type | Range | Default | Description
--   * Chapter 6 summary: Module | Description
--
-- Written for pandoc 2.9.x (Table { headers, widths, ... }). A fallback path
-- handles the post-2.10 (head.rows / colspecs) shape too.

local function normalize(text)
  return (text:lower():gsub("^%s+", ""):gsub("%s+$", ""))
end

local function header_texts(tbl)
  local out = {}
  if tbl.headers then
    for _, cell in ipairs(tbl.headers) do
      table.insert(out, normalize(pandoc.utils.stringify(cell)))
    end
  elseif tbl.head and tbl.head.rows and #tbl.head.rows > 0 then
    for _, cell in ipairs(tbl.head.rows[1].cells) do
      table.insert(out, normalize(pandoc.utils.stringify(cell)))
    end
  end
  return out
end

local function pick_widths(headers)
  local n = #headers
  if n == 4 and headers[1] == "port" then
    return { 0.07, 0.10, 0.13, 0.70 }
  elseif n == 5 and headers[1] == "parameter" then
    return { 0.14, 0.11, 0.13, 0.10, 0.52 }
  elseif n == 2 and headers[1] == "module" then
    return { 0.18, 0.82 }
  end
  return nil
end

function Table(tbl)
  local headers = header_texts(tbl)
  if #headers == 0 then return nil end

  local widths = pick_widths(headers)
  if not widths then return nil end

  if tbl.widths ~= nil then
    tbl.widths = widths
  elseif tbl.colspecs then
    for i = 1, math.min(#widths, #tbl.colspecs) do
      local cs = tbl.colspecs[i]
      if type(cs) == "table" then
        cs[2] = widths[i]
      end
    end
  end

  return tbl
end
