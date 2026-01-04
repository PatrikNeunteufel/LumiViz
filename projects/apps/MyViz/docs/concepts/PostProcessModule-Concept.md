# PostProcessModule Concept

**Version:** 0.1.0  
**Status:** Draft  
**Date:** January 2026

---

## 1. Overview

### 1.1 Purpose

The PostProcessModule provides a unified post-processing pipeline for all visualizers. It operates on rendered frames (via framebuffers) to apply visual effects that are independent of the specific visualizer implementation.

### 1.2 Position in Pipeline

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Visual Pipeline                                  │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│   Audio Input                                                            │
│       │                                                                  │
│       ▼                                                                  │
│   ┌────────────────────┐                                                │
│   │ AudioSourceModule  │  ─── Gain, Smoothing, Beat Detection           │
│   └────────────────────┘                                                │
│       │                                                                  │
│       ▼                                                                  │
│   ┌────────────────────┐                                                │
│   │ VisualizerModule   │  ─── Superscope, Waveform, Pulsing, etc.       │
│   │                    │      Renders to FBO instead of screen          │
│   └────────────────────┘                                                │
│       │                                                                  │
│       ▼                                                                  │
│   ┌────────────────────┐                                                │
│   │ PostProcessModule  │  ─── Blur, Glow, Mirror, Kaleidoscope, etc.    │
│   │                    │      Reads FBO, applies effects, outputs       │
│   └────────────────────┘                                                │
│       │                                                                  │
│       ▼                                                                  │
│   Screen Output                                                          │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### 1.3 Key Benefits

- **Universally applicable** - Same effects work for all visualizers
- **Stackable effects** - Multiple effects can be chained
- **Performance optimized** - Shared FBO management, efficient shader switching
- **Modular** - Each effect is a separate class, easy to add new ones

---

## 2. Architecture

### 2.1 Class Hierarchy

```
IPostEffect (Interface)
    │
    ├── BlurEffect
    │       ├── GaussianBlur
    │       └── KawaseBlur (faster)
    │
    ├── GlowEffect
    │       └── Uses BlurEffect internally + additive blend
    │
    ├── TransformEffect
    │       ├── MirrorEffect
    │       ├── KaleidoscopeEffect
    │       └── RotateEffect
    │
    ├── ColorEffect
    │       ├── ColorGrading
    │       ├── ChromaticAberration
    │       └── VignetteEffect
    │
    ├── DistortionEffect
    │       ├── WaveDistortion
    │       ├── FisheyeEffect
    │       └── BarrelDistortion
    │
    └── CompositeEffect
            ├── FeedbackEffect (trails/persistence)
            └── BlendFramesEffect

PostProcessModule
    │
    ├── manages IPostEffect chain
    ├── owns FBO resources
    └── provides paramDescs/getParam/setParam
```

### 2.2 Core Interfaces

```cpp
/**
 * @brief Single post-processing effect
 */
class IPostEffect
{
public:
    virtual ~IPostEffect() = default;
    
    /// Effect identifier
    virtual const char* effectId() const = 0;
    
    /// Human-readable name
    virtual const char* effectName() const = 0;
    
    /// Initialize OpenGL resources
    virtual bool initialize() = 0;
    
    /// Apply effect: read from inputFBO, write to outputFBO
    /// @param inputTexture  Source texture to process
    /// @param outputFBO     Target framebuffer (0 = screen)
    /// @param width         Viewport width
    /// @param height        Viewport height
    /// @param deltaTime     Time since last frame
    virtual void apply(GLuint inputTexture, 
                       GLuint outputFBO,
                       int width, int height,
                       float deltaTime) = 0;
    
    /// Release OpenGL resources
    virtual void cleanup() = 0;
    
    /// Enable/disable this effect
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
    
    /// Parameter interface
    virtual std::vector<ModuleParamDesc> paramDescs() const = 0;
    virtual bool getParam(const std::string& id, ParamValue& out) const = 0;
    virtual bool setParam(const std::string& id, const ParamValue& value) = 0;
};
```

