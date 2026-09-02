-- Springfield Nexus Test 67 FIX6
-- Dynamic DualSense prompt transformation for art\\frontend\\scrooby\\ingame.p3d.
--
-- FIX6 is generated from the runtime-approved FIX5 visual result. It does NOT
-- ship a whole ingame.p3d. Instead it reads an upstream direct frontend override
-- from another enabled mod when one exists, otherwise the untouched game file
-- through /GameDir, and applies only the validated prompt edits.
--
-- The /GameDir fallback is deliberate: /GameData is the CustomFiles virtual
-- view and can route the same handled path back through CustomFiles again.

local SELF_INTERNAL_NAME = "SpringfieldNexusDualSensePrompts"

local P3D_ROOT = 0xFF443350
local P3D_SPRITE = 0x00019005
local P3D_IMAGE = 0x00019001
local P3D_IMAGE_DATA = 0x00019002
local P3D_SCROOBY_PAGE = 0x00018002
local P3D_SCROOBY_GROUP = 0x00018004
local P3D_SCROOBY_SPRITE = 0x00018006
local P3D_SCROOBY_TEXT = 0x00018007

local function u32le(s, p)
    local a, b, c, d = string.byte(s, p, p + 3)
    if not d then return nil end
    return a + b * 0x100 + c * 0x10000 + d * 0x1000000
end

local function put_u32le(n)
    n = n % 0x100000000
    local a = n % 0x100
    n = math.floor(n / 0x100)
    local b = n % 0x100
    n = math.floor(n / 0x100)
    local c = n % 0x100
    n = math.floor(n / 0x100)
    local d = n % 0x100
    return string.char(a, b, c, d)
end

