#pragma once
// VXDrumKit.hpp — what a KIT is at runtime, and the user's kit library.
// Rack-free: compiled standalone by tests/vx_drums/.
//
// Until 2026-09-07 the module's kit was an enum into the factory table
// (VXDrumVoices.hpp KIT_SPECS). A user kit cannot be an enum value, so the
// module now carries a RESOLVED COPY of whichever kit it plays:
//
//   ModelChoice what one column plays: a ModelId, plus the audio file when the
//              id is MODEL_SAMPLE. The sample player is one model with one
//              frozen uuid; the file path is its PARAMETER (what to load),
//              never an identity — nothing is keyed or matched on it.
//   KitState   the module's kit: uuid, display name, the six models, and
//              whether it is one of the user's (custom). A patch saves all of
//              it, so a patch that names a custom kit still sounds right and
//              still shows its name on a machine that does not have the kit.
//   UserKit    one entry of the user's library: a KitState plus the 24 voice
//              knob positions (TUNE / DECAY / SHAPE / LEVEL per column). A
//              custom kit is the user's tuned sound, not only the circuits.
//   KitLibrary the in-memory library: lookup by uuid, add, update, rename,
//              remove. The file behind it (one JSON in the Rack user folder,
//              VXDrumKitStore.hpp) is Rack-bound and lives elsewhere.
//
// Identity: uuids, minted once (the factory kits' are frozen in KIT_SPECS;
// a user kit's is minted when it is saved). Names are display only, may
// collide, and are never matched on.
//

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "VXDrumVoices.hpp"

namespace vx_drums
{

static const int KIT_KNOBS_PER_COLUMN = 4;   // TUNE DECAY SHAPE LEVEL

// ── ModelChoice ──────────────────────────────────────────────────────────────
//
// `id` is always a real model for a kit column. For a per-column OVERRIDE the
// same type carries a "none" state (id == NUM_MODELS: hand the column back to
// the kit), so one value type serves both and the undo action copies it.
// `sample_path` is meaningful only with MODEL_SAMPLE and is "" otherwise;
// two choices are equal when id and path agree.
//
struct ModelChoice
{
    ModelId id = MODEL_KICK_808;
    std::string sample_path;

    ModelChoice() {}
    explicit ModelChoice(ModelId model) : id(model) {}
    ModelChoice(ModelId model, const std::string& path) : id(model), sample_path(model == MODEL_SAMPLE ? path : "") {}

    static ModelChoice none() { ModelChoice c; c.id = NUM_MODELS; return c; }
    static ModelChoice sample(const std::string& path) { return ModelChoice(MODEL_SAMPLE, path); }

    bool isNone() const   { return (int)id < 0 || (int)id >= NUM_MODELS; }
    bool isSample() const { return id == MODEL_SAMPLE; }

    // What the menus call it: the spec name, or the file name of a sample.
    std::string displayName() const
    {
        if (isNone()) return "";
        if (!isSample()) return modelSpec(id).name;
        if (sample_path.empty()) return "Sample (no file)";
        size_t slash = sample_path.find_last_of("/\\");
        std::string file = (slash == std::string::npos) ? sample_path : sample_path.substr(slash + 1);
        return "Sample: " + file;
    }

    bool operator==(const ModelChoice& o) const { return id == o.id && sample_path == o.sample_path; }
    bool operator!=(const ModelChoice& o) const { return !(*this == o); }
};

// A choice that is safe for a KIT column: an out-of-range id, or a sample
// with no file, falls back to `fallback`.
inline ModelChoice sanitizedChoice(const ModelChoice& c, ModelId fallback)
{
    if (c.isNone()) return ModelChoice(fallback);
    if (c.isSample() && c.sample_path.empty()) return ModelChoice(fallback);
    if (!c.isSample() && !c.sample_path.empty()) return ModelChoice(c.id);
    return c;
}

// ── KitState ─────────────────────────────────────────────────────────────────
struct KitState
{
    std::string uuid;
    std::string name;
    ModelChoice models[COLUMNS];
    bool custom = false;

    static KitState factory(KitId id)
    {
        const KitSpec& spec = kitSpec(id);
        KitState k;
        k.uuid = spec.uuid;
        k.name = spec.name;
        for (int c = 0; c < COLUMNS; c++) k.models[c] = ModelChoice(spec.models[c]);
        k.custom = false;
        return k;
    }

    bool sameKit(const KitState& o) const { return uuid == o.uuid; }

    bool operator==(const KitState& o) const
    {
        if (uuid != o.uuid || name != o.name || custom != o.custom) return false;
        for (int c = 0; c < COLUMNS; c++) if (models[c] != o.models[c]) return false;
        return true;
    }
    bool operator!=(const KitState& o) const { return !(*this == o); }
};

// A factory kit by uuid, or `fallback` when the uuid is not one.
inline bool factoryKitFromUuid(const std::string& uuid, KitState& out)
{
    const KitId id = kitFromUuid(uuid.c_str(), NUM_KITS);
    if (id == NUM_KITS) return false;
    out = KitState::factory(id);
    return true;
}

// ── UserKit ──────────────────────────────────────────────────────────────────
struct UserKit
{
    KitState kit;                                   // kit.custom is always true
    float knobs[COLUMNS][KIT_KNOBS_PER_COLUMN] = {};   // raw param values: TUNE/DECAY/SHAPE 0..1, LEVEL 0..1.2

    bool operator==(const UserKit& o) const
    {
        if (kit != o.kit) return false;
        for (int c = 0; c < COLUMNS; c++)
            for (int k = 0; k < KIT_KNOBS_PER_COLUMN; k++)
                if (knobs[c][k] != o.knobs[c][k]) return false;
        return true;
    }
    bool operator!=(const UserKit& o) const { return !(*this == o); }
};

// ── KitLibrary ───────────────────────────────────────────────────────────────
//
// Order is the order of saving; the menus list it as-is. Every mutation
// returns whether it changed anything, so the caller knows whether to write
// the file. Uuids are validated on add (non-empty, unique); a duplicate is
// refused rather than silently shadowed.
//
struct KitLibrary
{
    std::vector<UserKit> kits;

    int indexOf(const std::string& uuid) const
    {
        for (size_t i = 0; i < kits.size(); i++)
            if (kits[i].kit.uuid == uuid) return (int)i;
        return -1;
    }

    const UserKit* find(const std::string& uuid) const
    {
        const int i = indexOf(uuid);
        return i < 0 ? nullptr : &kits[i];
    }

    bool add(const UserKit& k)
    {
        if (k.kit.uuid.empty() || indexOf(k.kit.uuid) >= 0) return false;
        kits.push_back(k);
        kits.back().kit.custom = true;
        return true;
    }

    // Replace the kit with this uuid (models + knobs; the name is kept unless
    // the new one is non-empty).
    bool update(const UserKit& k)
    {
        const int i = indexOf(k.kit.uuid);
        if (i < 0) return false;
        UserKit next = k;
        next.kit.custom = true;
        if (next.kit.name.empty()) next.kit.name = kits[i].kit.name;
        if (kits[i] == next) return false;
        kits[i] = next;
        return true;
    }

    bool rename(const std::string& uuid, const std::string& name)
    {
        const int i = indexOf(uuid);
        if (i < 0 || name.empty() || kits[i].kit.name == name) return false;
        kits[i].kit.name = name;
        return true;
    }

    bool remove(const std::string& uuid)
    {
        const int i = indexOf(uuid);
        if (i < 0) return false;
        kits.erase(kits.begin() + i);
        return true;
    }
};

} // namespace vx_drums
