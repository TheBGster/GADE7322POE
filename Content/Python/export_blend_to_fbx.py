import bpy
import sys

argv = sys.argv
argv = argv[argv.index("--") + 1:] if "--" in argv else []
if len(argv) < 2:
    raise SystemExit("Usage: blender --background --python export_blend_to_fbx.py -- <blend> <fbx>")

blend_path, fbx_path = argv[0], argv[1]

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.wm.open_mainfile(filepath=blend_path)

mesh_count = 0
for obj in list(bpy.data.objects):
    if obj.type != "MESH":
        continue
    try:
        obj.hide_set(False)
        obj.hide_viewport = False
        obj.hide_render = False
    except Exception:
        pass
    mesh_count += 1
    print(f"MESH {obj.name} verts={len(obj.data.vertices)} loc={tuple(obj.location)} scale={tuple(obj.scale)}")
    if obj.data.vertices:
        xs = [obj.matrix_world @ v.co for v in obj.data.vertices]
        mins = [min(p[i] for p in xs) for i in range(3)]
        maxs = [max(p[i] for p in xs) for i in range(3)]
        print(f"  bounds min={mins} max={maxs}")

print(f"Exporting {mesh_count} mesh object(s) from {blend_path}")

bpy.ops.export_scene.fbx(
    filepath=fbx_path,
    use_selection=False,
    object_types={"MESH"},
    apply_scale_options="FBX_SCALE_ALL",
    axis_forward="-Y",
    axis_up="Z",
    mesh_smooth_type="FACE",
    use_mesh_modifiers=True,
    add_leaf_bones=False,
    bake_space_transform=True,
    path_mode="COPY",
    embed_textures=True,
)
print(f"Wrote {fbx_path}")
