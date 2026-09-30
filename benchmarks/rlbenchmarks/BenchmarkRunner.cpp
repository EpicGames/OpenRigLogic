#include <riglogic/RigLogic.h>
#include <riglogic/version/Version.h>

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4061 4365 4800 4987)
#endif
#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Indexed by CalculationType's numeric value; order must match the enum.
static const char* calculationTypes[] = {"scalar", "sse", "avx", "neon", "any-vector", "avx512f"};
static const char* floatingPointTypes[] = {"float", "half-float"};
static const char* floatingPointModels[] = {"precise", "fast"};
static const char* rotationTypes[] = {"euler angle", "quaternion"};
const std::uint16_t rawControlIndices[] = {2,   3,   4,   5,   6,   7,   10,  11,  14,  15,  16,  17,  18,  19,  20,
                                           21,  30,  31,  34,  35,  55,  56,  70,  71,  72,  73,  74,  79,  80,  87,
                                           88,  89,  90,  91,  92,  93,  94,  103, 104, 105, 106, 107, 108, 109, 110,
                                           111, 112, 113, 114, 143, 144, 147, 148, 191, 194, 198, 199, 202, 203, 218};
static constexpr std::size_t rawControlIndexCount = sizeof(rawControlIndices) / sizeof(std::uint16_t);

static rl4::ScopedPtr<dna::BinaryStreamReader> readDNA(const char* path, dna::DataLayer layer, dna::MemoryResource* memRes) {
    auto stream = rl4::makeScoped<rl4::FileStream>(path, rl4::FileStream::AccessMode::Read, rl4::FileStream::OpenMode::Binary);
    dna::Configuration config;
    config.layer = layer;
    auto dna = rl4::makeScoped<dna::BinaryStreamReader>(stream.get(), config, memRes);
    dna->read();
    if (!rl4::Status::isOk()) {
        std::cout << rl4::Status::get().message << std::endl;
        return nullptr;
    }
    return dna;
}

class DNAFixture : public benchmark::Fixture {
public:
    using benchmark::Fixture::SetUp;

    void SetUp(const ::benchmark::State& /*unused*/) override {
        reader = readDNA(path.c_str(), dna::DataLayer::All, &defaultMemRes);
    }

public:
    static std::string path;
    static std::size_t characterInstanceCount;
    static rl4::CalculationType calculationType;
    static rl4::FloatingPointType floatingPointType;
    static rl4::FloatingPointModel floatingPointModel;
    static rl4::RotationType rotationType;

protected:
    pma::DefaultMemoryResource defaultMemRes;
    rl4::AlignedMemoryResource alignedMemRes;
    dna::ScopedPtr<dna::BinaryStreamReader> reader;
};

std::string DNAFixture::path;
std::size_t DNAFixture::characterInstanceCount = 1ul;
rl4::CalculationType DNAFixture::calculationType = rl4::CalculationType::Scalar;
rl4::FloatingPointType DNAFixture::floatingPointType = rl4::FloatingPointType::Float;
rl4::FloatingPointModel DNAFixture::floatingPointModel = rl4::FloatingPointModel::Precise;
rl4::RotationType DNAFixture::rotationType = rl4::RotationType::EulerAngles;

