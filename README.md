# tuff3d

A small, minimalist polygon modeler for Windows with Autodesk Maya's navigation and hotkeys.
It is one ~640 KB exe with no installer, and it only uses DLLs that ship with Windows
(OpenGL 1.1, GDI, GDI+ for reference images, common dialogs, shell drag and drop). It redraws only when something changes, so
it sits at 0% CPU when idle. Meshes are drawn from cached vertex arrays that are only rebuilt after
an edit, so tumbling and hovering stay fast on dense meshes.

**Download:** [`bin/tuff3d.exe`](bin/tuff3d.exe)

## Features

Rigging and animation for game engines (shelf tabs **Rig** and **Animate**)
- Hierarchies: P parents the selection to the last selected object, Shift+P unparents; the Outliner
  (side panel tab) shows the tree, and dragging an item onto another parents it
- Edit Pivot (D): move an object's pivot without moving it, or a joint without its children; hold V
  to snap it to a vertex. Pivot to Base puts it at the bottom center, so a model stands on the floor
- Joints: the Joint tool places a chain with each click (clicked on a mesh, the joint sits halfway
  through it); or one-click skeletons sized to the selected mesh: Biped (Roblox R15 body part names),
  Quadruped, or a Chain of four joints for bending props. Mirror Joints copies a chain to the other
  side; with Symmetry on, moving a Left joint moves its Right partner
- Bind Skin: select the mesh(es) and a joint; automatic weights (each vertex follows the bones it
  sits beside, blended at the joints). The Weights tool paints them with a heat map (click a joint,
  drag to add, Ctrl removes, Shift smooths; Symmetry paints both sides). Mirror Weights, Automatic
  Weights, Rebind and Rest Pose too. Weights follow every later mesh edit (extrude, cut, smooth...)
- IK: select a hand or foot joint and Add IK; drag the handle and the limb bends to reach it
  (the Pole in the side panel sets which way the knee or elbow points)
- Keyframes: K keys the selected objects or joints on this frame; Auto Key (on by default) re-keys
  anything already animated whenever you move it. Ease Smooth, Linear or Step per key
- Timeline under the viewport: play (Space), scrub, step with Left / Right (Shift: between keys),
  drag a key diamond to retime it, right-click for keys, easing and copy / paste pose
- Clips: Idle, Walk, Run... each its own animation (New Clip, rename in the side panel); Turntable
  makes a spin clip in one click. Onion skin shows ghosts a few frames back and ahead; Mirror Pose
  copies a pose from left to right
- File > Export glTF (.glb), Ctrl+Shift+E: meshes with vertex colors and shading, the hierarchy,
  bones and weights, and every clip (baked frame by frame, IK included). Godot and Roblox Studio
  import it directly; Blender, three.js and most other tools read it too. The files pass the Khronos
  glTF validator with no issues

Symmetry and soft selection
- Symmetry (S, or the butterfly button; right-click it for X / Y / Z and world space): select on one
  side and the mirror image is picked too, then move, rotate, scale, extrude, type values or paint on
  one side and the other side follows. Vertices on the plane stay on it. A tinted plane shows the
  axis in the viewport. Symmetry Tolerance (side panel) sets how close a vertex must be to its mirror
  image to count as its partner
- Soft selection (tap B): moving a selection drags the vertices around it along with a smooth falloff,
  shown as yellow-to-red dots. Hold B and drag to set the radius (or type it in the side panel).
  Works together with symmetry
- Mesh > Mirror (and the heart button; right-click for the axis): mirrors each selected object across
  its own X, Y or Z plane and welds the seam. A half model becomes whole; a model that spans the plane
  keeps its bigger side and copies it over the other, which makes a lopsided model symmetric again

