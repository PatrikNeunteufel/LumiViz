# Implementation Plan — Standard for Implementation Plans

> **Version:** 1.2.0  
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
- `LumiPulse_Phase1_Foundation.md`
- `CMake_Phase8_ImplementationPlan.md`
- `Feature_Docking_ImplementationPlan.md`

---

## 3. Header Extensions

### 3.1 Required Fields

| Field | Description |
|-------|-------------|
| `Reference:` | Referenced concept or specification |

### 3.2 Optional Additional Fields

| Field | Description | When to Use |
|-------|-------------|-------------|
| `Phase:` | If part of a larger phase planning | Phase-based projects |
| `Estimated Duration:` | Time estimate before start | Always recommended |
| `Actual Duration:` | Real duration after completion | For completed phases |
| `Dependencies:` | Prerequisites (previous phases) | For phases > 1 |
| `Methodology:` | Development methodology (e.g., TDD) | For TDD projects |

### 3.3 Complete Header

```markdown
# [Project] — Implementation Plan [Phase/Feature]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Implementation Plan  
> **Status:** [In Development | In Progress | ✅ Completed (X%)]  
> **Target Audience:** Developers  
> **Reference:** [Concept Document]  
> **Phase:** X (optional)  
> **Dependencies:** Phase X (completed) (optional)  
> **Estimated Duration:** ~X weeks (optional)  
> **Actual Duration:** ~X days (after completion)  
> **Language:** English  
> **Methodology:** Test-Driven Development (TDD) (optional)  
```

### 3.4 Status Variants

| Status | Meaning |
|--------|---------|
| `In Development` | Plan is still being created |
| `In Progress` | Implementation is running |
| `✅ Completed (100%)` | All tasks done |
| `⏸️ Paused` | Temporarily stopped |

---

## 4. Required Sections

Implementation plans use this structure:

```
## Table of Contents

## Overview Checklist
### Step N: [Title]
### Progress (Table)

## 1. Overview
### 1.1 Phase Goal
### 1.2 Deliverables
### 1.3 TDD Core Principle (for TDD projects)

## 2. TDD Workflow (for TDD projects)

## 3. Prerequisites

## 4. Project Structure (optional)

## 5. Configuration (optional)

## 6. Implementation Steps
### Step N: [Title]
#### N.X Task

## 7. Documentation (REQUIRED - always last implementation step)
### 7.1 Module Documentation
### 7.2 API Reference
### 7.3 Example Code
### 7.4 Changelog

## 8. Acceptance Criteria
### 8.1 Functional Requirements
### 8.2 Non-Functional Requirements
### 8.3 TDD Requirements (for TDD projects)

## 9. Next Steps
### 9.1 After Phase X
### 9.2 Open Decisions

## Changelog
```

### 4.1 Section Details

| # | Section | Content | Required |
|---|---------|---------|----------|
| — | Table of Contents | Numbered chapters | ✅ |
| — | Overview Checklist | Compact overview + progress | ✅ |
| 1 | Overview | Goal, deliverables, TDD principle | ✅ |
| 2 | TDD Workflow | Test-first rules, structure | Optional (for TDD) |
| 3 | Prerequisites | External dependencies, tools | ✅ |
| 4 | Project Structure | Directory tree, namespaces | Optional |
| 5 | Configuration | Solution.json, CMake, etc. | Optional |
| 6 | Implementation Steps | Steps with detailed checklists | ✅ |
| 7 | **Documentation** | **Module docs, API, examples** | **✅ REQUIRED** |
| 8 | Acceptance Criteria | When is the phase "done"? | ✅ |
| 9 | Next Steps | What comes next? Open items | ✅ |

### 4.2 Documentation Step (REQUIRED)

> **Documentation is ALWAYS the last implementation step of a phase.**

The phase is only complete when the documentation has been updated.

**Documentation Checklist (in every phase):**

