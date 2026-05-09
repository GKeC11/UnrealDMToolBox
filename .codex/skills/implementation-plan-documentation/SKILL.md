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

1. Goal, principles, and first-version scope: what the plan builds, what it avoids, first-version behavior, non-goals, and future boundaries.
2. Module ownership, dependencies, and code structure: code location, plugin enablement, Build.cs modules, optional future modules, intentionally excluded dependencies, target directories, files, and ownership boundaries.
3. Core types: key classes, inheritance, data ownership, lifecycle, responsibility split, and non-responsibilities.
4. Feature implementation design: grouped implementation details such as third-party integration, data format conventions, parsing rules, write/update flow, editor or runtime call flow, and error handling.
5. Implementation steps: ordered steps that can be followed directly.
6. Usage rules: asset workflow, configuration workflow, naming rules, designer/developer usage guidance, and project conventions.
7. Verification: compile checks, editor checks, runtime checks, import/export checks, and minimal test assets.
8. Extension points: when and how to add later capabilities without changing the first-version boundary.
9. Current progress, remaining items, and completion criteria: completed work, next steps, explicitly deferred work, and concrete conditions that define done.

Use a shorter outline for small plans, but preserve the same grouping order: goal/scope, ownership/dependencies/structure, core types, feature implementation design, steps, usage guidance, verification, current progress and remaining items.

For small project modules whose first version is narrow, prefer this compact chapter order:

1. 目标、原则和首版范围: explain the target, purpose, implementation stance, first-version scope, and non-goals.
2. 模块归属、依赖和代码结构: list ownership, current required dependencies, conditional future dependencies, intentionally excluded dependencies, and target file layout.
3. 核心类型: explain key classes, inheritance, responsibilities, non-responsibilities, and where feature capability comes from.
4. 功能实现方案: group concrete design details such as integration plan, data format, parsing rules, write flow, editor call flow, and error handling.
5. 实现步骤: list executable implementation steps in order.
6. 使用规范: describe asset usage, naming, tag conventions, designer workflow, and project usage rules.
7. 验证标准: describe compile, editor, runtime, import/export, and minimal asset checks.
8. 后续扩展点: list deferred capabilities and when they should be added.
9. 当前进度和剩余事项: summarize completed or expected outputs, optional follow-up items, explicitly deferred work, and completion criteria.

## Section Ordering Rules

- Keep closely related plan context together. Put scope and non-goals under the goal/principles chapter instead of making a separate top-level chapter unless the scope is unusually large.
- Put file layout under module ownership/dependencies instead of making a separate top-level chapter unless the document is mainly about repository migration or package layout.
- Name the class-focused chapter "核心类型" when it primarily describes classes, structs, factories, assets, or editor toolkits. Use "核心设计" only when it covers broader architecture beyond types.
- Group concrete feature mechanics under "功能实现方案". Examples include third-party integration, input/output format, parser behavior, validation, write flow, editor call flow, transactions, and error reporting.
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
