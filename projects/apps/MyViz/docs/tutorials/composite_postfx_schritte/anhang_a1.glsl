// ============================================================================
// Anhang A1 - die Audio-Infrastruktur (Common-Block des Tutorials) am
// Sichtpruefstand. Der Tutorial-Block definiert nur Globals + bandLevel +
// audioFuellen; das mainImage unten ist eine LumiViz-Zugabe (NICHT im
// Tutorial-Text), damit die Chain etwas zeigt: fuenf Balken fuer
// gBass/gMid/gTreb/gVol/gGate, darueber die rohe FFT-Zeile als Kurve
// (die im Standalone-Testsignal dB-gesaettigt am Deckel klebt - die
// Skalen-Falle aus dem Tutorial-Text, live).
// ============================================================================

// ---- AUDIO (Common) - iChannel3 = Music in jedem lesenden Pass! ------------
float gBass = 0.0, gMid = 0.0, gTreb = 0.0, gVol = 0.0, gGate = 0.0;

float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel3, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}

// einmal am Anfang von mainImage jedes lesenden Passes aufrufen
void audioFuellen()
{
    gBass = bandLevel(0.00, 0.05);
    gMid  = bandLevel(0.05, 0.25);
    gTreb = bandLevel(0.25, 0.70);
    gVol  = bandLevel(0.00, 0.70);
    gGate = smoothstep(0.60, 0.75, gBass);   // Beat-Gate (Skala: Handarbeit, s. Schablone)

    // LumiViz-Anpassung (die B-Regel der Serie, NICHT der Shadertoy-Text):
    // App-Uniforms statt FFT-Absolutpegel. Die dB-FFT-Zeile des Standalone-
    // Testsignals saettigt bei 1.0 (Sonde S67) - Absolut-Schwellen wie
    // smoothstep(0.60, 0.75, ...) koennen dort nie mehr schalten; `beat`
    // ersetzt das handkalibrierte Gate. Auf shadertoy.com gilt der
    // Tutorial-Text oben unveraendert.
    gBass = bass;
    gMid  = mid;
    gTreb = treb;
    gVol  = vol;
    gGate = beat;
}

// ---- LumiViz-Sichtpruefstand (NICHT im Tutorial-Text) ----------------------
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 st = fragCoord / iResolution.xy;

    audioFuellen();

    float wert = st.x < 0.2 ? gBass : st.x < 0.4 ? gMid : st.x < 0.6 ? gTreb
               : st.x < 0.8 ? gVol  : gGate;

    float f = fract(st.x * 5.0);
    float balken = step(st.y, wert) * step(0.03, f) * step(f, 0.97);

    // rohe FFT-Zeile als Referenzkurve (Zeile 0 der 512x2-Audio-Textur)
    float fft = texture(iChannel3, vec2(st.x, 0.25)).x;
    float kurve = smoothstep(0.008, 0.0, abs(st.y - fft));

    vec3 farbe = mix(vec3(0.20, 0.45, 1.00), vec3(1.00, 0.55, 0.15), st.x);
    fragColor = vec4(farbe * balken + vec3(0.9, 0.95, 1.0) * kurve * 0.5
                     + 0.02, 1.0);
}
