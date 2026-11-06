import bpy
import sys

argv = sys.argv
argv = argv[argv.index("--") + 1:] if "--" in argv else []
blend_path = argv[0]
fbx_path = argv[1] if len(argv) > 1 else ""

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.wm.open_mainfile(filepath=blend_path)

print("SCENES", [s.name for s in bpy.data.scenes])
print("COLLECTIONS", [c.name for c in bpy.data.collections])
print("OBJECTS")
for obj in bpy.data.objects:
    print(f"  {obj.name} type={obj.type} hide={obj.hide_get()} loc={tuple(obj.location)} scale={tuple(obj.scale)}")

print("MESHES", [m.name for m in bpy.data.meshes])
print("CURVES", [c.name for c in bpy.data.curves])

# Convert curves to mesh if needed, then export.
if bpy.context.scene:
    view_layer = bpy.context.view_layer
    for obj in list(bpy.data.objects):
        try:
            view_layer.objects.active = obj
            obj.select_set(True)
        except Exception as exc:
            print("select failed", obj.name, exc)

    curve_objs = [o for o in bpy.data.objects if o.type == "CURVE"]
    for obj in curve_objs:
        try:
            view_layer.objects.active = obj
            bpy.ops.object.select_all(action="DESELECT")
            obj.select_set(True)
            bpy.ops.object.convert(target="MESH")
            print("converted curve", obj.name)
        except Exception as exc:
            print("convert failed", obj.name, exc)

if fbx_path:
    bpy.ops.export_scene.fbx(
        filepath=fbx_path,
        use_selection=False,
        object_types={"MESH", "CURVE", "EMPTY"},
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
    print("Wrote", fbx_path)