```cpp
/**
 * @brief Post-processing pipeline manager
 */
class PostProcessModule
{
public:
    PostProcessModule();
    ~PostProcessModule();
    
    // =========================================================================
    // Pipeline Management
    // =========================================================================
    
    /// Initialize with viewport size
    bool initialize(int width, int height);
    
    /// Resize framebuffers
    void resize(int width, int height);
    
    /// Get the FBO that visualizers should render to
    GLuint inputFBO() const { return m_inputFBO; }
    
    /// Process all enabled effects and output to screen (or target FBO)
    void process(float deltaTime, GLuint targetFBO = 0);
    
    /// Cleanup all resources
    void cleanup();
    
    // =========================================================================
    // Effect Management
    // =========================================================================
    
    /// Add an effect to the chain (takes ownership)
    void addEffect(std::unique_ptr<IPostEffect> effect);
    
    /// Remove effect by ID
    void removeEffect(const std::string& effectId);
    
    /// Get effect by ID
    IPostEffect* effect(const std::string& effectId);
    
    /// Reorder effects
    void moveEffect(const std::string& effectId, int newIndex);
    
    /// Get all effects (in order)
    const std::vector<std::unique_ptr<IPostEffect>>& effects() const;
    
    // =========================================================================
    // Quick Access to Common Effects
    // =========================================================================
    
    void setBlurEnabled(bool enabled);
    void setBlurStrength(float strength);
    
    void setGlowEnabled(bool enabled);
    void setGlowIntensity(float intensity);
    void setGlowSize(float size);
    
    void setMirrorEnabled(bool enabled);
    void setMirrorMode(MirrorMode mode);  // Horizontal, Vertical, Both, Quad
    
    void setKaleidoscopeEnabled(bool enabled);
    void setKaleidoscopeSegments(int segments);
    void setKaleidoscopeRotation(float degrees);
    
    void setFeedbackEnabled(bool enabled);
    void setFeedbackDecay(float decay);  // 0.9 = slow fade, 0.5 = fast fade
    void setFeedbackZoom(float zoom);    // 1.0 = no zoom, 1.01 = slight zoom out
    
    // =========================================================================
    // Parameter Interface (for ConfigPanel)
    // =========================================================================
    
    std::vector<ModuleParamDesc> paramDescs() const;
    bool getParam(const std::string& id, ParamValue& out) const;
    bool setParam(const std::string& id, const ParamValue& value);

private:
    // Framebuffer objects
    GLuint m_inputFBO = 0;       // Visualizer renders here
    GLuint m_inputTexture = 0;   // Color attachment for inputFBO
    
    GLuint m_pingFBO = 0;        // Ping-pong buffer A
    GLuint m_pingTexture = 0;
    
    GLuint m_pongFBO = 0;        // Ping-pong buffer B
    GLuint m_pongTexture = 0;
    
    GLuint m_feedbackFBO = 0;    // Previous frame for feedback effect
    GLuint m_feedbackTexture = 0;
    
    int m_width = 0;
    int m_height = 0;
    
    // Effect chain
    std::vector<std::unique_ptr<IPostEffect>> m_effects;
    
    // Screen quad for final output
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;
    std::unique_ptr<QOpenGLShaderProgram> m_blitShader;
    
    void createFramebuffers();
    void destroyFramebuffers();
    void blitToScreen(GLuint texture);
};
```

---

## 3. Effect Implementations

### 3.1 Blur Effects

#### 3.1.1 Gaussian Blur

Classic two-pass separable Gaussian blur.

```glsl
// Horizontal pass
uniform sampler2D uTexture;
uniform float uBlurSize;
uniform float uWeights[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

void main()
{
    vec2 texelSize = 1.0 / textureSize(uTexture, 0);
    vec3 result = texture(uTexture, vTexCoord).rgb * uWeights[0];
    
    for (int i = 1; i < 5; ++i)
    {
        result += texture(uTexture, vTexCoord + vec2(texelSize.x * i * uBlurSize, 0.0)).rgb * uWeights[i];
        result += texture(uTexture, vTexCoord - vec2(texelSize.x * i * uBlurSize, 0.0)).rgb * uWeights[i];
    }
    
    fragColor = vec4(result, 1.0);
}
```

