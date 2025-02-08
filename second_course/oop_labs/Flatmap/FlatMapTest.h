#ifndef FLATMAPTESTS_H
#define FLATMAPTESTS_H

#include <gtest/gtest.h>
#include "FlatMap.h"
using namespace FlatMapSpace;

TEST(FlatMapTest, InsertAndFindSingleElement) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["first_name"] = "Ilia";
    EXPECT_EQ(map["first_name"], "Ilia");
}

TEST(FlatMapTest, FindNonExistentKey) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    EXPECT_EQ(map["unknown_key"], "");
}

TEST(FlatMapTest, InsertMultipleElements) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["first_name"] = "Ilia";
    map["last_name"] = "Burlakov";
    EXPECT_EQ(map["first_name"], "Ilia");
    EXPECT_EQ(map["last_name"], "Burlakov");
}

TEST(FlatMapTest, CheckContainsMethod) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["city"] = "Novosibirsk";
    EXPECT_TRUE(map.contains("city"));
    EXPECT_FALSE(map.contains("country"));
}

TEST(FlatMapTest, InsertWithOverwrite) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["key"] = "first";
    map["key"] = "second";
    EXPECT_EQ(map["key"], "second");
}

TEST(FlatMapTest, CheckSizeMethod) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["one"] = "1";
    map["two"] = "2";
    EXPECT_EQ(map.size(), 2);
}

TEST(FlatMapTest, SizeOfEmptyMap) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    EXPECT_EQ(map.size(), 0);
}

TEST(FlatMapTest, ClearMap) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["first"] = "1";
    map["second"] = "2";
    map.clear();
    EXPECT_EQ(map.size(), 0);
    EXPECT_FALSE(map.contains("first"));
    EXPECT_FALSE(map.contains("second"));
}

TEST(FlatMapTest, CheckResize) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    for (int i = 0; i < 100; ++i) {
        map["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    EXPECT_EQ(map.size(), 100);
    EXPECT_EQ(map["key99"], "value99");
}

TEST(FlatMapTest, EraseElement) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["first"] = "1";
    map["second"] = "2";
    size_t erased = map.erase("first");
    EXPECT_EQ(erased, 1);
    EXPECT_EQ(map.size(), 1);
    EXPECT_FALSE(map.contains("first"));
    EXPECT_TRUE(map.contains("second"));
}

TEST(FlatMapTest, EraseNonExistentElement) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["first"] = "1";
    size_t erased = map.erase("nonexistent");
    EXPECT_EQ(erased, 0);
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains("first"));
}

TEST(FlatMapTest, EraseAllElements) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["key1"] = "value1";
    map["key2"] = "value2";
    map.erase("key1");
    map.erase("key2");
    EXPECT_EQ(map.size(), 0);
}

TEST(FlatMapTest, CheckCopyConstructor) {
    FlatMapSpace::FlatMap<std::string, std::string> map1;
    map1["first"] = "1";
    map1["second"] = "2";
    
    FlatMapSpace::FlatMap<std::string, std::string> map2 = map1;

    EXPECT_EQ(map2["first"], "1");
    EXPECT_EQ(map2["second"], "2");

    map2["first"] = "3";
    EXPECT_EQ(map1["first"], "1");
    EXPECT_EQ(map2["first"], "3");
}

TEST(FlatMapTest, CheckAssignmentOperator) {
    FlatMapSpace::FlatMap<std::string, std::string> map1;
    map1["first"] = "1";
    map1["second"] = "2";
    
    FlatMapSpace::FlatMap<std::string, std::string> map2;
    map2 = map1;

    EXPECT_EQ(map2["first"], "1");
    EXPECT_EQ(map2["second"], "2");

    map2["first"] = "3";
    EXPECT_EQ(map1["first"], "1");
    EXPECT_EQ(map2["first"], "3");
}

TEST(FlatMapTest, AssignmentToSelf) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["key"] = "value";
    map = map; 
    EXPECT_EQ(map["key"], "value");
}

TEST(FlatMapTest, BinarySearchPerformance) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    for (int i = 0; i < 1000; ++i) {
        map["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    EXPECT_EQ(map["key500"], "value500");
    EXPECT_TRUE(map.contains("key999"));
}

TEST(FlatMapTest, InsertionOrderDoesNotMatter) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map["z_key"] = "last";
    map["a_key"] = "first";
    map["m_key"] = "middle";

    EXPECT_EQ(map["z_key"], "last");
    EXPECT_EQ(map["a_key"], "first");
    EXPECT_EQ(map["m_key"], "middle");
}

TEST(FlatMapTest, ClearDoesNotCauseMemoryLeaks) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    for (int i = 0; i < 100; ++i) {
        map["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    map.clear();
    EXPECT_EQ(map.size(), 0);
    for (int i = 0; i < 100; ++i) {
        EXPECT_FALSE(map.contains("key" + std::to_string(i)));
    }
}

TEST(FlatMapTest, HandlesLargeKeysAndValues) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    std::string largeKey(10000, 'k');
    std::string largeValue(10000, 'v'); 

    map[largeKey] = largeValue;
    EXPECT_EQ(map[largeKey], largeValue);
}

TEST(FlatMapTest, HandlesEmptyKeyAndValue) {
    FlatMapSpace::FlatMap<std::string, std::string> map;
    map[""] = "";
    EXPECT_TRUE(map.contains(""));
    EXPECT_EQ(map[""], "");
}



TEST(FlatMapTest, IntAndString) {
    FlatMapSpace::FlatMap<int, std::string> map;
    map[1] = "Ilia";
    EXPECT_EQ(map[1], "Ilia");
}
TEST(FlatMapTest, DoubleAndShort) {
    FlatMapSpace::FlatMap<double, short> map;
    map[5.55] = 999;
    EXPECT_EQ(map[5.55], 999);
}
TEST(FlatMapTest, CharAndUnsignedLongLong) {
    FlatMapSpace::FlatMap<char, unsigned long long> map;
    map['c'] = 18446744073709551614;
    EXPECT_TRUE(map.contains('c'));
    EXPECT_FALSE(map.contains(18446744073709551614));
    EXPECT_EQ(map['c'], 18446744073709551614);
}
TEST(FlatMapTest, EraseAllElementsIntAndDoule) {
    FlatMapSpace::FlatMap<int, double> map;
    map[2] = 3.14;
    map[3565656] = 6.650;
    map.erase(2);
    EXPECT_FALSE(map.contains(2));
    EXPECT_TRUE(map.contains(3565656));
    map.erase(3565656);
    EXPECT_EQ(map.size(), 0);
}
#endif
