/**
 ****************************************************************************************
 * @file   SuperscopeModule.cpp
 * @brief  Implementation of programmable point/line visualizer
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/SuperscopeModule.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace lumi::modules {

// =============================================================================
// Construction
// =============================================================================

SuperscopeModule::SuperscopeModule()
{
    // Initialize color gradient with Linear mode and a nice default preset
    m_colorGradient.setMode(GradientMode::Linear);
    m_colorGradient.loadPreset("Neon");

    // Initialize variables map with defaults
    resetState();
}

// =============================================================================
// Expression Code
// =============================================================================

void SuperscopeModule::setInitCode(const std::string& code)
{
    m_initCode = code;
    m_initExecuted = false;  // Re-execute init when code changes
    m_preset = SuperscopePreset::Custom;
}

void SuperscopeModule::setBeatCode(const std::string& code)
{
    m_beatCode = code;
    m_preset = SuperscopePreset::Custom;
}

void SuperscopeModule::setFrameCode(const std::string& code)
{
    m_frameCode = code;
    m_preset = SuperscopePreset::Custom;
}

void SuperscopeModule::setPointCode(const std::string& code)
{
    m_pointCode = code;
    m_preset = SuperscopePreset::Custom;
}

// =============================================================================
// Preset System
// =============================================================================

void SuperscopeModule::setPreset(SuperscopePreset preset)
{
    m_preset = preset;
    loadPresetCode(preset);
    m_initExecuted = false;
}

void SuperscopeModule::loadPresetCode(SuperscopePreset preset)
{
    // Phase 1: We use hardcoded functions, but store representative code
    // for display purposes
    
    // Default: All presets use Waveform unless explicitly set to Spectrum
    m_audioSource = SuperscopeAudioSource::Waveform;
    
    switch (preset)
    {
        case SuperscopePreset::HorizontalScope:
            m_initCode = "n=256";
            m_beatCode = "";
            m_frameCode = "";
            m_pointCode = "x=i*2-1; y=v*0.8";
            m_pointCount = 256;
            break;

        case SuperscopePreset::VerticalScope:
            m_initCode = "n=256";
            m_beatCode = "";
            m_frameCode = "";
            m_pointCode = "y=i*2-1; x=v*0.8";
            m_pointCount = 256;
            break;

        case SuperscopePreset::Circle:
            m_initCode = "n=200; t=0; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "t=t+0.02";
            m_pointCode = "r=0.5+v*0.3; x=sin(i*pi*2+t)*r; y=cos(i*pi*2+t)*r";
            m_pointCount = 200;
            break;

        case SuperscopePreset::Spiral:
            m_initCode = "n=300; t=0; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "t=t+0.015";
            m_pointCode = "r=0.1+i*0.7+v*0.15; a=i*pi*8+t; x=sin(a)*r; y=cos(a)*r";
            m_pointCount = 300;
            break;

        case SuperscopePreset::Lissajous:
            m_initCode = "n=400; t=0; pi=acos(-1); a=3; b=4";
            m_beatCode = "a=rand(5)+1; b=rand(5)+2";
            m_frameCode = "t=t+0.01";
            m_pointCode = "x=sin(a*i*pi*2+t)*0.8; y=cos(b*i*pi*2)*0.8+v*0.2";
            m_pointCount = 400;
            break;

        case SuperscopePreset::Flower:
            m_initCode = "n=360; t=0; pi=acos(-1); k=5";
            m_beatCode = "k=rand(6)+3";
            m_frameCode = "t=t+0.01";
            m_pointCode = "th=i*pi*2; r=(0.4+v*0.2)*cos(k*th+t); x=r*cos(th); y=r*sin(th)";
            m_pointCount = 360;
            break;

        case SuperscopePreset::Star:
            m_initCode = "n=100; t=0; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "t=t+0.02";
            m_pointCode = "th=i*pi*2+t; pts=5; ang=mod(i*pts,1); inner=abs(ang-0.5)*2; "
                          "r1=0.7+v*0.2; r2=0.3; r=r2+(r1-r2)*(1-inner); x=r*sin(th); y=r*cos(th)";
            m_pointCount = 100;
            break;

        case SuperscopePreset::Starburst:
            m_initCode = "n=100; t=0; pi=acos(-1); points=5";
            m_beatCode = "points=rand(4)+4";
            m_frameCode = "t=t+0.02";
            m_pointCode = "th=i*pi*2+t; r1=0.7+v*0.2; r2=0.3; "
                          "r=if(sin(th*points)>0,r1,r2); x=r*sin(th); y=r*cos(th)";
            m_pointCount = 100;
            break;

        case SuperscopePreset::Heart:
            m_initCode = "n=200; t=0; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "t=t+0.01";
            m_pointCode = "th=i*pi*2; sc=0.04+v*0.01; "
                          "x=sc*16*pow(sin(th),3); "
                          "y=sc*(13*cos(th)-5*cos(2*th)-2*cos(3*th)-cos(4*th))";
            m_pointCount = 200;
            break;

        case SuperscopePreset::DNA:
            m_initCode = "n=200; t=0; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "t=t+0.03";
            m_pointCode = "y=i*2-1; x1=sin(i*pi*6+t)*0.3; x2=sin(i*pi*6+t+pi)*0.3; "
                          "x=if(sin(i*10)>0,x1,x2)+v*0.1";
            m_pointCount = 200;
            break;

        case SuperscopePreset::SpectrumBars:
            m_initCode = "n=64";
            m_beatCode = "";
            m_frameCode = "";
            m_pointCode = "x=i*2-1; y=v*1.5-0.9";  // v is spectrum here
            m_pointCount = 64;
            m_audioSource = SuperscopeAudioSource::Spectrum;  // Override default
            break;

        case SuperscopePreset::CircularSpectrum:
            m_initCode = "n=128; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "";
            m_pointCode = "th=i*pi*2; r=0.3+v*0.5; x=sin(th)*r; y=cos(th)*r";
            m_pointCount = 128;
            m_audioSource = SuperscopeAudioSource::Spectrum;  // Override default
            break;

        case SuperscopePreset::Butterfly:
            m_initCode = "n=500; t=0; pi=acos(-1)";
            m_beatCode = "";
            m_frameCode = "t=t+0.005";
            m_pointCode = "th=i*pi*24; "
                          "r=exp(cos(th))-2*cos(4*th)+pow(sin(th/12),5); "
                          "sc=0.25+v*0.05; x=sin(th+t)*r*sc; y=cos(th+t)*r*sc";
            m_pointCount = 500;
            break;

        case SuperscopePreset::Hypocycloid:
            m_initCode = "n=400; t=0; pi=acos(-1); R=5; r=3";
            m_beatCode = "R=rand(5)+3; r=rand(3)+1";
            m_frameCode = "t=t+0.01";
            m_pointCode = "th=i*pi*2*r+t; sc=0.15+v*0.05; "
                          "x=sc*((R-r)*cos(th)+r*cos((R-r)/r*th)); "
                          "y=sc*((R-r)*sin(th)-r*sin((R-r)/r*th))";
            m_pointCount = 400;
            break;

        case SuperscopePreset::Custom:
        default:
            // Keep current code and audio source
            break;
    }
}

// =============================================================================
// Render Settings
// =============================================================================

void SuperscopeModule::setPointCount(int count)
{
    m_pointCount = std::clamp(count, 8, 4096);
    m_n = static_cast<double>(m_pointCount);
}

// =============================================================================
// Execution
// =============================================================================

void SuperscopeModule::executeInit()
{
    // Phase 1: Hardcoded initialization based on preset
    m_t = 0.0;
    m_variables["t"] = 0.0;
    m_variables["pi"] = m_pi;
    m_variables["pi2"] = m_pi2;

    // Preset-specific init
    switch (m_preset)
    {
        case SuperscopePreset::Lissajous:
            m_variables["a"] = 3.0;
            m_variables["b"] = 4.0;
            break;

        case SuperscopePreset::Flower:
            m_variables["k"] = 5.0;
            break;

        case SuperscopePreset::Starburst:
            m_variables["points"] = 5.0;
            break;

        case SuperscopePreset::Hypocycloid:
            m_variables["R"] = 5.0;
            m_variables["r"] = 3.0;
            break;

        case SuperscopePreset::DNA:
            m_variables["coils"] = 3.0;        // Current number of coils
            m_variables["targetCoils"] = 3.0;  // Target for smooth interpolation
            m_variables["avgAmp"] = 0.0;       // Running average amplitude
            break;

        default:
            break;
    }

    m_initExecuted = true;
}

void SuperscopeModule::executeBeat()
{
    // Phase 1: Hardcoded beat handling
    static std::random_device rd;
    static std::mt19937 gen(rd());

    switch (m_preset)
    {
        case SuperscopePreset::Lissajous:
        {
            std::uniform_int_distribution<> distA(1, 5);
            std::uniform_int_distribution<> distB(2, 6);
            m_variables["a"] = distA(gen);
            m_variables["b"] = distB(gen);
            break;
        }

        case SuperscopePreset::Flower:
        {
            std::uniform_int_distribution<> dist(3, 8);
            m_variables["k"] = dist(gen);
            break;
        }

        case SuperscopePreset::Starburst:
        {
            std::uniform_int_distribution<> dist(4, 7);
            m_variables["points"] = dist(gen);
            break;
        }

        case SuperscopePreset::Hypocycloid:
        {
            std::uniform_int_distribution<> distR(3, 7);
            std::uniform_int_distribution<> distr(1, 3);
            m_variables["R"] = distR(gen);
            m_variables["r"] = distr(gen);
            break;
        }

        case SuperscopePreset::DNA:
        {
            // On beat: set new target coils based on average amplitude
            // avgAmp is updated every frame, ranges 0-1
            double avgAmp = m_variables.count("avgAmp") ? m_variables["avgAmp"] : 0.5;
            
            // Map amplitude to coils: low amp = more coils (tighter), high amp = fewer coils (looser)
            // Range: 2-5 coils
            double newTarget = 5.0 - avgAmp * 3.0;  // High amp -> 2 coils, low amp -> 5 coils
            newTarget = std::clamp(newTarget, 2.0, 5.0);
            
            m_variables["targetCoils"] = newTarget;
            break;
        }

        default:
            break;
    }
}

std::vector<SuperscopePoint> SuperscopeModule::execute(
    const float* waveformL,
    const float* waveformR,
    const float* spectrumL,
    const float* spectrumR,
    int sampleCount,
    int width,
    int height,
    bool isBeat,
    float deltaTime)
{
    // Initialize if needed
    if (!m_initExecuted)
    {
        executeInit();
    }

    // Update time
    m_totalTime += deltaTime;
    m_t += deltaTime * 60.0;  // ~60 units per second for compatibility
    m_variables["t"] = m_t;

    // Update viewport
    m_w = static_cast<double>(width);
    m_h = static_cast<double>(height);

    // Beat handling
    m_b = isBeat ? 1.0 : 0.0;
    if (isBeat)
    {
        executeBeat();
    }

    // Generate points
    std::vector<SuperscopePoint> points;
    points.reserve(m_pointCount);

    for (int p = 0; p < m_pointCount; ++p)
    {
        float i = static_cast<float>(p) / static_cast<float>(m_pointCount - 1);
        float v = getAudioValue(i, waveformL, waveformR, spectrumL, spectrumR, sampleCount);

        SuperscopePoint pt = executePoint(i, v, isBeat);

        // Apply aspect correction (skip for scope-style presets that should fill width)
        if (m_aspectCorrection && height > 0)
        {
            // Don't apply aspect correction to horizontal/vertical scope
            // These should use full width/height
            bool isScope = (m_preset == SuperscopePreset::HorizontalScope ||
                           m_preset == SuperscopePreset::VerticalScope ||
                           m_preset == SuperscopePreset::SpectrumBars);
            if (!isScope)
            {
                float aspect = static_cast<float>(width) / static_cast<float>(height);
                pt.x /= aspect;
            }
        }

        // Apply stretch
        pt.x *= m_stretchX;
        pt.y *= m_stretchY;

        points.push_back(pt);
    }

    return points;
}

float SuperscopeModule::getAudioValue(float i,
                                       const float* waveformL,
                                       const float* waveformR,
                                       const float* spectrumL,
                                       const float* spectrumR,
                                       int sampleCount) const
{
    if (sampleCount <= 0) return 0.0f;

    const float* dataL = (m_audioSource == SuperscopeAudioSource::Waveform) ? waveformL : spectrumL;
    const float* dataR = (m_audioSource == SuperscopeAudioSource::Waveform) ? waveformR : spectrumR;

    if (!dataL) return 0.0f;

    // Get sample index
    int idx = static_cast<int>(i * (sampleCount - 1));
    idx = std::clamp(idx, 0, sampleCount - 1);

    float valueL = dataL[idx];
    float valueR = dataR ? dataR[idx] : valueL;

    // Mix based on channel selection
    switch (m_audioChannel)
    {
        case SuperscopeAudioChannel::Left:
            return valueL;

        case SuperscopeAudioChannel::Right:
            return valueR;

        case SuperscopeAudioChannel::Mono:
        case SuperscopeAudioChannel::Mid:
            return (valueL + valueR) * 0.5f;

        case SuperscopeAudioChannel::Side:
            return (valueL - valueR) * 0.5f;

        default:
            return valueL;
    }
}

SuperscopePoint SuperscopeModule::executePoint(float i, float v, bool isBeat)
{
    (void)isBeat;  // Reserved for future expression support
    
    // Store current state
    m_i = i;
    m_v = v;
    
    // Reset skip flag - presets must explicitly set it
    m_skip = 0.0;

    // Execute hardcoded preset
    executeHardcodedPreset(i, v);

    // Build output point
    SuperscopePoint pt;
    pt.x = static_cast<float>(m_x);
    pt.y = static_cast<float>(m_y);
    pt.skip = (m_skip > 0.5);

    // Get color from ColorGradientModule (handles Solid/Linear/Radial/Outline)
    Color4f color = m_colorGradient.sample(i);
    pt.r = color[0];
    pt.g = color[1];
    pt.b = color[2];
    pt.a = color[3];

    return pt;
}

void SuperscopeModule::executeHardcodedPreset(float i, float v)
{
    // Phase 1: Hardcoded implementations of each preset
    double t = m_t;

    switch (m_preset)
    {
        case SuperscopePreset::HorizontalScope:
            m_x = i * 2.0 - 1.0;
            m_y = v * 0.8;
            break;

        case SuperscopePreset::VerticalScope:
            m_y = i * 2.0 - 1.0;
            m_x = v * 0.8;
            break;

        case SuperscopePreset::Circle:
        {
            double r = 0.5 + v * 0.3;
            double angle = i * m_pi2 + t * 0.02;
            m_x = std::sin(angle) * r;
            m_y = std::cos(angle) * r;
            break;
        }

        case SuperscopePreset::Spiral:
        {
            double r = 0.1 + i * 0.7 + v * 0.15;
            double angle = i * m_pi * 8.0 + t * 0.015;
            m_x = std::sin(angle) * r;
            m_y = std::cos(angle) * r;
            break;
        }

        case SuperscopePreset::Lissajous:
        {
            double a = m_variables.count("a") ? m_variables.at("a") : 3.0;
            double b = m_variables.count("b") ? m_variables.at("b") : 4.0;
            m_x = std::sin(a * i * m_pi2 + t * 0.01) * 0.8;
            m_y = std::cos(b * i * m_pi2) * 0.8 + v * 0.2;
            break;
        }

        case SuperscopePreset::Flower:
        {
            double k = m_variables.count("k") ? m_variables.at("k") : 5.0;
            double th = i * m_pi2;
            double r = (0.4 + v * 0.2) * std::cos(k * th + t * 0.01);
            m_x = r * std::cos(th);
            m_y = r * std::sin(th);
            break;
        }

        case SuperscopePreset::Star:
        {
            // Classic 5-pointed star
            // Uses polar coordinates with alternating inner/outer radii
            double pts = 5.0;  // 5 points
            double th = i * m_pi2 + t * 0.02;
            double r1 = 0.7 + v * 0.2;  // Outer radius (tips)
            double r2 = 0.3;            // Inner radius (valleys)
            
            // Create a smooth star shape using angular interpolation
            // Each star point spans pi/pts radians
            double angleInStar = std::fmod(i * pts, 1.0);  // 0-1 within each point
            double innerAngle = std::abs(angleInStar - 0.5) * 2.0;  // 0-1-0 triangle wave
            double r = r2 + (r1 - r2) * (1.0 - innerAngle);  // Interpolate radii
            
            m_x = r * std::sin(th);
            m_y = r * std::cos(th);
            break;
        }

        case SuperscopePreset::Starburst:
        {
            // Radiating burst pattern (original "Star" implementation)
            double pts = m_variables.count("points") ? m_variables.at("points") : 5.0;
            double th = i * m_pi2 + t * 0.02;
            double r1 = 0.7 + v * 0.2;
            double r2 = 0.3;
            double totalVertices = pts * 2.0;
            double vertexPhase = i * totalVertices;
            int vertexIndex = static_cast<int>(std::floor(vertexPhase)) % 2;
            double r = (vertexIndex == 0) ? r1 : r2;
            m_x = r * std::sin(th);
            m_y = r * std::cos(th);
            break;
        }

        case SuperscopePreset::Heart:
        {
            double th = i * m_pi2;
            double sc = 0.04 + v * 0.01;
            double sinTh = std::sin(th);
            m_x = sc * 16.0 * sinTh * sinTh * sinTh;
            m_y = sc * (13.0 * std::cos(th) - 5.0 * std::cos(2.0 * th)
                        - 2.0 * std::cos(3.0 * th) - std::cos(4.0 * th));
            break;
        }

        case SuperscopePreset::DNA:
        {
            // DNA double helix with "rungs" going to center and back
            // Pattern: sin:y1, sin:y2, CENTER:y2, sin:y2(back!), sin:y3, sin:y4, sin:y5, CENTER:y5, sin:y5, ...
            // Key: after center, return to SAME Y position on sine
            
            // === Dynamic coil count based on audio ===
            // Update average amplitude (EMA)
            double currentAvgAmp = m_variables.count("avgAmp") ? m_variables["avgAmp"] : 0.0;
            currentAvgAmp = currentAvgAmp * 0.95 + std::abs(v) * 0.05;  // Slow EMA
            m_variables["avgAmp"] = currentAvgAmp;
            
            // Get target coils (set on beat)
            double targetCoils = m_variables.count("targetCoils") ? m_variables["targetCoils"] : 3.0;
            
            // Smoothly interpolate current coils toward target
            double currentCoils = m_variables.count("coils") ? m_variables["coils"] : 3.0;
            currentCoils = currentCoils * 0.98 + targetCoils * 0.02;  // Slow interpolation
            m_variables["coils"] = currentCoils;
            
            double numCoils = std::clamp(currentCoils, 2.0, 5.0);
            double phase = t * 0.05;  // Slow rotation
            double coilRadius = 0.4;
            
            int totalPoints = static_cast<int>(m_n);
            int pointIdx = static_cast<int>(i * totalPoints);
            int halfPoints = totalPoints / 2;
            
            // Cycle: sin,sin,sin,sin,center,back = 6 points
            // Y advances for first 4, stays same for center+back
            int cycleLength = 6;
            
            int localIdx;
            int strandPoints;
            double phaseOffset;
            
            if (pointIdx < halfPoints)
            {
                // Strand 1
                localIdx = pointIdx;
                strandPoints = halfPoints;
                phaseOffset = 0.0;
            }
            else if (pointIdx == halfPoints)
            {
                // Skip to break line
                m_skip = 1.0;
                break;
            }
            else
            {
                // Strand 2 (180° offset)
                localIdx = pointIdx - halfPoints - 1;
                strandPoints = totalPoints - halfPoints - 1;
                phaseOffset = m_pi;
            }
            
            int cycle = localIdx / cycleLength;
            int posInCycle = localIdx % cycleLength;
            
            // Logical Y step (doesn't advance during center/back)
            int yStep;
            if (posInCycle < 4) {
                yStep = cycle * 4 + posInCycle;
            } else {
                yStep = cycle * 4 + 3;  // Stay at same Y for center and back
            }
            
            // Total Y steps for this strand
            int totalYSteps = (strandPoints / cycleLength) * 4 + std::min(strandPoints % cycleLength, 4);
            
            double progress = static_cast<double>(yStep) / static_cast<double>(std::max(1, totalYSteps - 1));
            progress = std::min(1.0, progress);
            
            double coilAngle = progress * m_pi2 * numCoils + phase + phaseOffset;
            double audioMod = v * 0.15;
            double radius = coilRadius + audioMod;
            double sinX = std::sin(coilAngle) * radius;
            
            m_y = progress * 1.8 - 0.9;
            
            if (posInCycle == 4) {
                // Center point - go toward x=0
                double centerAmount = 0.4 + std::abs(v) * 0.5;
                m_x = sinX * (1.0 - centerAmount);
            } else {
                // On sine (including "back" position which is same as before center)
                m_x = sinX;
            }
            break;
        }

        case SuperscopePreset::SpectrumBars:
        {
            // Vertical spectrum bars with gaps between them
            // Each bar is: bottom → top → skip → next bar
            int numBars = static_cast<int>(m_n) / 3;  // 3 points per bar (bottom, top, skip)
            if (numBars < 1) numBars = 1;
            
            int barIndex = static_cast<int>(i * numBars);
            double barPhase = std::fmod(i * numBars, 1.0);  // 0-1 within each bar
            
            // X position: bars spread across width with gaps
            double barWidth = 1.8 / numBars;  // Leave some margin
            double barX = -0.9 + barIndex * barWidth + barWidth * 0.5;
            m_x = barX;
            
            // Get audio value for this bar
            double barValue = v;
            
            if (barPhase < 0.45)
            {
                // Bottom of bar
                m_y = -0.9;
            }
            else if (barPhase < 0.9)
            {
                // Top of bar (audio-reactive height)
                m_y = -0.9 + barValue * 1.7;  // Scale up for visibility
            }
            else
            {
                // Skip between bars
                m_skip = 1.0;
            }
            break;
        }

        case SuperscopePreset::CircularSpectrum:
        {
            double th = i * m_pi2;
            double r = 0.3 + v * 0.5;
            m_x = std::sin(th) * r;
            m_y = std::cos(th) * r;
            break;
        }

        case SuperscopePreset::Butterfly:
        {
            double th = i * m_pi * 24.0;
            double r = std::exp(std::cos(th)) - 2.0 * std::cos(4.0 * th)
                     + std::pow(std::sin(th / 12.0), 5.0);
            double sc = 0.25 + v * 0.05;
            m_x = std::sin(th + t * 0.005) * r * sc;
            m_y = std::cos(th + t * 0.005) * r * sc;
            break;
        }

        case SuperscopePreset::Hypocycloid:
        {
            double R = m_variables.count("R") ? m_variables.at("R") : 5.0;
            double r = m_variables.count("r") ? m_variables.at("r") : 3.0;
            double th = i * m_pi2 * r + t * 0.01;
            double sc = 0.15 + v * 0.05;
            m_x = sc * ((R - r) * std::cos(th) + r * std::cos((R - r) / r * th));
            m_y = sc * ((R - r) * std::sin(th) - r * std::sin((R - r) / r * th));
            break;
        }

        case SuperscopePreset::Custom:
        default:
            // Custom: Just do a basic scope for now
            m_x = i * 2.0 - 1.0;
            m_y = v * 0.8;
            break;
    }
}

// =============================================================================
// Variable Access
// =============================================================================

double SuperscopeModule::getVariable(const std::string& name) const
{
    auto it = m_variables.find(name);
    if (it != m_variables.end())
    {
        return it->second;
    }
    return 0.0;
}

void SuperscopeModule::setVariable(const std::string& name, double value)
{
    m_variables[name] = value;
}

void SuperscopeModule::resetState()
{
    m_variables.clear();
    m_variables["pi"] = m_pi;
    m_variables["pi2"] = m_pi2;
    m_variables["t"] = 0.0;
    m_t = 0.0;
    m_totalTime = 0.0f;
    m_initExecuted = false;
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> SuperscopeModule::paramDescs(const std::string& prefix) const
{
    std::vector<ModuleParamDesc> params;
    int order = 0;

    // Preset
    {
        ModuleParamDesc p;
        p.id = prefix + "preset";
        p.displayName = "Preset";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(SuperscopePreset::Spiral);
        p.enumOptions = {"Custom", "Horizontal Scope", "Vertical Scope", "Circle", "Spiral",
                         "Lissajous", "Flower", "Star", "Starburst", "Heart", "DNA",
                         "Spectrum Bars", "Circular Spectrum", "Butterfly", "Hypocycloid"};
        p.subGroup = "Preset";
        p.order = order++;
        params.push_back(p);
    }

    // Render settings
    {
        ModuleParamDesc p;
        p.id = prefix + "pointCount";
        p.displayName = "Points";
        p.type = ParamType::Int;
        p.minValue = 8;
        p.maxValue = 4096;
        p.defaultValue = m_pointCount;
        p.subGroup = "Render";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "renderMode";
        p.displayName = "Render Mode";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(m_renderMode);
        p.enumOptions = {"Dots", "Lines", "Thick Lines"};
        p.subGroup = "Render";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "lineWidth";
        p.displayName = "Line Width";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 20.0f;
        p.defaultValue = m_lineWidth;
        p.subGroup = "Render";
        p.order = order++;
        p.dependsOn = prefix + "renderMode";
        p.dependsValues = {1, 2};  // Lines=1, ThickLines=2
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "dotSize";
        p.displayName = "Dot Size";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 50.0f;
        p.defaultValue = m_dotSize;
        p.subGroup = "Render";
        p.order = order++;
        p.dependsOn = prefix + "renderMode";
        p.dependsValues = {0, 2};  // Dots=0, ThickLines=2
        params.push_back(p);
    }

    // Audio settings
    {
        ModuleParamDesc p;
        p.id = prefix + "audioSource";
        p.displayName = "Audio Source";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(m_audioSource);
        p.enumOptions = {"Waveform", "Spectrum"};
        p.subGroup = "Audio";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "audioChannel";
        p.displayName = "Audio Channel";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(m_audioChannel);
        p.enumOptions = {"Left", "Right", "Mono", "Mid", "Side"};
        p.subGroup = "Audio";
        p.order = order++;
        params.push_back(p);
    }

    // Color gradient parameters (delegate to ColorGradientModule)
    // Includes mode (Solid/Linear/Radial/Outline), solidColor, preset with preview, angle, etc.
    for (const auto& gradParam : m_colorGradient.paramDescs())
    {
        ModuleParamDesc p = gradParam;
        p.id = prefix + "color." + gradParam.id;
        p.subGroup = "Color";
        p.order = order++;
        
        // Prefix the dependsOn reference if set (for conditional visibility)
        if (!p.dependsOn.empty())
        {
            p.dependsOn = prefix + "color." + p.dependsOn;
        }
        
        params.push_back(p);
    }

    // Glow settings
    {
        ModuleParamDesc p;
        p.id = prefix + "glowEnabled";
        p.displayName = "Glow";
        p.type = ParamType::Bool;
        p.defaultValue = m_glowEnabled;
        p.subGroup = "Glow";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "glowIntensity";
        p.displayName = "Glow Intensity";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 2.0f;
        p.defaultValue = m_glowIntensity;
        p.subGroup = "Glow";
        p.order = order++;
        p.dependsOn = prefix + "glowEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "glowSize";
        p.displayName = "Glow Size";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 10.0f;
        p.defaultValue = m_glowSize;
        p.subGroup = "Glow";
        p.order = order++;
        p.dependsOn = prefix + "glowEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Hold/Fade settings
    {
        ModuleParamDesc p;
        p.id = prefix + "holdEnabled";
        p.displayName = "Hold/Fade";
        p.tooltip = "Keep previous frames visible with fade effect";
        p.type = ParamType::Bool;
        p.defaultValue = m_holdEnabled;
        p.subGroup = "Hold/Fade";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "fadeTime";
        p.displayName = "Fade Time";
        p.tooltip = "Time in seconds for frames to fade out";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 10.0f;
        p.defaultValue = m_fadeTime;
        p.subGroup = "Hold/Fade";
        p.order = order++;
        p.unit = "s";
        p.dependsOn = prefix + "holdEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "maxHoldFrames";
        p.displayName = "Max Frames";
        p.tooltip = "Maximum number of frames to hold";
        p.type = ParamType::Int;
        p.minValue = 1;
        p.maxValue = 60;
        p.defaultValue = m_maxHoldFrames;
        p.subGroup = "Hold/Fade";
        p.order = order++;
        p.dependsOn = prefix + "holdEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Blend mode
    {
        ModuleParamDesc p;
        p.id = prefix + "blendMode";
        p.displayName = "Blend Mode";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(m_blendMode);
        p.enumOptions = {"Replace", "Additive", "Alpha"};
        p.subGroup = "Render";
        p.order = order++;
        params.push_back(p);
    }

    // Display settings
    {
        ModuleParamDesc p;
        p.id = prefix + "aspectCorrection";
        p.displayName = "Aspect Correction";
        p.type = ParamType::Bool;
        p.defaultValue = m_aspectCorrection;
        p.subGroup = "Display";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "stretchX";
        p.displayName = "Stretch X";
        p.tooltip = "Horizontal scale factor";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 4.0f;
        p.defaultValue = m_stretchX;
        p.subGroup = "Display";
        p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = prefix + "stretchY";
        p.displayName = "Stretch Y";
        p.tooltip = "Vertical scale factor";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 4.0f;
        p.defaultValue = m_stretchY;
        p.subGroup = "Display";
        p.order = order++;
        params.push_back(p);
    }

    return params;
}

bool SuperscopeModule::getParam(const std::string& id, ParamValue& out) const
{
    // Check for color gradient params first
    if (id.find("color.") != std::string::npos)
    {
        std::string gradientId = id.substr(id.find("color.") + 6);
        return m_colorGradient.getParam(gradientId, out);
    }

    if (id == "preset") { out = static_cast<int>(m_preset); return true; }
    if (id == "pointCount") { out = m_pointCount; return true; }
    if (id == "renderMode") { out = static_cast<int>(m_renderMode); return true; }
    if (id == "lineWidth") { out = m_lineWidth; return true; }
    if (id == "dotSize") { out = m_dotSize; return true; }
    if (id == "audioSource") { out = static_cast<int>(m_audioSource); return true; }
    if (id == "audioChannel") { out = static_cast<int>(m_audioChannel); return true; }
    if (id == "glowEnabled") { out = m_glowEnabled; return true; }
    if (id == "glowIntensity") { out = m_glowIntensity; return true; }
    if (id == "glowSize") { out = m_glowSize; return true; }
    if (id == "holdEnabled") { out = m_holdEnabled; return true; }
    if (id == "fadeTime") { out = m_fadeTime; return true; }
    if (id == "maxHoldFrames") { out = m_maxHoldFrames; return true; }
    if (id == "blendMode") { out = static_cast<int>(m_blendMode); return true; }
    if (id == "aspectCorrection") { out = m_aspectCorrection; return true; }
    if (id == "stretchX") { out = m_stretchX; return true; }
    if (id == "stretchY") { out = m_stretchY; return true; }

    return false;
}

bool SuperscopeModule::setParam(const std::string& id, const ParamValue& value)
{
    // Check for color gradient params first
    if (id.find("color.") != std::string::npos)
    {
        std::string gradientId = id.substr(id.find("color.") + 6);
        return m_colorGradient.setParam(gradientId, value);
    }

    // Helper for int/float conversion
    auto getInt = [&value]() -> int {
        if (auto* v = std::get_if<int>(&value)) return *v;
        if (auto* v = std::get_if<float>(&value)) return static_cast<int>(*v);
        return 0;
    };

    auto getFloat = [&value]() -> float {
        if (auto* v = std::get_if<float>(&value)) return *v;
        if (auto* v = std::get_if<int>(&value)) return static_cast<float>(*v);
        return 0.0f;
    };

    if (id == "preset")
    {
        setPreset(static_cast<SuperscopePreset>(getInt()));
        return true;
    }
    if (id == "pointCount")
    {
        setPointCount(getInt());
        return true;
    }
    if (id == "renderMode")
    {
        m_renderMode = static_cast<SuperscopeRenderMode>(getInt());
        return true;
    }
    if (id == "lineWidth")
    {
        setLineWidth(getFloat());
        return true;
    }
    if (id == "dotSize")
    {
        setDotSize(getFloat());
        return true;
    }
    if (id == "audioSource")
    {
        m_audioSource = static_cast<SuperscopeAudioSource>(getInt());
        return true;
    }
    if (id == "audioChannel")
    {
        m_audioChannel = static_cast<SuperscopeAudioChannel>(getInt());
        return true;
    }
    if (id == "glowEnabled")
    {
        if (auto* v = std::get_if<bool>(&value)) { m_glowEnabled = *v; return true; }
        return false;
    }
    if (id == "glowIntensity")
    {
        setGlowIntensity(getFloat());
        return true;
    }
    if (id == "glowSize")
    {
        setGlowSize(getFloat());
        return true;
    }
    if (id == "holdEnabled")
    {
        if (auto* v = std::get_if<bool>(&value)) { m_holdEnabled = *v; return true; }
        return false;
    }
    if (id == "fadeTime")
    {
        setFadeTime(getFloat());
        return true;
    }
    if (id == "maxHoldFrames")
    {
        setMaxHoldFrames(getInt());
        return true;
    }
    if (id == "blendMode")
    {
        m_blendMode = static_cast<SuperscopeBlendMode>(getInt());
        return true;
    }
    if (id == "aspectCorrection")
    {
        if (auto* v = std::get_if<bool>(&value)) { m_aspectCorrection = *v; return true; }
        return false;
    }
    if (id == "stretchX")
    {
        setStretchX(getFloat());
        return true;
    }
    if (id == "stretchY")
    {
        setStretchY(getFloat());
        return true;
    }

    return false;
}

// =============================================================================
// Utility
// =============================================================================

const char* SuperscopeModule::renderModeName(SuperscopeRenderMode mode)
{
    switch (mode)
    {
        case SuperscopeRenderMode::Dots:       return "Dots";
        case SuperscopeRenderMode::Lines:      return "Lines";
        case SuperscopeRenderMode::ThickLines: return "Thick Lines";
        default:                               return "Unknown";
    }
}

const char* SuperscopeModule::audioSourceName(SuperscopeAudioSource source)
{
    switch (source)
    {
        case SuperscopeAudioSource::Waveform: return "Waveform";
        case SuperscopeAudioSource::Spectrum: return "Spectrum";
        default:                              return "Unknown";
    }
}

const char* SuperscopeModule::audioChannelName(SuperscopeAudioChannel channel)
{
    switch (channel)
    {
        case SuperscopeAudioChannel::Left:  return "Left";
        case SuperscopeAudioChannel::Right: return "Right";
        case SuperscopeAudioChannel::Mono:  return "Mono";
        case SuperscopeAudioChannel::Mid:   return "Mid";
        case SuperscopeAudioChannel::Side:  return "Side";
        default:                            return "Unknown";
    }
}

const char* SuperscopeModule::blendModeName(SuperscopeBlendMode mode)
{
    switch (mode)
    {
        case SuperscopeBlendMode::Replace:  return "Replace";
        case SuperscopeBlendMode::Additive: return "Additive";
        case SuperscopeBlendMode::Alpha:    return "Alpha";
        default:                            return "Unknown";
    }
}

const char* SuperscopeModule::presetName(SuperscopePreset preset)
{
    switch (preset)
    {
        case SuperscopePreset::Custom:           return "Custom";
        case SuperscopePreset::HorizontalScope:  return "Horizontal Scope";
        case SuperscopePreset::VerticalScope:    return "Vertical Scope";
        case SuperscopePreset::Circle:           return "Circle";
        case SuperscopePreset::Spiral:           return "Spiral";
        case SuperscopePreset::Lissajous:        return "Lissajous";
        case SuperscopePreset::Flower:           return "Flower";
        case SuperscopePreset::Star:             return "Star";
        case SuperscopePreset::Heart:            return "Heart";
        case SuperscopePreset::DNA:              return "DNA";
        case SuperscopePreset::SpectrumBars:     return "Spectrum Bars";
        case SuperscopePreset::CircularSpectrum: return "Circular Spectrum";
        case SuperscopePreset::Butterfly:        return "Butterfly";
        case SuperscopePreset::Hypocycloid:      return "Hypocycloid";
        default:                                 return "Unknown";
    }
}

} // namespace lumi::modules
