/**
 * @file    test_servicecontainer.cpp
 * @brief   Self-contained unit tests for viz::core::ServiceContainer without external frameworks.
 *
 * The harness below provides CHECK/REQUIRE macros and minimal reporting, so this test
 * can run under CTest via add_test() without additional dependencies.
 */

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "common/Attributes.hpp" // uses VIZ_NODISCARD etc.
#include "core/servicecontainer/ServiceContainer.hpp"
#include "core/servicecontainer/ServiceErrors.hpp"
#include "core/servicecontainer/ServiceLifetime.hpp"
#include "core/servicecontainer/ServiceScope.hpp"

// ----------------------- Minimal test harness -------------------------------

namespace tinytest
{

    struct TestContext
    {
        int passed{0};
        int failed{0};
        bool current_failed{false};
        std::string current_name;
    };

    inline TestContext& ctx()
    {
        static TestContext c;
        return c;
    }

    struct TestCase
    {
        explicit TestCase(const char* name)
        {
            ctx().current_name = name;
            ctx().current_failed = false;
            std::cout << "[ RUN      ] " << name << "\n";
        }
        ~TestCase()
        {
            if (ctx().current_failed)
            {
                std::cout << "[  FAILED  ] " << ctx().current_name << "\n";
                ++ctx().failed;
            }
            else
            {
                std::cout << "[       OK ] " << ctx().current_name << "\n";
                ++ctx().passed;
            }
        }
    };

#define TT_STRINGIFY2(x) #x
#define TT_STRINGIFY(x) TT_STRINGIFY2(x)

#define REQUIRE(cond)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            tinytest::ctx().current_failed = true;                                                                     \
            std::cerr << "REQUIRE failed at " __FILE__ ":" TT_STRINGIFY(__LINE__) << " -> " #cond << "\n";             \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)

#define CHECK(cond)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            tinytest::ctx().current_failed = true;                                                                     \
            std::cerr << "CHECK failed at " __FILE__ ":" TT_STRINGIFY(__LINE__) << " -> " #cond << "\n";               \
        }                                                                                                              \
    } while (0)

    template<typename Func> bool expect_throw(Func&& f)
    {
        try
        {
            f();
        }
        catch (...)
        {
            return true;
        }
        return false;
    }

#define CHECK_THROW(stmt)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        bool threw = tinytest::expect_throw([&]() { (void)(stmt); });                                                  \
        if (!threw)                                                                                                    \
        {                                                                                                              \
            tinytest::ctx().current_failed = true;                                                                     \
            std::cerr << "CHECK_THROW failed at " __FILE__ ":" TT_STRINGIFY(__LINE__) << " -> " #stmt                  \
                      << " did not throw\n";                                                                           \
        }                                                                                                              \
    } while (0)

} // namespace tinytest

// ----------------------- Test doubles / fixtures ----------------------------

namespace fixtures
{

    struct ILogger
    {
        virtual ~ILogger() = default;
        virtual void log(const std::string&) = 0;
    };
    struct Logger : ILogger
    {
        std::vector<std::string> sink;
        void log(const std::string& s) override
        {
            sink.push_back(s);
        }
    };

    struct ISettings
    {
        virtual ~ISettings() = default;
        virtual int get(const std::string&) const = 0;
    };
    struct Settings : ISettings
    {
        int get(const std::string& key) const override
        {
            (void)key;
            return 42;
        }
    };

    struct EventBus
    {
        int counter{0};
        void ping()
        {
            ++counter;
        }
    };
    struct CommandBus
    {
        explicit CommandBus(std::shared_ptr<EventBus> eb) : eb_(std::move(eb)) {}
        void cmd()
        {
            eb_->ping();
        }
        std::shared_ptr<EventBus> eb_;
    };

    struct PanelManager
    {
        PanelManager(std::shared_ptr<EventBus> eb, std::shared_ptr<ILogger> lg) : eb_(std::move(eb)), lg_(std::move(lg))
        {
        }
        void ui()
        {
            lg_->log("draw");
            eb_->ping();
        }
        std::shared_ptr<EventBus> eb_;
        std::shared_ptr<ILogger> lg_;
    };

