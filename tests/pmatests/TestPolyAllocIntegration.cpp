// Copyright Epic Games, Inc. All Rights Reserved.

#include "pmatests/Defs.h"

#include "pma/TypeDefs.h"

// Old clang compiler in combination with new libstdc++
#if defined(__clang__) && (__clang_major__ < 9) && defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 10)
    #define PMA_OLD_CLANG_NEW_LIBSTDCPP
#endif

TEST(PolyAllocIntegrationTest, InstantiateTypes) {
    pma::String<char> str;
    pma::Vector<int> vec;
    pma::Matrix<int> mat;
#ifndef PMA_OLD_CLANG_NEW_LIBSTDCPP
    pma::Set<int> set;
    pma::Map<int, int> map;
#endif  // PMA_OLD_CLANG_NEW_LIBSTDCPP
    pma::UnorderedSet<int> uset;
    pma::UnorderedMap<int, int> umap;
    ASSERT_TRUE(true);
}

TEST(PolyAllocIntegrationTest, StringKeysHashInUnorderedContainers) {
    pma::UnorderedMap<pma::String<char>, int> umap;
    umap[pma::String<char>{"key"}] = 1;
    ASSERT_EQ(umap.at(pma::String<char>{"key"}), 1);

    pma::UnorderedSet<pma::String<char>> uset;
    uset.insert(pma::String<char>{"key"});
    ASSERT_EQ(uset.count(pma::String<char>{"key"}), 1ul);

    const std::hash<pma::String<char>> hasher;
    ASSERT_EQ(hasher(pma::String<char>{"abc"}), hasher(pma::String<char>{"abc"}));
    ASSERT_NE(hasher(pma::String<char>{"abc"}), hasher(pma::String<char>{"abd"}));
}