BENCHMARK_DEFINE_F(DNAFixture, EvaluationWithoutMLSharedRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    rlConfig.loadMachineLearnedBehavior = false;

    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);

    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    instances.reserve(characterInstanceCount);
    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            rigLogic->calculate(rigInstance.get());
            auto results = rigInstance->getJointOutputs();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationWithMLSharedRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);

    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    instances.reserve(characterInstanceCount);
    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            rigLogic->calculate(rigInstance.get());
            auto results = rigInstance->getJointOutputs();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationMLOnlySharedRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    rlConfig.loadJoints = false;
    rlConfig.loadBlendShapes = false;
    rlConfig.loadAnimatedMaps = false;
    rlConfig.loadRBFBehavior = false;
    rlConfig.loadTwistSwingBehavior = false;

    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);

    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    instances.reserve(characterInstanceCount);
    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            rigLogic->calculateMLControls(rigInstance.get());
            auto results = rigInstance->getMLControlValues();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationMLOnlyMaskedSharedRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;
    rlConfig.rotationType = DNAFixture::rotationType;

    rlConfig.loadJoints = false;
    rlConfig.loadBlendShapes = false;
    rlConfig.loadAnimatedMaps = false;
    rlConfig.loadRBFBehavior = false;
    rlConfig.loadTwistSwingBehavior = false;

    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);

    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    instances.reserve(characterInstanceCount);
    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        auto maskValues = rigInstance->getMLMaskValues();
        std::fill(maskValues.begin(), maskValues.end(), 0.5f);
        instances.push_back(std::move(rigInstance));
    }
    if (!instances.empty()) {
        state.counters["masks"] = static_cast<double>(instances.front()->getMLMaskValues().size());
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            rigLogic->calculateMLControls(rigInstance.get());
            auto results = rigInstance->getMLControlValues();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationPSDOnlySharedRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    rlConfig.loadJoints = false;
    rlConfig.loadBlendShapes = false;
    rlConfig.loadAnimatedMaps = false;
    rlConfig.loadMachineLearnedBehavior = false;
    rlConfig.loadRBFBehavior = false;
    rlConfig.loadTwistSwingBehavior = false;

    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);

    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    instances.reserve(characterInstanceCount);
    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            rigLogic->calculatePSDControls(rigInstance.get());
            auto results = rigInstance->getPSDControlValues();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationWithoutMLDedicatedRigLogic)(benchmark::State& state) {
    std::vector<rl4::ScopedPtr<rl4::RigLogic>> logics;
    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    logics.reserve(characterInstanceCount);
    instances.reserve(characterInstanceCount);
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    rlConfig.loadMachineLearnedBehavior = false;

    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        logics.push_back(std::move(rigLogic));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            logics[i]->calculate(rigInstance.get());
            auto results = rigInstance->getJointOutputs();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationWithMLDedicatedRigLogic)(benchmark::State& state) {
    std::vector<rl4::ScopedPtr<rl4::RigLogic>> logics;
    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    logics.reserve(characterInstanceCount);
    instances.reserve(characterInstanceCount);

    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        logics.push_back(std::move(rigLogic));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            logics[i]->calculate(rigInstance.get());
            auto results = rigInstance->getJointOutputs();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationMLOnlyDedicatedRigLogic)(benchmark::State& state) {
    std::vector<rl4::ScopedPtr<rl4::RigLogic>> logics;
    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    logics.reserve(characterInstanceCount);
    instances.reserve(characterInstanceCount);

    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    rlConfig.loadJoints = false;
    rlConfig.loadBlendShapes = false;
    rlConfig.loadAnimatedMaps = false;
    rlConfig.loadRBFBehavior = false;
    rlConfig.loadTwistSwingBehavior = false;

    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        logics.push_back(std::move(rigLogic));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            logics[i]->calculateMLControls(rigInstance.get());
            auto results = rigInstance->getMLControlValues();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, EvaluationPSDOnlyDedicatedRigLogic)(benchmark::State& state) {
    std::vector<rl4::ScopedPtr<rl4::RigLogic>> logics;
    std::vector<rl4::ScopedPtr<rl4::RigInstance>> instances;
    logics.reserve(characterInstanceCount);
    instances.reserve(characterInstanceCount);

    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    rlConfig.loadJoints = false;
    rlConfig.loadBlendShapes = false;
    rlConfig.loadAnimatedMaps = false;
    rlConfig.loadMachineLearnedBehavior = false;
    rlConfig.loadRBFBehavior = false;
    rlConfig.loadTwistSwingBehavior = false;

    for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
        auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);
        auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());
        rigInstance->setLOD(static_cast<std::uint16_t>(state.range(0)));
        logics.push_back(std::move(rigLogic));
        instances.push_back(std::move(rigInstance));
    }

    std::size_t frame = {};
    for (auto _ : state) {
        for (std::size_t i = 0ul; i < characterInstanceCount; ++i) {
            auto& rigInstance = instances[i];
            for (std::size_t ci = {}; ci < rawControlIndexCount; ++ci) {
                rigInstance->setRawControl(rawControlIndices[ci], static_cast<float>(frame % 10ul) / 50.0f);
            }
            logics[i]->calculatePSDControls(rigInstance.get());
            auto results = rigInstance->getPSDControlValues();
            benchmark::DoNotOptimize(results);
        }
        ++frame;
    }
}