    struct ScratchBuffer
    {
        explicit ScratchBuffer(size_t n) : data(n) {}
        std::vector<float> data;
    };

    // For cycle test:
    struct A
    {
        explicit A(std::shared_ptr<struct B>) {}
    };
    struct B
    {
        explicit B(std::shared_ptr<A>) {}
    };

} // namespace fixtures

// Bring types into shorter scope
using namespace viz::core;
using namespace fixtures;

// ----------------------- Tests ---------------------------------------------

static void Test_SingletonIdentity()
{
    tinytest::TestCase _t{"Singleton identity"};

    ServiceContainer di;
    di.addSingletonFactory<EventBus>([](IServiceResolver&) { return std::make_shared<EventBus>(); }, "EventBus", true);

    auto a = di.get<EventBus>();
    auto b = di.get<EventBus>();
    REQUIRE(a);
    REQUIRE(b);
    CHECK(a == b);
}

static void Test_ScopedIsolation()
{
    tinytest::TestCase _t{"Scoped isolation"};

    ServiceContainer di;
    di.addSingletonFactory<EventBus>([](IServiceResolver&) { return std::make_shared<EventBus>(); }, "EventBus");
    di.addScoped<PanelManager>(
        [](IServiceResolver& r)
        { return std::make_shared<PanelManager>(r.get<EventBus>(), std::make_shared<Logger>()); }, "PanelManager");

    auto s1 = di.createScope();
    auto s2 = di.createScope();

    auto pm1a = s1.get<PanelManager>();
    auto pm1b = s1.get<PanelManager>();
    auto pm2 = s2.get<PanelManager>();

    REQUIRE(pm1a && pm1b && pm2);
    CHECK(pm1a == pm1b); // same scope -> identical
    CHECK(pm1a != pm2);  // different scopes -> different instances
}

static void Test_TransientFreshInstances()
{
    tinytest::TestCase _t{"Transient fresh instances"};

    ServiceContainer di;
    di.addTransient<ScratchBuffer>([](IServiceResolver&) { return std::make_shared<ScratchBuffer>(8); },
                                   "ScratchBuffer");

    auto a = di.get<ScratchBuffer>();
    auto b = di.get<ScratchBuffer>();
    REQUIRE(a && b);
    CHECK(a != b);
    CHECK(a->data.size() == 8);
    CHECK(b->data.size() == 8);
}

static void Test_FactoryDependencyInjection()
{
    tinytest::TestCase _t{"Factory dependency injection"};

    ServiceContainer di;
    di.addSingletonFactory<EventBus>([](IServiceResolver&) { return std::make_shared<EventBus>(); }, "EventBus", true);
    di.addSingletonFactory<CommandBus>([](IServiceResolver& r)
                                       { return std::make_shared<CommandBus>(r.get<EventBus>()); }, "CommandBus", true);

    auto bus = di.get<EventBus>();
    auto cmd = di.get<CommandBus>();
    REQUIRE(bus && cmd);

    int before = bus->counter;
    cmd->cmd();
    CHECK(bus->counter == before + 1);
}

static void Test_TryGetSemantics()
{
    tinytest::TestCase _t{"tryGet semantics"};

    ServiceContainer di;
    auto missing = di.tryGet<ILogger>();
    CHECK(missing == nullptr);

    di.addSingletonFactory<ILogger>([](IServiceResolver&) { return std::make_shared<Logger>(); }, "Logger");
    auto present = di.tryGet<ILogger>();
    REQUIRE(present != nullptr);
}

static void Test_CycleDetection()
{
    tinytest::TestCase _t{"Cycle detection"};

    ServiceContainer di;
    di.addSingletonFactory<A>([](IServiceResolver& r) { return std::make_shared<A>(r.get<B>()); }, "A");
    di.addSingletonFactory<B>([](IServiceResolver& r) { return std::make_shared<B>(r.get<A>()); }, "B");

    CHECK_THROW(di.get<A>()); // must throw ServiceCycleError (caught as ...)
}

