// Heart Equation  -  y = |x|^(2/3) + 0.9*sin(k*x)*sqrt(3-x^2)
// k pendelt 0..100: aus der Sinus-Fuellung entsteht das Herz.
// Quelle der Formel: Threads-Post _mrqubit_; Umsetzung: LumiViz-Experiment.
// Laeuft unveraendert auf shadertoy.com und im LumiViz-Shadertoy-Node (keine iChannels).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TEMPO = 0.35;   // Pendel-Tempo von k
const float K_MAX = 100.0;  // obere k-Grenze
const float ZOOM  = 4.4;    // Welt-Hoehe in Einheiten
const float GLOW  = 0.014;  // Linien-Glow
const float VERBLASEN = 3.0; // Daempfung der Streifen AUSSERHALB des Herzens
                             // (0.0 = alter Look, groesser = schneller verblasst)
// ----------------------------------------------------------------------------

float kurve(float x, float k)
{
    float env = sqrt(max(3.0 - x*x, 0.0));       // Huellkurve (0 ausserhalb)
    return pow(abs(x), 2.0/3.0) + 0.9*sin(k*x)*env;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5*iResolution.xy)/iResolution.y * ZOOM;
    uv.y += 0.35;                                 // Herz ins Zentrum ruecken

    float k = 0.5*K_MAX*(1.0 - cos(iTime*TEMPO)); // 0..K_MAX, weiche Umkehr

    // Abstand zur Kurve, steigungsnormiert - sonst verschwinden steile Flanken
    float fx   = kurve(uv.x, k);
    float e    = 2.0*ZOOM/iResolution.y;
    float dfdx = (kurve(uv.x+e,k) - kurve(uv.x-e,k)) / (2.0*e);
    float d    = abs(uv.y - fx) / sqrt(1.0 + dfdx*dfdx);

    float imBereich = step(abs(uv.x), sqrt(3.0)); // Definitionsbereich

    // die Huellkurve selbst = der Herzrand
    float base  = pow(abs(uv.x), 2.0/3.0);
    float env   = 0.9*sqrt(max(3.0 - uv.x*uv.x, 0.0));
    float dRand = min(abs(uv.y-(base+env)), abs(uv.y-(base-env)));
    float innen = imBereich * step(base-env, uv.y) * step(uv.y, base+env);

    // Abstand ausserhalb des Herzrands: dort wird die Sinus-Spur weich verblasst
    float aussen = max(max(uv.y - (base+env), (base-env) - uv.y), 0.0);
    float blende = exp(-aussen * VERBLASEN);

    vec3 rosa = mix(vec3(1.0,0.15,0.35), vec3(1.0,0.55,0.75),
                    0.5 + 0.5*sin(uv.x*2.0 + iTime*0.7));
    vec3 col  = imBereich * GLOW/(d + 0.004) * rosa * blende // Sinus-Spur, aussen verblasst
              + imBereich * 0.004/(dRand + 0.006) * vec3(1.0,0.3,0.5)
              + innen * 0.03 * vec3(0.40,0.05,0.12);         // zarte Fuellung

    col = 1.0 - exp(-col);                                 // Tonemapping der Serie
    fragColor = vec4(pow(col, vec3(1.0/2.2)), 1.0);
}
