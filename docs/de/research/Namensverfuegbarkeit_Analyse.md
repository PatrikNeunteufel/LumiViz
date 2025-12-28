# Namensverfügbarkeits-Analyse für dein CMake Build-System

**CMakeCraft ist der stärkste Kandidat** — ohne existierende GitHub-Projekte, verfügbare Package-Registry-Namen und wahrscheinlich freie Domains. Zwei Namen — MASON und CLIMB — haben kritische Konflikte und sollten vermieden werden. Die restlichen drei (CMakeForge, Pulse/PulseCMake, JABS) haben moderate Herausforderungen.

---

## Zusammenfassung der Analyse

### Empfehlung: CMakeCraft 🏆

| Name | Risiko | Empfehlung |
|------|--------|------------|
| **CMakeCraft** | NIEDRIG-MITTEL | ✅ **Beste Wahl** |
| CMakeForge | MITTEL | ⚠️ Mit Vorsicht |
| JABS | MITTEL | ⚠️ SEO-Probleme |
| PulseCMake | MITTEL-HOCH | ❌ Nicht empfohlen |
| CLIMB | HOCH | ❌ Vermeiden |
| MASON | HOCH | ❌ Vermeiden |

### Warum CMakeCraft?

- ✅ **GitHub**: Kein Repository mit diesem Namen
- ✅ **Package-Registries**: npm, PyPI, crates.io alle frei
- ✅ **Domains**: cmakecraft.io/dev wahrscheinlich frei
- ✅ **Semantik**: Klar - "CMake-Builds handwerklich erstellen"
- ⚠️ **Einziges Bedenken**: Craft CMS existiert, aber verwendet "Craft" als Präfix, nicht Suffix

### Ausschlussgründe

- **MASON**: Mapbox/mason ist ein C++ Paketmanager mit CMake-Integration (256 Stars) - direkte Kollision!
- **CLIMB**: MRC CLIMB ist britische Computing-Infrastruktur + climb.dev ist belegt
- **Pulse**: PulseAudio (Linux Audio), Pulse Secure (VPN), LinkedIn Pulse - zu viele Assoziationen

---

**CMakeCraft** passt auch gut zur "handwerklichen Qualität" die du ursprünglich erwähnt hast! 🛠️

---

## Der klare Favorit: CMakeCraft

CMakeCraft hebt sich als risikoärmste Option in allen untersuchten Dimensionen ab. Kein GitHub-Repository verwendet diesen exakten Namen. Keine Pakete existieren auf npm, PyPI, crates.io, vcpkg oder Conan. Web-Suchen zeigen keine existierenden Software-Produkte mit diesem Namen, und Domains wie cmakecraft.io und cmakecraft.dev scheinen nicht registriert zu sein.

Die einzige Überlegung betrifft die Assoziationen des "-Craft"-Suffixes. Craft CMS (Pixel & Tonic) hält registrierte Marken, aber deren Richtlinien adressieren speziell Produkte die *mit* "Craft" *beginnen* — CMakeCraft verwendet es als Suffix. Die Minecraft/Warcraft Gaming-Assoziationen existieren, operieren aber in völlig anderen Markenklassen (Spiele vs. Entwickler-Tools). Die klare Einbindung von "CMake" im Namen signalisiert sofort den Zweck für Entwickler.

| Dimension | Status | Details |
|-----------|--------|---------|
| GitHub | ✅ Verfügbar | Keine Repositories gefunden |
| Package-Registries | ✅ Frei | Auf allen großen Registries verfügbar |
| Domains | ✅ Wahrscheinlich frei | Keine Web-Präsenz erkannt |
| Markenrechte | ⚠️ Geringe Bedenken | Craft CMS existiert, aber andere Kategorie |
| **Risikobewertung** | **NIEDRIG-MITTEL** | Bester Gesamtkandidat |

---

## Namen die erhebliche Vorsicht erfordern

### CMakeForge hat handhabbare aber reale Konflikte

Ein obskures cmakeforge-Projekt existiert auf einer privaten Gitea-Instanz (code.fueldner.net), das sich selbst als "Software-Paketmanager basierend auf cmake + git" beschreibt. Besorgniserregender ist **cforge** auf GitHub — ein TOML-basiertes CMake Build-Tool mit **174 Stars** und aktiver Entwicklung, das direkte Konkurrenz mit überlappender Funktionalität darstellt.