static void Test_EagerSingletons()
{
    tinytest::TestCase _t{"Eager singletons"};

    ServiceContainer di;
    std::atomic<int> constructed{0};

    struct Eager
    {
        explicit Eager(std::atomic<int>& c)
        {
            ++c;
        }
    };

    di.addSingletonFactory<Eager>([&](IServiceResolver&) { return std::make_shared<Eager>(constructed); }, "Eager",
                                  /*eager*/ true);

    // Before buildSingletons: not constructed yet (lazy pending)
    CHECK(constructed.load() == 0);

    // Eager build triggers construction now
    di.buildSingletons();
    CHECK(constructed.load() == 1);

    // Subsequent get() does not construct again
    auto e = di.get<Eager>();
    CHECK(constructed.load() == 1);
}

static void Test_ConcurrentSingletonResolution()
{
    tinytest::TestCase _t{"Concurrent singleton resolution"};

    ServiceContainer di;
    std::atomic<int> constructed{0};

    struct Heavy
    {
        explicit Heavy(std::atomic<int>& c)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++c;
        }
    };

    di.addSingletonFactory<Heavy>([&](IServiceResolver&) { return std::make_shared<Heavy>(constructed); }, "Heavy");

    std::shared_ptr<Heavy> a, b, c;
    std::thread t1([&] { a = di.get<Heavy>(); });
    std::thread t2([&] { b = di.get<Heavy>(); });
    std::thread t3([&] { c = di.get<Heavy>(); });
    t1.join();
    t2.join();
    t3.join();

    REQUIRE(a && b && c);
    CHECK(a == b && b == c);        // all received the same instance
    CHECK(constructed.load() == 1); // constructed exactly once
}
// ----------------------- Additional tests ------------------------------------

// Remove & isRegistered semantics
static void Test_RemoveAndIsRegistered()
{
    tinytest::TestCase _t{"Remove & isRegistered"};

    viz::core::ServiceContainer di;
    CHECK(!di.isRegistered<int>());
    di.addSingletonFactory<int>([](auto&) { return std::make_shared<int>(1); }, "I");
    CHECK(di.isRegistered<int>());

    bool removed = di.remove<int>();
    CHECK(removed);
    CHECK(!di.isRegistered<int>());

    // Removing again should report false
    CHECK(di.remove<int>() == false);

    // Getting now should throw
    CHECK_THROW(di.get<int>());
}

// replaceWithSingleton should overwrite prior registration
static void Test_ReplaceSingleton()
{
    tinytest::TestCase _t{"Replace singleton"};

    viz::core::ServiceContainer di;
    di.addSingletonFactory<std::string>([](auto&) { return std::make_shared<std::string>("A"); }, "S");
    auto a = di.get<std::string>();
    REQUIRE(a && *a == "A");

    di.replaceWithSingleton<std::string>(std::make_shared<std::string>("B"), "S2");
    auto b = di.get<std::string>();
    REQUIRE(b && *b == "B");

    // Identity after replace: subsequent gets return the replaced instance
    auto c = di.get<std::string>();
    CHECK(b == c);
}

// buildSingletons: eager-only and idempotence
static void Test_BuildSingletons_Idempotent()
{
    tinytest::TestCase _t{"buildSingletons eager & idempotent"};

    viz::core::ServiceContainer di;
    std::atomic<int> constructed{0};

    struct Eager
    {
        explicit Eager(std::atomic<int>& c)
        {
            ++c;
        }
    };
    struct Lazy
    {
        explicit Lazy(std::atomic<int>& c)
        {
            ++c;
        }
    };

    di.addSingletonFactory<Eager>([&](auto&) { return std::make_shared<Eager>(constructed); }, "Eager", /*eager*/ true);
    di.addSingletonFactory<Lazy>([&](auto&) { return std::make_shared<Lazy>(constructed); }, "Lazy", /*eager*/ false);

    CHECK(constructed.load() == 0);
    di.buildSingletons();
    // Only Eager constructed
    CHECK(constructed.load() == 1);

    // Calling again must not construct again
    di.buildSingletons();
    CHECK(constructed.load() == 1);

    // Accessing Lazy later constructs exactly once
    (void)di.get<Lazy>();
    CHECK(constructed.load() == 2);
}