```markdown
### Step N: Documentation (LAST STEP)

- [ ] N.1 Create/update module documentation
- [ ] N.2 Document API reference (Doxygen comments)
- [ ] N.3 Example code in documentation
- [ ] N.4 Update changelog
```

**Documentation Content:**

| Element | Description | Required |
|---------|-------------|----------|
| **Module Overview** | What does the module do? | ✅ |
| **API Reference** | All public classes/methods | ✅ |
| **Example Code** | Typical usage | ✅ |
| **Error Handling** | ErrorCodes and their meaning | Optional |
| **Thread Safety** | Which methods are thread-safe? | Optional |
| **Changelog** | Changes in this phase | ✅ |

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

### Step 1: [Title] ✅

- [x] 1.1 Task A
- [x] 1.2 Task B
- [x] 1.3 Task C

### Step 2: [Title] 🔄

- [x] 2.1 Task A
- [ ] 2.2 Task B

### Progress

| Step | Description | Tasks | Completed | Status |
|------|-------------|-------|-----------|--------|
| 1 | Project Skeleton | 6 | 6 | ✅ |
| 2 | Implementation | 5 | 1 | 🔄 |
| **Σ** | **Total** | **11** | **7** | **64%** |
```

### 5.3 Detailed Checklists

Each implementation step has its own detailed checklist:

```markdown
### Step 1: Set Up Project Skeleton

**Goal:** Build system works, empty window appears

**Expected Result:** [Concrete description]

---

#### 1.1 Solution.json

- [ ] Check schemaVersion
- [ ] Configure externals
- [ ] Test build

---

#### 1.2 Extend Headers

- [ ] Add Qt headers
- [ ] Add STL headers
- [ ] Test compilation
```

### 5.4 Status Symbols

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

### 6.4 Document TDD Semantics

For TDD projects: **Semantics tables BEFORE tests**:

```markdown
#### 2.1 Write Result_Tests.cpp (RED)

**Semantics Decisions:**

| Method | Expected Behavior |
|--------|-------------------|
| `value()` on Err | Throws exception |
| `valueOr(default)` | Returns value or default |
| Double `init()` | Returns false |
```

---

## 7. Example: Complete Implementation Plan

```markdown
# LumiPulse — Implementation Plan Phase 1

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Implementation Plan  
> **Status:** ✅ Completed (100%)  
> **Target Audience:** Developers  
> **Reference:** LumiPulse Concept v0.1.0  
> **Phase:** 1  
> **Actual Duration:** ~1 day  
> **Language:** English  
> **Methodology:** Test-Driven Development (TDD)  

---

## Table of Contents

