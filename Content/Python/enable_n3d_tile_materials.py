import unreal

PATHS = [
    "/Game/N3D/Materials/Examples/M_N3D_StylizedGround",
    "/Game/N3D/Materials/Examples/M_N3D_Ground",
    "/Game/N3D/Materials/Examples/Instances/MI_N3D_Stylized_Grass",
    "/Game/N3D/Materials/Examples/Instances/MI_N3D_Stylized_Ground",
]


def enable_instancing(asset):
    material = asset
    if isinstance(asset, unreal.MaterialInstance) or isinstance(asset, unreal.MaterialInstanceConstant):
        parent = asset.get_editor_property("parent")
        if parent:
            material = parent

    if not isinstance(material, unreal.Material):
        unreal.log_warning(f"Skipped {asset.get_path_name()}: not a material")
        return

    material.set_editor_property("used_with_instanced_static_meshes", True)
    try:
        material.set_editor_property("used_with_nanite", True)
    except Exception:
        pass
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(material.get_path_name())
    unreal.log(f"Enabled instanced static meshes on {material.get_path_name()}")


def main():
    for path in PATHS:
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            unreal.log_warning(f"Missing {path}")
            continue
        enable_instancing(unreal.load_asset(path))


if __name__ == "__unreal__" or __name__ == "__main__":
    main()
