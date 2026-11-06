import unreal

DEST_PATH = "/Game/Meshes"

MESHES = [
    ("SM_JungleScout.fbx", "SM_JungleScout"),
    ("SM_BananaCannon.fbx", "SM_BananaCannon"),
    ("SM_VineTrap.fbx", "SM_VineTrap"),
    ("SM_MainTree.fbx", "SM_MainTree"),
]


def import_one(filename, asset_name):
    fbx_path = rf"c:\Users\Barrie-George Davies\OneDrive\Documents\Unreal Projects\GADE7322POE\Content\Meshes\{filename}"
    task = unreal.AssetImportTask()
    task.filename = fbx_path
    task.destination_path = DEST_PATH
    task.destination_name = asset_name
    task.replace_existing = True
    task.automated = True
    task.save = True

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.import_animations = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.set_editor_property("combine_meshes", True)
    options.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
    task.options = options

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh_path = f"{DEST_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        unreal.log(f"Imported {mesh_path}")
    else:
        unreal.log_error(f"Failed to import {asset_name}")


def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(DEST_PATH):
        unreal.EditorAssetLibrary.make_directory(DEST_PATH)

    for filename, asset_name in MESHES:
        import_one(filename, asset_name)


if __name__ == "__unreal__" or __name__ == "__main__":
    main()