local function hex_bytes(hex)
    local out = {}
    for i = 1, #hex, 2 do
        out[#out + 1] = string.char(tonumber(string.sub(hex, i, i + 1), 16))
    end
    return table.concat(out)
end

local function chunk_header(data, pos)
    if pos < 1 or pos + 11 > #data then return nil end
    local id = u32le(data, pos)
    local header_len = u32le(data, pos + 4)
    local total_len = u32le(data, pos + 8)
    if not id or not header_len or not total_len then return nil end
    if header_len < 12 or total_len < header_len or pos + total_len - 1 > #data then return nil end
    return id, header_len, total_len
end

local function p3d_string(data, pos)
    local padded_len = string.byte(data, pos)
    if not padded_len or padded_len < 1 or pos + padded_len > #data then return nil end
    local raw = string.sub(data, pos + 1, pos + padded_len)
    local zero = string.find(raw, "\0", 1, true)
    if zero then raw = string.sub(raw, 1, zero - 1) end
    return raw
end

local function direct_children(data, parent_pos)
    local id, header_len, total_len = chunk_header(data, parent_pos)
    if not id then return nil end
    local out = {}
    local pos = parent_pos + header_len
    local finish = parent_pos + total_len
    while pos < finish do
        local cid, ch, ct = chunk_header(data, pos)
        if not cid or pos + ct > finish then return nil end
        out[#out + 1] = { pos = pos, id = cid, header_len = ch, total_len = ct }
        pos = pos + ct
    end
    if pos ~= finish then return nil end
    return out
end

local function named_chunk_name(data, child)
    if child.id ~= P3D_SCROOBY_PAGE and
       child.id ~= P3D_SCROOBY_GROUP and
       child.id ~= P3D_SCROOBY_SPRITE and
       child.id ~= P3D_SCROOBY_TEXT and
       child.id ~= P3D_SPRITE then
        return nil
    end
    return p3d_string(data, child.pos + 12)
end

local function find_direct_sprite(data, wanted)
    local kids = direct_children(data, 1)
    if not kids then return nil end
    for _, child in ipairs(kids) do
        if child.id == P3D_SPRITE then
            local name = p3d_string(data, child.pos + 12)
            if name and string.lower(name) == string.lower(wanted) then return child end
        end
    end
    return nil
end

local function find_image_data(data, sprite)
    local sprite_kids = direct_children(data, sprite.pos)
    if not sprite_kids then return nil end
    for _, image in ipairs(sprite_kids) do
        if image.id == P3D_IMAGE then
            local image_kids = direct_children(data, image.pos)
            if image_kids then
                for _, image_data in ipairs(image_kids) do
                    if image_data.id == P3D_IMAGE_DATA then return image, image_data end
                end
            end
        end
    end
    return nil
end

local function rebuild_with_replacement(data, chunk, replacement_pos, replacement_old_len, replacement_bytes)
    local before = string.sub(data, chunk.pos + 12, replacement_pos - 1)
    local after = string.sub(data, replacement_pos + replacement_old_len, chunk.pos + chunk.total_len - 1)
    local body = before .. replacement_bytes .. after
    return put_u32le(chunk.id) .. put_u32le(chunk.header_len) .. put_u32le(12 + #body) .. body
end

local function replace_back_with_circle(data)
    local root_id, root_header_len, root_total_len = chunk_header(data, 1)
    if root_id ~= P3D_ROOT or root_total_len ~= #data then return data, false end

    local back = find_direct_sprite(data, "back.png")
    local circle = find_direct_sprite(data, "but_brak.png") or find_direct_sprite(data, "but_run.png")
    if not back or not circle then return data, false end

    local back_image, back_data = find_image_data(data, back)
    local circle_image, circle_data = find_image_data(data, circle)
    if not back_image or not back_data or not circle_image or not circle_data then return data, false end

    local circle_bytes = string.sub(data, circle_data.pos, circle_data.pos + circle_data.total_len - 1)
    local rebuilt_image = rebuild_with_replacement(data, back_image, back_data.pos, back_data.total_len, circle_bytes)
    local rebuilt_sprite = rebuild_with_replacement(data, back, back_image.pos, back_image.total_len, rebuilt_image)
    local root = { pos = 1, id = root_id, header_len = root_header_len, total_len = root_total_len }
    return rebuild_with_replacement(data, root, back.pos, back.total_len, rebuilt_sprite), true
end

-- Exact Scrooby Sprite chunks copied from SHAR's own console frontend and
-- already runtime-approved in FIX5. Keys are page name then parent Group name.
local SPRITE_RULES = {
    ["Hud.pag"] = {
        ["ActionButton"] = hex_bytes("068001004e0000004e0000000c416374696f6e427574746f6e01000000270100007c01000032000000320000000400000004000000ffffffff00000000000000000100000008616374696f6e0000"),
    },
    ["LetterBoxButtons.pag"] = {
        ["SkipLabel"] = hex_bytes("06800100460000004600000004536b69700100000018010000200000001e0000001e0000000400000004000000ffffffff000000000000000001000000086163636570740000"),
        ["BackLabel"] = hex_bytes("0680010046000000460000000843616e63656c000001000000a4010000200000001e0000001e0000000400000004000000ffffffff000000000000000001000000046261636b"),
        ["AcceptLabel"] = hex_bytes("068001004a0000004a0000000841636365707400000100000064000000200000001e0000001e0000000400000004000000ffffffff000000000000000001000000086163636570740000"),
    },
    ["Accept.pag"] = {
        ["AcceptLabel"] = hex_bytes("068001004a0000004a00000008416363657074000001000000b30100005a0000001e0000001e0000000400000004000000ffffffff000000000000000001000000086163636570740000"),
    },
    ["Accept3.pag"] = {
        ["AcceptLabel"] = hex_bytes("068001004a0000004a00000008416363657074000001000000b3010000370000001e0000001e0000000400000004000000ffffffff000000000000000001000000086163636570740000"),
    },
    ["Back.pag"] = {
        ["BackLabel"] = hex_bytes("068001004200000042000000044261636b01000000b3010000370000001e0000001e0000000400000004000000ffffffff000000000000000001000000046261636b"),
    },
    ["Buy.pag"] = {
        ["AcceptLabel"] = hex_bytes("068001004a0000004a0000000841636365707400000100000050000000280000001e0000001e0000000400000004000000ffffffff000000000000000001000000086163636570740000"),
    },
    ["Cancel.pag"] = {
        ["BackLabel"] = hex_bytes("068001004200000042000000044261636b010000009f010000280000001e0000001e0000000400000004000000ffffffff000000000000000001000000046261636b"),
    },
    ["Continue.pag"] = {
        ["AcceptLabel"] = hex_bytes("068001004a0000004a00000008416363657074000001000000b3010000370000001e0000001e0000000400000004000000ffffffff000000000000000001000000086163636570740000"),
    },
    ["MissionLoad.pag"] = {
        ["Abort"] = hex_bytes("0680010046000000460000000841626f7274000000010000005e010000370000001e0000001e0000000000000002000000ffffffff000000000000000001000000046261636b"),
        ["Continue"] = hex_bytes("068001004a0000004a00000008436f6e74696e7565010000005a000000390000001e0000001e0000000000000002000000ffffffff000000000000000001000000086163636570740000"),
    },
    ["Rewards.pag"] = {
        ["ToggleView"] = hex_bytes("068001004a0000004a0000000c546f67676c6556696577000001000000500000004b0000001e0000001e0000000000000002000000ffffffff0000000000000000010000000461757858"),
    },
    ["Tutorial.pag"] = {
        ["DisableTutorial"] = hex_bytes("068001004e0000004e0000001044697361626c655475746f7269616c000100000050000000370000001e0000001e0000000400000004000000ffffffff0000000000000000010000000461757858"),
    },
}

-- mode "xy" moves both coordinates; mode "y" preserves X and moves only Y.
local TEXT_RULES = {
    ["Hud.pag"] = {
        ActionTextLabel = { mode = "xy", x = 10000, y = 10000 },
        ActionTextButton = { mode = "xy", x = 10000, y = 10000 },
    },
    ["LetterBoxButtons.pag"] = {
        SkipButton = { mode = "y", y = -2000 },
        BackButton = { mode = "y", y = -2000 },
        AcceptButton = { mode = "y", y = -2000 },
    },
    ["Back.pag"] = { Esc = { mode = "xy", x = 10000, y = 10000 } },
    ["Buy.pag"] = { AcceptButton = { mode = "xy", x = 10000, y = 10000 } },
    ["Cancel.pag"] = { BackButton = { mode = "xy", x = 10000, y = 10000 } },
    ["Continue.pag"] = { AcceptButton = { mode = "xy", x = 10000, y = 10000 } },
    ["Rewards.pag"] = { ToggleButton = { mode = "xy", x = 10000, y = 10000 } },
    ["Tutorial.pag"] = { DisableButton = { mode = "xy", x = 10000, y = 10000 } },
}

local function patch_text_coordinates(data, child, rule)
    local padded_len = string.byte(data, child.pos + 12)
    if not padded_len then return nil end
    local x_pos = child.pos + 13 + padded_len
    if x_pos + 7 > child.pos + child.header_len - 1 then return nil end

    if rule.mode == "xy" then
        return string.sub(data, child.pos, x_pos - 1) ..
               put_u32le(rule.x) .. put_u32le(rule.y) ..
               string.sub(data, x_pos + 8, child.pos + child.total_len - 1)
    elseif rule.mode == "y" then
        return string.sub(data, child.pos, x_pos + 3) ..
               put_u32le(rule.y) ..
               string.sub(data, x_pos + 8, child.pos + child.total_len - 1)
    end
    return nil
end

local function transform_chunk(data, pos, current_page)
    local id, header_len, total_len = chunk_header(data, pos)
    if not id then return nil, false, 0 end

    local original = string.sub(data, pos, pos + total_len - 1)
    local node = { pos = pos, id = id, header_len = header_len, total_len = total_len }
    local name = named_chunk_name(data, node)
    if id == P3D_SCROOBY_PAGE then current_page = name end

    local children = direct_children(data, pos)
    if children == nil then return original, false, 0 end

    -- Text nodes are leaves in these Scrooby pages. Preserve the object itself;
    -- only move the PC keyboard-binding text off-screen when the approved rule matches.
    if id == P3D_SCROOBY_TEXT and current_page and name then
        local page_rules = TEXT_RULES[current_page]
        local rule = page_rules and page_rules[name] or nil
        if rule then
            local patched = patch_text_coordinates(data, node, rule)
            if patched then return patched, patched ~= original, 1 end
        end
    end

    local rebuilt_children = {}
    local changed = false
    local edit_count = 0
    for _, child in ipairs(children) do
        local rebuilt, child_changed, child_edits = transform_chunk(data, child.pos, current_page)
        if not rebuilt then return original, false, edit_count end
        rebuilt_children[#rebuilt_children + 1] = rebuilt
        if child_changed then changed = true end
        edit_count = edit_count + child_edits
    end

    if id == P3D_SCROOBY_GROUP and current_page and name then
        local page_sprites = SPRITE_RULES[current_page]
        local sprite_bytes = page_sprites and page_sprites[name] or nil
        if sprite_bytes then
            local wanted_name = p3d_string(sprite_bytes, 13)
            local already_present = false
            for _, child in ipairs(children) do
                if child.id == P3D_SCROOBY_SPRITE then
                    local child_name = named_chunk_name(data, child)
                    if child_name == wanted_name then already_present = true break end
                end
            end
            if not already_present then
                rebuilt_children[#rebuilt_children + 1] = sprite_bytes
                changed = true
                edit_count = edit_count + 1
            end
        end
    end

    if not changed then return original, false, edit_count end
    local header_extra = string.sub(data, pos + 12, pos + header_len - 1)
    local body = header_extra .. table.concat(rebuilt_children)
    return put_u32le(id) .. put_u32le(header_len) .. put_u32le(12 + #body) .. body, true, edit_count
end

local function choose_source_path(requested)
    -- Never read the handled path through /GameData here. /GameData includes the
    -- CustomFiles virtual view, while /GameDir is the physical game install.
    local selected = "/GameDir/" .. requested

    -- Best-effort composition with another enabled mod that directly overrides
    -- this frontend. We explicitly ignore ourselves because FIX6 has no direct
    -- CustomFiles copy of ingame.p3d.
    GetEnabledMods(function(mod_name)
        if mod_name ~= SELF_INTERNAL_NAME then
            local candidate = GetModPath(mod_name) .. "/CustomFiles/" .. requested
            if Exists(candidate) then selected = candidate end
        end
        return true
    end)
    return selected
end

local requested = FixSlashes(GetPath(), false, true)
local source_path = choose_source_path(requested)
local source = ReadFile(source_path)
if not source or #source < 12 then
    print("[Springfield Nexus DualSense Prompts] Could not read upstream ingame.p3d; using game file.")
    Redirect("/GameDir/" .. requested)
    return
end

local patched = source
local back_changed = false
patched, back_changed = replace_back_with_circle(patched)

local rebuilt, tree_changed, edit_count = transform_chunk(patched, 1, nil)
if rebuilt then patched = rebuilt end

print("[Springfield Nexus DualSense Prompts] Dynamic ingame.p3d handler ran; source=" .. source_path ..
      ", back=" .. tostring(back_changed) .. ", scrooby_edits=" .. tostring(edit_count))
Output(patched)
