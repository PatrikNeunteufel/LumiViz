// ============================================================================
// BUFFER A (Pruefstand A1) - Zustandsmaschine + Audio-Zustand
// Pixel (0,0): x = phase, y = welt, z = haltezeit, w = cooldown
// Pixel (1,0): x = glatterBass, y = beatEnv, z = energieVorrat, w = glatteLautheit
// iChannel0: Buffer A (Selbstreferenz), iChannel1: Music
// ============================================================================

const float HALTEDAUER  = 24.0;  // Timer ist nur noch FALLBACK (greift bei Stille)
const float BLENDEDAUER = 4.0;
const float COOLDOWN    = 2.0;
const float MIN_HALTE   = 3.0;   // Mindest-Verweilzeit: Hysterese gegen Geflacker

float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel1, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec4 zust  = texelFetch(iChannel0, ivec2(0, 0), 0);
    vec4 audio = texelFetch(iChannel0, ivec2(1, 0), 0);
    if (iFrame == 0) { zust = vec4(0.0); audio = vec4(0.0); }

    float dt = clamp(iTimeDelta, 1.0 / 240.0, 1.0 / 24.0);

    // ---- AUDIO-ZUSTAND: das B3-Muster der Schablone, voll ausgefuehrt ------
    float bass  = bandLevel(0.00, 0.05);
    float vol   = bandLevel(0.00, 0.70);

    float glatt = mix(audio.x, bass, 0.10);            // Tiefpass ueber die Zeit
    // LumiViz-Anpassung (Regel aus Anhang B1, NICHT der Shadertoy-Text):
    // Die dB-FFT-Zeile des Standalone-Testsignals saettigt bei 1.0
    // (Sonde S67) - `bass` steht damit konstant auf 1.0, und der
    // adaptive Trigger `bass > glatt*1.35 + 0.02` kann nie feuern
    // (im Probe-Lauf blieb nur der Timer-Fallback). B1 nennt den
    // Ausweg: schlag aus dem App-Uniform `beat` speisen (0/1 je Frame,
    // BeatEstimator der App). Auf shadertoy.com gilt weiter der
    // Tutorial-Text (adaptiver Trigger auf der Music-Textur).
    float schlag = step(0.5, beat);
    float env   = max(audio.y * 0.90, schlag);         // zuendet hart, klingt weich aus

    float vorrat = audio.z + bass * dt * 0.4;          // laedt kontinuierlich ...
    float drop   = schlag * step(1.5, vorrat);         // ... Kick nach viel Ruhe = DROP
    vorrat *= 1.0 - schlag * 0.8;                      // jeder Schlag entlaedt

    float lautheit = mix(audio.w, vol, 0.05);          // sehr traege Lautheit

    // ---- WECHSEL-ZUSTAND: die Maschine aus Schritt 11, Trigger = Beat ------
    float phase = zust.x, welt = zust.y, halte = zust.z, cool = zust.w;

    if (phase <= 0.0) {
        halte += dt;
        // DER Trigger-Tausch: Beat schaltet den Welt-Wechsel (Timer als Fallback)
        bool wechsel = (schlag > 0.5 && halte > MIN_HALTE) || halte > HALTEDAUER;
        if (wechsel && cool <= 0.0) phase = 0.0001;
    } else {
        phase += dt / BLENDEDAUER;
        if (phase >= 1.0) {
            phase = 0.0; welt = 1.0 - welt;
            halte = 0.0; cool = COOLDOWN;
        }
    }
    cool = max(cool - dt, 0.0);

    // ---- beide Zustandspixel schreiben -------------------------------------
    if (ivec2(fragCoord) == ivec2(1, 0))
        fragColor = vec4(glatt, env, vorrat, lautheit);
    else
        fragColor = vec4(phase, welt, halte, cool);
}