Modeling
- Primitives with editable parameters (in the side panel until you edit the mesh's components);
  whole numbers have - / + buttons, so picking the number of sides is one click each
  - Cube: width/height/depth, subdivisions per axis
  - Sphere: sides, rings, radius
  - Cylinder: sides, radius, height, height divisions, cap rings, top/bottom cap on/off
  - Cone: sides, radius, height, height divisions, cap rings
  - Torus: sides, tube sides, radius, tube radius
  - Pipe: sides, radius, wall thickness, height, height divisions
  - Plane: width/depth, subdivisions per axis
- Mesh > Smooth: Catmull-Clark subdivision (every face splits in four and rounds off; open borders and
  hard edges stay creased). Smooth preview (3 on, 1 off) shows the smoothed surface while you keep
  editing the light cage; Preview Level 1-3 in the side panel
- Move / rotate / scale manipulators: arrow = 1 axis, square = 2 axes, center = all 3. A faint line shows
  the axis a drag is locked to. Scaling components stops at 0, so dragging a handle past the pivot
  flattens them onto one plane (e.g. line up 2 vertices on X)
- Snapping: hold X (grid), V (onto vertices, with a marker on the target) or J (15 degree rotation);
  Grid Snap and Point Snap are also toggle buttons
- Extrude faces (kept together, then pull along the normal) and extrude edges; Merge to Center.
  Extrude Thickness and Offset (inset) in the side panel redo the last extrusion live
- Bevel (Ctrl+B) the selected edges, the edges of selected faces, or whole objects; Bridge two open
  borders. Bevel Width and Bridge Divisions in the side panel redo the last bevel/bridge live
- Insert Edge Loop tool: hover an edge for a yellow preview of the loop, click to add it
- Multi-Cut tool: click points on edges or vertices to cut faces (Enter or right click finishes,
  Backspace drops the last point, Esc cancels, clicking the first point closes the cut, Ctrl+click
  an edge inserts an edge loop)
- Target Weld tool (drag one vertex onto another), Merge Vertices by distance (Merge Distance in the
  side panel), Delete Edge/Vertex (Delete or Ctrl+Delete on edges or vertices joins their faces)
- Combine / Separate (Mesh menu and toolbar): Separate splits each selected object into its loose
  pieces (handy for an OBJ exported as one mesh), or, in face mode, turns the selected faces into a
  new object. Triangulate, Quadrangulate, Reverse Normals
- Fill Hole: caps the open border your selected vertices, edges or faces touch (or every hole of the
  selected objects in object mode); the new face is left selected, ready to extrude
- Modify menu: Freeze Transformations, Center Pivot, Reset Transformations
- Vertex paint brush (color, opacity, radius) and "Apply Color to Selection" (tint by opacity)
- Duplicate, delete objects or faces, hide / show objects

Selection
- Objects, vertices, edges, faces, or Multi mode (F7): all three at once, like Maya's multi-component
  selection (a click picks a nearby vertex, then an edge, then the face under the cursor)
- Click, marquee (only what's visible in shaded mode), Shift/Ctrl modifiers
- Double-click an edge for its edge loop, a face for the face loop running across the edge nearest
  the cursor, a vertex for its vertex loop; Select > Edge Ring; grow / shrink (> / <); convert
  between vertices, edges and faces (Ctrl+F9/F10/F11); select all / invert
- The side panel's Selection section shows the selected components' center and size: type a center
  to move them there, or a size to scale them to it (symmetry and soft selection included)

Feedback
- Preselection highlight: whatever a click would pick (object, vertex, edge or face) lights up red
- Heads-up display with vertex/edge/face/tri/object counts (selected counts in gold)
- Short on-screen notes for every command ("Extruded", "Undo Move", "Edge loop: 20 edges")
- The move/rotate/scale amount next to the cursor while dragging; Esc cancels a drag
- Named undo steps (Edit > Undo Extrude) and a status line that explains the button or component
  under the mouse, or how to use the current tool
- Filled marquee with +, - or +/- for add, remove and toggle; cursors for orbit/pan/zoom and painting
- Tips on an empty scene and in an empty side panel

Comfort
- Icon toolbars: a toolbox on the left for the tools and selection modes, and a shelf on top for
  shapes, mesh operations, symmetry / soft selection / snapping and display toggles. The icons are
  drawn anti-aliased in the theme's colors, short hotkeys sit in each button's corner, and hovering
  a button shows a tooltip with its name, hotkey and what it does
- Display > UI Size (Ctrl+Plus / Ctrl+Minus): 90% to 250%; buttons, panel, text and handles all scale.
  The default is 125%
- Side panel: rename objects, type values (Up / Down arrow keys step them), or drag a label
  left/right to scrub its value (Shift fast, Ctrl fine). Choices are buttons (Off | X | Y | Z),
  sections fold away when you click their header, and the mouse wheel scrolls the panel
- Smooth camera moves for frame (F/A) and views; Home view, front/side/top views, and clicking
  X/Y/Z on the corner gizmo to look down that axis (click again for the opposite side)
- File > Open Recent, drag and drop .obj files onto the window (Shift adds them to the scene), and
  open a file passed on the command line (e.g. Open With)
- Autosave every minute while there are unexported changes; after a crash, tuff3d offers to recover
- The title bar shows the file name with `*` for unexported changes, and New / Open / Exit ask before
  discarding them
- Remembers the window position, side panel, wireframe, heads-up display, manipulator size, theme
  and the last shading mode picked
- 48 themes (Classic, 33 dark, 14 light), picked in the theme picker (Ctrl+T or Display > Theme):
  a tile per theme showing tuff3d in its colors; hover a tile to preview it on the whole window,
  click to keep it, arrow keys to step through, R for a random one. Ctrl+Shift+T steps to the next
  theme. Themes color the viewport, side panel, toolbar, status bar, menu bar and (Windows 10/11)
  the title bar
- X-Ray (see and select through surfaces), Backface Culling, Isolate Select (Ctrl+1: show only the
  selection)
- Hard Shade, Smooth Shade and Auto Smooth per object (Display menu and toolbar). Auto Smooth keeps
  edges sharper than the Normal Angle (default 30°, editable in the side panel) hard and smooths the
  rest; Harden / Soften Edge override single edges (hard ones draw thicker while editing)
- Front / side / top views are orthographic, with the grid on the view plane; orbiting goes back to
  perspective. Display > Image Planes puts a reference picture behind each of those views
- Help > Hotkey Editor: change any command's keys (two per command); menus and toolbar follow
- Save scenes as `.tuff` (File > Save): everything is kept, including hidden objects, primitive
  settings, edge overrides, smooth preview, symmetry, the camera and image planes. Autosave uses it too
- Export OBJ (world space, vertex colors written as `v x y z r g b`, shading written as `vn` normals)
  and open/import OBJ; Ctrl+S on an opened OBJ saves back to it. Shading comes back when you reopen the file, and smoothing groups from other
  apps' OBJs (`s 1` / `s off`) open smooth or hard

See [FEATURES.md](FEATURES.md) for a checklist of what's in and ideas for what could come next.

## Controls (Maya defaults)

| Input | Action |
| --- | --- |
| Alt + LMB / MMB / RMB drag | Tumble / track / dolly |
| Mouse wheel | Dolly |
| F / A / Home | Frame selected / frame all / home view |
| Click X / Y / Z on the corner gizmo | Look down that axis |
| Q W E R | Select / move / rotate / scale tool |
| F8 / F9 / F10 / F11 / F7 | Object / vertex / edge / face / multi mode |
| S | Symmetry on / off (right-click its button for the axis) |
| B / hold B + drag | Soft selection on / off / its radius |
| Ctrl+F9 / F10 / F11 | Convert selection to vertices / edges / faces |
| RMB on object | Component mode menu |
| Shift / Ctrl / Ctrl+Shift + click | Toggle / deselect / add |
| Double-click an edge / face / vertex | Select its edge / face / vertex loop |
| > / < | Grow / shrink selection |
| Ctrl+Shift+A / Ctrl+Shift+I | Select all / invert selection |
| Ctrl + drag axis handle | Move / scale on the other two axes |
| MMB drag | Drag with the last used handle |
| hold X / V / J | Snap to grid / to vertices / rotation to 15° |
| Esc while dragging | Cancel the drag |
| + / - | Manipulator size |
| Ctrl+E / Ctrl+B | Extrude / bevel |
| Ctrl+D | Duplicate |
| Delete / Ctrl+Delete | Delete objects or faces / delete edges or vertices (their faces join) |
| Ctrl+H / Ctrl+Shift+H | Hide selected / show all |
| Z or Ctrl+Z / Shift+Z or Ctrl+Y | Undo / redo |
| G | Repeat last command |
| 4 / 5 | Wireframe / shaded |
| 3 / 1 | Smooth preview on / off |
| Ctrl+1 | Isolate select |
| Ctrl+Plus / Ctrl+Minus | Bigger / smaller UI |
| P / Shift+P | Parent / unparent |
| D | Edit pivot |
| K / Shift+K | Set key / delete key |
| Space | Play / stop |
| Left / Right, Shift+Left / Right | Previous / next frame, previous / next key |
| Ctrl+Shift+E | Export glTF (.glb) |
| B + LMB drag | Brush radius (paint tool) |
| Ctrl+N / Ctrl+O / Ctrl+S | New / open / save (.tuff scene, or back to the open OBJ) |
| Ctrl+Shift+O / Ctrl+Shift+S | Import into the scene / save as |
| Ctrl+A | Toggle side panel |
| Ctrl+T / Ctrl+Shift+T | Theme picker / next theme |
| F1 | Hotkey list |

Tools without a Maya default hotkey (Paint, Target Weld, Insert Edge Loop, Multi-Cut, Bridge, Merge,
Mirror, Smooth, X-Ray, ...) are in the toolbar and the Mesh / Edit Mesh / Tools menus; Help > Hotkey Editor can give them keys.

## Maya LT scenes (.mlt)

Maya LT is discontinued and nothing else reads its `.mlt` scenes, so tuff3d opens them itself:
File > Open, drag and drop, or `tuff3d.exe "apple c.mlt"`. They are Maya's binary scene format
with scrambled chunk sizes and names. Objects saved without history store their mesh, which tuff3d
reads as is. Everything else is stored as construction history (the steps that build a model), so
tuff3d replays that history to rebuild every visible mesh in world space, with its vertex colors.
Save (Ctrl+S) then writes an `.obj` next to it; the `.mlt` itself is never written to.

Supported history steps: cube, cylinder, plane, sphere and torus primitives, Smooth, Extract (chip
off), Separate, Extrude faces (including thickness) and edges, Bridge, Split (Insert Edge Loop /
Multi-Cut cuts), Bevel, Fill Hole (close border), Delete faces and edges, Delete Edge, Combine
(unite), Merge Vertices, tweaks (moved vertices) and Paint Vertex Color, plus object transforms.
UV, normal and soft-edge steps are skipped since tuff3d doesn't use them.

Later steps pick vertices, edges and faces by number, so tuff3d numbers them the way Maya does.
Those rules were worked out against the checks Maya leaves in the file (each extrusion records the
bounding box of what it extruded, and vertex color steps list the faces around every vertex);
`tests/mlt_dump.c` with `MLT_DEBUG=1` prints them. Limits:

- Steps tuff3d doesn't know yet are skipped, and a warning names them, so the shape may be missing
  that edit.
- A few numbering cases are still guessed. When an extrusion's selection falls clearly outside the
  box Maya recorded, tuff3d swaps in the matching faces or edges, but a later step can still land
  on a slightly different face.
- Bevel follows Maya exactly when every edge is bevelled; partial bevels are approximated.
- Vertex colors are per vertex in tuff3d, so where Maya had a hard color edge the colors blend.
- Holes inside faces (rare) are dropped from stored meshes.

If a file doesn't open or looks wrong, send it in so support can be added.

### Converting with full Maya

With full Maya 2023 or newer (the free 30-day trial works), [`tools/mlt_convert.py`](tools/mlt_convert.py)
gives an exact conversion of a whole folder in one go. For every `.mlt` (subfolders included) it
writes into a `converted` folder:

- `name.obj`: every visible mesh in world space, with vertex colors as `v x y z r g b` (tuff3d reads
  these; Maya's own OBJ exporter drops vertex colors)
- `name.fbx`: UVs, materials and colors, for other 3D apps and game engines
- `name.ma`: a Maya ASCII copy of the whole scene with its history, so the edit steps survive too

The `.mlt` files are only read. A `convert_log.txt` lists what was converted. To run it:

1. In Maya: Windows > General Editors > Script Editor, Python tab, File > Load Script... and pick
   `mlt_convert.py`, then Execute (Ctrl+Enter). It asks for the folder.
2. Or from a command prompt, without opening Maya's window:
   `"C:\Program Files\Autodesk\Maya2025\bin\mayapy.exe" mlt_convert.py "D:\My MLT files"`
   (options: `--no-fbx`, `--no-ma`, `--overwrite`)

Files that were already converted are skipped, so it can be re-run after adding more.

## Building

Plain C99 with MinGW-w64:

- Linux/macOS: `./build.sh` (needs `x86_64-w64-mingw32-gcc` and `x86_64-w64-mingw32-windres`)
- Windows: `build.bat` (needs `gcc` and `windres` from MSYS2 or w64devkit on PATH)

The icon is `res/tuff3d.ico` (16 to 256 px), built from `res/tuff3d.png` with `python3 res/make_icon.py res/tuff3d.png res/tuff3d.ico` (needs Pillow).

Geometry tests (any C compiler): `cc -Isrc tests/test_mesh.c src/mesh.c -lm && ./a.out`

Animation, skinning, IK and glTF tests: `cc -Isrc tests/test_anim.c src/anim.c src/gltf.c src/mesh.c -lm && ./a.out out.glb`
(also writes a rigged, animated test file you can check with the Khronos glTF validator)

`src/mesh.c` is the platform-independent geometry core (primitives, extrude, merge, edge loops,
selection grow/shrink/convert, symmetry pairing, mirror, Catmull-Clark smoothing, OBJ).
`src/anim.c` holds keyframes and easing, automatic weights, skinning and two bone IK;
`src/gltf.c` writes `.glb` files.
`src/main.c` is the Win32/OpenGL UI.
