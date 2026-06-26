# Creates ABP_Andre: an Animation Blueprint parented to URANDAnimInstance and
# targeting the Manny skeleton. The AnimGraph is left empty (outputs ref pose)
# for in-editor wiring of the BS_MM_WalkRun blendspace + jump/fall blend.
#
# Run headless:
#   UnrealEditor-Cmd.exe RAND.uproject -ExecutePythonScript="Scripts/make_anim_bp.py"

import unreal

PACKAGE_PATH = "/Game/Characters"
ASSET_NAME = "ABP_Andre"
FULL_PATH = PACKAGE_PATH + "/" + ASSET_NAME
SKELETON_PATH = "/MoverExamples/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin"

skeleton = unreal.load_object(None, SKELETON_PATH)
if not skeleton:
    unreal.log_error("RAND_ABP_CREATE: skeleton not found at %s" % SKELETON_PATH)
else:
    factory = unreal.AnimBlueprintFactory()
    factory.set_editor_property("target_skeleton", skeleton)
    factory.set_editor_property("parent_class", unreal.RANDAnimInstance)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # Idempotent: replace any prior asset so re-runs are clean.
    if unreal.EditorAssetLibrary.does_asset_exist(FULL_PATH):
        unreal.EditorAssetLibrary.delete_asset(FULL_PATH)

    abp = asset_tools.create_asset(ASSET_NAME, PACKAGE_PATH, unreal.AnimBlueprint, factory)
    if abp:
        unreal.EditorAssetLibrary.save_asset(FULL_PATH)
        cdo = unreal.get_default_object(abp.generated_class())
        unreal.log("RAND_ABP_CREATE: created %s  parented_to_URANDAnimInstance=%s  skeleton=%s" % (
            FULL_PATH, isinstance(cdo, unreal.RANDAnimInstance), skeleton.get_name()))
    else:
        unreal.log_error("RAND_ABP_CREATE: create_asset returned None")
