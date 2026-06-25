# Creates the city ground Materials for RAND:
#   M_Road     -> Megascans asphalt, samples MPC_Road
#   M_Pavement -> Megascans concrete, samples MPC_Pavement
#
# Run headless (or from the editor Python console), AFTER make_surface_collections.py:
#   UnrealEditor-Cmd.exe RAND.uproject -ExecutePythonScript="Scripts/make_megascans_materials.py"
#
# Each material wires:
#   - a TextureCoordinate multiplied by the MPC "Tiling" scalar -> all sampler UVs
#   - BaseColor (albedo) * MPC "Tint"
#   - Normal map
#   - Roughness from the roughness texture (or the MPC "Roughness" scalar if no map)
#
# The Megascans texture paths are left blank below: paste the imported texture
# asset paths once the asphalt/concrete surfaces are downloaded from FAB and
# imported, then re-run. Missing textures are skipped gracefully so the material
# still compiles (falling back to the MPC tint/roughness).

import unreal

MATERIAL_PATH = "/Game/RAND/Materials"
COLLECTION_PATH = "/Game/RAND/Materials/Collections"

# --- Megascans texture slots (fill in after importing from FAB) -------------
ROAD_ALBEDO = ""     # e.g. "/Game/Megascans/Surfaces/Asphalt/T_Asphalt_Albedo.T_Asphalt_Albedo"
ROAD_NORMAL = ""
ROAD_ROUGHNESS = ""

PAVEMENT_ALBEDO = ""  # e.g. "/Game/Megascans/Surfaces/Concrete/T_Concrete_Albedo.T_Concrete_Albedo"
PAVEMENT_NORMAL = ""
PAVEMENT_ROUGHNESS = ""

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def load_texture(path):
    if path and unreal.EditorAssetLibrary.does_asset_exist(path):
        tex = unreal.EditorAssetLibrary.load_asset(path)
        if isinstance(tex, unreal.Texture):
            return tex
    return None


def make_material(name, collection_name, albedo_path, normal_path, roughness_path):
    full_path = "%s/%s" % (MATERIAL_PATH, name)
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        unreal.EditorAssetLibrary.delete_asset(full_path)

    mat = asset_tools.create_asset(name, MATERIAL_PATH, unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_error("RAND: failed to create %s" % name)
        return None

    collection = unreal.EditorAssetLibrary.load_asset("%s/%s" % (COLLECTION_PATH, collection_name))

    # Tiling: TexCoord * MPC.Tiling -> shared UVs.
    tex_coord = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -900, 0)
    tiling = mel.create_material_expression(mat, unreal.MaterialExpressionCollectionParameter, -900, 150)
    if collection:
        tiling.set_editor_property("collection", collection)
        tiling.set_editor_property("parameter_name", "Tiling")
    uv_mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -650, 50)
    mel.connect_material_expressions(tex_coord, "", uv_mul, "A")
    mel.connect_material_expressions(tiling, "", uv_mul, "B")

    def sampler(tex, y):
        node = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, y)
        if tex:
            node.set_editor_property("texture", tex)
        mel.connect_material_expressions(uv_mul, "", node, "UVs")
        return node

    albedo = load_texture(albedo_path)
    normal = load_texture(normal_path)
    rough = load_texture(roughness_path)

    # Base colour: albedo * MPC.Tint (tint works even with no albedo texture).
    tint = mel.create_material_expression(mat, unreal.MaterialExpressionCollectionParameter, -400, -200)
    if collection:
        tint.set_editor_property("collection", collection)
        tint.set_editor_property("parameter_name", "Tint")

    if albedo:
        albedo_node = sampler(albedo, -50)
        base_mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, -120)
        mel.connect_material_expressions(albedo_node, "RGB", base_mul, "A")
        mel.connect_material_expressions(tint, "", base_mul, "B")
        mel.connect_material_property(base_mul, "", unreal.MaterialProperty.MP_BASE_COLOR)
    else:
        mel.connect_material_property(tint, "", unreal.MaterialProperty.MP_BASE_COLOR)

    if normal:
        normal_node = sampler(normal, 200)
        normal_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        mel.connect_material_property(normal_node, "RGB", unreal.MaterialProperty.MP_NORMAL)

    if rough:
        rough_node = sampler(rough, 450)
        mel.connect_material_property(rough_node, "R", unreal.MaterialProperty.MP_ROUGHNESS)
    else:
        # No roughness map yet -> drive roughness from the MPC scalar.
        rough_param = mel.create_material_expression(mat, unreal.MaterialExpressionCollectionParameter, -400, 450)
        if collection:
            rough_param.set_editor_property("collection", collection)
            rough_param.set_editor_property("parameter_name", "Roughness")
        mel.connect_material_property(rough_param, "", unreal.MaterialProperty.MP_ROUGHNESS)

    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    unreal.log("RAND: created %s" % full_path)
    return mat


make_material("M_Road", "MPC_Road", ROAD_ALBEDO, ROAD_NORMAL, ROAD_ROUGHNESS)
make_material("M_Pavement", "MPC_Pavement", PAVEMENT_ALBEDO, PAVEMENT_NORMAL, PAVEMENT_ROUGHNESS)

unreal.log("RAND: megascans materials build complete")
