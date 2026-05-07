---
name: implementation-plan-documentation
description: Create, review, restructure, and maintain implementation-plan Markdown documents. Use when Codex writes or edits module implementation plans, feature design plans, phased rollout plans, dependency plans, integration plans, acceptance criteria, or project-specific build steps for work that has not yet been fully implemented.
---

# 实现方案文档

Use this skill for implementation-plan documentation: Markdown files whose main job is to define what this project should build, why, where it belongs, what dependencies it needs, and how to implement and verify it.

Do not use this skill for pure analysis of an existing plugin, engine subsystem, or source-code behavior. Use the analysis-documentation skill for that.

## Workflow

1. Identify the implementation target: module, feature, class, asset type, integration path, or migration.
2. Separate current scope from future scope. State explicitly what the first version does and does not implement.
3. Define ownership and boundaries before listing steps. Make clear which plugin, project module, asset, or subsystem owns each responsibility.
4. List dependencies in tiers: current required dependencies, conditional future dependencies, and dependencies intentionally not added yet.
5. Explain the core design before the step-by-step implementation. Readers should understand the key class, asset, or subsystem before seeing the commands and file edits.
6. Write implementation steps in executable order: enable plugins, add module dependencies, create directories, add classes, wire config, compile, create test assets, verify.
7. Finish with acceptance criteria, current progress, and remaining items so the implementation cannot expand silently.
8. After editing, scan headings and ensure the document reads as a plan, not a source analysis dump.

## Recommended Outline

For module or feature implementation plans, prefer this shape and trim sections that do not apply:

1. Goal and principles: what the plan builds, what it avoids, and the core implementation stance.
2. Scope and non-goals: first-version behavior, excluded systems, and future boundaries.
3. Module ownership and dependencies: code location, plugin enablement, Build.cs modules, optional future modules, and intentionally excluded dependencies.
4. Current code structure: directories, files, classes, and whether the module is project code, plugin code, or asset-only.
5. Core design: key classes, inheritance, data ownership, lifecycle, and responsibility split.
6. Implementation steps: ordered steps that can be followed directly.
7. Asset or configuration workflow: how designers or developers create and configure assets after code exists.
8. Verification: compile checks, editor checks, runtime checks, and minimal test assets.
9. Extension points: when and how to add later capabilities without changing the first-version boundary.
10. Completion criteria: concrete conditions that define done.

Use a shorter outline for small plans, but preserve the same order: goal, scope, dependencies, code structure, core design, steps, usage guidance, current progress and remaining items.

For small project modules whose first version is narrow, prefer this compact chapter order:

1. 模块定位: explain the target, purpose, first-version scope, and non-goals.
2. 依赖关系: list current required dependencies, conditional future dependencies, and dependencies intentionally not added.
3. 核心类型和功能: explain key classes, inheritance, responsibilities, non-responsibilities, and where feature capability comes from.
4. 实现步骤: list executable implementation steps in order.
5. 使用规范和建议: describe asset usage, naming, tag conventions, designer workflow, and project usage rules.
6. 当前进度和剩余事项: summarize current file layout, completed or expected outputs, optional follow-up items, and explicitly deferred work.

## Section Ordering Rules

- Put core types and feature responsibilities before implementation steps. A reader should know what is being built before reading how to create files or edit config.
- Do not put code skeletons only in the implementation steps when the same type also has a core-design section. Keep the main explanation in core design; implementation steps can reference the file path and include only the minimal skeleton needed for execution.
- Put usage rules, asset conventions, naming guidance, and designer workflow after implementation steps unless they are prerequisites for writing the code.
- End with current progress, remaining items, completion criteria, or first-version boundary. This keeps status and scope-control information out of the middle of the plan.
- If a document has both "current code structure" and "implementation steps", code structure should describe the target file layout; steps should describe how to create or update it.

## Dependency Rules

- Separate "required now" from "required later". Do not add every plausible module to the current dependency list.
- Tie each dependency to a concrete code reference or planned feature. Example: `NinjaInventory` is required because `UNOItem` inherits `UNinjaInventoryItemDataAsset`.
- Put UI, interaction, equipment, editor, and gameplay modules in future or conditional sections unless the current implementation directly includes their headers or types.
- State when a plugin must be enabled in `.uproject` separately from when a module must be added to `.Build.cs`.
- If a dependency is intentionally excluded, say why and when it should be added.

## Implementation Step Rules

- Steps should be ordered so a developer can execute them without guessing prerequisites.
- Prefer concrete paths and exact class names.
- Include minimal code skeletons only when they clarify the implementation contract.
- Do not include broad tutorials for engine systems unless the plan depends on a specific project convention.
- Keep non-goals near the relevant step when they prevent common over-implementation.

## Writing Rules

- Prefer concise Chinese prose for Chinese project documentation.
- Keep Unreal, plugin, class, function, tag, file, and module names in their original spelling and wrap them in backticks.
- Use "当前第一版" and "后续再接入" style wording to control scope.
- Avoid vague promises such as "完善系统". Use verifiable outputs such as "创建 `UNOItem` DataAsset 并能在 Editor 中创建资产".
- Preserve source-defined names and casing. Verify uncertain symbols in local source before documenting them.

## Anti-Patterns

- Turning an implementation plan into a plugin analysis document.
- Adding project-owned managers before the ownership boundary requires one.
- Reimplementing plugin-provided runtime systems in a project plan without a documented reason.
- Mixing future UI, interaction, inventory, equipment, and editor tooling into the first-version checklist.
- Listing dependencies without explaining what code or feature needs them.