**Parameters:**
- `blur.enabled` (Bool) - Enable blur
- `blur.strength` (Float 0-20) - Blur radius in pixels
- `blur.passes` (Int 1-5) - Number of blur passes (more = smoother)

#### 3.1.2 Kawase Blur (Recommended)

Faster dual-filter blur, better for real-time use.

```glsl
// Downsample pass
vec3 downsample(sampler2D tex, vec2 uv, vec2 halfpixel)
{
    vec3 sum = texture(tex, uv) * 4.0;
    sum += texture(tex, uv - halfpixel.xy);
    sum += texture(tex, uv + halfpixel.xy);
    sum += texture(tex, uv + vec2(halfpixel.x, -halfpixel.y));
    sum += texture(tex, uv - vec2(halfpixel.x, -halfpixel.y));
    return sum / 8.0;
}

// Upsample pass
vec3 upsample(sampler2D tex, vec2 uv, vec2 halfpixel)
{
    vec3 sum = texture(tex, uv + vec2(-halfpixel.x * 2.0, 0.0));
    sum += texture(tex, uv + vec2(-halfpixel.x, halfpixel.y)) * 2.0;
    sum += texture(tex, uv + vec2(0.0, halfpixel.y * 2.0));
    sum += texture(tex, uv + vec2(halfpixel.x, halfpixel.y)) * 2.0;
    sum += texture(tex, uv + vec2(halfpixel.x * 2.0, 0.0));
    sum += texture(tex, uv + vec2(halfpixel.x, -halfpixel.y)) * 2.0;
    sum += texture(tex, uv + vec2(0.0, -halfpixel.y * 2.0));
    sum += texture(tex, uv + vec2(-halfpixel.x, -halfpixel.y)) * 2.0;
    return sum / 12.0;
}
```

### 3.2 Glow Effect

Combines blur with additive blending for bloom/glow.

**Pipeline:**
1. Extract bright areas (threshold)
2. Blur bright areas
3. Add blurred result to original

**Parameters:**
- `glow.enabled` (Bool)
- `glow.threshold` (Float 0-1) - Brightness threshold for glow
- `glow.intensity` (Float 0-3) - Glow brightness multiplier
- `glow.size` (Float 1-20) - Blur size for glow
- `glow.color` (Color4f) - Tint color for glow

### 3.3 Mirror Effect

Simple UV manipulation for symmetry.

```glsl
uniform int uMirrorMode;  // 0=H, 1=V, 2=Both, 3=Quad

void main()
{
    vec2 uv = vTexCoord;
    
    if (uMirrorMode == 0 || uMirrorMode == 2)  // Horizontal
        uv.x = uv.x < 0.5 ? uv.x * 2.0 : (1.0 - uv.x) * 2.0;
    
    if (uMirrorMode == 1 || uMirrorMode == 2)  // Vertical
        uv.y = uv.y < 0.5 ? uv.y * 2.0 : (1.0 - uv.y) * 2.0;
    
    if (uMirrorMode == 3)  // Quad (4-way symmetry)
    {
        uv = abs(uv - 0.5) * 2.0;
    }
    
    fragColor = texture(uTexture, uv);
}
```

**Parameters:**
- `mirror.enabled` (Bool)
- `mirror.mode` (Enum) - Horizontal, Vertical, Both, Quad

### 3.4 Kaleidoscope Effect

Radial symmetry with configurable segments.

```glsl
uniform int uSegments;
uniform float uRotation;
uniform vec2 uCenter;

void main()
{
    vec2 uv = vTexCoord - uCenter;
    
    float angle = atan(uv.y, uv.x) + uRotation;
    float radius = length(uv);
    
    // Map to single segment
    float segmentAngle = 2.0 * PI / float(uSegments);
    angle = mod(angle, segmentAngle);
    
    // Mirror alternate segments
    if (mod(floor(angle / segmentAngle), 2.0) == 1.0)
        angle = segmentAngle - angle;
    
    // Convert back to UV
    uv = vec2(cos(angle), sin(angle)) * radius + uCenter;
    
    fragColor = texture(uTexture, uv);
}
```

