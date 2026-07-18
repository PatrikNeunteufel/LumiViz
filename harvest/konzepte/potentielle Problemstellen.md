# EventBus & Settings – Verbesserungen & kleine Tretminen (Kurz-Memo)

## TL;DR

* **IDs bleiben Werttypen** (`size_t`/`uint64_t`), **nur Zähler** sind atomar.
* Dein aktuelles `std::atomic<long long> m_TypedNextId{1};` ist **ok**.
* Sauberer wird’s mit **eigenen Aliassen** und konsistenten Typen.

---

## Sinnvolle Verbesserungen (optional)

1. **Konsistente ID-Aliasse**

    * ```cpp
     using HandlerId = std::size_t;    // per-Event API
     using TokenId   = std::uint64_t;  // typed Topic API
     ```
    * Vermeidet Mischformen (`size_t` vs. `long long`) in Signaturen/Structs.

2. **Atomik nur für Zähler**

    * ```cpp
     std::atomic<HandlerId> m_NextId{1};        // wenn ohne Lock inkrementiert
     std::atomic<TokenId>   m_TypedNextId{1};   // bleibt atomar
     ```
    * Alternativ: **unter Lock** inkrementieren → Zähler können normale Typen sein.

3. **Header-Includes präzisieren**

    * In `EventBus.hpp` zusätzlich:

      ```cpp
      #include <algorithm>   // remove_if
      #include <type_traits> // decay_t
      #include <cstdint>     // uint64_t
      #include <atomic>      // atomic
      ```
    * GCC/Clang sind strenger als MSVC.

4. **Einheitliche Signaturen**

    * Typed-API auf `TokenId` umstellen:

      ```cpp
      template<class T, class Fn> TokenId subscribe(const std::string& topic, Fn&&);
      void unsubscribe(TokenId token);
      ```

5. **Snapshot beim Publish**

    * Du machst es schon: Handler-Kopien außerhalb des Locks aufrufen → reentranzfest.

6. **Speicher/Performance**

    * `reserve()` bei Vektoren/Maps, wenn erwartbare Größen bekannt sind.
    * `std::function` bleibt flexibel; bei Hotpaths ggf. Small-Function-Optimierung prüfen (später).

---

## Kleine Tretminen (häufige Stolpersteine)

1. **Atomics in Containern**

    * **Kein** `std::atomic<T>` als **ID-Typ** in `std::vector`/`std::pair` etc.
      → schwer kopierbar/vergleichbar, unnötig.

2. **Mischen von ID-Typen**

    * `long long` hier, `size_t` dort → implizite Konvertierungen, spätere Bugs.
      → lieber feste Aliasse (`HandlerId`, `TokenId`) **überall** nutzen.

3. **Speichersemantik**

    * Für Zähler reicht `memory_order_relaxed`. Keine falschen Erwartungen an „Synchronisation“ knüpfen – die Synchronisation macht der **Mutex** um die Container.

4. **Nebenläufiges Unsubscribe**

    * Beim `unsubscribe(token)` über **alle** Topics iterieren → O(n).
      → Später evtl. `unordered_map<TokenId, (topic,index)>` als Rückwärtsindex erwägen.

5. **ID-Überlauf**

    * Theoretisch nach 2⁶⁴−1 Inkrementen. Praktisch irrelevant – aber **Token > 0** prüfen (wie du es tust).

6. **Linux-Build-Details**

    * Linke Threads, wenn `std::thread` genutzt:

      ```cmake
      find_package(Threads REQUIRED)
      target_link_libraries(PlugInSystem PRIVATE Threads::Threads)
      ```
    * Fehlende Includes führen unter GCC/Clang schneller zu Fehlern als unter MSVC.

7. **Typ-Erasure & Cast**

    * Beim typed-Invoker immer `std::type_index` prüfen (machst du) → sonst UB.

8. **Exception Safety**

    * Callbacks können werfen. Option: `try/catch` um Handler-Aufruf + Logging, damit ein Handler nicht alle anderen „mitreißt“.

---

## Wann solltest du umstellen?

* Sobald dich **Mischtypen** in Signaturen nerven → `TokenId` einführen.
* Wenn du **Lock-Scope** vereinheitlichen willst → entweder *alle* IDs unter Lock generieren (keine Atomics nötig) **oder** beide Zähler atomar lassen und vor dem Containerzugriff locken (aktuelles Muster).

---

## Mini-Snippets (Drop-in)

**Aliasse & Zähler**

```cpp
using HandlerId = std::size_t;
using TokenId   = std::uint64_t;

std::atomic<HandlerId> m_NextId{1};      // falls ohne Lock ++
std::atomic<TokenId>   m_TypedNextId{1}; // bleibt wie gehabt
```

**Typed-API Signaturen**

```cpp
template <class T, class Fn>
TokenId subscribe(const std::string& topic, Fn&& callback);

void unsubscribe(TokenId token);
```

**Publish-try/catch (optional robust)**

```cpp
for (const auto& h : snapshot) {
    if (h.type == want) {
        try { h.invoker(&payload); }
        catch (...) { /* log & continue */ }
    }
}
```

---

**Fazit:**
Dein aktueller Stand ist **funktionsfähig**. Für saubere Portabilität/Lesbarkeit lohnt sich mittelfristig die **Trennung „ID als Wert“ vs. „Zähler als Atomik“** und konsistente **Aliasse**.
