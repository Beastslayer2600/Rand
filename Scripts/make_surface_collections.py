# Creates the surface Material Parameter Collections (MPCs) for RAND.
# Run headless (or from the editor's Python console):
#   UnrealEditor-Cmd.exe RAND.uproject -ExecutePythonScript="Scripts/make_surface_collections.py"
#
# These MPCs centralise the tiling/roughness/tint of the city ground surfaces so
# that the road and pavement materials can be tuned (and their Megascans textures
# swapped) in one place once the FAB assets are downloaded:
#   MPC_Road     -> Megascans asphalt
#   MPC_Pavement -> Megascans concrete
#
# The MPCs are created once and persist; they are independent of L_TestBox, so
# regenerating the level (make_test_level.py) does not touch them. The actual
# road/pavement Materials that sample these collections + the Megascans textures
# are authored later, once those assets exist.

import unreal

COLLECTION_PATH = "/Game/RAND/Materials/Collections"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def make_collection(name, tint, tiling, roughness):
    """Create (or replace) an MPC with Tiling/Roughness scalars and a Tint vector."""
    full_path = "%s/%s" % (COLLECTION_PATH, name)
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        unreal.EditorAssetLibrary.delete_asset(full_path)

    factory = unreal.MaterialParameterCollectionFactoryNew()
    mpc = asset_tools.create_asset(name, COLLECTION_PATH,
                                   unreal.MaterialParameterCollection, factory)
    if not mpc:
        unreal.log_error("RAND: failed to create %s" % name)
        return None

    tiling_param = unreal.CollectionScalarParameter()
    tiling_param.set_editor_property("parameter_name", "Tiling")
    tiling_param.set_editor_property("default_value", tiling)

    rough_param = unreal.CollectionScalarParameter()
    rough_param.set_editor_property("parameter_name", "Roughness")
    rough_param.set_editor_property("default_value", roughness)

    tint_param = unreal.CollectionVectorParameter()
    tint_param.set_editor_property("parameter_name", "Tint")
    tint_param.set_editor_property("default_value", tint)

    mpc.set_editor_property("scalar_parameters", [tiling_param, rough_param])
    mpc.set_editor_property("vector_parameters", [tint_param])

    unreal.EditorAssetLibrary.save_loaded_asset(mpc)
    unreal.log("RAND: created %s" % full_path)
    return mpc


# Road: Megascans asphalt — dark, fairly rough, single-tile.
make_collection("MPC_Road", unreal.LinearColor(0.50, 0.50, 0.50, 1.0), 1.0, 0.85)

# Pavement: Megascans concrete — lighter, slightly smoother, tighter tiling.
make_collection("MPC_Pavement", unreal.LinearColor(0.72, 0.71, 0.68, 1.0), 2.0, 0.70)

unreal.log("RAND: surface collections build complete")
