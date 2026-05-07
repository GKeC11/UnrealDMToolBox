---
name: analysis-documentation
description: Create, review, restructure, and maintain analysis-style Markdown documentation. Use when Codex works on plugin analysis, engine subsystem analysis, architecture notes, source-code investigation reports, comparison documents, risk reviews, troubleshooting references, or other documents whose purpose is to explain how an existing system works.
---

# 分析文档

Use this skill for analysis documentation: Markdown files that explain an existing system, plugin, engine subsystem, source-code behavior, architecture, workflow, convention, risk, or troubleshooting path.

Do not use this skill for implementation proposal documents whose main purpose is to define what this project should build next. Use the implementation-plan-documentation skill for that.

## Workflow

1. Inspect the current outline before editing. Use heading search to understand hierarchy, numbering, and section order.
2. Identify the reader path: why the topic exists, how it is structured, what core types and mechanisms matter, how to use it and adopt it, project conventions, troubleshooting, and glossary.
3. Move sections to the level where they belong. Preserve technical meaning and source names while improving structure.
4. After moving or merging sections, renumber headings and scan for gaps, duplicates, and inconsistent heading depth.
5. Verify source-defined names before correcting apparent typos in enums, APIs, tags, protocols, or file names.

## Structure Rules

- Treat documentation as a reader path, not a dump of facts. Prefer this order when it fits the topic: positioning and design intent, module or system map, core types and mechanisms, usage and adoption guidance, project conventions, troubleshooting, glossary.
- Put conceptual material under design or architecture sections. Plugin positioning, design tradeoffs, and pain-point improvements belong together instead of being split into unrelated top-level chapters.
- Keep sibling sections at the same abstraction level. Do not mix setup steps, API reference, design rationale, and troubleshooting under one heading unless they are clearly separated by subsections.
- Keep project-specific dependency locations, concrete call sites, and per-plugin usage examples below a project practice, usage notes, or adoption section. They should not be top-level siblings of conceptual sections such as positioning, core concepts, or architecture unless the whole document is a project implementation report.
- Combine usage instructions and recommended adoption into one chapter when they would repeat each other. Use subsections only when the full option reference and the minimum project path are both substantial enough to justify separation.
- Combine project practice and project conventions into one chapter when both describe how this repository uses the system. Split conventions into a separate chapter only when naming rules, tag domains, protocol ranges, or similar standards are large enough to be used as an independent reference.

## Recommended Outline

For plugin, engine subsystem, or project-system analysis documents, strongly prefer this chapter shape, trimming sections that do not apply:

1. Positioning and design intent: what problem the system solves, its boundary, and what it should not be used for.
2. Modules and dependencies: runtime/editor modules, plugin dependencies, directories, and high-level system map.
3. Implementation principles: core design ideas, lifecycle, data flow, scheduling path, authority/replication behavior, and responsibility split.
4. Core types and mechanisms: key classes, interfaces, structs, enums, tags, and how their mechanisms work.
5. Usage flow and adoption guidance: how to use the system and the recommended project path, combined unless they are clearly large independent topics.
6. Project practice and conventions: current repository usage, dependency locations, concrete call sites, existing examples, naming rules, directory rules, tag domains, and similar project-specific standards.
7. Plugin defects: source-confirmed bugs, questionable implementations, design limitations, version mismatches, missing lifecycle events, broken APIs, project integration gaps, and adoption risks. Put plugin defects and risk points under one chapter.
8. FAQ and troubleshooting: common user questions, repeated adoption mistakes, diagnostic checks, and concrete fixes. Keep FAQ as a separate top-level chapter when it exists; do not bury it inside the defects chapter.
9. Appendix: file index, glossary, references, and other material that should not interrupt the main reader path.

Use a shorter outline for small documents, but preserve the same order. For example: positioning, implementation principles, usage flow and adoption guidance, project practice and conventions, plugin defects, FAQ.

## Heading Rules

- Use chapter titles that describe the section's job. Avoid vague titles such as "core value" when the content is really about previous pain points and how the system improves them; prefer titles like "痛点改进", "接入流程", "项目使用建议", or "常见问题".
- Keep heading depth readable. Use numbered top-level and second-level headings for navigation, but avoid heavy deep numbering when a subsection is mainly explanatory. A section such as `### 1.7 痛点改进` can use plain `#### 多目标管理` style children.
- When the document uses numbered headings, keep FAQ entries consistently numbered too.
- After structural edits, run a heading search such as `rg -n "^(#|##|###|####) " <file>` and check the outline before finishing.

## Writing Rules

- Add short guide paragraphs at high-friction boundaries: after positioning, before long core-type lists, and before recommended adoption flows. These should explain how to read the next section, not repeat the whole document.
- For long type or API summary sections, add a recommended reading order before the details. Start with the small set of types that explain the system's lifecycle and data flow.
- Keep FAQ entries action-oriented. Each FAQ should start with checks, likely causes, or concrete fixes, not background explanation.
- For plugin analysis documents, add a dedicated defects chapter when the review finds source-confirmed defects, questionable implementation, integration gaps, or adoption risks. Each entry should include the affected symbol, file, subsystem, or project area; observed behavior or current gap; expected behavior; impact; and recommended handling. Keep all defect and risk entries in this chapter instead of creating a separate top-level risks chapter.
- Keep FAQ as its own chapter when there are actual common questions or troubleshooting procedures. FAQ entries should be action-oriented and separate from defect records, even when they mention the same subsystem.
- Prefer concise Chinese prose for Chinese project documentation. Keep engine, protocol, class, function, tag, and file names in their original spelling and wrap them in backticks.
- Avoid unrelated polishing while restructuring. Preserve examples, technical intent, and source names unless the requested documentation change requires an update.

## Source Accuracy

- Verify spelling that might be source-defined before correcting it. If an enum, method, protocol name, or tag is misspelled in source, preserve the source spelling in documentation and add a short note that it is the source name.
- Prefer source files, existing project docs, and local configuration over memory when documenting project-specific behavior.
- When referencing code symbols, use exact names and casing from the repository.
