// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "dna/TypeDefs.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <functional>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace dna {

template<typename TFrom, typename TTo = TFrom>
struct SurjectiveMapping {
public:
    struct Pair {
        TFrom from;
        TTo to;
    };

public:
    explicit SurjectiveMapping(MemoryResource* memRes) :
        from{memRes},
        to{memRes} {
    }

    Pair get(std::size_t index) const {
        if (index >= size()) {
            return {};
        }
        return {from[index], to[index]};
    }

    void add(TFrom from_, TTo to_) {
        from.push_back(from_);
        to.push_back(to_);
    }

    void set(std::size_t index, TFrom from_, TTo to_) {
        if (index >= size()) {
            from.resize(index + 1ul);
            to.resize(index + 1ul);
        }
        from[index] = from_;
        to[index] = to_;
    }

    void removeIf(std::function<bool(const TFrom&, const TTo&)> predicate) {
        assert(from.size() == to.size());

        auto itFrom = from.begin();
        auto itTo = to.begin();

        // Both iterators are advanced in lockstep, so the walk must stop at whichever array ends first: the two are
        // independently deserialized and a malformed DNA can make `to` shorter, in which case running to from.end()
        // would dereference *itTo past the end and erase() an end() iterator. See size() for the full rationale.
        while ((itFrom != from.end()) && (itTo != to.end())) {
            if (predicate(*itFrom, *itTo)) {
                itFrom = from.erase(itFrom);
                itTo = to.erase(itTo);
            } else {
                ++itFrom;
                ++itTo;
            }
        }
    }

    void updateFrom(const UnorderedMap<TFrom, TFrom>& mapping) {
        update(from, mapping);
    }

    void updateTo(const UnorderedMap<TTo, TTo>& mapping) {
        update(to, mapping);
    }

    // The number of COMPLETE pairs. `from` and `to` are serialized as two independently length-prefixed arrays
    // (RawSurjectiveMapping::serialize) with nothing cross-checking them, so a malformed DNA can present different
    // lengths. Returning from.size() behind an assert (which is compiled out in release) let get() pass its
    // `index >= size()` check and then read to[index] out of bounds - reached in practice through
    // ReaderImpl::getMeshBlendShapeChannelMapping() during BinaryStreamReader::read(). The minimum is the only
    // length at which both subscripts in get() are valid, and it is unchanged for well-formed data.
    std::size_t size() const {
        assert(from.size() == to.size());
        return std::min(from.size(), to.size());
    }

    // Both sides individually, so the reader's load-path integrity check can report the mismatch (and reject the
    // DNA) rather than silently evaluating the truncated intersection that size() exposes.
    std::size_t sourceSize() const {
        return from.size();
    }

    std::size_t targetSize() const {
        return to.size();
    }

    void clear() {
        from.clear();
        to.clear();
    }

private:
    template<typename U>
    void update(Vector<U>& target, const UnorderedMap<U, U>& mapping) {
        std::transform(target.begin(), target.end(), target.begin(), [&mapping](U oldValue) { return mapping.at(oldValue); });
    }

protected:
    Vector<TFrom> from;
    Vector<TTo> to;
};

}  // namespace dna
