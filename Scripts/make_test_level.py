# Builds a minimal test level for verifying André's movement.
# Run headless:
#   UnrealEditor-Cmd.exe RAND.uproject -ExecutePythonScript="Scripts/make_test_level.py"
#
# Creates a flat 50m x 50m floor, a PlayerStart, and Joburg-afternoon lighting.
# The world game mode comes from GlobalDefaultGameMode (ARANDGameMode) set in
# DefaultEngine.ini, so PIE spawns André automatically.

import unreal

MAP_PACKAGE = "/Game/Maps/L_TestBox"

level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def spawn_class(actor_class, location=(0.0, 0.0, 0.0), rotation=None):
    rot = rotation if rotation is not None else unreal.Rotator()
    return actor_sub.spawn_actor_from_class(actor_class, unreal.Vector(*location), rot)


# Fresh empty level (becomes the current level).
level_sub.new_level(MAP_PACKAGE)

# Floor: engine basic plane (1m) scaled to 50m x 50m.
plane_mesh = unreal.load_object(None, "/Engine/BasicShapes/Plane.Plane")
floor = actor_sub.spawn_actor_from_object(plane_mesh, unreal.Vector(0.0, 0.0, 0.0))
floor.set_actor_label("Floor")
floor.set_actor_scale3d(unreal.Vector(50.0, 50.0, 1.0))

# Spawn André just above the floor (capsule half-height ~96).
player_start = spawn_class(unreal.PlayerStart, (0.0, 0.0, 120.0))
player_start.set_actor_label("PlayerStart")

# Joburg afternoon sun: warm directional light, low-ish pitch.
sun = spawn_class(
    unreal.DirectionalLight,
    (0.0, 0.0, 1500.0),
    unreal.Rotator(roll=0.0, pitch=-38.0, yaw=-55.0),
)
sun.set_actor_label("Sun")
sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
if sun_comp:
    sun_comp.set_intensity(6.0)
    sun_comp.set_light_color(unreal.LinearColor(1.0, 0.92, 0.78, 1.0))

# Sky atmosphere + skylight + height fog so the scene is lit and visible.
spawn_class(unreal.SkyAtmosphere).set_actor_label("SkyAtmosphere")
spawn_class(unreal.SkyLight, (0.0, 0.0, 300.0)).set_actor_label("SkyLight")
spawn_class(unreal.ExponentialHeightFog, (0.0, 0.0, 200.0)).set_actor_label("HeightFog")

# Persist the level to disk.
saved = level_sub.save_current_level()
unreal.log("RAND: test level build complete (%s), saved=%s" % (MAP_PACKAGE, saved))