// Resolving a scoped type from ROOT should not cache (fresh each time),
// resolving from a SCOPE should cache within the scope.
static void Test_ScopedRootVsScopeCaching()
{
    tinytest::TestCase _t{"Scoped: root vs scope caching"};

    using namespace viz::core;

    struct Ctx
    {
        int id;
        explicit Ctx(int i) : id(i) {}
    };

    ServiceContainer di;
    std::atomic<int> seq{0};
    di.addScoped<Ctx>([&](auto&) { return std::make_shared<Ctx>(++seq); }, "Ctx");

    // Root resolution returns a new instance every call (no root cache for scoped)
    auto r1 = di.get<Ctx>();
    auto r2 = di.get<Ctx>();
    REQUIRE(r1 && r2);
    CHECK(r1 != r2);
    CHECK(r1->id != r2->id);

    // Scope caches within the same scope
    auto s = di.createScope();
    auto a = s.get<Ctx>();
    auto b = s.get<Ctx>();
    REQUIRE(a && b);
    CHECK(a == b);

    // New scope gets a different instance
    auto s2 = di.createScope();
    auto c = s2.get<Ctx>();
    CHECK(a != c);
}

// Transient thread-safety sanity: parallel resolves produce distinct objects
static void Test_TransientParallel()
{
    tinytest::TestCase _t{"Transient parallel resolves"};

    viz::core::ServiceContainer di;
    di.addTransient<int>([](auto&) { return std::make_shared<int>(42); }, "T");

    std::shared_ptr<int> a, b, c, d;
    std::thread t1([&] { a = di.get<int>(); });
    std::thread t2([&] { b = di.get<int>(); });
    std::thread t3([&] { c = di.get<int>(); });
    std::thread t4([&] { d = di.get<int>(); });
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    REQUIRE(a && b && c && d);
    CHECK(!(a == b && b == c && c == d)); // at least one is different (very likely all)
}

// Factory returns null -> must throw ServiceError
static void Test_FactoryReturnsNullThrows()
{
    tinytest::TestCase _t{"Factory returns null throws"};

    viz::core::ServiceContainer di;
    di.addTransient<int>([](auto&) -> std::shared_ptr<int> { return {}; }, "BadFactory");
    CHECK_THROW(di.get<int>());
}

// Unregistered type throws (diagnostic)
static void Test_UnregisteredThrows()
{
    tinytest::TestCase _t{"Unregistered throws"};

    viz::core::ServiceContainer di;
    CHECK_THROW(di.get<double>());
}

// Indirect cycle A->B->C->A should be detected
static void Test_IndirectCycle()
{
    tinytest::TestCase _t{"Indirect cycle detection (A->B->C->A)"};

    // Local forward declarations (OK inside a function)
    struct CycA;
    struct CycB;
    struct CycC;

    // Local types with explicit dependency constructors
    struct CycA
    {
        explicit CycA(std::shared_ptr<CycB>) {}
    };
    struct CycB
    {
        explicit CycB(std::shared_ptr<CycC>) {}
    };
    struct CycC
    {
        explicit CycC(std::shared_ptr<CycA>) {}
    };

    viz::core::ServiceContainer di;

    di.addSingletonFactory<CycA>([](viz::core::IServiceResolver& r) { return std::make_shared<CycA>(r.get<CycB>()); },
                                 "CycA");

    di.addSingletonFactory<CycB>([](viz::core::IServiceResolver& r) { return std::make_shared<CycB>(r.get<CycC>()); },
                                 "CycB");

    di.addSingletonFactory<CycC>([](viz::core::IServiceResolver& r) { return std::make_shared<CycC>(r.get<CycA>()); },
                                 "CycC");

    CHECK_THROW(di.get<CycA>()); // must detect cycle
}



