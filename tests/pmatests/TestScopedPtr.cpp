// Copyright Epic Games, Inc. All Rights Reserved.

#include "pmatests/Defs.h"

#include <pma/MemoryResource.h>
#include <pma/PolyAllocator.h>
#include <pma/ScopedPtr.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <vector>
#ifdef _MSC_VER
    #pragma warning(pop)
    #pragma warning(disable : 4068)
#endif

namespace pmatests {

namespace {

struct Counters {
    int constructed;
    int destructed;
};

class Client {
public:
    static Client* create(Counters& counters) {
        return new Client(counters);
    }

    static void destroy(Client* instance) {
        delete instance;
    }

    Client(Counters& counters_) :
        counters{&counters_} {
        counters->constructed += 1;
    }

    ~Client() {
        counters->destructed += 1;
    }

    Client(const Client&) = default;
    Client& operator=(const Client&) = default;

    Client(Client&&) = default;
    Client& operator=(Client&&) = default;

private:
    Counters* counters;
};

struct Base {
    virtual ~Base() = default;
};

struct Derived : public Base {

    static Derived* create() {
        return new Derived{};
    }

    static void destroy(Derived* ptr) {
        delete ptr;
    }
};

}  // namespace

}  // namespace pmatests

TEST(ScopedPtrTest, EmptyConstruction) {
    pma::ScopedPtr<pmatests::Client> sp;
    ASSERT_EQ(sp.get(), nullptr);
    ASSERT_FALSE(sp);
}

TEST(ScopedPtrTest, ProperCleanup) {
    pmatests::Counters counters{};
    ASSERT_EQ(counters.constructed, 0);
    ASSERT_EQ(counters.destructed, 0);
    {
        auto sp = pma::makeScoped<pmatests::Client, pma::FactoryCreate, pma::FactoryDestroy>(counters);
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 0);
    }

    ASSERT_EQ(counters.constructed, 1);
    ASSERT_EQ(counters.destructed, 1);
}

TEST(ScopedPtrTest, MoveAssign) {
    pmatests::Counters counters{};
    ASSERT_EQ(counters.constructed, 0);
    ASSERT_EQ(counters.destructed, 0);
    {
        pma::ScopedPtr<pmatests::Client, pma::FactoryDestroy<pmatests::Client>> sp;
        sp = pma::makeScoped<pmatests::Client, pma::FactoryCreate, pma::FactoryDestroy>(counters);
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 0);
    }
    ASSERT_EQ(counters.constructed, 1);
    ASSERT_EQ(counters.destructed, 1);
}

TEST(ScopedPtrTest, MoveConstruct) {
    pmatests::Counters counters{};
    ASSERT_EQ(counters.constructed, 0);
    ASSERT_EQ(counters.destructed, 0);
    {
        auto sp = pma::makeScoped<pmatests::Client, pma::FactoryCreate, pma::FactoryDestroy>(counters);
        pma::ScopedPtr<pmatests::Client, pma::FactoryDestroy<pmatests::Client>> dest = std::move(sp);
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 0);
    }
    ASSERT_EQ(counters.constructed, 1);
    ASSERT_EQ(counters.destructed, 1);
}

TEST(ScopedPtrTest, ReAssign) {
    pmatests::Counters counters{};
    ASSERT_EQ(counters.constructed, 0);
    ASSERT_EQ(counters.destructed, 0);
    {
        auto sp = pma::makeScoped<pmatests::Client, pma::FactoryCreate, pma::FactoryDestroy>(counters);
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 0);
        sp = nullptr;
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 1);
    }
    ASSERT_EQ(counters.constructed, 1);
    ASSERT_EQ(counters.destructed, 1);
}

TEST(ScopedPtrTest, Reset) {
    pmatests::Counters counters{};
    ASSERT_EQ(counters.constructed, 0);
    ASSERT_EQ(counters.destructed, 0);
    {
        auto sp = pma::makeScoped<pmatests::Client, pma::FactoryCreate, pma::FactoryDestroy>(counters);
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 0);
        sp.reset(new pmatests::Client(counters));
        ASSERT_EQ(counters.constructed, 2);
        ASSERT_EQ(counters.destructed, 1);
    }
    ASSERT_EQ(counters.constructed, 2);
    ASSERT_EQ(counters.destructed, 2);
}

TEST(ScopedPtrTest, UseNewDelete) {
    auto sp = pma::makeScoped<int, pma::New, pma::Delete>(42);
    ASSERT_EQ(*sp, 42);
}