1. [Overview](#1-overview)
2. [TDD Workflow](#2-tdd-workflow)
3. [Prerequisites](#3-prerequisites)
4. [Project Structure](#4-project-structure)
5. [Implementation Steps](#5-implementation-steps)
6. [Documentation](#6-documentation)
7. [Acceptance Criteria](#7-acceptance-criteria)
8. [Next Steps](#8-next-steps)

---

## Overview Checklist

### Step 1: Project Skeleton ✅

- [x] 1.1 Configure Solution.json
- [x] 1.2 Create directory structure
- [x] 1.3 Test build

### Step 2: Core/Types (TDD) ✅

- [x] 2.1 Write Types_Tests.cpp (RED)
- [x] 2.2 Implement Types.hpp (GREEN)
- [x] 2.3 Refactoring (REFACTOR)

### Step 3: Documentation ✅

- [x] 3.1 Update module documentation
- [x] 3.2 Doxygen comments in headers
- [x] 3.3 Document example code
- [x] 3.4 Update changelog

### Progress

| Step | Description | Tasks | Completed | Status |
|------|-------------|-------|-----------|--------|
| 1 | Project Skeleton | 3 | 3 | ✅ |
| 2 | Core/Types (TDD) | 3 | 3 | ✅ |
| 3 | Documentation | 4 | 4 | ✅ |
| **Σ** | **Total** | **10** | **10** | **✅ 100%** |

---

## 1. Overview

### 1.1 Phase Goal

**Phase 1: Foundation**

Goal is a compilable project skeleton with basic types.

### 1.2 Deliverables

| Component | Description | Priority |
|-----------|-------------|----------|
| Types.hpp | ParamValue, Color4f, Vec2f, Rect | P1 |

### 1.3 TDD Core Principle

> **Tests define the behavior — the code follows.**

---

## 2. TDD Workflow

### 2.1 Test-First Rule

**NEVER write production code without a failing test.**

### 2.2 Clarify Semantics BEFORE Implementation

| Question | Test Case |
|----------|-----------|
| What happens with `contains(-1, 50)`? | "returns false" |

---

## 3. Prerequisites

| Prerequisite | Version | Status |
|--------------|---------|--------|
| CMake Architecture | 1.0+ | ✅ |
| C++ Standard | C++20 | ✅ |

---

## 4. Project Structure

```
projects/apps/LumiPulse/
├── include/Core/
│   └── Types.hpp
└── tests/unit/Core_UnitTests/
    └── Types_Tests.cpp
```

---

## 5. Implementation Steps

### Step 2: Core/Types (TDD)

**Goal:** Basic data types for LumiPulse

---

#### 2.1 Write Types_Tests.cpp (RED)

**Semantics Decisions:**

| Type | Expected Behavior |
|------|-------------------|
| Color4f | RGBA with operator[], equality |
| Vec2f | 2D vector with +, -, *, / |

**Checklist:**

- [x] Write tests
- [x] Run tests → RED

---

## 6. Documentation

**Goal:** Create/update module documentation

**Checklist:**

- [x] Types.hpp Doxygen comments complete
- [x] Example code for Color4f, Vec2f documented
- [x] README.md updated with Core module info
- [x] Changelog updated

---

## 7. Acceptance Criteria

### 7.1 Functional Requirements

| # | Criterion | Test Method | Status |
|---|-----------|-------------|--------|
| A1 | Types compile | Unit Test | ✅ |

### 7.2 Non-Functional Requirements

| # | Criterion | Test Method | Status |
|---|-----------|-------------|--------|
| N1 | Build without warnings | CI | ✅ |

### 7.3 TDD Requirements

| # | Criterion | Status |
|---|-----------|--------|
| T1 | Tests BEFORE implementation | ✅ |

---

## 8. Next Steps

### 8.1 After Phase 1

**Phase 2: Audio & Rendering**

| Component | Dependency on Phase 1 |
|-----------|----------------------|
| PlayerEngine | Application |

---

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| **1.0.0** | **2025-12-26** | **Phase 1 COMPLETED** |
```

---

## 8. Review Checklist

In addition to the Doc.md checklist:

**Structure:**

- [ ] Table of contents present
- [ ] Overview checklist directly after table of contents
- [ ] Progress table in overview checklist
- [ ] Each step has goal and expected result
- [ ] Acceptance criteria defined
- [ ] **Documentation is last implementation step**

**Checklists:**

- [ ] Markdown checkboxes (not in code blocks)
- [ ] Numbering in format `Step.Subtask`
- [ ] Tasks are concrete and completable
- [ ] Imperative form used

**Documentation Step (REQUIRED):**

- [ ] Module documentation included as task
- [ ] API reference (Doxygen) included as task
- [ ] Example code included as task
- [ ] Changelog update included as task

**For TDD Projects:**

- [ ] TDD workflow section present
- [ ] Semantics tables before implementation
- [ ] TDD requirements in acceptance criteria
- [ ] RED → GREEN → REFACTOR documented

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
| **1.2.0** | **2025-12-26** | **Documentation as REQUIRED step at end of every phase. Review checklist extended.** |
| 1.1.0 | 2025-12-26 | New fields: Methodology, Actual Duration, Status variants. TDD-specific sections documented. |
| 1.0.0 | 2025-12-26 | Release version: Structure, checklist format, example |
| 0.1.0 | 2025-12-21 | Initial |
