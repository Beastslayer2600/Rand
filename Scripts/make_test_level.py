# Builds a minimal test level for verifying André's movement and NPC spawning.
# Run headless:
#   UnrealEditor-Cmd.exe RAND.uproject -ExecutePythonScript="Scripts/make_test_level.py"
#
# Creates a flat 50m x 50m floor, a PlayerStart, Joburg golden-hour lighting
# (26S sun, ~4500K, height fog), an unbound warm post-process volume, two NPC
# spawners (Hillbrow + Marshalltown), Mission 1, buyable properties, and a
# placeholder Marshalltown city block (4 buildings, road, pavements, street
# lights). The world game mode comes from GlobalDefaultGameMode (ARANDGameMode)
# in DefaultEngine.ini, so PIE spawns André automatically.
#
# City Sample buildings and Megascans road/pavement materials are wired as
# placeholders until those assets are downloaded from FAB (see CITY_SAMPLE_*
# below and Scripts/make_surface_collections.py for the surface MPCs).

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

# NavMesh bounds so ambient NPC patrol (ARANDNPCAIController) can path. Default
# NavMeshBoundsVolume is a 200cm cube; scale it to cover the 50m floor.
navbounds = spawn_class(unreal.NavMeshBoundsVolume, (0.0, 0.0, 250.0))
navbounds.set_actor_label("NavMeshBounds")
navbounds.set_actor_scale3d(unreal.Vector(30.0, 30.0, 5.0))

# Spawn André just above the floor (capsule half-height ~96).
player_start = spawn_class(unreal.PlayerStart, (0.0, 0.0, 120.0))
player_start.set_actor_label("PlayerStart")