TEST(ScopedPtrTest, UseNewDeleteForArrays) {
    auto sp = pma::makeScoped<int[], pma::New, pma::Delete>(100ul);
    ASSERT_EQ(sp[0], 0);
}

TEST(ScopedPtrTest, CustomDestroyer) {
    std::size_t timesCalled = 0ul;
    {
        pma::ScopedPtr<int, std::function<void(int*)>> sp{new int{}, [&timesCalled](int* ptr) {
                                                              delete ptr;
                                                              ++timesCalled;
                                                          }};
        ASSERT_EQ(timesCalled, 0ul);
    }
    ASSERT_EQ(timesCalled, 1ul);
}

TEST(ScopedPtrTest, MoveAssignWithCustomDestroyer) {
    std::size_t timesCalled = 0ul;
    {
        using PtrType = pma::ScopedPtr<pmatests::Base, std::function<void(pmatests::Base*)>>;

        PtrType sp;
        sp = PtrType{new pmatests::Derived{}, [&timesCalled](pmatests::Base* ptr) {
                         delete ptr;
                         ++timesCalled;
                     }};
        ASSERT_EQ(timesCalled, 0ul);
    }
    ASSERT_EQ(timesCalled, 1ul);
}

TEST(ScopedPtrTest, MoveConstructWithCustomDestroyer) {
    std::size_t timesCalled = 0ul;
    {
        pma::ScopedPtr<pmatests::Base, std::function<void(pmatests::Base*)>> sp{new pmatests::Derived{},
                                                                                [&timesCalled](pmatests::Base* ptr) {
                                                                                    delete ptr;
                                                                                    ++timesCalled;
                                                                                }};
        ASSERT_EQ(timesCalled, 0ul);
    }
    ASSERT_EQ(timesCalled, 1ul);
}

TEST(ScopedPtrTest, UseDefaultCreateDestroy) {
    auto spPrimitive = pma::makeScoped<int>(42);
    ASSERT_EQ(*spPrimitive, 42);

    auto spArray = pma::makeScoped<int[]>(10ul);
    ASSERT_EQ(spArray[0], 0);
}

TEST(ScopedPtrTest, StoreDerivedInBasePointer) {
    using NewCreator = pma::New<pmatests::Derived, pmatests::Base>;
    using NewDestroyer = pma::Delete<pmatests::Derived, pmatests::Base>;
    pma::ScopedPtr<pmatests::Base, NewDestroyer> vbp = pma::makeScoped<pmatests::Derived, NewCreator, NewDestroyer>();

    using FactoryCreator = pma::FactoryCreate<pmatests::Derived, pmatests::Base>;
    using FactoryDestroyer = pma::FactoryDestroy<pmatests::Derived, pmatests::Base>;
    pma::ScopedPtr<pmatests::Base, FactoryDestroyer> fbp = pma::makeScoped<pmatests::Derived, FactoryCreator, FactoryDestroyer>();
}

TEST(ScopedPtrTest, UsePolyAllocator) {
    pma::DefaultMemoryResource memRes;
    pmatests::Counters counters{};
    ASSERT_EQ(counters.constructed, 0);
    ASSERT_EQ(counters.destructed, 0);
    {
        auto sp = pma::makeScoped(pma::PolyAllocatorCreate<pmatests::Client>{&memRes},
                                  pma::PolyAllocatorDestroy<pmatests::Client>{&memRes},
                                  counters);
        ASSERT_EQ(counters.constructed, 1);
        ASSERT_EQ(counters.destructed, 0);
        ASSERT_NE(sp.get(), nullptr);
    }
    ASSERT_EQ(counters.constructed, 1);
    ASSERT_EQ(counters.destructed, 1);
}

TEST(ScopedPtrTest, UsePolyAllocatorForPrimitive) {
    pma::DefaultMemoryResource memRes;
    auto sp = pma::makeScoped(pma::PolyAllocatorCreate<int>{&memRes}, pma::PolyAllocatorDestroy<int>{&memRes}, 42);
    ASSERT_EQ(*sp, 42);
}

TEST(ScopedPtrTest, UsePolyAllocatorWithDerivedInBasePointer) {
    pma::DefaultMemoryResource memRes;
    using Creator = pma::PolyAllocatorCreate<pmatests::Derived>;
    using Destroyer = pma::PolyAllocatorDestroy<pmatests::Derived, pmatests::Base>;
    pma::ScopedPtr<pmatests::Base, Destroyer> sp = pma::makeScoped(Creator{&memRes}, Destroyer{&memRes});
    ASSERT_NE(sp.get(), nullptr);
}

