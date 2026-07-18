#include <catch2/catch_all.hpp>
#include <string>

#include "core/basetypes/BaseRegistry.hpp"

namespace
{
    using Key = std::string;
    struct Value
    {
        int a{};
        int b{};
        bool operator==(const Value& o) const noexcept
        {
            return a == o.a && b == o.b;
        }
    };
} // namespace

TEST_CASE("BaseRegistry: insert/get/contains", "[basetypes][registry]")
{
    viz::core::BaseRegistry<Key, Value> reg;

    REQUIRE(reg.empty());
    REQUIRE(reg.size() == 0);

    SECTION("insert new")
    {
        REQUIRE(reg.insert("k1", Value{1, 2}) == true);
        REQUIRE(reg.contains("k1"));
        auto p = reg.getPtr("k1");
        REQUIRE(p != nullptr);
        REQUIRE(p->a == 1);
        REQUIRE(p->b == 2);
        REQUIRE(reg.size() == 1);
    }

    SECTION("insert existing no-replace")
    {
        REQUIRE(reg.insert("k1", Value{1, 2}) == true);
        REQUIRE(reg.insert("k1", Value{9, 9}, /*replace=*/false) == false);
        auto v = reg.getCopy("k1");
        REQUIRE(v.has_value());
        REQUIRE(v->a == 1);
        REQUIRE(v->b == 2);
    }

    SECTION("insert existing with replace")
    {
        REQUIRE(reg.insert("k1", Value{1, 2}) == true);
        REQUIRE(reg.insert("k1", Value{7, 8}, /*replace=*/true) == false);
        auto v = reg.getCopy("k1");
        REQUIRE(v.has_value());
        REQUIRE(v->a == 7);
        REQUIRE(v->b == 8);
    }

    SECTION("emplace variants")
    {
        REQUIRE(reg.emplace("k1", /*replace=*/false, Value{3, 4}) == true);
        REQUIRE(reg.emplace("k1", /*replace=*/false, Value{9, 9}) == false);
        auto v1 = reg.getCopy("k1");
        REQUIRE(v1->a == 3);
        REQUIRE(v1->b == 4);

        REQUIRE(reg.emplace("k1", /*replace=*/true, Value{5, 6}) == false);
        auto v2 = reg.getCopy("k1");
        REQUIRE(v2->a == 5);
        REQUIRE(v2->b == 6);
    }

    SECTION("erase/clear")
    {
        REQUIRE(reg.insert("k1", Value{1, 2}));
        REQUIRE(reg.insert("k2", Value{3, 4}));
        REQUIRE(reg.size() == 2);

        REQUIRE(reg.erase("k2") == true);
        REQUIRE_FALSE(reg.contains("k2"));
        REQUIRE(reg.size() == 1);

        reg.clear();
        REQUIRE(reg.empty());
        REQUIRE(reg.size() == 0);
    }

    SECTION("forEach iteration")
    {
        REQUIRE(reg.insert("a", Value{1, 0}));
        REQUIRE(reg.insert("b", Value{2, 0}));
        int sum = 0;
        reg.forEach([&](const Key&, const Value& v) { sum += v.a; });
        REQUIRE(sum == 3);
    }
}