**Parameters:**
- `kaleidoscope.enabled` (Bool)
- `kaleidoscope.segments` (Int 2-16)
- `kaleidoscope.rotation` (Float 0-360) - Rotation in degrees
- `kaleidoscope.center` (Vec2) - Center point (0.5, 0.5 = screen center)
- `kaleidoscope.beatRotation` (Bool) - Rotate on beat

### 3.5 Feedback Effect (Trails/Persistence)

Blends current frame with previous frame(s) for motion trails.

```glsl
uniform sampler2D uCurrentFrame;
uniform sampler2D uPreviousFrame;
uniform float uDecay;       // 0.9 = slow fade, 0.5 = fast fade
uniform float uZoom;        // 1.01 = slight zoom out per frame
uniform float uRotation;    // Rotation per frame

void main()
{
    // Transform UV for zoom/rotation effect on feedback
    vec2 uv = vTexCoord - 0.5;
    uv *= uZoom;
    
    float s = sin(uRotation);
    float c = cos(uRotation);
    uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
    
    uv += 0.5;
    
    vec4 previous = texture(uPreviousFrame, uv) * uDecay;
    vec4 current = texture(uCurrentFrame, vTexCoord);
    
    // Max blend for persistence, add for trails
    fragColor = max(current, previous);
}
```

**Parameters:**
- `feedback.enabled` (Bool)
- `feedback.decay` (Float 0.5-0.99) - How fast old frames fade
- `feedback.zoom` (Float 0.98-1.02) - Zoom per frame (creates tunnel effect)
- `feedback.rotation` (Float -5-5) - Rotation per frame in degrees
- `feedback.blendMode` (Enum) - Max, Add, Screen

### 3.6 Color Effects

#### 3.6.1 Chromatic Aberration

RGB channel separation.

```glsl
uniform float uStrength;
uniform vec2 uDirection;

void main()
{
    vec2 offset = uDirection * uStrength;
    
    float r = texture(uTexture, vTexCoord + offset).r;
    float g = texture(uTexture, vTexCoord).g;
    float b = texture(uTexture, vTexCoord - offset).b;
    
    fragColor = vec4(r, g, b, 1.0);
}
```

#### 3.6.2 Vignette

Darkens edges of screen.

```glsl
uniform float uStrength;
uniform float uSoftness;

void main()
{
    vec2 uv = vTexCoord - 0.5;
    float dist = length(uv) * 2.0;
    float vignette = smoothstep(1.0, 1.0 - uSoftness, dist * uStrength);
    
    fragColor = texture(uTexture, vTexCoord) * vignette;
}
```

---

## 4. Integration with Visualizers

### 4.1 Modified Render Loop

```cpp
void VisualizerWidget::paintGL()
{
    // Get post-process module from visualizer (or create shared instance)
    PostProcessModule* postProcess = getPostProcessModule();
    
    if (postProcess && postProcess->hasEnabledEffects())
    {
        // Render visualizer to FBO instead of screen
        postProcess->bindInputFBO();
        
        m_visualizer->render(m_deltaTime);
        
        postProcess->unbindInputFBO();
        
        // Apply post-processing and output to screen
        postProcess->process(m_deltaTime);
    }
    else
    {
        // Direct render to screen (no post-processing)
        m_visualizer->render(m_deltaTime);
    }
}
```

### 4.2 Shared vs Per-Visualizer PostProcess

**Option A: Shared PostProcessModule (Recommended)**
- One PostProcessModule instance in VisualizerWidget
- Same settings apply to all visualizers
- Simpler, less memory

**Option B: Per-Visualizer PostProcessModule**
- Each visualizer owns its own PostProcessModule
- Different effects per visualizer type
- Settings saved per visualizer preset

### 4.3 ConfigPanel Integration

PostProcessModule exposes parameters via `paramDescs()`, allowing ConfigPanel to auto-generate UI:

```cpp
// In ConfigPanel, after visualizer params
if (postProcess)
{
    for (const auto& p : postProcess->paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "fx." + p.id;
        prefixed.group = "Effects";
        params.push_back(prefixed);
    }
}
```

---

## 5. UI Structure

### 5.1 ConfigPanel Layout