TEST(ScopedPtrTest, InstanceOverloadWithStatefulLambdas) {
    std::size_t createCalls = 0ul;
    std::size_t destroyCalls = 0ul;
    {
        auto sp = pma::makeScoped(
            [&createCalls](int v) {
                ++createCalls;
                return new int{v};
            },
            [&destroyCalls](int* p) {
                delete p;
                ++destroyCalls;
            },
            7);
        ASSERT_EQ(createCalls, 1ul);
        ASSERT_EQ(destroyCalls, 0ul);
        ASSERT_EQ(*sp, 7);
    }
    ASSERT_EQ(createCalls, 1ul);
    ASSERT_EQ(destroyCalls, 1ul);
}

namespace {

// Byte-tracking resource: pins that erased destruction returns the CONCRETE
// type's size, not the base's.
class TrackingMemoryResource : public pma::MemoryResource {
public:
    void* allocate(std::size_t size, std::size_t alignment) override {
        allocated += size;
        return fallback.allocate(size, alignment);
    }

    void deallocate(void* ptr, std::size_t size, std::size_t alignment) override {
        deallocated += size;
        fallback.deallocate(ptr, size, alignment);
    }

    std::size_t allocated{};
    std::size_t deallocated{};

private:
    pma::DefaultMemoryResource fallback;
};

struct ErasedBase {
    explicit ErasedBase(int& destructed_) :
        destructed{&destructed_} {
    }

    virtual ~ErasedBase() {
        *destructed += 1;
    }

    int* destructed;
};

struct SmallDerived : ErasedBase {
    using ErasedBase::ErasedBase;
    int payload{};
};

struct LargeDerived : ErasedBase {
    using ErasedBase::ErasedBase;
    char payload[128]{};
};

}  // namespace

TEST(ScopedPtrTest, PolyAllocatorDynamicDestroyMixesTypesAndDeallocatesTrueSizes) {
    using Destroyer = pma::PolyAllocatorDynamicDestroy<ErasedBase>;
    using Owner = pma::ScopedPtr<ErasedBase, Destroyer>;

    TrackingMemoryResource tracking;
    int destructed = 0;
    {
        // Heterogeneous ownership behind ONE owner type.
        std::vector<Owner> owners;
        pma::PolyAllocator<SmallDerived> small{&tracking};
        owners.push_back(Owner{small.newObject(destructed), Destroyer::bound<SmallDerived>(&tracking)});
        pma::PolyAllocator<LargeDerived> large{&tracking};
        owners.push_back(Owner{large.newObject(destructed), Destroyer::bound<LargeDerived>(&tracking)});
    }
    ASSERT_EQ(destructed, 2);
    ASSERT_NE(tracking.allocated, 0u);
    ASSERT_EQ(tracking.allocated, tracking.deallocated) << "erased destroy must return the concrete sizes";

    // A default-constructed destroyer (empty / moved-from owner) is inert.
    Owner empty;
    static_cast<void>(empty);
}

TEST(ScopedPtrTest, PolyAllocatorDynamicDestroyUnboundAdoptionIsInertNotUB) {
    using UnboundOwner = pma::ScopedPtr<ErasedBase, pma::PolyAllocatorDynamicDestroy<ErasedBase>>;
#ifdef NDEBUG
    // Adopting a raw pointer without bound() leaves the destroyer with no destroy function; the
    // erased type is unknown, so the contract is a deliberate no-op (never a call through a null
    // function pointer, never a guessed destroy-as-B). Debug builds assert on this misuse instead.
    int destructed = 0;
    pma::DefaultMemoryResource memRes;
    pma::PolyAllocator<SmallDerived> alloc{&memRes};
    auto* raw = alloc.newObject(destructed);
    {
        UnboundOwner misused{raw};
    }
    ASSERT_EQ(destructed, 0);
    alloc.deleteObject(raw);
#elif GTEST_HAS_DEATH_TEST
    // Debug half of the contract: unbound adoption must trip the assert, not silently leak.
    EXPECT_DEATH(
        {
            int destructed = 0;
            pma::DefaultMemoryResource memRes;
            pma::PolyAllocator<SmallDerived> alloc{&memRes};
            UnboundOwner misused{alloc.newObject(destructed)};
        },
        "destroy != nullptr");
#else
    GTEST_SKIP() << "unbound adoption asserts in debug, but this platform lacks death-test support";
#endif
}

