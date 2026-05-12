---
name: puerts-ts
description: Write, debug, and maintain Puerts TypeScript code for this Unreal project. Use when Codex needs to work in `TypeScript/`, `Content/JavaScript`, `Typing/ue`, `Plugins/Puerts`, or `MixinDefine.ts`, including implementing gameplay/UI scripts, fixing Puerts runtime issues, maintaining Blueprint Mixins, setting up build or watch commands, or attaching the VS Code debugger to Unreal Editor.
---

# Puerts TS

Use this skill for the project's Puerts TypeScript layer. Keep the workflow centered on source files in `TypeScript/`; treat `Content/JavaScript` and `Typing/ue` as generated output unless the user explicitly asks to touch generated artifacts.

## Generated Artifact Boundary

This boundary is mandatory for this project.

- Do not run TypeScript compilation, watch tasks, Puerts typing generation, or cache-clearing commands by default.
- Do not run `npm run build`, `npm run watch`, `npx tspc`, `tspc`, `tsc`, typing generation scripts, or ts-patch cache cleanup unless the user explicitly asks for that exact generated-output workflow.
- Do not hand-edit `Content/JavaScript/**/*.js`, `Content/JavaScript/**/*.js.map`, `Typing/ue/ue.d.ts`, or `Typing/ue/ue_bp.d.ts` unless the user explicitly asks to edit generated artifacts.
- After TS source edits, stop at source changes and tell the user that generated JS still needs their normal build flow.
- If generated typings are stale or missing, report the missing/stale typing and work with local source-level types only when appropriate; do not regenerate typings unless explicitly requested.
- Local TypeScript interfaces inside business TS files are allowed when they document Blueprint fields on a mixin instance. They are not UE typing generation and must not be used as a reason to edit generated `.d.ts` files.

## Workflow

1. Inspect the relevant TS source plus `package.json`, `tsconfig.json`, and `Plugins/Puerts/ReadMe.md` when build, debug, or mixin behavior matters.
2. Before adding an internal helper, search `TypeScript/Library/` and existing UE Blueprint function libraries exposed on `UE.*` for equivalent behavior.
3. Edit TypeScript in `TypeScript/` or project-owned config files. Do not hand-edit generated JS or generated UE typings by default.
4. Do not compile after `TypeScript/` edits by default. Run build/watch/type-generation commands only when the user explicitly asks for generated output or explicitly asks you to run that command.
5. When the task involves Blueprint Mixins or Unreal runtime behavior, verify the target blueprint type in `Typing/ue/ue_bp.d.ts` and confirm the mixin path is registered in `TypeScript/Framework/Mixin/MixinDefine.ts`.
6. When debugging is requested, use the existing VS Code attach flow and keep source maps enabled.

## Non-Negotiables

- Keep TypeScript on `~5.3.3`. Do not upgrade to 5.4+ unless the user explicitly wants a migration.
- When compilation is explicitly requested by the user, compile with `tspc`, not plain `tsc`.
- Keep `compilerOptions.module` as `commonjs`, `compilerOptions.moduleResolution` as `node`, and `sourceMap` enabled unless the user asks for a config change.
- Keep path imports consistent with `tsconfig.json`: business code uses `@root/*`, while mixin dynamic registration may still use `@Root/...` entries inside `MixinDefine.ts`.
- Treat `Content/JavaScript/**/*.js` and `Content/JavaScript/**/*.js.map` as generated output from `TypeScript/`. Do not hand-edit generated JS or source maps unless the user explicitly asks to change generated artifacts.
- Check `Typing/ue/ue_bp.d.ts` before assuming Blueprint namespace paths.
- Treat `Typing/ue/ue.d.ts` as the source of truth for UE runtime classes, methods, and delegates. If a UE type is already declared there, use it directly and do not re-declare it in business TS via local `type`, anonymous object shapes, or intersection overlays.
- Do not hand-edit `Typing/ue/ue.d.ts` or `Typing/ue/ue_bp.d.ts`. They are generated typings. If declarations are missing or stale, report the issue and do not regenerate them unless the user explicitly asks.
- Do not clear `.tsp-cache`, `node_modules/.cache/ts-patch`, or other build caches unless the user explicitly asks you to fix the build cache state.
- In project TypeScript, do not use leading underscores for private field names; prefer plain `camelCase` such as `displayIndex` or `bindRetryTimer`.
- Respect the project's `strictNullChecks: false` setup. In this codebase it is acceptable for functions typed as returning a concrete UE/object type to return `null`, and callers may still perform `if (!value)` guards.
- When a helper is conceptually returning a specific UE runtime type such as `UE.NOLobbyPlayerController`, prefer the direct concrete return type over `| undefined` unions. In this repository, use the concrete UE type and return `null` when unavailable.
- For wait-until-ready or wait-for-state-change logic, prefer event-driven flow: bind a UE delegate or register a Gameplay Message listener instead of polling with TypeScript timers.

## Code Writing Rules

