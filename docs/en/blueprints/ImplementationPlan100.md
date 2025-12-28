# Implementation Plan — Standard for Implementation Plans

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** Stable  
> **Based on:** Doc v1.0, Blueprint v1.0  
> **Target Audience:** Documentation Authors, Project Managers  
> **Language:** English  
> **Deutsch:** [Implementierungsplan.md](../../de/blueprints/Implementierungsplan.md)

---

## Table of Contents

1. [Overview](#1-overview)
2. [Scope](#2-scope)
3. [Header Extensions](#3-header-extensions)
4. [Required Sections](#4-required-sections)
5. [Checklist Format](#5-checklist-format)
6. [Writing Style](#6-writing-style)
7. [Example: Complete Implementation Plan](#7-example-complete-implementation-plan)
8. [Review Checklist](#8-review-checklist)
9. [See Also](#9-see-also)
10. [Changelog](#10-changelog)

---

## 1. Overview

This blueprint defines the **structure for implementation plans**. An implementation plan describes **how** a phase or feature is systematically implemented.

### Target Audience

- Developers implementing a phase or feature
- Project managers tracking progress

### Differentiation

| Documentation Type | Question | Example |
|-------------------|----------|---------|
| **Implementation Plan** | "What do I need to do in what order?" | Phase 1 Implementation |
| **Concept** | "What should the architecture look like?" | MyVisualizer Concept |
| **Guide** | "How do I configure X?" | Qt6 Integration |
| **Tutorial** | "Show me step by step" | Creating First Project |

### Core Characteristics

An implementation plan:

- Breaks down a large task into concrete, checkable steps
- Defines acceptance criteria for "done"
- Enables progress tracking
- Is practice-oriented, not conceptual

---

## 2. Scope

This blueprint applies to documents that:

- Describe concrete implementation steps for phases or features
- Contain task checklists
- Are located in `docs/[lang]/projects/` or project-specific folders

**Examples:**

- `MyVisualizer_Phase1_ImplementationPlan.md`
- `CMake_Phase8_ImplementationPlan.md`
- `Feature_Docking_ImplementationPlan.md`

---

## 3. Header Extensions

### 3.1 Required Fields

| Field | Description |
|-------|-------------|
| `Reference:` | Referenced concept or specification |

### 3.2 Optional Additional Fields

| Field | Description |
|-------|-------------|
| `Phase:` | If part of a larger phase planning |
| `Estimated Duration:` | Time estimate |
| `Dependencies:` | Prerequisites (previous phases, etc.) |

### 3.3 Complete Header

```markdown
# [Project] — Implementation Plan [Phase/Feature]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Implementation Plan  
> **Status:** [In Development | In Progress | Completed]  
> **Target Audience:** Developers  
> **Reference:** [Concept Document]  
> **Phase:** X (optional)  
> **Estimated Duration:** ~X weeks (optional)  
> **Language:** English  
```

---

## 4. Required Sections

Implementation plans use this structure:

```
## Overview Checklist

## 1. Overview
### 1.1 Phase Goal
### 1.2 Deliverables
### 1.3 Estimated Duration

## 2. Prerequisites

## 3. Project Structure (optional)

## 4. Configuration (optional)

## 5. Implementation Steps
### Step N: [Title]
#### Checklist Step N

## 6. Files and Interfaces (optional)

## 7. Quick Reference (optional)

## 8. Acceptance Criteria
### 8.1 Functional Requirements
### 8.2 Non-Functional Requirements

## 9. Next Steps

## Changelog
```

### 4.1 Section Details

| # | Section | Content | Required |
|---|---------|---------|----------|
| — | Overview Checklist | Compact overview of all tasks | ✅ |
| 1 | Overview | Goal, deliverables, duration | ✅ |
| 2 | Prerequisites | External dependencies, tools, prior knowledge | ✅ |
| 3 | Project Structure | Directory tree, file organization | Optional |
| 4 | Configuration | Solution.json, CMake, etc. | Optional |
| 5 | Implementation Steps | Steps with detailed checklists | ✅ |
| 6 | Files and Interfaces | Code sketches, API definitions | Optional |
| 7 | Quick Reference | Daily checklist, brief overview | Optional |
| 8 | Acceptance Criteria | When is the phase "done"? | ✅ |
| 9 | Next Steps | What comes next? Open items | ✅ |

---

## 5. Checklist Format

### 5.1 Markdown Checkboxes

**Always** use real Markdown checkboxes:

```markdown
- [ ] Task open
- [x] Task completed
```

**Never** put checkboxes in code blocks — these are not rendered interactively.

### 5.2 Overview Checklist

The overview checklist appears directly after the table of contents and provides a compact overview:

```markdown
## Overview Checklist

### Step 1: [Title]

- [ ] 1.1 Task A
- [ ] 1.2 Task B
- [ ] 1.3 Task C

### Step 2: [Title]

- [ ] 2.1 Task A
- [ ] 2.2 Task B
```

### 5.3 Detailed Checklists

Each implementation step has its own detailed checklist with subtasks:

```markdown
### Step 1: Set Up Project Skeleton

**Goal:** Build system works, empty window appears

**Expected Result:** [Concrete description]

#### Checklist Step 1

**1.1 Solution.json**

- [ ] Check schemaVersion
- [ ] Configure externals
- [ ] Test build

**1.2 Extend Headers**

- [ ] Add Qt headers
- [ ] Add STL headers
- [ ] Test compilation
```

### 5.4 Progress Table

At the end of implementation steps, include a progress table:

```markdown
### Overall Progress

| Step | Description | Subtasks | Status |
|------|-------------|----------|--------|
| 1 | Project Skeleton | 7 | ⬜ |
| 2 | Interfaces | 6 | ⬜ |
| 3 | Implementation | 8 | ⬜ |
| **Σ** | **Total** | **21** | **0%** |
```

**Status Symbols:**

| Symbol | Meaning |
|--------|---------|
| ⬜ | Open |
| 🔄 | In Progress |
| ✅ | Completed |

### 5.5 Numbering

Tasks are numbered in the format `Step.Subtask`:

- `1.1`, `1.2`, `1.3` for Step 1
- `2.1`, `2.2` for Step 2
- etc.

This enables unambiguous referencing in commit messages or discussions.

---

## 6. Writing Style

### 6.1 Imperative for Tasks

Checklist entries in imperative form:

| ❌ Passive/Descriptive | ✅ Imperative |
|----------------------|--------------|
| "Header should be created" | "Create header" |
| "Tests are being written" | "Write tests" |
| "Configuration needs to be adjusted" | "Adjust configuration" |

### 6.2 Concrete and Measurable

Tasks must be clearly completable:

| ❌ Vague | ✅ Concrete |
|---------|-----------|
| "Improve code" | "Implement error handling" |
| "Add tests" | "Write unit test for loadFile()" |
| "Documentation" | "Update README.md with build instructions" |

### 6.3 Explicit Dependencies

When tasks build upon each other, mark this:

```markdown
**3.4 BassAudioSource Implementation**

- [ ] loadFile(): BASS_StreamCreateFile()
- [ ] play(): BASS_ChannelPlay() *(requires loadFile)*
- [ ] getFFT(): BASS_ChannelGetData() *(requires play)*
```

---

## 7. Example: Complete Implementation Plan

```markdown
# MyVisualizer — Implementation Plan Phase 1

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Implementation Plan  
> **Status:** In Development  
> **Target Audience:** Developers  
> **Reference:** MyVisualizer Concept v1.0.0  
> **Estimated Duration:** ~1 week  
> **Language:** English  

---

## Table of Contents

1. [Overview](#1-overview)
2. [Prerequisites](#2-prerequisites)
3. [Implementation Steps](#3-implementation-steps)
4. [Acceptance Criteria](#4-acceptance-criteria)
5. [Next Steps](#5-next-steps)

---

## Overview Checklist

### Step 1: Project Skeleton

- [ ] 1.1 Solution.json configured
- [ ] 1.2 PCH extended
- [ ] 1.3 Build successful

### Step 2: PlayerEngine

- [ ] 2.1 Header created
- [ ] 2.2 Implementation complete
- [ ] 2.3 Unit tests pass

### Progress

| Step | Tasks | Status |
|------|-------|--------|
| 1 | 3 | ⬜ |
| 2 | 3 | ⬜ |
| **Σ** | **6** | **0%** |

---

## 1. Overview

### 1.1 Phase Goal

**Phase 1: Base Window with Audio Playback**

Goal is a functional audio player without visualization.

### 1.2 Deliverables

| Component | Description | Priority |
|-----------|-------------|----------|
| MainWindow | Qt6 main window | P1 |
| PlayerEngine | BASS integration | P1 |

### 1.3 Estimated Duration

~1 week

---

## 2. Prerequisites

| Prerequisite | Version | Status |
|--------------|---------|--------|
| Qt6 | 6.5+ | - [ ] |
| CMake | 3.25+ | - [ ] |
| BASS | 2.4+ | - [ ] |

---

## 3. Implementation Steps

### Step 1: Set Up Project Skeleton

**Goal:** Build works

**Expected Result:** Empty window appears

#### Checklist Step 1

**1.1 Solution.json**

- [ ] Check schemaVersion
- [ ] Configure externals
- [ ] Add app configuration

**1.2 Extend PCH**

- [ ] Add Qt headers
- [ ] Test build

**1.3 Build & Test**

- [ ] CMake Configure successful
- [ ] Build without errors
- [ ] Application starts

---

### Step 2: PlayerEngine

**Goal:** Audio playback works

#### Checklist Step 2

**2.1 Create Header**

- [ ] Create include/PlayerEngine.hpp
- [ ] Apply Pimpl pattern

**2.2 Implementation**

- [ ] BASS_Init() in constructor
- [ ] play(), pause(), stop()
- [ ] getFFT() for visualization

**2.3 Tests**

- [ ] Create test_PlayerEngine.cpp
- [ ] All tests pass

---

## 4. Acceptance Criteria

### 4.1 Functional Requirements

| # | Criterion | Test Method |
|---|-----------|-------------|
| A1 | Application starts | Manual |
| A2 | Play audio file | Unit Test |
| A3 | Play/Pause works | Manual |

### 4.2 Non-Functional Requirements

| # | Criterion | Test Method |
|---|-----------|-------------|
| N1 | Build without warnings | CI |
| N2 | All tests pass | CI |

---

## 5. Next Steps

### 5.1 After Phase 1

Phase 2 builds on Phase 1:
- VisualizerWidget requires PlayerEngine::getFFT()

### 5.2 Open Items

| # | Topic | Decision Needed |
|---|-------|-----------------|
| 1 | Icon Set | Which design? |

---

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-12-26 | Initial |
```

---

## 8. Review Checklist

In addition to the Doc.md checklist:

**Structure:**

- [ ] Overview checklist directly after table of contents
- [ ] Each step has goal and expected result
- [ ] Progress table present
- [ ] Acceptance criteria defined

**Checklists:**

- [ ] Markdown checkboxes (not in code blocks)
- [ ] Numbering in format `Step.Subtask`
- [ ] Tasks are concrete and completable
- [ ] Imperative form used

**Content:**

- [ ] Reference to concept document present
- [ ] Prerequisites listed
- [ ] Next steps defined
- [ ] Open items documented

**Practicability:**

- [ ] Steps are in logical order
- [ ] Dependencies between tasks clear
- [ ] Estimated duration realistic

---

## 9. See Also

- [Doc.md](Doc.md) — General documentation rules
- [Concept.md](Concept.md) — For architecture concepts
- [Guide.md](Guide.md) — For user guides

---

## 10. Changelog

| Version | Date | Changes |
|---------|------|---------|
| **1.0.0** | **2025-12-26** | **Release version: Structure, checklist format, example** |
| 0.1.0 | 2025-12-21 | Initial |
