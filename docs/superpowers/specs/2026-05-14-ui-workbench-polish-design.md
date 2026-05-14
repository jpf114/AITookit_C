# UI Workbench Polish Design

## 1. Goal

This spec defines the next UI polish pass for `AITookit_C`. The goal is not to expand feature scope. The goal is to turn the current functional prototype into a clearer desktop workbench that feels closer to `GIS_TOOL` in temperament while staying centered on the single-image detection workflow.

This pass must improve three things, in order:

1. Professional desktop-tool feel closer to `GIS_TOOL`
2. A smoother single-image inference workflow
3. Clearer result reading and review experience

The work should stay within the existing phase-1 feature boundary. No new task types, no plugin system, and no training workflow are part of this spec.

## 2. Current Context

The current project already has:

- A Qt Widgets main shell with left navigation, center pages, and a right context panel
- Pages for home, models, inference, results, and settings
- Working single-image flow: load manifest, choose image, run detection, view result, export JSON
- Recent model and recent image persistence
- Basic UI regression coverage for main window and selected pages

The current shortcoming is not missing core plumbing. The shortcoming is that the interface still reads like a stitched-together functional build instead of a deliberate desktop workbench. Several pages are too sparse, the center pane is under-structured, and the right context panel still behaves more like a loose status dump than a stable operator aid.

## 3. Scope

This design includes:

- Restructuring the page interiors of the models, inference, and results pages
- Tightening the role of the right context panel
- Unifying button language, state text, and information hierarchy
- Extending the existing theme so the app reads more like a professional desktop tool

This design does not include:

- Any new inference backend
- Folder batch inference
- Video inference
- New export formats beyond the current JSON flow
- Rewriting the service, model, or runtime architecture

## 4. Chosen Direction

Three approaches were considered:

1. Conservative cleanup: keep the current page structures and only tune spacing, labels, and colors
2. Centered workbench: keep the existing three-column shell, but redesign each page so the center pane becomes the main work surface and the right pane becomes a stable task summary
3. Aggressive consolidation: collapse models, inference, and results into a single continuous workbench view

Approach 2 is the chosen direction.

It preserves the current architecture, keeps risk moderate, fits the existing tests, and gets much closer to the desktop-tool language the user wants. It also maps cleanly onto the current task flow without forcing a full navigation rethink.

## 5. High-Level Layout

The outer shell remains a three-column structure.

### Left Column

The left column remains stable navigation. Its job is only to answer: where am I in the workflow?

Expected characteristics:

- Persistent dark sidebar
- Compact navigation items with a clear selected state
- Small amount of framing text near the top and bottom
- No operational controls beyond navigation

### Center Column

The center column becomes the primary work surface for the current page. It is where the user reads, previews, edits, and executes the main task.

Expected characteristics:

- Strong page title and short support text
- One dominant work area per page
- Page-specific grouping instead of loose vertical stacks
- Predictable reading order from action to context to detail

### Right Column

The right column becomes a stable task context rail rather than a loose page-specific note area.

It should continuously answer:

- Which model is active?
- Which image is active?
- Has inference run yet?
- What is the current result status?
- What is the next recommended action?

This panel should change content values as state changes, but the structure should remain stable across pages.

## 6. Page Designs

### 6.1 Models Page

The models page becomes a preparation surface rather than a bare file picker.

Center-pane structure:

- Header area: page title and short description
- Primary action section:
  - `加载模型清单` button
  - current manifest path
  - short instruction text
- Model summary section:
  - model name
  - task type
  - backend type
  - input size
  - label count
  - model file path

The summary should be visually grouped so the user can quickly confirm that the selected package is the intended one before moving to inference.

### 6.2 Inference Page

The inference page becomes the operational center of the app.

Center-pane structure:

- Header area: page title and short description
- Split work area:
  - left: large image preview area
  - right: action rail for the current image and run controls

The action rail contains:

- current image path
- `选择图像` button
- `开始检测` primary button
- readiness status text

The preview and the action controls must live close together so the user does not need to bounce between disconnected parts of the window.

### 6.3 Results Page

The results page becomes a review surface with a clear reading order.

Center-pane structure:

