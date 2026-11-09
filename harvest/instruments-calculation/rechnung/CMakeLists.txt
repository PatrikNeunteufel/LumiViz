# Rechenkerne der Instrumentklassen (AUFTRAG_BAU.md §3) — reines C++20, ohne Qt, ohne windows.h.
#
# Feste Rechenregeln fuer ganz rechnung/ (FRAGEN_KONSOLIDIERT E1 a, Entscheid Patrik 2026-10-29): kein fast-math,
# keine automatische FMA (Kontraktion aus), feste Reihenfolge — derselbe Build liefert denselben Ton.
#
#   cmake -S rechnung -B rechnung/out -DCMAKE_BUILD_TYPE=Release
#   cmake --build rechnung/out -j
#   ctest --test-dir rechnung/out
cmake_minimum_required(VERSION 3.20)
project(InstrumentsCalculation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/W4 /utf-8 /fp:precise /fp:contract- /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -ffp-contract=off -fno-fast-math)
endif()

find_package(Threads REQUIRED)

# --- Bausteine aller Klassen ---
add_library(ic_gemeinsam STATIC
  gemeinsam/Kurve.cpp
  gemeinsam/Eigenzerlegung.cpp
  gemeinsam/Fft.cpp
  gemeinsam/Json.cpp
  gemeinsam/Werkzeug.cpp
  gemeinsam/Modal.cpp
  gemeinsam/Balken.cpp
  gemeinsam/Koerper.cpp
  gemeinsam/Rohr.cpp
  gemeinsam/Rohrschleife.cpp
  gemeinsam/Membran.cpp
  gemeinsam/Reibung.cpp
  # die Bausteine der Messkette haengen nur an gemeinsam/ — die Klassen nutzen sie (YIN beim Stimmen der Blaeser)
  messung/Bausteine.cpp
  messung/Samples.cpp
)
target_include_directories(ic_gemeinsam PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ic_gemeinsam SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/externals/pocketfft)
target_link_libraries(ic_gemeinsam PUBLIC Threads::Threads)

# --- Klassen ---
# alles ausser test_*; kein [A-Z] im Muster — unter Windows vergleicht GLOB kleingeschrieben und findet dann nichts
file(GLOB IC_KLASSEN_QUELLEN CONFIGURE_DEPENDS klassen/*/*.cpp)
list(FILTER IC_KLASSEN_QUELLEN EXCLUDE REGEX "/test_[^/]*$")
add_library(ic_klassen STATIC ${IC_KLASSEN_QUELLEN})
target_link_libraries(ic_klassen PUBLIC ic_gemeinsam)

# --- Messkette und Setz-Werkzeug ---
file(GLOB IC_MESSUNG_QUELLEN CONFIGURE_DEPENDS setzen/*.cpp)
list(FILTER IC_MESSUNG_QUELLEN EXCLUDE REGEX "/test_[^/]*$")
if(IC_MESSUNG_QUELLEN)
  add_library(ic_messung STATIC ${IC_MESSUNG_QUELLEN})
  target_link_libraries(ic_messung PUBLIC ic_klassen)
else()
  add_library(ic_messung INTERFACE)
  target_link_libraries(ic_messung INTERFACE ic_klassen)
endif()

# --- Tests (doctest) ---
enable_testing()
file(GLOB IC_TESTS CONFIGURE_DEPENDS gemeinsam/test_*.cpp klassen/*/test_*.cpp messung/test_*.cpp setzen/test_*.cpp)
add_executable(ic_tests werkzeuge/test_main.cpp ${IC_TESTS} werkzeuge/Sammlung.cpp werkzeuge/test_sammlung.cpp)
target_include_directories(ic_tests SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/externals/doctest)
target_link_libraries(ic_tests PRIVATE ic_messung)
target_compile_definitions(ic_tests PRIVATE IC_WERTE="${CMAKE_CURRENT_SOURCE_DIR}/werte")
add_test(NAME ic_tests COMMAND ic_tests)

# --- Werkzeuge: Hoerproben, Rechenzeit, Abdruecke ---
add_executable(ic_stimme_blech werkzeuge/stimme_blech.cpp)
target_link_libraries(ic_stimme_blech PRIVATE ic_messung)

add_executable(ic_stimme_rohrblatt werkzeuge/stimme_rohrblatt.cpp)
target_link_libraries(ic_stimme_rohrblatt PRIVATE ic_messung)

add_executable(ic_stimme_luftstrahl werkzeuge/stimme_luftstrahl.cpp)
target_link_libraries(ic_stimme_luftstrahl PRIVATE ic_messung)

add_executable(ic_stimme_bogen werkzeuge/stimme_bogen.cpp)
target_link_libraries(ic_stimme_bogen PRIVATE ic_messung)

add_executable(ic_stimme_zunge werkzeuge/stimme_zunge.cpp)
target_link_libraries(ic_stimme_zunge PRIVATE ic_messung)

add_executable(ic_stimme_stab werkzeuge/stimme_stab.cpp)
target_link_libraries(ic_stimme_stab PRIVATE ic_messung)

add_executable(ic_messe werkzeuge/messe.cpp)
target_link_libraries(ic_messe PRIVATE ic_messung)

add_executable(ic_glaette_ziel werkzeuge/glaette_ziel.cpp)
target_link_libraries(ic_glaette_ziel PRIVATE ic_messung)

add_executable(ic_hoerproben werkzeuge/hoerproben.cpp werkzeuge/Sammlung.cpp)
target_link_libraries(ic_hoerproben PRIVATE ic_klassen)
