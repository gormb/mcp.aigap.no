"""Imports the EB Garamond source files into /Game/Fonts.

Run headless:
  UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>" -unattended -nopause -nosplash -nullrhi
"""

import os

import unreal

FONT_DIR = os.path.join(unreal.Paths.project_dir(), "Fonts", "EBGaramond")
DESTINATION = "/Game/Fonts"

FONTS = [
    "EBGaramond_Regular.ttf",
    "EBGaramond_Bold.ttf",
    "EBGaramond_Italic.ttf",
    "EBGaramond_BoldItalic.ttf",
]


def import_font(filename):
    path = os.path.join(FONT_DIR, filename)
    if not os.path.exists(path):
        unreal.log_warning("FONT missing source file: %s" % path)
        return

    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = DESTINATION
    task.destination_name = os.path.splitext(filename)[0]
    task.automated = True
    task.replace_existing = True
    task.save = True

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = "%s/%s" % (DESTINATION, task.destination_name)
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        asset = unreal.load_asset(asset_path)
        # Inline the font data so packaged builds do not depend on the source .ttf path.
        try:
            asset.set_editor_property("loading_policy", unreal.FontLoadingPolicy.INLINE)
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
        except Exception as error:
            unreal.log_warning("FONT could not set inline loading on %s: %s" % (asset_path, error))

        unreal.log("FONT imported %s as %s (policy %s)" % (
            asset_path,
            asset.get_class().get_name(),
            asset.get_editor_property("loading_policy")))
    else:
        unreal.log_error("FONT failed importing %s" % path)


if __name__ == "__main__":
    # Remove the earlier imports so the static weights replace them cleanly.
    for stale in ("EBGaramond_Regular", "EBGaramond_Italic", "EBGaramond_Bold", "EBGaramond_BoldItalic"):
        asset_path = "%s/%s" % (DESTINATION, stale)
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            unreal.EditorAssetLibrary.delete_asset(asset_path)

    for font in FONTS:
        import_font(font)
    unreal.log("FONT import finished")
    # Font import re-caches through Slate, so this must run in the full editor (-ExecutePythonScript),
    # not in a headless commandlet.
    unreal.SystemLibrary.quit_editor()
