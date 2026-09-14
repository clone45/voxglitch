// VX Drums kit model: KitState (factory resolution, identity by uuid) and
// KitLibrary (add / find / update / rename / remove and their refusals).
// The file behind the library is Rack-bound and not tested here.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrums -o kit_test kit_test.cpp && ./kit_test

#include "VXDrumKit.hpp"
#include "check.hpp"

using namespace vx_drums;

static UserKit sample(const char* uuid, const char* name, ModelId bd)
{
    UserKit k;
    k.kit.uuid = uuid;
    k.kit.name = name;
    k.kit.custom = true;
    k.kit.models[0] = ModelChoice(bd);
    for (int c = 0; c < COLUMNS; c++)
        for (int i = 0; i < KIT_KNOBS_PER_COLUMN; i++) k.knobs[c][i] = 0.25f * (float)i;
    return k;
}

static void kitState()
{
    std::printf("KitState\n");
    const KitState house = KitState::factory(KIT_HOUSE);
    check("factory House has the table's uuid", house.uuid == kitSpec(KIT_HOUSE).uuid);
    check("and its name", house.name == "House");
    check("and its models", house.models[1] == ModelChoice(kitSpec(KIT_HOUSE).models[1]));
    check("and is not custom", !house.custom);

    KitState out;
    check("a factory uuid resolves", factoryKitFromUuid(kitSpec(KIT_TR909).uuid, out));
    check("to that kit", out.uuid == kitSpec(KIT_TR909).uuid && out.name == "TR-909");
    check("an unknown uuid does not", !factoryKitFromUuid("not-a-kit", out));
    check("and leaves the output alone", out.name == "TR-909");

    KitState a = house, b = house;
    check("equal copies compare equal", a == b);
    b.name = "House (renamed)";
    check("but still the SAME kit by uuid", a.sameKit(b));
    check("while not equal", a != b);
    b = house;
    b.models[3] = ModelChoice(MODEL_TOM);
    check("a model difference is inequality", a != b);
}

static void modelChoice()
{
    std::printf("ModelChoice\n");
    ModelChoice kick(MODEL_KICK_808);
    check("a model choice is not a sample", !kick.isSample() && !kick.isNone());
    check("and shows the spec name", kick.displayName() == "Kick 808");
    check("a bare model carries no path", ModelChoice(MODEL_KICK_808, "/x.wav").sample_path.empty());

    ModelChoice s = ModelChoice::sample("/kits/one/hat.wav");
    check("a sample choice is a sample", s.isSample() && !s.isNone());
    check("shown by file name", s.displayName() == "Sample: hat.wav");
    check("backslash paths too", ModelChoice::sample("C:\\kits\\hat.wav").displayName() == "Sample: hat.wav");
    check("two samples of the same file are equal", s == ModelChoice::sample("/kits/one/hat.wav"));
    check("of different files are not", s != ModelChoice::sample("/kits/one/snare.wav"));
    check("and a sample is never equal to a model", s != ModelChoice(MODEL_SAMPLE));

    ModelChoice none = ModelChoice::none();
    check("none is none", none.isNone() && !none.isSample());
    check("and has no name", none.displayName().empty());

    check("sanitized: a model passes", sanitizedChoice(kick, MODEL_TOM) == kick);
    check("sanitized: a sample with a file passes", sanitizedChoice(s, MODEL_TOM) == s);
    check("sanitized: a sample without a file falls back", sanitizedChoice(ModelChoice(MODEL_SAMPLE), MODEL_TOM) == ModelChoice(MODEL_TOM));
    check("sanitized: none falls back", sanitizedChoice(none, MODEL_TOM) == ModelChoice(MODEL_TOM));

    KitState a = KitState::factory(KIT_HOUSE), b = a;
    b.models[5] = s;
    check("a kit with a sample column differs from the factory kit", a != b);
    b.models[5] = ModelChoice::sample("/kits/one/hat.wav");
    check("and equals another with the same file", b == b);
}

static void library()
{
    std::printf("KitLibrary\n");
    KitLibrary lib;
    check("empty at first", lib.kits.empty());
    check("find on empty is null", lib.find("x") == nullptr);

    check("add", lib.add(sample("u1", "One", MODEL_KICK_FM)));
    check("add a second", lib.add(sample("u2", "Two", MODEL_KICK_SINE)));
    check("a duplicate uuid is refused", !lib.add(sample("u1", "One again", MODEL_KICK_DIST)));
    check("an empty uuid is refused", !lib.add(sample("", "Nameless", MODEL_KICK_808)));
    checkEq("two kits", (long long)lib.kits.size(), 2);
    checkEq("in saving order", lib.indexOf("u2"), 1);
    check("add marks the kit custom even if the caller forgot", lib.kits[0].kit.custom);

    const UserKit* one = lib.find("u1");
    check("find by uuid", one && one->kit.name == "One" && one->kit.models[0] == ModelChoice(MODEL_KICK_FM));

    // update: models + knobs replaced, name kept when empty
    UserKit u = sample("u1", "", MODEL_KICK_DIST);
    u.knobs[2][1] = 0.9f;
    check("update changes it", lib.update(u));
    one = lib.find("u1");
    check("models replaced", one->kit.models[0] == ModelChoice(MODEL_KICK_DIST));
    check("knobs replaced", one->knobs[2][1] == 0.9f);
    check("the name is kept when the update has none", one->kit.name == "One");
    check("an identical update is a no-op", !lib.update(u));
    check("updating an unknown uuid is refused", !lib.update(sample("u9", "Nine", MODEL_KICK_808)));

    check("rename", lib.rename("u2", "Deux"));
    check("renamed", lib.find("u2")->kit.name == "Deux");
    check("rename to the same name is a no-op", !lib.rename("u2", "Deux"));
    check("rename to empty is refused", !lib.rename("u2", ""));
    check("rename of an unknown uuid is refused", !lib.rename("u7", "Seven"));

    check("remove", lib.remove("u1"));
    check("gone", lib.find("u1") == nullptr);
    checkEq("one left", (long long)lib.kits.size(), 1);
    check("removing again is a no-op", !lib.remove("u1"));
    checkEq("the survivor moved up", lib.indexOf("u2"), 0);
}

int main()
{
    kitState();
    modelChoice();
    library();
    return finish("kit_test");
}