# Joburg afternoon sun. Johannesburg is at ~26 degrees S; in late-afternoon
# golden hour the sun sits low in the north-western sky, so the directional
# light points down at a shallow ~12 degree pitch from the NW. Warmth is pinned
# to ~4500K golden-hour colour temperature via the light's temperature control.
sun = spawn_class(
    unreal.DirectionalLight,
    (0.0, 0.0, 1500.0),
    unreal.Rotator(roll=0.0, pitch=-12.0, yaw=-55.0),
)
sun.set_actor_label("Sun")
sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
if sun_comp:
    sun_comp.set_intensity(5.0)
    sun_comp.set_editor_property("use_temperature", True)
    sun_comp.set_editor_property("temperature", 4500.0)
    sun_comp.set_light_color(unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

# Sky atmosphere + skylight + height fog so the scene is lit and visible.
spawn_class(unreal.SkyAtmosphere).set_actor_label("SkyAtmosphere")
spawn_class(unreal.SkyLight, (0.0, 0.0, 300.0)).set_actor_label("SkyLight")

fog = spawn_class(unreal.ExponentialHeightFog, (0.0, 0.0, 200.0))
fog.set_actor_label("HeightFog")
fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
if fog_comp:
    # Light Joburg haze.
    fog_comp.set_editor_property("fog_density", 0.02)


def spawn_npc_spawner(label, district, location, count=5, radius=500.0):
    """Place an ARANDNPCSpawner; it populates NPCs at runtime (PIE), not now."""
    spawner = spawn_class(unreal.RANDNPCSpawner, location)
    spawner.set_actor_label(label)
    spawner.set_editor_property("district", district)
    spawner.set_editor_property("npc_class", unreal.RANDCharacter_NPC)
    spawner.set_editor_property("spawn_count", count)
    spawner.set_editor_property("spawn_radius", radius)
    return spawner


# Two district spawners on opposite sides of the box, raised so the NPC capsules
# sit above the floor (capsule half-height ~96).
spawn_npc_spawner("Spawner_Hillbrow", unreal.EDistrict.HILLBROW, (-1000.0, 0.0, 120.0))
spawn_npc_spawner("Spawner_Marshalltown", unreal.EDistrict.MARSHALL_TOWN, (1000.0, 0.0, 120.0))

# Mission 1 — The Consultation. Placed at the Marshalltown office; it spawns
# Thandi (Afrikaans Official) and the tender desk, and starts the mission.
mission = spawn_class(unreal.RANDMission_Consultation, (1400.0, 0.0, 100.0))
mission.set_actor_label("Mission_Consultation")


def spawn_property(label, name, ptype, price, rent, location):
    """Place an ARANDProperty the player can buy (R<price>)."""
    prop = spawn_class(unreal.RANDProperty, location)
    prop.set_actor_label(label)
    prop.set_editor_property("property_name", name)
    prop.set_editor_property("property_type", ptype)
    prop.set_editor_property("purchase_price", price)
    prop.set_editor_property("monthly_rental_income", rent)
    return prop


# A couple of buyable properties for testing purchase + minimap gold dots.
spawn_property("Prop_HillbrowFlats", "Hillbrow Flats",
               unreal.EPropertyType.RESIDENTIAL, 250000.0, 8000.0, (-700.0, 600.0, 200.0))
spawn_property("Prop_MarshallWarehouse", "Marshalltown Warehouse",
               unreal.EPropertyType.INDUSTRIAL, 600000.0, 22000.0, (700.0, 600.0, 200.0))

# ---------------------------------------------------------------------------
# Marshalltown city block (+X side). Placeholder geometry now; swap in City
# Sample buildings and Megascans road/pavement materials once the FAB assets are
# downloaded. Each building that fails to load falls back to a scaled cube so the
# block always builds.
# ---------------------------------------------------------------------------

# Point these at real City Sample building meshes once the FAB pack is installed.
CITY_SAMPLE_BUILDINGS = [
    # "/Game/CitySample/Buildings/.../SM_BuildingA.SM_BuildingA",
    # "/Game/CitySample/Buildings/.../SM_BuildingB.SM_BuildingB",
]

cube_mesh = unreal.load_object(None, "/Engine/BasicShapes/Cube.Cube")


def load_building_mesh(index):
    """Returns (mesh, is_placeholder) for building slot `index`."""
    if index < len(CITY_SAMPLE_BUILDINGS):
        mesh = unreal.load_object(None, CITY_SAMPLE_BUILDINGS[index])
        if mesh:
            return mesh, False
    return cube_mesh, True


def spawn_mesh(label, mesh, location, scale, rotation=None):
    rot = rotation if rotation is not None else unreal.Rotator()
    actor = actor_sub.spawn_actor_from_object(mesh, unreal.Vector(*location), rot)
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor


def spawn_building(label, index, location_xy):
    mesh, is_placeholder = load_building_mesh(index)
    if is_placeholder:
        # 100cm cube scaled to a ~6x6x18m block, sitting its base on the floor.
        spawn_mesh(label, mesh, (location_xy[0], location_xy[1], 900.0), (6.0, 6.0, 18.0))
    else:
        spawn_mesh(label, mesh, (location_xy[0], location_xy[1], 0.0), (1.0, 1.0, 1.0))


def spawn_street_light(label, location_xy):
    # Thin pole + warm point light, evoking a Joburg street lamp.
    spawn_mesh(label + "_Pole", cube_mesh, (location_xy[0], location_xy[1], 250.0),
               (0.12, 0.12, 5.0))
    light = spawn_class(unreal.PointLight, (location_xy[0], location_xy[1], 520.0))
    light.set_actor_label(label)
    light_comp = light.get_component_by_class(unreal.PointLightComponent)
    if light_comp:
        light_comp.set_intensity(4000.0)
        light_comp.set_attenuation_radius(900.0)
        light_comp.set_light_color(unreal.LinearColor(1.0, 0.85, 0.6, 1.0))


# Road surface running through the block (plane is 100cm @ scale 1 -> 12m x 4m).
spawn_mesh("Road_Marshalltown", plane_mesh, (1300.0, 0.0, 1.0), (12.0, 4.0, 1.0))

# Pavements flanking the road.
spawn_mesh("Pavement_West", plane_mesh, (1300.0, -350.0, 2.0), (12.0, 2.0, 1.0))
spawn_mesh("Pavement_East", plane_mesh, (1300.0, 350.0, 2.0), (12.0, 2.0, 1.0))

# Four buildings, two on each side of the street.
spawn_building("Building_1", 0, (1000.0, -750.0))
spawn_building("Building_2", 1, (1600.0, -750.0))
spawn_building("Building_3", 2, (1000.0, 750.0))
spawn_building("Building_4", 3, (1600.0, 750.0))

# Street lights along both pavements.
spawn_street_light("StreetLight_1", (1000.0, -350.0))
spawn_street_light("StreetLight_2", (1600.0, -350.0))
spawn_street_light("StreetLight_3", (1000.0, 350.0))
spawn_street_light("StreetLight_4", (1600.0, 350.0))

# ---------------------------------------------------------------------------
# Level-wide post process: warm Joburg grade, slight haze, cinematic contrast,
# moderate bloom. Unbound so it covers the whole level.
# ---------------------------------------------------------------------------
ppv = spawn_class(unreal.PostProcessVolume, (0.0, 0.0, 300.0))
ppv.set_actor_label("GlobalPostProcess")
ppv.set_editor_property("unbound", True)

pp = ppv.get_editor_property("settings")
# Warm golden-hour white balance.
pp.set_editor_property("override_white_temp", True)
pp.set_editor_property("white_temp", 7200.0)
# Cinematic contrast and a warm gain.
pp.set_editor_property("override_color_contrast", True)
pp.set_editor_property("color_contrast", unreal.Vector4(1.08, 1.06, 1.04, 1.0))
pp.set_editor_property("override_color_gain", True)
pp.set_editor_property("color_gain", unreal.Vector4(1.05, 1.0, 0.92, 1.0))
# Slight haze: pull a touch of saturation out.
pp.set_editor_property("override_color_saturation", True)
pp.set_editor_property("color_saturation", unreal.Vector4(0.95, 0.95, 0.95, 1.0))
# Moderate bloom.
pp.set_editor_property("override_bloom_intensity", True)
pp.set_editor_property("bloom_intensity", 0.7)
ppv.set_editor_property("settings", pp)

# Persist the level to disk.
saved = level_sub.save_current_level()
unreal.log("RAND: test level build complete (%s), saved=%s" % (MAP_PACKAGE, saved))