// ServiceScope move semantics & scopedCount
static void Test_ScopeMoveAndCount()
{
    tinytest::TestCase _t{"Scope move & scopedCount"};

    using namespace viz::core;
    struct S
    {
        int x = 0;
    };

    ServiceContainer di;
    di.addScoped<S>([](auto&) { return std::make_shared<S>(); }, "S");

    auto scope1 = di.createScope();
    CHECK(scope1.scopedCount() == 0);
    auto s1 = scope1.get<S>();
    CHECK(scope1.scopedCount() == 1);

    // Move-construct scope2 from scope1
    auto scope2 = std::move(scope1);
    CHECK(scope2.scopedCount() == 1);

    // Move-assign to scope3
    auto scope3 = di.createScope();
    scope3 = std::move(scope2);
    CHECK(scope3.scopedCount() == 1);
}

// Replace after eager build must swap the instance seen by future gets
static void Test_ReplaceAfterEagerBuild()
{
    tinytest::TestCase _t{"Replace after eager build"};
    viz::core::ServiceContainer di;

    struct S
    {
        int v;
        explicit S(int x) : v(x) {}
    };

    di.addSingletonFactory<S>([](auto&) { return std::make_shared<S>(1); }, "S", /*eager*/ true);
    di.buildSingletons();

    auto a = di.get<S>();
    REQUIRE(a && a->v == 1);

    di.replaceWithSingleton<S>(std::make_shared<S>(2), "S2");
    auto b = di.get<S>();
    REQUIRE(b && b->v == 2);
    CHECK(a != b); // replaced identity
}

// Singleton factory throws once; second attempt should succeed
static void Test_SingletonFactoryThrowOnceThenSuccess()
{
    tinytest::TestCase _t{"Singleton factory throw once then success"};
    viz::core::ServiceContainer di;

    struct Flaky
    {
        int x{0};
    };

    std::atomic<int> attempts{0};
    di.addSingletonFactory<Flaky>(
        [&](auto&)
        {
            int n = ++attempts;
            if (n == 1)
            {
                throw std::runtime_error("first attempt fails");
            }
            return std::make_shared<Flaky>();
        },
        "Flaky");

    // First call throws
    CHECK_THROW(di.get<Flaky>());

    // Second call should succeed (call_once allows retry after exception)
    auto ok = di.get<Flaky>();
    REQUIRE(ok);
    CHECK(attempts.load() == 2);
}

// tryGet semantics on transient & scoped and unregistered types
static void Test_TryGetMoreSemantics()
{
    tinytest::TestCase _t{"tryGet more semantics"};

    using namespace viz::core;
    ServiceContainer di;

    // Unregistered
    CHECK(!di.tryGet<double>());

    // Transient
    di.addTransient<int>([](auto&) { return std::make_shared<int>(7); }, "T");
    auto t1 = di.tryGet<int>();
    auto t2 = di.tryGet<int>();
    REQUIRE(t1 && t2);
    CHECK(t1 != t2); // transient => fresh each time

    // Scoped
    di.addScoped<std::string>([](auto&) { return std::make_shared<std::string>("x"); }, "S");
    auto s1 = di.tryGet<std::string>(); // root: no scope cache, but still construct
    auto s2 = di.tryGet<std::string>();
    REQUIRE(s1 && s2);
    CHECK(s1 != s2);

    auto scope = di.createScope();
    auto a = scope.tryGet<std::string>();
    auto b = scope.tryGet<std::string>();
    REQUIRE(a && b);
    CHECK(a == b); // scoped cache inside scope
}

// Multi-threaded tryGet on singleton after init should always return same identity
static void Test_TryGetSingletonParallelAfterInit()
{
    tinytest::TestCase _t{"tryGet singleton parallel after init"};

    viz::core::ServiceContainer di;
    struct S
    {
    };

    di.addSingletonFactory<S>([](auto&) { return std::make_shared<S>(); }, "S");
    auto init = di.get<S>(); // initialize once

    std::shared_ptr<S> a, b, c, d;
    std::thread t1([&] { a = di.tryGet<S>(); });
    std::thread t2([&] { b = di.tryGet<S>(); });
    std::thread t3([&] { c = di.tryGet<S>(); });
    std::thread t4([&] { d = di.tryGet<S>(); });
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    REQUIRE(a && b && c && d);
    CHECK(a == init && b == init && c == init && d == init);
}

