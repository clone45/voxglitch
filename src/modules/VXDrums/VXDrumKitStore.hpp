#pragma once
// VXDrumKitStore.hpp — the user's kit library on disk.
//
// ONE file for every VX Drums in every patch:
//     <Rack user folder>/voxglitch/vx_drums_kits.json
// (asset::user), the idiom Bidoo, Geodesics and Stoermelder use for
// plugin-wide state. Not Rack's pluginSettings hook: that blob is written
// when Rack decides to save its own settings (mostly at quit), so a kit saved
// before a crash would be lost; this file is written the moment the library
// changes. Not module presets either: those capture the whole module and
// live in Rack's Preset menu, not the KIT display (rack-port-design.md §11).
//
// Shape (version 1.0.0):
//   { "version": "1.0.0",
//     "kits": [ { "uuid": "<v4>", "name": "...",
//                 "models": ["<model uuid>" x6],
//                 "knobs": [[tune, decay, shape, level] x6] }, ... ] }
//
// Models are referenced BY UUID (VXDrumVoices.hpp MODEL_SPECS), never by
// enum value or name, so the table can be reordered or renamed. An unknown
// model uuid (a kit saved by a newer plugin) falls back to the House model for
// that column rather than dropping the kit.
//
// Loaded lazily on first use, UI thread only (every caller is a menu). The
// write is atomic: a temp file then system::rename over the old one, so a
// crash mid-write cannot truncate the library.
//
// Included from the module TU after plugin.hpp (jansson, asset, system,
// random come from there).
//

#include <algorithm>
#include <cstdio>
#include <string>

#include "VXDrumKit.hpp"

namespace vx_drums
{

inline std::string kitLibraryDir()  { return rack::asset::user("voxglitch"); }
inline std::string kitLibraryPath() { return rack::asset::user("voxglitch/vx_drums_kits.json"); }

// A v4 uuid from Rack's generator (random.hpp), the frozen-once identity the
// house rule demands (CLAUDE.md).
inline std::string mintUuid()
{
    const uint64_t a = rack::random::u64();
    const uint64_t b = rack::random::u64();
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%08x-%04x-4%03x-%04x-%012llx",
                  (unsigned)(a >> 32),
                  (unsigned)((a >> 16) & 0xffffu),
                  (unsigned)(a & 0x0fffu),
                  (unsigned)(0x8000u | ((b >> 48) & 0x3fffu)),
                  (unsigned long long)(b & 0xffffffffffffull));
    return std::string(buf);
}

// ── JSON ─────────────────────────────────────────────────────────────────────
// json_object_set_new / json_array_append_new EXCLUSIVELY (PianoRoll.hpp:711-717).

inline json_t* userKitToJson(const UserKit& k)
{
    json_t* root = json_object();
    json_object_set_new(root, "uuid", json_string(k.kit.uuid.c_str()));
    json_object_set_new(root, "name", json_string(k.kit.name.c_str()));

    json_t* models = json_array();
    for (int c = 0; c < COLUMNS; c++) json_array_append_new(models, json_string(modelSpec(k.kit.models[c]).uuid));
    json_object_set_new(root, "models", models);

    json_t* knobs = json_array();
    for (int c = 0; c < COLUMNS; c++)
    {
        json_t* column = json_array();
        for (int i = 0; i < KIT_KNOBS_PER_COLUMN; i++) json_array_append_new(column, json_real(k.knobs[c][i]));
        json_array_append_new(knobs, column);
    }
    json_object_set_new(root, "knobs", knobs);
    return root;
}

// Six model uuids into `models`; a missing, short or unknown entry keeps
// House's model for that column.
inline void modelsFromJson(json_t* arr, ModelId models[COLUMNS])
{
    const KitSpec& house = kitSpec(KIT_HOUSE);
    for (int c = 0; c < COLUMNS; c++) models[c] = house.models[c];
    if (!json_is_array(arr)) return;
    const size_t n = std::min(json_array_size(arr), (size_t)COLUMNS);
    for (size_t c = 0; c < n; c++)
    {
        json_t* s = json_array_get(arr, c);
        if (!json_is_string(s)) continue;
        const ModelId id = modelFromUuid(json_string_value(s), NUM_MODELS);
        if (id != NUM_MODELS) models[c] = id;
    }
}