1. Summary strip
   - model name
   - detection count
   - elapsed time
   - `导出 JSON` action
2. Main preview region
   - image with overlays remains the visual anchor
3. Detection detail table
   - category id
   - label
   - confidence
   - bounding box text

The user should be able to understand the result in this order:

- What happened overall
- What the image looks like with detections
- Which exact detections were produced

### 6.4 Settings Page

The settings page stays lighter than the main workflow pages. It remains a support page, not a primary workspace.

Center-pane structure:

- Export directory group
- Recent models group
- Recent images group

The page should visually match the rest of the workbench but should not compete with the model, inference, or result pages.

## 7. Context Rail Design

The right-side context panel is standardized into a stable card-like task summary.

Its sections are:

- `当前模型`
- `当前图像`
- `当前结果`
- `下一步`

Section behavior:

- `当前模型`: shows whether a manifest is loaded and, when present, the current manifest path or display name
- `当前图像`: shows whether an image is selected and, when present, the selected path
- `当前结果`: shows not-run state or the latest detection count and elapsed time
- `下一步`: derives one concise action hint from the current state

Examples:

- No model selected: `下一步：请先加载模型清单。`
- Model loaded, no image selected: `下一步：请选择一张待推理图像。`
- Model and image ready, no run yet: `下一步：模型和图像已就绪，可以开始检测。`
- Result available: `下一步：可查看结果明细，或直接导出 JSON。`

The panel should read as calm and operational, not chatty.

## 8. Content and Tone

All visible text in this pass must be normalized to Chinese and must use restrained tool-style phrasing.

Rules:

- Use direct action labels
- Avoid marketing language
- Avoid overly technical runtime jargon in user-facing status text
- Prefer short complete statements

Target examples:

- `加载模型清单`
- `选择图像`
- `开始检测`
- `请先加载模型清单。`
- `当前还没有推理结果`
- `本次检测完成，共识别 X 个目标。`

## 9. Visual Style

This pass should continue moving toward the `GIS_TOOL` desktop feel without copying its content.

Desired visual traits:

- restrained color palette
- dark, compact navigation rail
- light center workspace and context rail
- consistent borders, spacing, and button states
- stronger hierarchy through grouping, not decoration

Specific intent:

- The app should feel like an operator tool, not a demo screen
- Sections should read as functional groupings, not floating marketing cards
- The main preview areas should dominate where visual inspection matters

## 10. Implementation Boundaries

The implementation should remain mostly inside:

- `src/ui/main_window.*`
- `src/ui/nav_panel.*`
- `src/ui/pages/models_page.*`
- `src/ui/pages/inference_page.*`
- `src/ui/pages/results_page.*`
- `src/ui/pages/settings_page.*`
- `resources/themes/app.qss`

Minor state-handling additions in the UI layer are allowed where needed to support the context rail and page grouping.

The implementation should avoid changing:

- service-layer responsibilities
- model/runtime boundaries
- persistence semantics
- the core single-image inference flow

## 11. Testing and Verification

At minimum, the implementation must preserve and, if needed, update verification for:

- main-window shell behavior
- recent-model activation path
- recent-image activation path
- model-page summary behavior
- settings-page signal behavior

Expected verification steps:

- build affected UI tests
- run `test_main_window`
- run `test_models_page`
- run `test_settings_page`

If object names or widget structure change, tests should be updated rather than dropped.

## 12. Risks and Mitigations

### Risk: UI polish accidentally changes workflow behavior

Mitigation:

- Keep service wiring intact
- Limit behavior changes to page layout and UI-state presentation
- Re-run the existing UI regression tests

### Risk: deeper page restructuring breaks widget lookups used by tests

Mitigation:

- Preserve existing object names where practical
- Add new wrappers without renaming stable test anchors unless necessary

### Risk: trying to do too much in one pass

Mitigation:

- Focus this pass on the models, inference, and results pages
- Leave batch/video and release-packaging work for later milestones

## 13. Success Criteria

This pass is successful when:

- The app reads as a more deliberate desktop workbench
- The models, inference, and results pages have clearer internal hierarchy
- The single-image workflow is easier to follow visually
- The results page supports faster review
- The right context panel gives useful and stable task guidance
- Existing UI regression coverage still passes

