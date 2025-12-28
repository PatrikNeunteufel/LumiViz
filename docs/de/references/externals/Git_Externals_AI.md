# Git Externals: AI & GPU Computing — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-14  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [Git_Externals_AI.md](../../en/references/externals/Git_Externals_AI.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [LLM Inference](#2-llm-inference)
3. [ML Frameworks](#3-ml-frameworks)
4. [CUDA / GPU Computing](#4-cuda--gpu-computing)
5. [Hinweise](#5-hinweise)
6. [Schnellreferenz](#6-schnellreferenz)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Dieses Dokument beschreibt Bibliotheken für Machine Learning, LLM Inference und GPU Computing.

### Kategorien

| Kategorie | Bibliotheken |
|-----------|--------------|
| LLM Inference | llama.cpp, whisper.cpp, ggml |
| ML Frameworks | onnxruntime, ncnn |
| CUDA/GPU | thrust, cub, cutlass |

### Komplexitätshinweis

⚠️ Viele dieser Bibliotheken haben komplexe Build-Anforderungen (CUDA, spezielle Hardware). Prüfe die Dokumentation der jeweiligen Bibliothek vor der Integration.

---

## 2. LLM Inference

### 2.1 llama.cpp

> **Zweck:** Effiziente LLM Inference in C/C++ (LLaMA, Mistral, Phi, etc.).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/ggerganov/llama.cpp.git` |
| **Aktueller Tag** | `b2000+` (häufige Releases) |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **CUDA** | Optional (signifikante Beschleunigung) |
| **Komplexität** | 🟡 Mittel |

```json
"llama_cpp": {
    "git": "https://github.com/ggerganov/llama.cpp.git",
    "tag": "b2500"
}
```

**PreFetch Hook (llama_cpp.cmake):**
```cmake
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# GPU Acceleration (optional)
set(LLAMA_CUDA OFF CACHE BOOL "" FORCE)      # NVIDIA CUDA
set(LLAMA_VULKAN OFF CACHE BOOL "" FORCE)    # Vulkan (cross-platform)
set(LLAMA_METAL OFF CACHE BOOL "" FORCE)     # Apple Metal
```

**Verwendung:**
```cpp
#include "llama.h"

llama_backend_init();
llama_model_params model_params = llama_model_default_params();
llama_model* model = llama_load_model_from_file("model.gguf", model_params);

llama_context_params ctx_params = llama_context_default_params();
ctx_params.n_ctx = 2048;
llama_context* ctx = llama_new_context_with_model(model, ctx_params);

// Tokenize, sample, generate...
```

---

### 2.2 whisper.cpp

> **Zweck:** Automatic Speech Recognition (OpenAI Whisper in C++).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/ggerganov/whisper.cpp.git` |
| **Aktueller Tag** | `v1.6.0+` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **CUDA** | Optional |
| **Komplexität** | 🟡 Mittel |

```json
"whisper_cpp": {
    "git": "https://github.com/ggerganov/whisper.cpp.git",
    "tag": "v1.6.0"
}
```

**PreFetch Hook (whisper_cpp.cmake):**
```cmake
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_CUDA OFF CACHE BOOL "" FORCE)
```

**Verwendung:**
```cpp
#include "whisper.h"

whisper_context_params cparams = whisper_context_default_params();
whisper_context* ctx = whisper_init_from_file_with_params("model.bin", cparams);

whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
wparams.language = "de";

whisper_full(ctx, wparams, pcm_data, n_samples);

int n_segments = whisper_full_n_segments(ctx);
for (int i = 0; i < n_segments; ++i) {
    const char* text = whisper_full_get_segment_text(ctx, i);
    printf("%s\n", text);
}
```

---

### 2.3 ggml

> **Zweck:** Tensor-Bibliothek für Machine Learning (Basis für llama.cpp, whisper.cpp).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/ggerganov/ggml.git` |
| **Aktueller Tag** | Aktuelle Releases |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Komplexität** | 🟡 Mittel |

```json
"ggml": {
    "git": "https://github.com/ggerganov/ggml.git",
    "tag": "master"
}
```

---

## 3. ML Frameworks

### 3.1 onnxruntime

> **Zweck:** ONNX Model Inference von Microsoft.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/microsoft/onnxruntime.git` |
| **Aktueller Tag** | `v1.17.0` |
| **CMake Support** | ⚠️ Komplex |
| **Hook** | PreFetch erforderlich |
| **Komplexität** | 🔴 Hoch |

```json
"onnxruntime": {
    "git": "https://github.com/microsoft/onnxruntime.git",
    "tag": "v1.17.0"
}
```

**PreFetch Hook (onnxruntime.cmake):**
```cmake
set(onnxruntime_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(onnxruntime_BUILD_SHARED_LIB ON CACHE BOOL "" FORCE)
set(onnxruntime_ENABLE_PYTHON OFF CACHE BOOL "" FORCE)
```

> ⚠️ **Hinweis:** Sehr komplexer Build. Empfehlung: Pre-built Binaries von der [Release Page](https://github.com/microsoft/onnxruntime/releases) verwenden.

---

### 3.2 ncnn

> **Zweck:** Hochperformante Neural Network Inference (Mobile-optimiert, von Tencent).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/Tencent/ncnn.git` |
| **Aktueller Tag** | `20240102` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Vulkan** | Optional (GPU Acceleration) |
| **Komplexität** | 🟡 Mittel |

```json
"ncnn": {
    "git": "https://github.com/Tencent/ncnn.git",
    "tag": "20240102"
}
```

**PreFetch Hook (ncnn.cmake):**
```cmake
set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
# Vulkan GPU Acceleration
set(NCNN_VULKAN OFF CACHE BOOL "" FORCE)
```

---

## 4. CUDA / GPU Computing

### 4.1 thrust

> **Zweck:** CUDA-basierte parallele Algorithmen (STL-ähnliche API).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/NVIDIA/thrust.git` |
| **Aktueller Tag** | `2.2.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Header-only** | ✅ |
| **Voraussetzung** | CUDA Toolkit |

```json
"thrust": {
    "git": "https://github.com/NVIDIA/thrust.git",
    "tag": "2.2.0"
}
```

**PreFetch Hook (thrust.cmake):**
```cmake
set(THRUST_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(THRUST_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
```

**Verwendung:**
```cpp
#include <thrust/device_vector.h>
#include <thrust/sort.h>

thrust::device_vector<int> d_vec(1000000);
// Fill vector...
thrust::sort(d_vec.begin(), d_vec.end());
```

---

### 4.2 cub

> **Zweck:** CUDA Unbound - Primitive für CUDA Kernel Development.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/NVIDIA/cub.git` |
| **Aktueller Tag** | `2.2.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Header-only** | ✅ |

```json
"cub": {
    "git": "https://github.com/NVIDIA/cub.git",
    "tag": "2.2.0"
}
```

---

### 4.3 cutlass

> **Zweck:** CUDA Templates for Linear Algebra Subroutines (Matrix Operations).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/NVIDIA/cutlass.git` |
| **Aktueller Tag** | `v3.4.1` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Voraussetzung** | CUDA Toolkit |
| **Komplexität** | 🔴 Hoch |

```json
"cutlass": {
    "git": "https://github.com/NVIDIA/cutlass.git",
    "tag": "v3.4.1"
}
```

**PreFetch Hook (cutlass.cmake):**
```cmake
set(CUTLASS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(CUTLASS_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CUTLASS_ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
```

---

## 5. Hinweise

### 5.1 CUDA Toolkit (System)

CUDA muss als System-Installation vorliegen:

```cmake
find_package(CUDAToolkit REQUIRED)
target_link_libraries(MyApp PRIVATE CUDA::cudart CUDA::cublas)
```

### 5.2 Build-Komplexität

| Bibliothek | Build-Zeit | Abhängigkeiten |
|------------|------------|----------------|
| llama.cpp | Mittel | Optional: CUDA, Vulkan, Metal |
| whisper.cpp | Mittel | Optional: CUDA |
| onnxruntime | Sehr lang | Viele (empfehle Pre-built) |
| ncnn | Mittel | Optional: Vulkan |
| thrust/cub | Schnell | CUDA Toolkit |
| cutlass | Lang | CUDA Toolkit |

### 5.3 Empfohlener Ansatz

Für onnxruntime und komplexe CUDA-Projekte:
1. Pre-built Binaries herunterladen
2. Als Local External einbinden
3. Custom Include.cmake schreiben

---

## 6. Schnellreferenz

| Bibliothek | Tag | CMake | Hook | Hauptverwendung |
|------------|-----|-------|------|-----------------|
| llama.cpp | b2500 | ✅ | PreFetch | LLM inference |
| whisper.cpp | v1.6.0 | ✅ | PreFetch | Speech recognition |
| ggml | master | ✅ | PreFetch | Tensor operations |
| onnxruntime | v1.17.0 | ⚠️ | PreFetch | ONNX inference |
| ncnn | 20240102 | ✅ | PreFetch | Mobile ML inference |
| thrust | 2.2.0 | ✅ | PreFetch | CUDA parallel algorithms |
| cub | 2.2.0 | ✅ | PreFetch | CUDA primitives |
| cutlass | v3.4.1 | ✅ | PreFetch | CUDA linear algebra |

---

## 7. Siehe auch

- [Git_Externals_Reference.md](Git_Externals.md) — Hauptübersicht
- [Git_Externals_Core.md](Git_Externals_Core.md) — Testing & Benchmarking
- [Git_Externals_Network.md](Git_Externals_Network.md) — Threading

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Initial: Ausgelagert aus Git_Externals_Reference** |