Das "Forge"-Suffix trägt schweres Gepäck bei Entwickler-Tools: Atlassian Forge (Cloud-App-Plattform), Electron Forge (**6.9k Stars**), SourceForge und Forgejo konkurrieren alle um Aufmerksamkeit. Eine neue FORGE-Marke wurde im Juli 2025 in Software-Kategorien angemeldet (Serial #99264555), was Unsicherheit hinzufügt.

Package-Registries und Domains scheinen frei zu sein, und keine registrierte "CMakeForge"-Marke existiert. Jedoch erzeugt die Kombination aus dem existierenden (wenn auch obskuren) cmakeforge-Projekt, dem ähnlichen cforge-Konkurrenten und Kitwares CMake-Marke Reibung.

**Risikobewertung: MITTEL** — Nutzbar mit sorgfältiger Positionierung und Kontaktaufnahme zu bestehenden Projekt-Maintainern.

### JABS hat Akronym-Sättigungsprobleme

"JSON-Architected Build System" als vollständiger Name ist komplett einzigartig — keine Suchergebnisse existieren für diese Phrase. Jedoch kollidiert "JABS" als Akronym mit mehreren aktiven Projekten:

- **hyajam/jabs**: "Just Another Blockchain Simulator" (51 Stars, IEEE 2023 Paper)
- **JYU-IBA/jabs**: Rutherford-Rückstreu-Spektrometrie Simulationstool
- **KumarLabJax/JABS-behavior-classifier**: "JAX Animal Behavior System"
- **matbme/JABS.nvim**: Neovim Buffer-Switcher

Regierungssysteme nutzen ebenfalls JABS (Joint Automated Booking System für US Marshals, Judicial Access Browser System für Washington State Courts). Post-COVID dominiert "jabs" Suchergebnisse als britischer Slang für Impfungen.

Der jabs-Paketname scheint auf npm und crates.io verfügbar zu sein, ist aber auf PyPI belegt (jabs-mimir vom Animal-Behavior-System). Kurze Domains wie jabs.io und jabs.com sind mit ziemlicher Sicherheit Premium oder registriert, da "jabs" ein gebräuchliches englisches Wort ist.

**Risikobewertung: MITTEL** — Kein Build-System-Konflikt existiert, aber SEO-Herausforderungen und Akronym-Sättigung reduzieren die Auffindbarkeit.

---

## Namen die komplett vermieden werden sollten

### MASON kollidiert direkt mit existierendem C++/CMake-Tooling

Dieser Name hat den schädlichsten Konflikt aller Kandidaten. **Mapbox's mason** (github.com/mapbox/mason) ist ein plattformübergreifender C++ Paketmanager mit direkter CMake-Integration via `mason.cmake`. Obwohl nicht mehr gewartet, hat es **256 Stars** und wird weiterhin in Dokumentation im gesamten Ökosystem referenziert. Dies stellt *exakte Namespace-Kollision* in deinem Zielbereich dar.

Über Mapbox hinaus dominiert **mason.nvim** die Entwickler-Tools-Landschaft mit **9.834+ Stars** als Neovim-Paketmanager für LSP-Server, Linter und Formatter. Die mason-org Organisation kontrolliert mason-registry.dev. Flutters/Darts mason CLI Template-Generator fügt eine weitere Konfliktebene hinzu.

Jede große Package-Registry ist blockiert: mason ist belegt auf npm (Static-File-Builder), PyPI (SQL-Query-Builder), crates.io (Solana-Helper-Bibliothek) und pub.dev (Dart-Template-Generator). Premium-Domains (mason.dev, mason.io) sind registriert, wobei mason.dev ein Entwickler-Portfolio hostet und mason-registry.dev das Neovim-Ökosystem bedient.

**Risikobewertung: HOCH** — Die existierende C++/CMake-Assoziation macht diesen Namen für dein Projekt unhaltbar.

### CLIMB kollidiert mit großer wissenschaftlicher Computing-Infrastruktur

**MRC CLIMB** (Cloud Infrastructure for Microbial Bioinformatics) operiert als gut finanzierte britische nationale Computing-Infrastruktur, etabliert 2014, mit einer aktiven GitHub-Organisation (CLIMB-TRE) die 15+ Repositories enthält. Dies platziert "CLIMB" direkt im Computing/Entwickler-Tools-Bereich den du anvisierst.

Unternehmenskonflikte verschärfen das Problem: **Climb Global Solutions (NASDAQ: CLMB)** ist ein börsennotiertes IT-Distributionsunternehmen das seit 1982 operiert mit $100M+ Umsatz. Crytek hält registrierte Marken für "The Climb" VR-Spiele. Amazon Technologies registrierte "THE CLIMB" für Videospiele (USPTO 87736228).

Alle Premium-Domains sind belegt. climb.dev hostet einen aktiven AI/ML Model-Hosting-Service. climb.io ist zum Verkauf geparkt. climb.ac.uk bedient MRC CLIMB. Paketkonflikte existieren auf npm und PyPI (beide verlassen aber registriert).

**Risikobewertung: HOCH** — Direkter Computing-Infrastruktur-Konflikt macht Differenzierung nahezu unmöglich.

### Pulse ist auf jeder Ebene überwältigt von Konflikten

"Pulse" scheitert auf praktisch jeder Dimension. **PulseAudio** ist der Standard-Linux-Soundserver der von Ubuntu, Fedora, Debian und den meisten Distributionen verwendet wird — Entwickler assoziieren "Pulse" sofort mit Audio, nicht mit Builds. **Pulse Secure** (jetzt Ivanti Secure Access) ist Enterprise-VPN-Software mit globaler Verbreitung. **LinkedIn Pulse** war eine $90 Millionen Akquisition die in Microsofts Plattform integriert wurde. GitHubs eingebautes Repository-Aktivitäts-Feature heißt buchstäblich "Pulse".

Package-Registries sind gesättigt: pulse ist belegt auf npm (Mozilla-Client), PyPI (WSGI-Middleware) und crates.io (Async-Wake-Signals-Bibliothek). Domains pulse.io (von Google 2015 akquiriert) und pulse.dev sind registriert.

**PulseCMake** reduziert das Risiko erheblich — keine Paketkonflikte existieren, und pulsecmake.*-Domains scheinen verfügbar. Jedoch bleibt die "Pulse"-Wurzel problematisch, mit potenziellen Markenrechts-Herausforderungen aus Ivantis Portfolio und anhaltender Assoziation mit PulseAudio im Entwickler-Bewusstsein.

| Variante | Risikobewertung | Hauptbedenken |
|----------|-----------------|---------------|
| Pulse | HOCH | Unbrauchbar — zu viele große Konflikte |
| PulseCMake | MITTEL-HOCH | Verbessert aber "Pulse"-Assoziation bleibt |

---

## Vergleichende Risikomatrix

| Name | GitHub | Markenrechte | Domains | Registries | Gesamt | Empfehlung |
|------|--------|--------------|---------|------------|--------|------------|
| **CMakeCraft** | ✅ Frei | ⚠️ Gering | ✅ Wahrsch. frei | ✅ Frei | **NIEDRIG-MITTEL** | **Beste Wahl** |
| CMakeForge | ⚠️ Obskurer Konflikt | ⚠️ Forge überstrapaziert | ✅ Wahrsch. frei | ✅ Frei | MITTEL | Mit Vorsicht fortfahren |
| JABS | ⚠️ Mehrere Projekte | ✅ Keine Software-TM | ⚠️ Premium-Wort | ⚠️ Teilweise | MITTEL | Alternativen erwägen |
| PulseCMake | ⚠️ Pulse-Assoziationen | ⚠️ Ivanti/Microsoft | ⚠️ Kurze Domains belegt | ✅ Frei | MITTEL-HOCH | Nicht empfohlen |
| CLIMB | ❌ MRC-Infrastruktur | ❌ Mehrere TMs | ❌ Alle belegt | ⚠️ Teilweise | **HOCH** | Vermeiden |
| MASON | ❌ Mapbox mason | ⚠️ Diverse | ❌ Alle belegt | ❌ Alle belegt | **HOCH** | Vermeiden |

---

## Fazit und strategische Empfehlungen

**CMakeCraft sollte deine primäre Wahl sein.** Es kombiniert Namespace-Verfügbarkeit über alle Plattformen mit klarer semantischer Bedeutung ("CMake-Builds handwerklich erstellen"). Das Craft-CMS-Markenrechtsbedenken ist minimal angesichts deiner Suffix-Positionierung und unterschiedlichen Software-Kategorie.

**Falls CMakeCraft nicht akzeptabel ist**, stellt CMakeForge eine vernünftige Alternative dar, wobei du den existierenden cmakeforge-Maintainer kontaktieren und dich klar vom cforge-Projekt differenzieren solltest. JABS bleibt gangbar wenn du dich verpflichtest, den vollständigen Namen "JSON-Architected Build System" prominent für SEO-Differenzierung zu verwenden.

**Vermeide MASON und CLIMB komplett** — beide haben Konflikte direkt im C++/CMake- oder Computing-Infrastruktur-Bereich die anhaltende Verwirrung und Auffindbarkeits-Probleme erzeugen würden.

---

## Nächste Schritte

Für welchen Namen auch immer du dich entscheidest:

1. **Domain-Verfügbarkeit prüfen** über Registrare und .io/.dev/.com-Varianten sichern
2. **Paketnamen präventiv registrieren** auf npm, PyPI und crates.io
3. **GitHub-Organisation/Repository erstellen** um den Namespace zu beanspruchen
4. **Kitware kontaktieren** bezüglich CMake-Markennutzungs-Richtlinien (optional aber empfohlen)
