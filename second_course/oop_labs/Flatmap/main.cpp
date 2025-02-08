#include <gtest/gtest.h>
#include "FlatMapTest.h"
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    FlatMap<int, std::string> map;

    map[1] = "one";
    map[2] = "two";
    map[3] = "three";
    for (auto iter = map.begin(); iter != map.end(); ++iter)
    {
        std::cout << "Key: " <<iter->key << ", Value: " << iter->value << "\n";
    }

    return RUN_ALL_TESTS();
}