- Prefer small helper functions in shared libraries such as `TypeScript/Library/CommonLibrary.ts` when the same UE lookup logic is repeated across views/widgets.
- If a helper looks reusable across widgets, gameplay scripts, or native bridge calls, prefer implementing it in the appropriate `TypeScript/Library/*Library.ts` file, or in a native `DM*Library` if it needs C++/UE access. If ownership is unclear, notify the developer that it is likely a common helper instead of silently burying it in one view.
- In this project, do not add `| undefined` to every UE-returning helper by default. If the codebase convention for that helper is a concrete return type, it may still return `null` and let callers perform null checks.
- For UE lookup helpers in business TS, prefer signatures like `private GetLobbyPlayerController(): UE.NOLobbyPlayerController` instead of `UE.NOLobbyPlayerController | undefined` when the helper naturally models a concrete UE object lookup.
- When following the concrete-return-type convention in this repository, prefer plain `return null;` over noisy casts like `null as unknown as Foo` because `strictNullChecks` is disabled.
- Keep business TS aligned with the generated UE typings when they are current. If the same turn changes C++ UFUNCTION/UPROPERTY/API surface that will be exposed to Puerts, business TS may call the new native API directly without waiting for regenerated typings.
- If a Blueprint function library API is already present on `UE.*`, call it directly, for example `UE.DMPuertsLibrary.DiagnoseBlueprintClassLoad(path)`, and do not add `as any` just to bypass typings.
- Avoid editing generated `Content/JavaScript` output unless the user explicitly asks for generated JS changes.
- Do not use generated JS as the patch target when fixing runtime errors. Find and patch the corresponding `TypeScript/` source file, then tell the user to run their normal TS build flow.
- When a Puerts widget needs to listen to Enhanced Input `UInputAction` events, use `TypeScript/Library/WidgetLibrary.ts` as the TS-facing helper. It wraps `UE.DMUILibrary.BindEnhancedInputActionForWidget(...)` / `UnbindEnhancedInputActionForWidget(...)`, returns binding handles, and should be paired with widget `Construct`/`Destruct` lifecycle cleanup. The `UInputAction` reference should come from a generated Blueprint variable or another typed UE object reference; do not hand-roll direct `EnhancedInputComponent.BindAction` calls in business TS.
- When code needs to wait for replication, initialization, async completion, or gameplay phase changes, listen for the owning delegate or message and advance the flow from that callback. Do not solve these waits with `setTimeout`, `setInterval`, or similar TS timer polling unless the user explicitly asks for a temporary workaround.
- In this project, prefer the existing TS-facing Gameplay Message APIs before adding wrappers: send with `UE.GameplayMessageSubsystem.Generic_BroadcastMessage(...)`, and listen with `UE.AsyncAction_ListenForGameplayMessage.ListenForGameplayMessages(...)`, `OnMessageReceived`, and `GetPayload(...)`.
- If using the Gameplay Message pattern, follow `Plugins/GameplayMessageRouter` lifecycle rules: listen on the specific channel/message type, keep the async action or handle alive only for the needed scope, and release or unbind it when the wait is fulfilled or the owner is destroyed.
- If no suitable delegate or Gameplay Message exists yet, prefer declaring the event in C++/UE first and then exposing it to TS, instead of falling back to TS timer polling. Add the minimal native surface needed for an event-driven flow.
- For UI/gameplay flow scripts, add targeted `LogUtil.Log(...)` calls at key transitions such as widget construct/destruct, delegate bind/unbind, validation failures, user actions, RPC dispatch, and major success/failure branches.
- Keep log messages concise and searchable with a stable prefix like `Login View ...` or `Lobby View ...`.
- Use Chinese for comments in Puerts TypeScript code unless matching an existing third-party or engine-facing English comment block.
- Add short comments only when they explain sequencing, safety guards, or Unreal-specific intent that is not obvious from the code itself.

## Mixin Rules

- Build Blueprint Mixins with `PuertsUtil.LoadClass(...)`, declare an interface extending the generated UE type, define the TS class, and call `PuertsUtil.Mixin`.
- If the generated Blueprint type already contains widget members, keep the interface empty instead of re-declaring fields.
- Prefer `PuertsUtil.LoadClass(...)` over direct `UE.Class.Load(...)`.
- When UE runtime typings are missing or stale, report that the generated typings need to be refreshed through the project's normal flow. Do not update, regenerate, or hand-edit generated `.d.ts` files unless the user explicitly asks.
- If `ue_bp.d.ts` does not yet expose the target Blueprint type, report that the generated Blueprint typings are missing and do not implement/register the mixin yet. Do not bypass the missing typing with `any`, `CommonUserWidget`, anonymous widget-member interfaces, inferred `UE.Game...` paths that do not typecheck, or string-path class loading.
- Only use a full asset class path string when the user explicitly asks for a temporary experiment and accepts that it is not the normal project pattern.
- After adding a new mixin file, register its path in `TypeScript/Framework/Mixin/MixinDefine.ts`.
- When consuming an already-mixed Blueprint instance, cast to the concrete mixin class instead of repeating anonymous intersection types.
- If a flow truly must wait, prefer a one-shot delegate binding or Gameplay Message listener over any TS timer. Only use a timer as a last-resort temporary patch when no event source exists and the user accepts that tradeoff.

## Debugging Rules

- Use `npm run build` or `npm run watch` before attaching the debugger only when the user explicitly asks you to generate fresh JavaScript for that debug session.
- Use the VS Code configuration `Attach Unreal Editor` and keep its port aligned with Puerts `DebugPort`.
- Remember multi-process offsets: editor default `8080`, server `9079`, clients `8090`, `8100`, `8110`, and so on.
- If VS Code diagnostics disagree with `tspc`, make sure VS Code is using the workspace TypeScript SDK from `node_modules/typescript/lib` instead of a newer bundled version.

## Read More

Read [references/puerts-typescript.md](references/puerts-typescript.md) when you need the detailed project conventions, common failure modes, or the exact mixin/debug setup.
