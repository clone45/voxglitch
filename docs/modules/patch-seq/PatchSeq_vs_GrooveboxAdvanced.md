# PatchSeq vs Groovebox Advanced

Both modules are step sequencers that sequence entire synthesizer patches rather than just notes or CV values. However, they use fundamentally different approaches to patch organization.

## The Key Difference

**PatchSeq** uses a **1:1 model**: Each of the 16 steps has exactly one patch. Step 1 plays Patch 1, Step 2 plays Patch 2, and so on.

**Groovebox Advanced** uses a **library model**: You create a library of up to 16 reusable patches, then assign them freely to any of 32 steps. Multiple patches can play simultaneously on the same step.

## Quick Comparison

| Feature | PatchSeq | Groovebox Advanced |
|---------|----------|-------------------|
| Steps | 16 | 32 |
| Patches | 16 (one per step) | Up to 16 (reusable library) |
| Patch assignment | Fixed (step = patch) | Flexible (any patch to any step) |
| Layering | One patch per step | Multiple patches per step |
| External CV inputs | 4 | 6 |
| Audio outputs | 4 | 4 |
| Randomization | Yes (multiple strategies) | No |
| Mode/Length CV | Yes | No (use Sequencer Control patch) |
| Global effects patch | No | Yes |
| Sequencer control patch | No | Yes |

## Choose PatchSeq If You Want...

- **Maximum variety per step** - Every step can have a completely different sound engine
- **Quick randomization** - Generate instant patches with multiple sonic strategies
- **CV control of sequencer behavior** - Modulate mode and length from external sources
- **Simpler mental model** - Step 5 is always Patch 5, no assignment to remember

## Choose Groovebox Advanced If You Want...

- **Longer sequences** - 32 steps vs 16
- **Patch reuse** - Create a kick drum patch once, assign it to steps 1, 5, 9, 13
- **Layered sounds** - Stack a bass patch and a percussion patch on the same step
- **Programmable sequencer logic** - Build custom sequencing behavior with the Sequencer Control patch
- **Master effects** - Apply global reverb, compression, or creative effects to all patches
- **More external modulation** - 6 CV inputs instead of 4

## Workflow Comparison

### PatchSeq Workflow

1. Click step 1, build a patch
2. Click step 2, build a different patch
3. Repeat for all steps you want to use
4. Each step is independent - changing step 3's patch only affects step 3

### Groovebox Advanced Workflow

1. Create patches in your library (kick, snare, bass, lead, etc.)
2. Select a patch, click steps to assign it
3. Select another patch, click steps (including the same steps for layering)
4. Changing a patch in the library affects all steps using that patch

## Use Case Examples

### "I want 16 completely different sounds in sequence"
**Use PatchSeq** - The 1:1 model means maximum sonic variety with no repeated patches.

### "I want a drum pattern with a reusable kick and snare"
**Use Groovebox Advanced** - Create one kick patch and one snare patch, then assign them to create your rhythm pattern across 32 steps.

### "I want generative patches I didn't design myself"
**Use PatchSeq** - The randomization system generates complete, playable patches with various sonic strategies.

### "I want to build my own custom sequencer logic"
**Use Groovebox Advanced** - The Sequencer Control patch lets you create probability gates, euclidean rhythms, or any custom step logic.

### "I want global reverb on everything"
**Use Groovebox Advanced** - The Effects patch processes all audio before it reaches the outputs.

### "I want to modulate sequence length with an LFO"
**Use PatchSeq** - The LENGTH CV input directly controls sequence length with external CV.

## They're Complementary

These modules solve different problems. PatchSeq excels at maximum timbral variety and quick experimentation through randomization. Groovebox Advanced excels at structured compositions where patches are building blocks you arrange and layer.

Many users find value in both - PatchSeq for sound design exploration and happy accidents, Groovebox Advanced for more deliberate, arranged sequences.
