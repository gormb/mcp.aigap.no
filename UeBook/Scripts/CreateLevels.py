"""Generates the UeBook levels.

Run headless:
  UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>" -unattended -nopause -nosplash -nullrhi
"""

import unreal

CUBE = "/Engine/BasicShapes/Cube.Cube"

BOOK_PATH = "Book/b_NO_FREE.md"

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def open_level(package_path):
    """Opens the level and clears it, creating it when it does not exist yet."""
    if unreal.EditorAssetLibrary.does_asset_exist(package_path):
        level_editor.load_level(package_path)
        for actor in actor_subsystem.get_all_level_actors():
            actor_subsystem.destroy_actor(actor)
    elif not level_editor.new_level(package_path):
        raise RuntimeError("Could not create level %s" % package_path)


def spawn(actor_class, location, rotation=(0.0, 0.0, 0.0), label=None):
    actor = actor_subsystem.spawn_actor_from_class(
        actor_class, unreal.Vector(*location), unreal.Rotator(*rotation))
    if label:
        actor.set_actor_label(label)
    return actor


def add_box(label, location, scale):
    actor = spawn(unreal.StaticMeshActor, location, label=label)
    component = actor.static_mesh_component
    component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    component.set_static_mesh(unreal.load_asset(CUBE))
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor


def add_light(label, location, intensity, color):
    light = spawn(unreal.PointLight, location, label=label)
    component = light.point_light_component
    # Movable so the level needs no baked lighting build.
    component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    component.set_editor_property("intensity", intensity)
    component.set_editor_property("light_color", unreal.Color(*color))
    component.set_editor_property("attenuation_radius", 2400.0)
    return light


def add_reader(location):
    reader = spawn(unreal.BookReaderActor, location, label="BookReader")
    reader.set_editor_property("book_path", BOOK_PATH)
    reader.set_editor_property("open_on_begin_play", True)
    return reader


def build_reading_room():
    open_level("/Game/Maps/L_Main")

    add_box("Floor", (0.0, 0.0, -10.0), (24.0, 24.0, 0.2))
    add_box("Pedestal", (0.0, 120.0, 50.0), (1.2, 1.2, 1.0))
    add_box("Book", (0.0, 120.0, 112.0), (0.9, 0.6, 0.08))

    add_light("WarmKey", (0.0, -60.0, 320.0), 9000.0, (255, 214, 170))
    add_light("CoolFill", (-500.0, 700.0, 420.0), 3000.0, (150, 180, 255))

    spawn(unreal.PlayerStart, (0.0, -420.0, 120.0), (0.0, 0.0, 0.0), label="PlayerStart")
    add_reader((0.0, 120.0, 120.0))

    level_editor.save_current_level()


def build_close_reading():
    open_level("/Game/Maps/L_Reader")

    add_box("Floor", (0.0, 0.0, -10.0), (16.0, 16.0, 0.2))
    add_box("Lectern", (0.0, 90.0, 40.0), (0.8, 0.8, 0.8))
    add_box("OpenPages", (0.0, 90.0, 92.0), (1.1, 0.8, 0.04))

    add_light("DeskLight", (0.0, 40.0, 240.0), 6000.0, (255, 226, 190))

    spawn(unreal.PlayerStart, (0.0, -220.0, 60.0), (0.0, 0.0, 0.0), label="PlayerStart")
    add_reader((0.0, 90.0, 100.0))

    level_editor.save_current_level()


if __name__ == "__main__":
    build_reading_room()
    build_close_reading()
    unreal.log("UeBook levels created: /Game/Maps/L_Main, /Game/Maps/L_Reader")