// False when the object has no usable uuid; everything else defaults.
inline bool userKitFromJson(json_t* root, UserKit& out)
{
    if (!json_is_object(root)) return false;
    json_t* uuid = json_object_get(root, "uuid");
    if (!json_is_string(uuid) || json_string_value(uuid)[0] == '\0') return false;

    UserKit k;
    k.kit.uuid = json_string_value(uuid);
    k.kit.custom = true;

    json_t* name = json_object_get(root, "name");
    k.kit.name = json_is_string(name) ? json_string_value(name) : "Custom kit";

    modelsFromJson(json_object_get(root, "models"), k.kit.models);

    // Knobs default to the models' own defaults, so a kit saved without them
    // (or with a short array) still plays as designed.
    for (int c = 0; c < COLUMNS; c++)
    {
        const ModelSpec& ms = modelSpec(k.kit.models[c]);
        k.knobs[c][0] = (float)knobDefault01(ms.tune);
        k.knobs[c][1] = (float)knobDefault01(ms.decay);
        k.knobs[c][2] = (float)knobDefault01(ms.shape);
        k.knobs[c][3] = 0.8f;
    }
    json_t* knobs = json_object_get(root, "knobs");
    if (json_is_array(knobs))
    {
        const size_t n = std::min(json_array_size(knobs), (size_t)COLUMNS);
        for (size_t c = 0; c < n; c++)
        {
            json_t* column = json_array_get(knobs, c);
            if (!json_is_array(column)) continue;
            const size_t m = std::min(json_array_size(column), (size_t)KIT_KNOBS_PER_COLUMN);
            for (size_t i = 0; i < m; i++)
            {
                json_t* v = json_array_get(column, i);
                if (json_is_number(v))
                {
                    const float hi = (i == 3) ? 1.2f : 1.f;
                    k.knobs[c][i] = rack::math::clamp((float)json_number_value(v), 0.f, hi);
                }
            }
        }
    }

    out = k;
    return true;
}

inline json_t* kitLibraryToJson(const KitLibrary& lib)
{
    json_t* root = json_object();
    json_object_set_new(root, "version", json_string("1.0.0"));
    json_t* kits = json_array();
    for (size_t i = 0; i < lib.kits.size(); i++) json_array_append_new(kits, userKitToJson(lib.kits[i]));
    json_object_set_new(root, "kits", kits);
    return root;
}

inline void kitLibraryFromJson(json_t* root, KitLibrary& lib)
{
    lib.kits.clear();
    if (!json_is_object(root)) return;
    json_t* kits = json_object_get(root, "kits");
    if (!json_is_array(kits)) return;
    const size_t n = json_array_size(kits);
    for (size_t i = 0; i < n; i++)
    {
        UserKit k;
        if (userKitFromJson(json_array_get(kits, i), k)) lib.add(k);   // add() refuses a duplicate uuid
    }
}

// ── The store ────────────────────────────────────────────────────────────────

struct KitStore
{
    KitLibrary library;
    bool loaded = false;

    void load()
    {
        loaded = true;
        library.kits.clear();
        const std::string path = kitLibraryPath();
        if (!rack::system::isFile(path)) return;

        json_error_t error;
        json_t* root = json_load_file(path.c_str(), 0, &error);
        if (!root)
        {
            WARN("VXDrums: could not read %s: %s (line %d)", path.c_str(), error.text, error.line);
            return;
        }
        kitLibraryFromJson(root, library);
        json_decref(root);
    }

    // Atomic: temp file, then rename over the old one.
    bool save()
    {
        rack::system::createDirectories(kitLibraryDir());
        const std::string path = kitLibraryPath();
        const std::string tmp = path + ".tmp";

        json_t* root = kitLibraryToJson(library);
        const int result = json_dump_file(root, tmp.c_str(), JSON_INDENT(2));
        json_decref(root);
        if (result != 0)
        {
            WARN("VXDrums: could not write %s", tmp.c_str());
            return false;
        }
        if (!rack::system::rename(tmp, path))
        {
            WARN("VXDrums: could not replace %s", path.c_str());
            rack::system::remove(tmp);
            return false;
        }
        return true;
    }
};

// Process-wide, UI thread only, loaded on first use.
inline KitStore& kitStore()
{
    static KitStore store;
    if (!store.loaded) store.load();
    return store;
}

} // namespace vx_drums
