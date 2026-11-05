import unreal

MATERIALS = "/Game/Materials"
VFX = "/Game/VFX"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_or_load(name, path, asset_class, factory):
    full = f"{path}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        return unreal.load_asset(full)
    return unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, path, asset_class, factory)


def set_scalar_param(expr, name, default):
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", default)


def set_vector_param(expr, name, default):
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", default)


def connect(mat, from_expr, from_output, to_property):
    unreal.MaterialEditingLibrary.connect_material_property(from_expr, from_output, to_property)


def create_jungle_sway():
    mat = create_or_load("M_JungleSway", MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

    color = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -500, -180)
    set_vector_param(color, "Color", unreal.LinearColor(0.12, 0.55, 0.18, 1.0))
    connect(mat, color, "", unreal.MaterialProperty.MP_BASE_COLOR)

    time_expr = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTime, -900, 80)
    speed = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -900, 180)
    set_scalar_param(speed, "SwaySpeed", 1.8)
    amount = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -900, 280)
    set_scalar_param(amount, "SwayAmount", 8.0)
    extra_time = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -900, 380)
    set_scalar_param(extra_time, "Time", 0.0)

    mul_speed = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -680, 120)
    unreal.MaterialEditingLibrary.connect_material_expressions(time_expr, "", mul_speed, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(speed, "", mul_speed, "B")

    sine = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionSine, -500, 120)
    unreal.MaterialEditingLibrary.connect_material_expressions(mul_speed, "", sine, "")

    cosine = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionCosine, -500, 240)
    unreal.MaterialEditingLibrary.connect_material_expressions(mul_speed, "", cosine, "")

    z_off = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -320, 80)
    unreal.MaterialEditingLibrary.connect_material_expressions(sine, "", z_off, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(amount, "", z_off, "B")

    y_off = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -320, 200)
    unreal.MaterialEditingLibrary.connect_material_expressions(cosine, "", y_off, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(amount, "", y_off, "B")
    y_scale = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -160, 200)
    const_y = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -320, 320)
    const_y.set_editor_property("r", 0.35)
    unreal.MaterialEditingLibrary.connect_material_expressions(y_off, "", y_scale, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(const_y, "", y_scale, "B")

    append = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionAppendVector, -40, 140)
    zero = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -160, 40)
    zero.set_editor_property("r", 0.0)
    unreal.MaterialEditingLibrary.connect_material_expressions(zero, "", append, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(y_scale, "", append, "B")

    append2 = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionAppendVector, 140, 140)
    unreal.MaterialEditingLibrary.connect_material_expressions(append, "", append2, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(z_off, "", append2, "B")
    connect(mat, append2, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)

    unreal.MaterialEditingLibrary.layout_material_expressions(mat)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset("/Game/Materials/M_JungleSway")
    return mat


def create_living_gold():
    mat = create_or_load("M_LivingGold", MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)

    color = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, -40)
    set_vector_param(color, "Color", unreal.LinearColor(0.35, 0.45, 0.18, 1.0))
    base_color = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, 80)
    set_vector_param(base_color, "BaseColor", unreal.LinearColor(0.35, 0.45, 0.18, 1.0))
    gold = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, 200)
    set_vector_param(gold, "GoldColor", unreal.LinearColor(0.92, 0.74, 0.16, 1.0))
    tier = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, 320)
    set_scalar_param(tier, "UpgradeTier", 0.0)
    time_expr = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTime, -700, 420)

    sine = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionSine, -480, 420)
    unreal.MaterialEditingLibrary.connect_material_expressions(time_expr, "", sine, "")
    pulse = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -320, 400)
    const_amp = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -480, 500)
    const_amp.set_editor_property("r", 0.18)
    unreal.MaterialEditingLibrary.connect_material_expressions(sine, "", pulse, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(const_amp, "", pulse, "B")
    bias = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionAdd, -160, 380)
    const_bias = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -320, 500)
    const_bias.set_editor_property("r", 0.35)
    unreal.MaterialEditingLibrary.connect_material_expressions(pulse, "", bias, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(const_bias, "", bias, "B")

    saturate_tier = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionSaturate, -480, 300)
    unreal.MaterialEditingLibrary.connect_material_expressions(tier, "", saturate_tier, "")
    alpha = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, 0, 300)
    unreal.MaterialEditingLibrary.connect_material_expressions(saturate_tier, "", alpha, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(bias, "", alpha, "B")

    lerp = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, 180, 80)
    unreal.MaterialEditingLibrary.connect_material_expressions(color, "", lerp, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(gold, "", lerp, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(alpha, "", lerp, "Alpha")
    connect(mat, lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)

    unreal.MaterialEditingLibrary.layout_material_expressions(mat)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset("/Game/Materials/M_LivingGold")
    return mat


def create_grove_pressure():
    mat = create_or_load("M_PP_GrovePressure", MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
    mat.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    blend_names = (
        "AFTER_TONEMAPPING",
        "BL_AFTER_TONEMAPPING",
        "TRANSLUCENCY_AFTER_DOF",
        "BEFORE_TONEMAPPING",
        "BL_BEFORE_TONEMAPPING",
    )
    for name in blend_names:
        if hasattr(unreal.BlendableLocation, name):
            mat.set_editor_property("blendable_location", getattr(unreal.BlendableLocation, name))
            break

    scene = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionSceneTexture, -700, 0)
    try:
        scene.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
    except Exception:
        pass

    pressure = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, 180)
    set_scalar_param(pressure, "Pressure", 0.0)
    pulse = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, 280)
    set_scalar_param(pulse, "DamagePulse", 0.0)

    desat = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionDesaturation, -400, 0)
    unreal.MaterialEditingLibrary.connect_material_expressions(scene, "Color", desat, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(pressure, "", desat, "Fraction")

    uv = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -700, 400)
    sub = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionSubtract, -520, 400)
    half = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -700, 500)
    half.set_editor_property("r", 0.5)
    unreal.MaterialEditingLibrary.connect_material_expressions(uv, "", sub, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(half, "", sub, "B")
    dist = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionDistance, -340, 400)
    zero2 = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant2Vector, -520, 500)
    unreal.MaterialEditingLibrary.connect_material_expressions(sub, "", dist, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(zero2, "", dist, "B")

    vignette = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionSaturate, -160, 400)
    unreal.MaterialEditingLibrary.connect_material_expressions(dist, "", vignette, "")
    dark = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, 20, 360)
    unreal.MaterialEditingLibrary.connect_material_expressions(vignette, "", dark, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(pressure, "", dark, "B")
    one_minus = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionOneMinus, 180, 360)
    unreal.MaterialEditingLibrary.connect_material_expressions(dark, "", one_minus, "")

    mul = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, 360, 40)
    unreal.MaterialEditingLibrary.connect_material_expressions(desat, "", mul, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(one_minus, "", mul, "B")

    red = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, 180, 200)
    set_vector_param(red, "PulseColor", unreal.LinearColor(0.55, 0.05, 0.04, 1.0))
    lerp = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, 540, 80)
    unreal.MaterialEditingLibrary.connect_material_expressions(mul, "", lerp, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(red, "", lerp, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(pulse, "", lerp, "Alpha")
    connect(mat, lerp, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    unreal.MaterialEditingLibrary.layout_material_expressions(mat)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset("/Game/Materials/M_PP_GrovePressure")
    return mat


def create_niagara(name):
    system = create_or_load(name, VFX, unreal.NiagaraSystem, unreal.NiagaraSystemFactoryNew())
    unreal.EditorAssetLibrary.save_asset(f"{VFX}/{name}")
    return system


def main():
    ensure_dir(MATERIALS)
    ensure_dir(VFX)
    create_jungle_sway()
    create_living_gold()
    try:
        create_grove_pressure()
    except Exception as Error:
        unreal.log_error(f"Grove pressure material failed: {Error}")
    try:
        create_niagara("NS_BananaBurst")
        create_niagara("NS_UpgradeBloom")
    except Exception as Error:
        unreal.log_error(f"Niagara create failed: {Error}")
    unreal.log("Part 3 materials and Niagara systems created.")


if __name__ == "__unreal__" or __name__ == "__main__":
    main()