```
▼ Effects
    ┌─ Glow ────────────────────────────────┐
    │ ☑ Enabled                             │
    │ Threshold    ●────────────○  0.7      │
    │ Intensity    ●──────○──────  1.5      │
    │ Size         ●────○────────  5.0      │
    └───────────────────────────────────────┘
    
    ┌─ Feedback (Trails) ───────────────────┐
    │ ☑ Enabled                             │
    │ Decay        ●──────────○──  0.92     │
    │ Zoom         ●──────○──────  1.01     │
    │ Rotation     ●────○────────  0.5°     │
    │ Blend Mode   [Max ▼]                  │
    └───────────────────────────────────────┘
    
    ┌─ Kaleidoscope ────────────────────────┐
    │ ☐ Enabled                             │
    │ Segments     [8 ▼]                    │
    │ Rotation     ●────────────○  0°       │
    │ ☐ Beat Rotation                       │
    └───────────────────────────────────────┘
    
    ┌─ Mirror ──────────────────────────────┐
    │ ☐ Enabled                             │
    │ Mode         [Horizontal ▼]           │
    └───────────────────────────────────────┘
```

---

## 6. Performance Considerations

### 6.1 FBO Management

- **Lazy allocation** - Only create FBOs when effects are enabled
- **Size optimization** - Blur can use half-resolution FBOs
- **Texture reuse** - Ping-pong buffers minimize texture count

### 6.2 Shader Switching

- **Batch similar effects** - Group effects that use same shader
- **Uber-shader option** - Single shader with uniform-controlled paths
- **Compile on demand** - Only compile shaders for enabled effects

### 6.3 Benchmarks (Target)

| Effect | Target Performance | GPU Load |
|--------|-------------------|----------|
| Glow (Kawase) | < 1ms | Low |
| Mirror | < 0.1ms | Minimal |
| Kaleidoscope | < 0.2ms | Low |
| Feedback | < 0.5ms | Low |
| Full chain | < 2ms | Medium |

---

## 7. Implementation Plan

### Phase 1: Core Infrastructure
- [ ] FBO management (input + ping-pong)
- [ ] IPostEffect interface
- [ ] PostProcessModule basic pipeline
- [ ] Simple blit shader

### Phase 2: Basic Effects
- [ ] BlurEffect (Kawase)
- [ ] GlowEffect
- [ ] MirrorEffect

### Phase 3: Advanced Effects
- [ ] KaleidoscopeEffect
- [ ] FeedbackEffect
- [ ] ChromaticAberration
- [ ] Vignette

### Phase 4: Integration
- [ ] VisualizerWidget integration
- [ ] ConfigPanel UI generation
- [ ] Preset saving/loading
- [ ] Performance optimization

---

## 8. File Structure

```
include/visualizers/modules/postprocess/
    ├── IPostEffect.hpp
    ├── PostProcessModule.hpp
    ├── BlurEffect.hpp
    ├── GlowEffect.hpp
    ├── MirrorEffect.hpp
    ├── KaleidoscopeEffect.hpp
    ├── FeedbackEffect.hpp
    └── ColorEffects.hpp

src/visualizers/modules/postprocess/
    ├── PostProcessModule.cpp
    ├── BlurEffect.cpp
    ├── GlowEffect.cpp
    ├── MirrorEffect.cpp
    ├── KaleidoscopeEffect.cpp
    ├── FeedbackEffect.cpp
    └── ColorEffects.cpp
```

---

## 9. Open Questions

1. **Shared vs per-visualizer?** - Recommend shared for simplicity
2. **Effect ordering UI?** - Allow drag-drop reordering?
3. **Presets for effects?** - Save/load effect combinations?
4. **Audio-reactive effects?** - Beat-synced parameters?
5. **Resolution scaling?** - Allow lower-res post-processing for performance?

---

## Appendix A: Shader Template

```glsl
// postprocess_common.glsl

#version 330 core

// Vertex shader (shared by all post effects)
#ifdef VERTEX_SHADER
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
#endif

// Fragment shader template
#ifdef FRAGMENT_SHADER
in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform vec2 uResolution;
uniform float uTime;

// Effect-specific uniforms and code here...

void main()
{
    // Effect implementation
    fragColor = texture(uTexture, vTexCoord);
}
#endif
```
