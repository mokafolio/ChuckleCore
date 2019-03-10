#ifndef CHUCKLECORE_CHUCKLECORE_HPP
#define CHUCKLECORE_CHUCKLECORE_HPP

#include <Crunch/PerlinNoise.hpp>
#include <Crunch/Randomizer.hpp>

#include <Paper2/Document.hpp>
#include <Paper2/Group.hpp>
#include <Paper2/Path.hpp>
#include <Paper2/Symbol.hpp>
#include <Paper2/Tarp/TarpRenderer.hpp>

#include <Dab/Dab.hpp>

#include <Luke/Luke.hpp>

namespace chuckle
{
using namespace stick;
using namespace crunch;
using namespace paper;
using namespace luke;
using namespace dab;

STICK_API PerlinNoise & noiseInstance();
STICK_API Randomizer & randomizerInstance();

STICK_API void setRandomSeed(typename Randomizer::IntegerType _seed);
STICK_API void randomizeSeed();
STICK_API Float32 randomf(Float32 _min = 0.0f, Float32 _max = 1.0f);
STICK_API Float64 randomd(Float64 _min = 0.0, Float64 _max = 1.0);
STICK_API Int32 randomi(Int32 _min = 0, Int32 _max = std::numeric_limits<Int32>::max());
STICK_API UInt32 randomui(UInt32 _min = 0, UInt32 _max = std::numeric_limits<UInt32>::max());

STICK_API void setNoiseSeed(Int32 _seed);
STICK_API void randomizeNoiseSeed();
STICK_API Float32 noise(Float32 _x);
STICK_API Float32 noise(Float32 _x, Float32 _y);
STICK_API Float32 noise(Float32 _x, Float32 _y, Float32 _z);
STICK_API Float32 noise(Float32 _x, Float32 _y, Float32 _z, Float32 _w);

// This mainly houses paper related path utilities that don't really make sense to sit inside paper
// directly.
namespace path
{
STICK_API void longestCurves(Path * _path, Size _count, DynamicArray<Curve> & _output);

STICK_API void matchSegmentCount(Path * _a, Path * _b);

STICK_API void applyNoise(
    Item * _item, Float32 _noiseSeed, Float32 _noiseDiv, Float32 _noiseScale, Float32 _sampleDist);

STICK_API void morph(Path * _a, Path * _b, Float32 _t, Path * _output);
} // namespace path

} // namespace chuckle

#endif // CHUCKLECORE_CHUCKLECORE_HPP