// Replacing a singleton while a scope exists must NOT affect scoped instances.
static void Test_ReplaceSingletonWhileScopeAlive()
{
    tinytest::TestCase _t{"Replace singleton while scope alive"};

    struct Cfg
    {
        int v;
        explicit Cfg(int x) : v(x) {}
    };
    struct Ctx
    {
        int v;
        explicit Ctx(std::shared_ptr<Cfg> c) : v(c->v) {}
    };

    viz::core::ServiceContainer di;
    di.addSingletonFactory<Cfg>([](auto&) { return std::make_shared<Cfg>(1); }, "Cfg");
    di.addScoped<Ctx>([](auto& r) { return std::make_shared<Ctx>(r.get<Cfg>()); }, "Ctx");

    auto s1 = di.createScope();
    auto a = s1.get<Ctx>();
    REQUIRE(a && a->v == 1);

    // Replace singleton
    di.replaceWithSingleton<Cfg>(std::make_shared<Cfg>(2), "Cfg2");

    // Existing scope keeps its cached scoped instance
    auto b = s1.get<Ctx>();
    CHECK(b == a);
    CHECK(b->v == 1);

    // New scope sees the new singleton
    auto s2 = di.createScope();
    auto c = s2.get<Ctx>();
    REQUIRE(c && c->v == 2);
}

// Removing a service prevents future resolves; existing shared_ptr remain valid.
static void Test_RemovePreventsFutureResolve()
{
    tinytest::TestCase _t{"Remove prevents future resolve"};

    struct S
    {
        int x{0};
    };

    viz::core::ServiceContainer di;
    di.addSingletonFactory<S>([](auto&) { return std::make_shared<S>(); }, "S");

    auto s = di.get<S>();
    REQUIRE(s);

    CHECK(di.remove<S>());    // first remove succeeds
    CHECK(!di.remove<S>());   // second remove reports false
    CHECK_THROW(di.get<S>()); // cannot resolve anymore

    // Existing pointer still valid
    CHECK(s.use_count() >= 1);
}

// Register Base -> return Derived; resolves as Base and preserves polymorphism.
static void Test_PolymorphicRegistration()
{
    tinytest::TestCase _t{"Polymorphic registration"};

    struct Base
    {
        virtual ~Base() noexcept = default;
        virtual int id() const
        {
            return 1;
        }
    };
    struct Derived : Base
    {
        int id() const override
        {
            return 7;
        }
    };

    viz::core::ServiceContainer di;
    di.addSingletonFactory<Base>([](auto&) { return std::make_shared<Derived>(); }, "Base->Derived");

    auto b = di.get<Base>();
    REQUIRE(b);
    CHECK(b->id() == 7);
}


// ----------------------- main -----------------------------------------------

int main()
{
    Test_SingletonIdentity();
    Test_ScopedIsolation();
    Test_TransientFreshInstances();
    Test_FactoryDependencyInjection();
    Test_TryGetSemantics();
    Test_CycleDetection();
    Test_EagerSingletons();
    Test_ConcurrentSingletonResolution();

    // NEW:
    Test_RemoveAndIsRegistered();
    Test_ReplaceSingleton();
    Test_BuildSingletons_Idempotent();
    Test_ScopedRootVsScopeCaching();
    Test_TransientParallel();
    Test_FactoryReturnsNullThrows();
    Test_UnregisteredThrows();
    Test_IndirectCycle();
    Test_ScopeMoveAndCount();

    Test_ReplaceAfterEagerBuild();
    Test_SingletonFactoryThrowOnceThenSuccess();
    Test_TryGetMoreSemantics();
    Test_TryGetSingletonParallelAfterInit();

    Test_ReplaceSingletonWhileScopeAlive();
    Test_RemovePreventsFutureResolve();
    Test_PolymorphicRegistration();


    auto& C = tinytest::ctx();
    std::cout << "\n[ SUMMARY ] passed=" << C.passed << " failed=" << C.failed << "\n";
    return (C.failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
