/**
 ****************************************************************************************
 * @file   test_ServiceContainer.cpp
 * @brief  Unit-Tests für den ServiceContainer (DI)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @note   Portiert/adaptiert aus harvest/tests (NewViz2025, Catch2) auf die
 *         MyViz-API (registerSingleton/registerInstance/registerTransient/
 *         resolve/tryResolve/createTransient/has/unregister/clear).
 ****************************************************************************************
 */

#include <doctest.h>

#include "services/ServiceContainer.hpp"

#include <memory>
#include <stdexcept>
#include <string>

// =============================================================================
// Test-Services
// =============================================================================

namespace
{

class IGreeter
{
public:
    virtual ~IGreeter() = default;
    [[nodiscard]] virtual std::string greet() const = 0;
};

class GermanGreeter : public IGreeter
{
public:
    [[nodiscard]] std::string greet() const override { return "Hallo"; }
};

class EnglishGreeter : public IGreeter
{
public:
    [[nodiscard]] std::string greet() const override { return "Hello"; }
};

class ICounter
{
public:
    virtual ~ICounter() = default;
    virtual int next() = 0;
};

class Counter : public ICounter
{
public:
    int next() override { return ++m_value; }

private:
    int m_value = 0;
};

/// Service mit Abhängigkeit — für Factory-Tests
class IComposite
{
public:
    virtual ~IComposite() = default;
    [[nodiscard]] virtual std::string greetTwice() const = 0;
};

class Composite : public IComposite
{
public:
    explicit Composite(IGreeter& greeter) : m_greeter(greeter) {}
    [[nodiscard]] std::string greetTwice() const override
    {
        return m_greeter.greet() + " " + m_greeter.greet();
    }

private:
    IGreeter& m_greeter;
};

} // namespace

// =============================================================================
// Singleton
// =============================================================================

TEST_CASE("ServiceContainer: Singleton liefert bei jeder Aufloesung dieselbe Instanz")
{
    ServiceContainer c;
    c.registerSingleton<ICounter, Counter>();

    auto& first = c.resolve<ICounter>();
    auto& second = c.resolve<ICounter>();

    CHECK(&first == &second);
    CHECK(first.next() == 1);
    CHECK(second.next() == 2); // gleicher Zustand -> gleiche Instanz
}

TEST_CASE("ServiceContainer: Interface-zu-Implementierung-Mapping")
{
    ServiceContainer c;
    c.registerSingleton<IGreeter, GermanGreeter>();

    CHECK(c.resolve<IGreeter>().greet() == "Hallo");
}

TEST_CASE("ServiceContainer: Re-Registrierung ersetzt die Implementierung")
{
    ServiceContainer c;
    c.registerSingleton<IGreeter, GermanGreeter>();
    CHECK(c.resolve<IGreeter>().greet() == "Hallo");

    c.registerSingleton<IGreeter, EnglishGreeter>();
    CHECK(c.resolve<IGreeter>().greet() == "Hello"); // alte Instanz verworfen
}

// =============================================================================
// Factory & Instanz
// =============================================================================

TEST_CASE("ServiceContainer: Factory-Singleton mit Abhaengigkeits-Aufloesung")
{
    ServiceContainer c;
    c.registerSingleton<IGreeter, GermanGreeter>();
    c.registerSingleton<IComposite>([](ServiceContainer& container) {
        return std::make_unique<Composite>(container.resolve<IGreeter>());
    });

    CHECK(c.resolve<IComposite>().greetTwice() == "Hallo Hallo");
}

TEST_CASE("ServiceContainer: registerInstance behaelt die uebergebene Instanz")
{
    ServiceContainer c;
    auto greeter = std::make_unique<EnglishGreeter>();
    IGreeter* raw = greeter.get();

    c.registerInstance<IGreeter>(std::move(greeter));

    CHECK(&c.resolve<IGreeter>() == raw);
}

// =============================================================================
// Transient
// =============================================================================

TEST_CASE("ServiceContainer: createTransient erzeugt jedes Mal eine neue Instanz")
{
    ServiceContainer c;
    c.registerTransient<ICounter, Counter>();

    auto a = c.createTransient<ICounter>();
    auto b = c.createTransient<ICounter>();

    CHECK(a.get() != b.get());
    CHECK(a->next() == 1);
    CHECK(b->next() == 1); // unabhaengiger Zustand
}

TEST_CASE("ServiceContainer: createTransient auf Unregistriertes wirft"
          )
{
    ServiceContainer c;
    CHECK_THROWS_AS((void)c.createTransient<ICounter>(), std::runtime_error);
}

// BEKANNTER BUG (dokumentiert 2026-07-18, Fix fuer Phase 4 vorgemerkt):
// tryResolve()/resolve() auf einen TRANSIENT-Service gibt einen Pointer auf eine
// Instanz zurueck, die beim Verlassen von tryResolve() zerstoert wird (die lokale
// InstancePtr wird bei Transient nicht in m_instances gespeichert) -> Dangling
// Pointer / UB beim Zugriff. Fuer Transients ist ausschliesslich createTransient()
// sicher. Test ist geskippt, weil er UB ausfuehren wuerde.
TEST_CASE("ServiceContainer: resolve auf Transient (BEKANNTER BUG: dangling pointer)"
          * doctest::skip(true))
{
    ServiceContainer c;
    c.registerTransient<ICounter, Counter>();
    auto& dangling = c.resolve<ICounter>(); // Instanz stirbt schon in tryResolve()
    (void)dangling.next();                  // <- UB
}

// =============================================================================
// Query & Verwaltung
// =============================================================================

TEST_CASE("ServiceContainer: has/unregister/clear")
{
    ServiceContainer c;
    CHECK_FALSE(c.has<IGreeter>());

    c.registerSingleton<IGreeter, GermanGreeter>();
    CHECK(c.has<IGreeter>());

    c.unregister<IGreeter>();
    CHECK_FALSE(c.has<IGreeter>());
    CHECK(c.tryResolve<IGreeter>() == nullptr);

    c.registerSingleton<IGreeter, GermanGreeter>();
    c.registerSingleton<ICounter, Counter>();
    c.clear();
    CHECK_FALSE(c.has<IGreeter>());
    CHECK_FALSE(c.has<ICounter>());
}

TEST_CASE("ServiceContainer: resolve auf Unregistriertes wirft, tryResolve liefert nullptr")
{
    ServiceContainer c;
    CHECK_THROWS_AS((void)c.resolve<IGreeter>(), std::runtime_error);
    CHECK(c.tryResolve<IGreeter>() == nullptr);
}