BENCHMARK_DEFINE_F(DNAFixture, InitializeRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    for (auto _ : state) {
        auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);
        benchmark::DoNotOptimize(rigLogic);
    }
}

BENCHMARK_DEFINE_F(DNAFixture, RestoreRigLogic)(benchmark::State& state) {
    rl4::Configuration rlConfig{};
    rlConfig.calculationType = DNAFixture::calculationType;
    rlConfig.floatingPointType = DNAFixture::floatingPointType;
    rlConfig.rotationType = DNAFixture::rotationType;
    rlConfig.floatingPointModel = DNAFixture::floatingPointModel;

    auto stream = rl4::makeScoped<rl4::MemoryStream>();
    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig, &alignedMemRes);
    rigLogic->dump(stream.get());
    for (auto _ : state) {
        stream->seek(0);
        auto restored = rl4::RigLogic::restore(stream.get());
        benchmark::DoNotOptimize(restored);
        rl4::RigLogic::destroy(restored);
    }
}

BENCHMARK_REGISTER_F(DNAFixture, EvaluationWithoutMLSharedRigLogic)
    ->Arg(0)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationWithMLSharedRigLogic)->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Arg(5)->Arg(6)->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationMLOnlySharedRigLogic)->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Arg(5)->Arg(6)->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationMLOnlyMaskedSharedRigLogic)
    ->Arg(0)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationPSDOnlySharedRigLogic)->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Arg(5)->Arg(6)->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationWithoutMLDedicatedRigLogic)
    ->Arg(0)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationWithMLDedicatedRigLogic)
    ->Arg(0)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationMLOnlyDedicatedRigLogic)
    ->Arg(0)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, EvaluationPSDOnlyDedicatedRigLogic)
    ->Arg(0)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7);
BENCHMARK_REGISTER_F(DNAFixture, InitializeRigLogic);
BENCHMARK_REGISTER_F(DNAFixture, RestoreRigLogic);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Missing DNA path argument." << std::endl;
        return -1;
    }

    DNAFixture::path = argv[1];

    auto isArg = [](const char* lhs, const char* rhs) { return (std::strcmp(lhs, rhs) == 0); };

    int i = 2;
    while (i < argc) {
        if (isArg(argv[i], "-c") || isArg(argv[i], "--characters")) {
            if ((i + 1) == argc) {
                std::cerr << "Error: missing character instance count argument." << std::endl;
                return -1;
            }
            DNAFixture::characterInstanceCount = static_cast<std::size_t>(std::stoi(argv[i + 1]));
            ++i;
        } else if (isArg(argv[i], "--sse")) {
            DNAFixture::calculationType = rl4::CalculationType::SSE;
        } else if (isArg(argv[i], "--avx")) {
            DNAFixture::calculationType = rl4::CalculationType::AVX;
        } else if (isArg(argv[i], "--avx512")) {
            DNAFixture::calculationType = rl4::CalculationType::AVX512F;
        } else if (isArg(argv[i], "--neon")) {
            DNAFixture::calculationType = rl4::CalculationType::NEON;
        } else if (isArg(argv[i], "--hf")) {
            DNAFixture::floatingPointType = rl4::FloatingPointType::HalfFloat;
        } else if (isArg(argv[i], "--precise")) {
            DNAFixture::floatingPointModel = rl4::FloatingPointModel::Precise;
        } else if (isArg(argv[i], "--fast")) {
            DNAFixture::floatingPointModel = rl4::FloatingPointModel::Fast;
        } else if (isArg(argv[i], "--quat")) {
            DNAFixture::rotationType = rl4::RotationType::Quaternions;
        }
        ++i;
    }

    std::cout << std::endl
              << ":::::[ RigLogic " << RL_VERSION_STRING << " ]:::::" << std::endl
              << std::endl
              << ":::::[ benchmark options: " << calculationTypes[static_cast<std::size_t>(DNAFixture::calculationType)] << ","
              << floatingPointTypes[static_cast<std::size_t>(DNAFixture::floatingPointType)] << ","
              << floatingPointModels[static_cast<std::size_t>(DNAFixture::floatingPointModel)] << ","
              << rotationTypes[DNAFixture::rotationType == rl4::RotationType::EulerAngles ? 0 : 1] << " ]:::::" << std::endl
              << std::endl;

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
