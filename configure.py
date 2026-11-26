#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "R2PE9B",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.3"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-enc SJIS",
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

# Game code flags. Evidence: extab entries for game functions => C++ exceptions on;
# -O4 with scheduling reproduces the matched layout functions, -O3/-O2/noschedule do not.
cflags_game = [
    *cflags_base,
    "-Cpp_exceptions on",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

config.linker_version = "Wii/1.0"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "game",
        # GameCube-generation CodeWarrior: it beat every Wii version on 20 of 30 test units
        # (GC 3.0a3 to 3.0a5.2 give identical code; 3.0a5.2 is also the SDK's compiler).
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_game,
        "progress_category": "game",
        "objects": [
            Object(NonMatching, "game/ui/layout_elements_and_hit_testing.cpp"),
            Object(NonMatching, "game/chara/pet_texture_manager_part2.cpp"),
            Object(Equivalent, "game/ui/shop2_main_task.cpp"),
            Object(Equivalent, "game/ui/model_operation_exhibition_preview.cpp"),
            Object(Equivalent, "game/ui/chatmsg_singleton_teardown.cpp"),
            Object(NonMatching, "game/golf/club_customization_table_lookup.cpp"),
            Object(Equivalent, "game/system/directinput_keyboard_adapter.cpp"),
            Object(Equivalent, "game/chara/wpuppet_mesh_bounds.cpp"),
            Object(NonMatching, "game/system/std_string_and_shared_ptr_deleter.cpp"),
            Object(NonMatching, "game/ui/com_select_stroke_screen.cpp"),
            Object(NonMatching, "game/system/string_map_tree_utils.cpp"),
            Object(NonMatching, "engine/gtl/cfileio_flash_file_operations.cpp"),
            Object(NonMatching, "game/golf/course_object_and_selection.cpp"),
            Object(Equivalent, "game/chara/pet_animation.cpp"),
            Object(Equivalent, "game/system/pad_motion_history.cpp"),
            Object(NonMatching, "game/system/generic3d_pool_registry.cpp"),
            Object(Equivalent, "game/system/bp_ctrl_entry_manager.cpp"),
            Object(NonMatching, "game/save/shared_doc_player_state_and_course.cpp"),
            Object(NonMatching, "game/system/task_manager_and_actors.cpp"),
            Object(NonMatching, "game/golf/drive_line_shot_prediction.cpp"),
            Object(NonMatching, "game/ui/bgm_collection_screen_part1.cpp"),
            Object(NonMatching, "game/ui/font_atlas_and_mwstring.cpp"),
            Object(NonMatching, "game/golf/golf_cup_preview_capture.cpp"),
            Object(NonMatching, "game/ui/name_entry_soft_keyboard.cpp"),
            Object(NonMatching, "game/fx/tour_load_effect_particles.cpp"),
            Object(NonMatching, "engine/gtl/render_packet_vertex_buffers.cpp"),
            Object(NonMatching, "game/save/save_banner_text_sections.cpp"),
            Object(NonMatching, "game/system/tinyxml_parser.cpp"),
            Object(NonMatching, "game/modes/party_item_select.cpp"),
            Object(NonMatching, "game/system/check_drive_status_dialog.cpp"),
            Object(NonMatching, "game/golf/course_model_scene_loader.cpp"),
            Object(NonMatching, "game/ui/bp_object_sprite_resources.cpp"),
            Object(NonMatching, "game/system/game_input_manager_cursor_rumble.cpp"),
            Object(NonMatching, "game/ui/loading_screen_b2d_player.cpp"),
            Object(NonMatching, "game/ui/course_select_menu.cpp"),
            Object(Equivalent, "game/system/wstring_array_utils.cpp"),
            Object(Equivalent, "game/system/owned_allocation_list_cleanup.cpp"),
            Object(Equivalent, "engine/gtl/engine_render_state_restore_dtors.cpp"),
            Object(NonMatching, "game/chara/puppet_resource_name_refcounts.cpp"),
            Object(NonMatching, "game/ui/overlay_sprite_drawing.cpp"),
            Object(NonMatching, "game/modes/story_conte_cutscene.cpp"),
            Object(NonMatching, "game/save/save_data_records_and_defaults.cpp"),
            Object(NonMatching, "game/ui/msgdraw_text_rendering.cpp"),
            Object(NonMatching, "game/modes/dolfini_round_main.cpp"),
            Object(NonMatching, "game/ui/wii_pause_score_screen.cpp"),
            Object(NonMatching, "game/system/resource_manager_cache_and_loading.cpp"),
            Object(NonMatching, "game/modes/tutorial_command_queue.cpp"),
            Object(NonMatching, "game/modes/tour_character_map_board.cpp"),
            Object(NonMatching, "game/save/save_rewards_item_unlock_flags.cpp"),
            Object(NonMatching, "sdk/mw_string_stream_and_text_tags.cpp"),
            Object(NonMatching, "game/audio/sound_manager.cpp"),
            Object(NonMatching, "game/modes/event_conte_script_interpreter.cpp"),
            Object(NonMatching, "game/chara/wpuppet_model_bone_animation.cpp"),
            Object(NonMatching, "game/system/heap_alloc_and_operator_new.cpp"),
            Object(NonMatching, "game/system/memory_heap_init_and_alloc.cpp"),
            Object(NonMatching, "game/system/gsr_resource_bitmap_and_heap.cpp"),
            Object(Equivalent, "game/chara/pet_animation_init_and_dtors.cpp"),
            Object(NonMatching, "game/chara/pet_texture_manager_part1.cpp"),
            Object(NonMatching, "game/chara/puppet_bone_nodes.cpp"),
            Object(NonMatching, "game/system/resource_path_and_file_utils.cpp"),
            Object(NonMatching, "game/system/raw_file_stream_and_matrix_utils.cpp"),
            Object(NonMatching, "game/system/wvector_affine_matrix_math.cpp"),
            Object(NonMatching, "game/system/math_matrix_quat_vec_utils.cpp"),
            Object(Matching, "game/system/woverlay_resource_ctor.cpp"),
            Object(NonMatching, "game/chara/puppet_construction_and_init.cpp"),
            Object(NonMatching, "sdk/rbtree_rebalance.cpp"),
            Object(NonMatching, "game/golf/course_model_part_samples_bounds.cpp"),
            Object(NonMatching, "game/system/shared_resource_loader.cpp"),
            Object(Matching, "game/system/string_storage_and_container_utils.cpp"),
            Object(NonMatching, "game/system/wview_camera_projection.cpp"),
            Object(NonMatching, "game/system/endian_format_and_binary_table.cpp"),
            Object(Matching, "game/modes/pangya_enemy_data_stub_vfuncs.cpp"),
            Object(NonMatching, "game/modes/party_balloon_system.cpp"),
            Object(NonMatching, "game/fx/balloon_system.cpp"),
            Object(NonMatching, "game/save/save_data_records_and_inventory_init.cpp"),
            Object(Equivalent, "game/ui/layout_cursor_pointer_controller.cpp"),
            Object(NonMatching, "game/system/name_entry_and_misc_utilities.cpp"),
            Object(NonMatching, "game/system/shared_texture_resource.cpp"),
            Object(NonMatching, "game/ui/shop2_task_and_soft_keyboard.cpp"),
            Object(NonMatching, "game/system/wgeneric3d_render_queue_and_soft_keyboard.cpp"),
            Object(NonMatching, "engine/gtl/wgeneric3dr_renderer_core.cpp"),
            Object(NonMatching, "engine/gtl/wgeneric3dr_immediate_renderer.cpp"),
            Object(NonMatching, "game/ui/party_item_select_ui.cpp"),
            Object(NonMatching, "game/modes/party_main_task_and_phf_helpers.cpp"),
            Object(NonMatching, "game/modes/story_event_script_shared_state.cpp"),
            Object(NonMatching, "game/modes/event_conte_script_commands.cpp"),
            Object(NonMatching, "game/modes/story_message_event_load.cpp"),
            Object(NonMatching, "game/modes/story_message_window.cpp"),
            Object(NonMatching, "game/modes/story_event_message_commands.cpp"),
            Object(NonMatching, "game/modes/party_goods_item_display.cpp"),
            Object(NonMatching, "game/ui/font_atlas_page_loading.cpp"),
            Object(NonMatching, "game/ui/msgdraw_text_reveal_and_pair_vector.cpp"),
            Object(NonMatching, "game/ui/text_format_trees_and_msg_resources.cpp"),
            Object(NonMatching, "game/ui/bp_sprite_animation_and_draw_callbacks.cpp"),
            Object(NonMatching, "game/ui/bp_layout_animation_and_sprite_draw.cpp"),
            Object(NonMatching, "game/ui/bp_sprite_controller_and_drawing.cpp"),
            Object(NonMatching, "game/ui/bp_sprite_script_commands.cpp"),
            Object(NonMatching, "game/ui/b2d_file_scripts_and_input.cpp"),
            Object(NonMatching, "game/ui/b2d_sprite_and_bp_texture_draw.cpp"),
            Object(NonMatching, "game/ui/ui_group_and_bp_edit_sprite.cpp"),
            Object(Equivalent, "game/system/bp_object_and_loader_teardown.cpp"),
            Object(NonMatching, "game/ui/bp_sprite_pool_and_texture_binding.cpp"),
            Object(NonMatching, "game/system/spd_sprite_manager.cpp"),
            Object(NonMatching, "game/system/task_registry_actor_and_debug_text.cpp"),
            Object(NonMatching, "game/golf/ball_actor_and_ambient_sound.cpp"),
            Object(NonMatching, "game/system/token_parser_and_bubble.cpp"),
            Object(Matching, "game/golf/camera_input_and_ground_view.cpp"),
            Object(NonMatching, "game/golf/camera_movement_and_input.cpp"),
            Object(NonMatching, "game/golf/camera_work_aliases_and_containers.cpp"),
            Object(NonMatching, "game/golf/camera_work_string_maps.cpp"),
            Object(NonMatching, "game/golf/camera_work_modes.cpp"),
            Object(NonMatching, "game/golf/camera_work_shot_cameras.cpp"),
            Object(NonMatching, "game/golf/camera_resource_groups.cpp"),
            Object(NonMatching, "game/golf/camera_work_resource_trees.cpp"),
            Object(NonMatching, "game/golf/camera_work_animation_resources.cpp"),
            Object(NonMatching, "game/system/resource_group_tree_tables.cpp"),
            Object(NonMatching, "game/system/string_map_tree_and_length_error.cpp"),
            Object(NonMatching, "game/system/camera_string_map_containers.cpp"),
            Object(NonMatching, "game/ui/clock_hud_and_camera_maps.cpp"),
            Object(NonMatching, "game/golf/club_selection_and_shot_params.cpp"),
            Object(NonMatching, "game/fx/club_trail_and_ring_effects.cpp"),
            Object(NonMatching, "game/golf/spot_marker_actor.cpp"),
            Object(NonMatching, "game/golf/spot_marker_draw_and_messages.cpp"),
            Object(NonMatching, "game/ui/golf_hud_spot_markers_and_fonts.cpp"),
            Object(NonMatching, "game/ui/hud_distance_to_pin_and_cup_info.cpp"),
            Object(NonMatching, "game/ui/fly_info_hud.cpp"),
            Object(NonMatching, "game/golf/golf_hud_flight_info_and_pang_rewards.cpp"),
            Object(NonMatching, "game/ui/golf_hud_hole_info_and_fonts.cpp"),
            Object(NonMatching, "game/fx/pvs_fx_box_attachment_cache.cpp"),
            Object(NonMatching, "game/fx/cfx_shot_effects.cpp"),
            Object(NonMatching, "game/fx/fx_shot_effects_cobra_tomahawk.cpp"),
            Object(NonMatching, "game/fx/fx_shot_effects_tomahawk_spike.cpp"),
            Object(NonMatching, "game/fx/fx_box_effect_trees.cpp"),
            Object(NonMatching, "game/fx/fx_box_resource_caches.cpp"),
            Object(NonMatching, "game/fx/fx_box_spray_sequence_registry.cpp"),
            Object(NonMatching, "game/fx/fx_box_interpreter_resources.cpp"),
            Object(NonMatching, "game/fx/fx_sequence_scripts_and_attachments.cpp"),
            Object(NonMatching, "game/fx/fx_sequence_script_parser.cpp"),
            Object(NonMatching, "game/fx/fx_particle_and_spray.cpp"),
            Object(NonMatching, "game/fx/fx_spray_list_and_loading.cpp"),
            Object(NonMatching, "game/fx/fx_spray_config_assign.cpp"),
            Object(NonMatching, "game/fx/fx_spray_track_list_assign_part1.cpp"),
            Object(NonMatching, "game/fx/fx_spray_track_list_assign_part2.cpp"),
            Object(NonMatching, "game/system/list_assign_templates.cpp"),
            Object(NonMatching, "game/system/string_and_name_map_utils.cpp"),
            Object(NonMatching, "game/fx/fx_spray_script_parser.cpp"),
            Object(NonMatching, "game/fx/fx_spray_particles.cpp"),
            Object(NonMatching, "game/golf/golf_ball_predata_and_gallery.cpp"),
            Object(NonMatching, "game/golf/golf_bg_hole_resources.cpp"),
            Object(NonMatching, "game/golf/course_entry_vectors_and_doc_accessors.cpp"),
            Object(NonMatching, "game/golf/golf_background_load.cpp"),
            Object(NonMatching, "game/golf/golf_bg_course_reload_and_camera.cpp"),
            Object(NonMatching, "game/golf/golf_bg_course_paths_and_caddie.cpp"),
            Object(NonMatching, "game/golf/golf_caddie_load_and_cleanup.cpp"),
            Object(NonMatching, "game/golf/golf_caddie_dialogue_entries.cpp"),
            Object(NonMatching, "game/golf/golf_caddie_animation_process.cpp"),
            Object(NonMatching, "game/golf/caddie_situational_commentary.cpp"),
            Object(NonMatching, "game/golf/golf_caddie_commentary_and_draw.cpp"),
            Object(NonMatching, "game/golf/golf_caddie_messages.cpp"),
            Object(NonMatching, "game/chara/golf_caddie_model_and_reactions.cpp"),
            Object(NonMatching, "game/golf/golf_caddie_dialogue_strings.cpp"),
            Object(NonMatching, "game/golf/golf_doc_turns_and_parameter_xml.cpp"),
            Object(NonMatching, "game/system/string_tree_and_vector_utils.cpp"),
            Object(NonMatching, "game/golf/terrain_declaration_vector.cpp"),
            Object(NonMatching, "game/golf/golf_player_lifecycle.cpp"),
            Object(NonMatching, "game/golf/golf_player_actor.cpp"),
            Object(NonMatching, "game/chara/golf_player_animation_and_model_loads.cpp"),
            Object(NonMatching, "game/chara/golf_player_script_animation.cpp"),
            Object(NonMatching, "game/golf/golf_player_rtti_and_stub_hooks.cpp"),
            Object(NonMatching, "game/golf/golf_rule_round_flow_and_shot_sim.cpp"),
            Object(NonMatching, "game/golf/golf_task_lifecycle.cpp"),
            Object(NonMatching, "game/system/golf_task_loading_and_pause.cpp"),
            Object(NonMatching, "game/golf/golf_grid_layout_and_task.cpp"),
            Object(NonMatching, "game/ui/screen_ball_preview.cpp"),
            Object(NonMatching, "game/golf/ball_preview_and_guide_line.cpp"),
            Object(NonMatching, "game/golf/guide_line_trajectory.cpp"),
            Object(NonMatching, "game/ui/ingame_hud_screen.cpp"),
            Object(NonMatching, "game/ui/head_icon_and_aux_menu_ui.cpp"),
            Object(NonMatching, "game/ui/head_icon_and_help_notice.cpp"),
            Object(NonMatching, "game/system/input_manager_and_notice_queue.cpp"),
            Object(NonMatching, "game/system/input_manager_and_item_manager_lifetime.cpp"),
            Object(NonMatching, "game/system/item_inventory_iff_loading.cpp"),
            Object(NonMatching, "game/golf/item_inventory_tables.cpp"),
            Object(NonMatching, "game/save/iff_item_inventory_table_loaders.cpp"),
            Object(NonMatching, "game/save/item_inventory_iff_table_readers.cpp"),
            Object(NonMatching, "game/chara/item_inventory_iff_tables.cpp"),
            Object(NonMatching, "game/chara/item_inventory_lookup.cpp"),
            Object(NonMatching, "game/ui/item_window_hud.cpp"),
            Object(NonMatching, "game/fx/sun_rays_lens_flare_effects.cpp"),
            Object(NonMatching, "game/ui/golf_letter_overlays.cpp"),
            Object(NonMatching, "game/system/main_frame_letter_and_mouse_cursor.cpp"),
            Object(NonMatching, "game/ui/mouse_cursor_hit_testing.cpp"),
            Object(NonMatching, "game/fx/npc_creatures_course_objects.cpp"),
            Object(NonMatching, "game/chara/creature_puppet_loading.cpp"),
            Object(NonMatching, "game/chara/creature_path_movement_and_draw.cpp"),
            Object(NonMatching, "game/fx/course_creatures_flocks_and_flyers.cpp"),
            Object(NonMatching, "game/chara/npc_pola_ship_and_penguin.cpp"),
            Object(NonMatching, "game/chara/npc_pola_penguin.cpp"),
            Object(NonMatching, "game/chara/npc_creatures_and_spline.cpp"),
            Object(NonMatching, "game/fx/creature_dtor_thunks_and_marker.cpp"),
            Object(NonMatching, "game/fx/particle_effect_pool.cpp"),
            Object(NonMatching, "game/fx/particle_effect_emitters.cpp"),
            Object(NonMatching, "game/fx/course_ball_shot_effects.cpp"),
            Object(NonMatching, "game/chara/player_pet_model_resources.cpp"),
            Object(NonMatching, "game/chara/avatar_part_puppet_outfit.cpp"),
            Object(NonMatching, "game/fx/petal_silica_particles.cpp"),
            Object(NonMatching, "game/chara/pet_body_animation_and_movement.cpp"),
            Object(NonMatching, "game/golf/course_data_and_polysoup_cleanup.cpp"),
            Object(NonMatching, "game/system/record_vector_and_state_utils.cpp"),
            Object(NonMatching, "game/golf/course_node_records_and_containers.cpp"),
            Object(NonMatching, "game/golf/course_model_name_registry.cpp"),
            Object(NonMatching, "game/golf/course_data_geometry_and_surfaces.cpp"),
            Object(NonMatching, "game/golf/course_surface_sbin_loading.cpp"),
            Object(NonMatching, "game/golf/course_surface_triangle_mappings.cpp"),
            Object(NonMatching, "game/fx/water_caustic_gene_tex3d.cpp"),
            Object(NonMatching, "game/fx/course_water_surface_rendering.cpp"),
            Object(NonMatching, "game/system/project_base_and_grid_surface.cpp"),
            Object(NonMatching, "game/system/project_g_app_and_devices.cpp"),
            Object(NonMatching, "game/golf/quadtree_terrain_collision.cpp"),
            Object(NonMatching, "game/golf/course_collision_and_ball_physics.cpp"),
            Object(NonMatching, "game/fx/rain_effect_and_random_table.cpp"),
            Object(NonMatching, "game/golf/com_shot_cpu_rival_ai.cpp"),
            Object(NonMatching, "game/fx/river_lava_texture_layers.cpp"),
            Object(NonMatching, "game/fx/river_lava_layout.cpp"),
            Object(NonMatching, "game/fx/scene_manager_pools_and_render_lists.cpp"),
            Object(Equivalent, "game/fx/draw_scene_named_effect_list.cpp"),
            Object(NonMatching, "game/golf/course_scene_render_and_geometry.cpp"),
            Object(NonMatching, "game/golf/score_actor_and_generated_surfaces.cpp"),
            Object(NonMatching, "game/fx/texture_capture_queue_and_mesh.cpp"),
            Object(NonMatching, "game/fx/shadow_manager.cpp"),
            Object(NonMatching, "game/fx/shadow_manager_capture.cpp"),
            Object(NonMatching, "game/fx/shadow_model_cache_and_shared_doc.cpp"),
            Object(NonMatching, "game/system/shared_doc_singleton.cpp"),
            Object(NonMatching, "game/fx/pvsfxbox_sky_and_wind_particles.cpp"),
            Object(NonMatching, "game/fx/snow_weather_particles.cpp"),
            Object(NonMatching, "game/fx/tail_effect_spin_and_bubble.cpp"),
            Object(NonMatching, "game/system/task_actor_loading.cpp"),
            Object(NonMatching, "game/system/ctask_transition_command_queue.cpp"),
            Object(NonMatching, "game/system/title_task_and_ring_buffer.cpp"),
            Object(NonMatching, "game/fx/ball_trajectory_effects.cpp"),
            Object(NonMatching, "game/fx/skidmark_ball.cpp"),
            Object(NonMatching, "game/modes/tutorial_mode_ctutorial.cpp"),
            Object(NonMatching, "game/modes/tutorial_commands_and_input.cpp"),
            Object(NonMatching, "game/fx/beach_wavelet_ground_strip.cpp"),
            Object(NonMatching, "game/fx/wind_indicator_wavelet_cloud_overlay.cpp"),
            Object(NonMatching, "game/golf/wind_system.cpp"),
            Object(NonMatching, "game/golf/wind_regions_and_vectors.cpp"),
            Object(NonMatching, "game/system/game_main_loop_and_startup.cpp"),
            Object(Equivalent, "game/modes/mode_task_factory_creators.cpp"),
            Object(NonMatching, "game/system/engine_startup_and_ani_sprite.cpp"),
            Object(NonMatching, "game/ui/font_markup_text_rendering.cpp"),
            Object(NonMatching, "game/ui/title_font_atlas_and_fixed_font.cpp"),
            Object(NonMatching, "game/chara/pet_mesh_and_fixed_font.cpp"),
            Object(NonMatching, "engine/gtl/w3d_sprite_and_billboard.cpp"),
            Object(NonMatching, "game/modes/event_script_model_commands.cpp"),
            Object(NonMatching, "game/system/event_fx_sound_and_pad_swing_accele.cpp"),
            Object(NonMatching, "game/golf/pad_swing_ctrl_motion.cpp"),
            Object(NonMatching, "game/golf/pad_swing_ctrl_and_com_think.cpp"),
            Object(NonMatching, "game/golf/com_think_ctrl_conditions.cpp"),
            Object(NonMatching, "game/golf/com_think_ctrl_and_shot_pos.cpp"),
            Object(NonMatching, "game/golf/com_think_shot_pos_candidates.cpp"),
            Object(NonMatching, "game/ui/shop_product_set_items.cpp"),
            Object(NonMatching, "game/ui/shop_product_lists_part1.cpp"),
            Object(NonMatching, "game/ui/shop_product_lists_part2.cpp"),
            Object(NonMatching, "game/ui/shop_product_caddie_lists.cpp"),
            Object(NonMatching, "game/ui/shop_product_records.cpp"),
            Object(NonMatching, "game/ui/shop_product_ring_and_equip_lists.cpp"),
            Object(NonMatching, "game/ui/shop_product_categories.cpp"),
            Object(NonMatching, "game/fx/effect_fade_and_camera_script.cpp"),
            Object(NonMatching, "game/system/vcg_motion_player_and_fade.cpp"),
            Object(NonMatching, "game/system/fade_overlay_and_shared_file.cpp"),
            Object(NonMatching, "game/system/shared_file_loading.cpp"),
            Object(NonMatching, "game/ui/event_face_and_mpbase1_menu.cpp"),
            Object(NonMatching, "game/ui/chara_select_screen_part1.cpp"),
            Object(NonMatching, "game/ui/chara_select_screen_part2.cpp"),
            Object(NonMatching, "game/ui/chara_select_screen_part3.cpp"),
            Object(NonMatching, "game/chara/character_preview_avatars.cpp"),
            Object(NonMatching, "game/modes/chara_num_select_and_training_main.cpp"),
            Object(NonMatching, "game/modes/training_and_single_main_tasks.cpp"),
            Object(NonMatching, "game/chara/chara_entry_ctor_dtor.cpp"),
            Object(NonMatching, "game/modes/party_character_entry_menu.cpp"),
            Object(NonMatching, "game/ui/chara_entry_screen_part1.cpp"),
            Object(NonMatching, "game/ui/chara_entry_screen_part2.cpp"),
            Object(NonMatching, "game/chara/chara_entry_render_context.cpp"),
            Object(NonMatching, "game/golf/computer_player_random_and_stroke.cpp"),
            Object(NonMatching, "game/ui/font_num_draw_entries.cpp"),
            Object(NonMatching, "game/ui/font_num_draw_text_table.cpp"),
            Object(NonMatching, "game/system/font_registry_and_round_setup_mixed.cpp"),
            Object(NonMatching, "game/ui/in_game_dialog_and_snapshot_copy.cpp"),
            Object(NonMatching, "game/ui/ingame_dialog_and_fade_dialog.cpp"),
            Object(NonMatching, "game/ui/golf_tips_and_tutorial_dialogs.cpp"),
            Object(NonMatching, "game/modes/party_item_gameplay_hooks.cpp"),
            Object(NonMatching, "game/system/blocked_word_ring_buffer.cpp"),
            Object(NonMatching, "game/audio/movie_audio_mixing_and_playback.cpp"),
            Object(NonMatching, "game/ui/ingame_tutorial_dialog.cpp"),
            Object(NonMatching, "game/system/pad_sample_history_buffer.cpp"),
            Object(NonMatching, "game/chara/async_exhibition_and_pet_format.cpp"),
            Object(NonMatching, "game/chara/async_exhibition_avatar_preview.cpp"),
            Object(NonMatching, "game/ui/warning_logo_screens.cpp"),
            Object(NonMatching, "game/chara/preload_avatar_wpuppet_async.cpp"),
            Object(NonMatching, "game/chara/preload_avatar_async_queue.cpp"),
            Object(NonMatching, "game/system/preload_file_parts_async_loaders.cpp"),
            Object(NonMatching, "game/system/preload_seq_spr_async.cpp"),
            Object(NonMatching, "game/chara/preload_wpuppet.cpp"),
            Object(NonMatching, "game/system/preload_file_and_sequence.cpp"),
            Object(NonMatching, "game/save/pad_memory_menu_save.cpp"),
            Object(NonMatching, "game/save/pad_memory_menu_load.cpp"),
            Object(NonMatching, "game/save/file_io_pad_and_flash_io.cpp"),
            Object(Equivalent, "game/save/file_io_access_read_write.cpp"),
            Object(NonMatching, "engine/gtl/file_io_flash_and_write_access.cpp"),
            Object(NonMatching, "game/fx/draw_drive_line.cpp"),
            Object(NonMatching, "game/chara/pet_preload_async.cpp"),
            Object(NonMatching, "game/ui/common_dialog_yes_no.cpp"),
            Object(NonMatching, "game/save/flash_memory_menu_save.cpp"),
            Object(NonMatching, "game/save/flash_memory_menu_load_part1.cpp"),
            Object(NonMatching, "game/save/flash_memory_menu_load_part2.cpp"),
            Object(NonMatching, "game/save/file_io_menu_save_load_base.cpp"),
            Object(NonMatching, "game/save/save_form_records_and_flash_io.cpp"),
            Object(NonMatching, "game/save/save_process_main_task.cpp"),
            Object(NonMatching, "game/modes/party_main_and_versus_main.cpp"),
            Object(NonMatching, "game/modes/party_main_render_context.cpp"),
            Object(NonMatching, "game/ui/pop_item_dialog_and_loading_draw.cpp"),
            Object(NonMatching, "game/ui/loading_draw_tips_and_background.cpp"),
            Object(NonMatching, "game/ui/small_common_dialog.cpp"),
            Object(NonMatching, "game/system/home_button_menu.cpp"),
            Object(NonMatching, "game/system/shared_file_registry.cpp"),
            Object(NonMatching, "sdk/des_crypt_and_angle_period_init.cpp"),
            Object(NonMatching, "game/system/password_des_base64_texture_preload.cpp"),
            Object(NonMatching, "game/system/pack_texture_resource_loading.cpp"),
            Object(NonMatching, "game/system/system_reset_and_callbacks.cpp"),
            Object(NonMatching, "game/ui/movie_controller_and_banner.cpp"),
            Object(NonMatching, "game/save/save_banner_image_setup.cpp"),
            Object(NonMatching, "game/ui/player_info_screen.cpp"),
            Object(NonMatching, "game/golf/unplayable_ball_drop_sequence.cpp"),
            Object(NonMatching, "game/system/packed_file_loader_and_exhibition_pos.cpp"),
            Object(NonMatching, "game/system/md5_and_disk_error_dialog.cpp"),
            Object(Matching, "game/ui/chat_msg_interface.cpp"),
            Object(Matching, "game/system/actor_factory_registry.cpp"),
            Object(NonMatching, "game/system/direct_input_device_factory.cpp"),
            Object(NonMatching, "game/system/directinput_mouse_keyboard.cpp"),
            Object(NonMatching, "game/system/tinyxml_node_element_string.cpp"),
            Object(NonMatching, "game/system/tinyxml_print_and_clone.cpp"),
            Object(NonMatching, "game/chara/actor_animation_commands_and_attachments.cpp"),
            Object(NonMatching, "game/audio/sound_playback_state_slots.cpp"),
            Object(NonMatching, "game/audio/sound_channel_group_reset.cpp"),
            Object(NonMatching, "game/ui/exhibition_model_preview.cpp"),
            Object(Equivalent, "game/golf/club_custom_table.cpp"),
            Object(NonMatching, "game/ui/title_top_and_multiplay_menu.cpp"),
            Object(NonMatching, "game/ui/top_submenu_and_multiplay_menu.cpp"),
            Object(NonMatching, "game/ui/title_main_and_top_menu.cpp"),
            Object(NonMatching, "game/ui/shop_products_tab_and_top_menu.cpp"),
            Object(NonMatching, "game/ui/pangya2_shop_screen.cpp"),
            Object(NonMatching, "game/ui/clubhouse_shop_screen.cpp"),
            Object(NonMatching, "game/ui/pangya2_shop_product_screen.cpp"),
            Object(NonMatching, "game/ui/shop_product_selection_and_purchase.cpp"),
            Object(NonMatching, "game/ui/pangya2_shop_purchase.cpp"),
            Object(NonMatching, "game/ui/pangya2_shop_item_selection.cpp"),
            Object(NonMatching, "game/ui/shop_outfit_unowned_markers.cpp"),
            Object(NonMatching, "game/ui/shop_screen_selection_and_ownership.cpp"),
            Object(NonMatching, "game/ui/top_menu_base_and_menu_message.cpp"),
            Object(NonMatching, "sdk/zlib_inflate_codes.cpp"),
            Object(NonMatching, "sdk/zlib_inflate_blocks.cpp"),
            Object(NonMatching, "sdk/zlib_inflate_and_checksums.cpp"),
            Object(NonMatching, "game/system/zip_archive_directory_search.cpp"),
            Object(NonMatching, "engine/gtl/zip_central_directory_reader.cpp"),
            Object(NonMatching, "game/system/zip_archive_reader.cpp"),
            Object(NonMatching, "game/system/archive_zip_reader_and_game_hooks.cpp"),
            Object(NonMatching, "game/ui/shop_product_list_builder.cpp"),
            Object(NonMatching, "game/ui/shop_model_preview_and_icons.cpp"),
            Object(NonMatching, "game/chara/model_operation_chara_caddie.cpp"),
            Object(NonMatching, "game/ui/item_info_and_clubhouse_msg.cpp"),
            Object(NonMatching, "game/ui/shop_purchase_dialog_and_categories.cpp"),
            Object(NonMatching, "game/ui/club_dialog_and_item_cart.cpp"),
            Object(NonMatching, "game/ui/club_lvup_dialog_and_shop_submenus.cpp"),
            Object(NonMatching, "game/ui/club_submenu_and_lvup_dialog.cpp"),
            Object(NonMatching, "game/ui/player_status_hud.cpp"),
            Object(NonMatching, "game/modes/darts_system.cpp"),
            Object(NonMatching, "game/modes/drive_contest_and_darts.cpp"),
            Object(NonMatching, "game/modes/drive_system_distance_ranks.cpp"),
            Object(NonMatching, "game/audio/voice_manager_candidates.cpp"),
            Object(NonMatching, "game/ui/stroke_point_2d_selector.cpp"),
            Object(NonMatching, "game/ui/stroke_point_2d_dialog.cpp"),
            Object(NonMatching, "game/ui/map_camera_2d_panel.cpp"),
            Object(NonMatching, "game/ui/option_dialog_control_mode_audio.cpp"),
            Object(NonMatching, "game/modes/tour_map_preview_tree.cpp"),
            Object(Matching, "game/golf/map_data_default_virtuals.cpp"),
            Object(NonMatching, "game/modes/tour_board_preview_controller.cpp"),
            Object(NonMatching, "game/modes/tour_task_item_map_and_model_preview.cpp"),
            Object(NonMatching, "game/modes/tour_vs_window_part1.cpp"),
            Object(NonMatching, "game/modes/tour_vs_window_part2.cpp"),
            Object(NonMatching, "game/modes/tour_area_select.cpp"),
            Object(NonMatching, "game/modes/tour_area_selection_menu.cpp"),
            Object(NonMatching, "game/modes/tour_change_character_window.cpp"),
            Object(NonMatching, "game/modes/tour_hint_window_and_fade.cpp"),
            Object(NonMatching, "game/modes/tour_hint_message_window.cpp"),
            Object(NonMatching, "game/modes/tour_hint_window.cpp"),
            Object(NonMatching, "game/save/item_request_state.cpp"),
            Object(NonMatching, "game/modes/tour_main_task.cpp"),
            Object(NonMatching, "game/ui/ball_wind_2d_hud.cpp"),
            Object(NonMatching, "game/modes/round_dol3d_spinning_dolfini_scene.cpp"),
            Object(NonMatching, "game/modes/round_dol_gurudol_minigame.cpp"),
            Object(NonMatching, "game/ui/golf_base_2d_hud_layout.cpp"),
            Object(Matching, "game/audio/bgm_collection_dtor.cpp"),
            Object(NonMatching, "game/ui/bgm_collection_screen_part2.cpp"),
            Object(NonMatching, "game/ui/course_and_hole_select_screens.cpp"),
            Object(NonMatching, "game/ui/swing_result_hud_and_equipped_stats.cpp"),
            Object(NonMatching, "game/modes/tour_hint_window_and_description.cpp"),
            Object(NonMatching, "game/ui/com_select_base_menu.cpp"),
            Object(NonMatching, "game/ui/com_select_match_screen.cpp"),
            Object(NonMatching, "game/modes/stroke_match_selection_text.cpp"),
            Object(NonMatching, "game/modes/minigame_aim_and_party_tiers.cpp"),
            Object(NonMatching, "game/ui/create_new_account_and_ingame_status.cpp"),
            Object(NonMatching, "game/ui/ingame_status_hud_and_ranking.cpp"),
            Object(NonMatching, "game/ui/view_rank_and_fight_result_menu.cpp"),
            Object(NonMatching, "game/ui/result_menu_and_fight_result.cpp"),
            Object(NonMatching, "game/ui/wii_pause_score_ctor.cpp"),
            Object(NonMatching, "game/fx/tour_particle_effects.cpp"),
            Object(NonMatching, "game/ui/password_screen_task.cpp"),
            Object(NonMatching, "game/ui/darts_dialog_and_load_data_menu.cpp"),
            Object(NonMatching, "game/ui/load_data_menu.cpp"),
            Object(NonMatching, "game/ui/load_data_menu_in_entry.cpp"),
            Object(NonMatching, "game/ui/tutorial_illust_2d_and_load_data_menu.cpp"),
            Object(NonMatching, "game/chara/bone_controller_registry.cpp"),
            Object(NonMatching, "game/chara/bone_controller_map_registry.cpp"),
            Object(NonMatching, "engine/gtl/render_colored_quad_and_transform.cpp"),
            Object(NonMatching, "game/fx/trail_ribbon_pointer.cpp"),
            Object(NonMatching, "game/fx/trail_ribbon_build.cpp"),
            Object(NonMatching, "game/ui/retry_menu.cpp"),
            Object(NonMatching, "game/ui/retry_menu_and_render_packet.cpp"),
            Object(NonMatching, "engine/gtl/gtl_memory_allocators_and_heaps.cpp"),
            Object(NonMatching, "engine/gtl/gtl_heap_allocators.cpp"),
            Object(NonMatching, "engine/gtl/gx_render_state_and_texture_upload.cpp"),
            Object(NonMatching, "engine/gtl/gx_renderer_state_and_resources.cpp"),
            Object(NonMatching, "engine/gtl/gx_renderer_state_and_efb_copy.cpp"),
            Object(NonMatching, "engine/gtl/engine_render_cache_and_texture_load.cpp"),
            Object(NonMatching, "engine/gtl/gtl_graphics_texture_buffer_resources.cpp"),
            Object(NonMatching, "engine/gtl/gtl_texture_vertex_formats.cpp"),
            Object(NonMatching, "engine/gtl/gtl_texture_stage_tev_setup.cpp"),
            Object(NonMatching, "engine/gtl/gx_fifo_vertex_and_tev_helpers.cpp"),
            Object(NonMatching, "engine/gtl/gx_vertex_submit_and_cpu_skinning.cpp"),
            Object(NonMatching, "engine/gtl/skin_vertex_transform.cpp"),
            Object(Matching, "game/system/noop_hook_stubs.cpp"),
            Object(NonMatching, "engine/gtl/gtl_render_state_and_primitive.cpp"),
            Object(NonMatching, "engine/gtl/gtl_builtin_effect_resources.cpp"),
            Object(NonMatching, "engine/gtl/gtl_texture_projection_input_device.cpp"),
            Object(NonMatching, "engine/gtl/gtl_pad_drivers_gc_classic_freestyle.cpp"),
            Object(NonMatching, "game/system/input_device_drivers.cpp"),
            Object(NonMatching, "engine/gtl/gtl_vector_matrix_math.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_buffer_stream_resources.cpp"),
            Object(NonMatching, "engine/gtl/sound_resource_chunk_tags.cpp"),
            Object(NonMatching, "engine/gtl/gtl_cfile_io.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_hio.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_devices_and_registry.cpp"),
            Object(NonMatching, "game/system/file_io_facade.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_path_registry.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_hio_open.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_hio_stub.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_hio_fmem.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_callback.cpp"),
            Object(Matching, "engine/gtl/file_io_device_hio_close.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_fmem_open.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_stubs_part1.cpp"),
            Object(Matching, "engine/gtl/file_io_device_hio_read.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_close.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_stubs_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_hio_write.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_read.cpp"),
            Object(NonMatching, "engine/gtl/file_io_directory_size.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_find_handle.cpp"),
            Object(Matching, "engine/gtl/file_io_device_hio_seek.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_write.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_part3.cpp"),
            Object(Matching, "game/system/fileio_free_thunk.cpp"),
            Object(Matching, "engine/gtl/file_io_device_hio_get_file_size.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_fmem_seek.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_stub.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_vfunc.cpp"),
            Object(Matching, "engine/gtl/file_io_device_hio_stat.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_fmem_get_file_size.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_nand_save_area_check.cpp"),
            Object(Matching, "engine/gtl/gtl_check_save_area_callback.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_hio_flush.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_stat.cpp"),
            Object(NonMatching, "engine/gtl/gtl_move_file_callback.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_nand_callbacks.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_fmem_delete.cpp"),
            Object(Matching, "engine/gtl/gtl_file_directory_callbacks.cpp"),
            Object(Matching, "engine/gtl/gtl_callback_find_first_file_dtor.cpp"),
            Object(Matching, "engine/gtl/gtl_callback_file_list_dtor.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_fmem_nand_move.cpp"),
            Object(Matching, "engine/gtl/gtl_create_directory_callback.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_nand_dir_listing.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_callbacks_part1.cpp"),
            Object(NonMatching, "engine/gtl/file_io_fmem_nand_move.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_callbacks_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_callback_get_file_size.cpp"),
            Object(Matching, "engine/gtl/gtl_callback_set_file_pointer.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_fmem_disk_space.cpp"),
            Object(Matching, "engine/gtl/gtl_file_delete_callback.cpp"),
            Object(Matching, "engine/gtl/gtl_file_size_callback.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_callbacks_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fmem_file_io_device.cpp"),
            Object(Matching, "engine/gtl/gtl_file_callbacks_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_ccallback_create_file.cpp"),
            Object(Matching, "engine/gtl/gtl_callback_open_file.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_fmem_directory.cpp"),
            Object(Matching, "engine/gtl/gtl_callback_read_file_complete.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fmem_create_file_callback.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_padmem_and_string.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_padmem_handle.cpp"),
            Object(NonMatching, "engine/gtl/file_io_padmem_async_callback.cpp"),
            Object(NonMatching, "engine/gtl/pad_memory_io_completion.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_fmem_create_directory.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_padmem_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_padmem_vfunc20.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_padmem_part6.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_fmem_remove_directory.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_padmem_transfer.cpp"),
            Object(Matching, "sdk/timer_tick_frequency.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_device_fmem_nand.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_padmem_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_device_fmem_read_async.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_padmem_open.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fileio_fmem_write_async.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_padmem_read_async.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_fmem_seek.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_padmem_async.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_query.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_padmem_part3.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_part4.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_padmem_part4.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_fmem_part5.cpp"),
            Object(Matching, "engine/gtl/file_io_device_padmem.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_padmem_part5.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_padmem_part7.cpp"),
            Object(NonMatching, "engine/gtl/gtl_media_sample_pool.cpp"),
            Object(NonMatching, "engine/gtl/gtl_word_deque_ring_buffer.cpp"),
            Object(NonMatching, "engine/gtl/gtl_pointer_ring_and_sample_deque.cpp"),
            Object(NonMatching, "engine/gtl/gtl_media_pipeline_filters.cpp"),
            Object(NonMatching, "engine/gtl/gtl_media_pipeline_filter_manager.cpp"),
            Object(NonMatching, "engine/gtl/gtl_media_sample_and_sampling_filter.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sampling_filter_queues.cpp"),
            Object(NonMatching, "engine/gtl/gtl_pointer_ring_and_paged_queue.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sampling_filter_queue_reset.cpp"),
            Object(NonMatching, "engine/gtl/gtl_source_split_transform_filters.cpp"),
            Object(NonMatching, "engine/gtl/gtl_transform_filter_show_counter.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_filter_factory_part1.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_filter_manager_part1.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_filter_manager_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_sampling_filter_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_sampling_filter_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_source_filter.cpp"),
            Object(NonMatching, "engine/gtl/revolution_split_filter.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_stransfilter.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_vtrans_filter_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_effect_reverb_hi_part1.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_effect_reverb_hi_part3.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_effect_reverb_hi_dpl2_part2.cpp"),
            Object(NonMatching, "engine/gtl/sound_effect_reverb_hi_dpl2.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_effect_reverb_std_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_show_counter_part1.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_filter_factory_part2.cpp"),
            Object(Matching, "engine/gtl/revolution_sampling_filter_fill.cpp"),
            Object(Matching, "engine/gtl/revolution_source_filter_dtor.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_show_counter_part2.cpp"),
            Object(NonMatching, "engine/gtl/revolution_filter_factory_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_sampling_filter_part3.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_split_filter.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_strans_filter_alloc.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_vtrans_filter_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_effect_reverb_hi_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_effect_chorus_reverb.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_effect_delay_chorus.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_runtime_and_delay_effect.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_manager_and_sound_factory.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_spatial_and_runtime.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_buffer_envelope_lfo.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_buffer_and_effects.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_stream_player.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_stream_interface.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_stream_source_callbacks.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_stream_source_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_stream_source_part4.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_string.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_controller_effects.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_voice_ax_control.cpp"),
            Object(NonMatching, "engine/gtl/gtl_ax_voice_control.cpp"),
            Object(NonMatching, "engine/gtl/gtl_voice_pool_basic_player_chorus.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fx_chorus_params.cpp"),
            Object(NonMatching, "engine/gtl/gtl_nw4r_fx_delay_chorus.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fx_delay_audio_effect.cpp"),
            Object(NonMatching, "engine/gtl/gtl_fx_reverb_hi.cpp"),
            Object(Matching, "engine/gtl/gtl_fx_reverb_hi_params.cpp"),
            Object(NonMatching, "engine/gtl/gtl_nw4r_fx_reverb_hi_dpl2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_revolution_filter_factory_part3.cpp"),
            Object(Matching, "engine/gtl/revolution_strans_audio_decode.cpp"),
            Object(Matching, "engine/gtl/revolution_vtrans_thp_decode.cpp"),
            Object(NonMatching, "engine/gtl/sound_effect_reverb_hi_get_parameter.cpp"),
            Object(NonMatching, "engine/gtl/revolution_strans_filter_factory.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_effect_reverb_hi_dpl2_part1.cpp"),
            Object(Matching, "engine/gtl/sound_effect_chorus_params_part1.cpp"),
            Object(Matching, "engine/gtl/sound_effect_delay.cpp"),
            Object(NonMatching, "engine/gtl/revolution_filter_factory_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_effect_reverb_hidpl2.cpp"),
            Object(Matching, "engine/gtl/sound_effect_chorus_params_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_effect_delay_params.cpp"),
            Object(Matching, "engine/gtl/revolution_filter_manager.cpp"),
            Object(NonMatching, "engine/gtl/sound_effect_reverb_hidpl2.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_buffer_source.cpp"),
            Object(Matching, "engine/gtl/gtl_file_load_task_and_stream_callback.cpp"),
            Object(NonMatching, "engine/gtl/revolution_filter_show_counter.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_effect_reverb_std_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_dvd_file_stream_read_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_fx_chorus_shutdown.cpp"),
            Object(Matching, "engine/gtl/gtl_revolution_filter_manager_part3.cpp"),
            Object(Matching, "engine/gtl/sound_effect_reverb_std.cpp"),
            Object(Matching, "engine/gtl/gtl_sound_stream_source_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_fx_chorus_process_buffer.cpp"),
            Object(Matching, "engine/gtl/sound_effect_reverb_std_params.cpp"),
            Object(Matching, "engine/gtl/sound_stream_source_callback.cpp"),
            Object(Matching, "engine/gtl/gtl_fx_delay_shutdown.cpp"),
            Object(Matching, "engine/gtl/nw4r_fx_reverb_hi_shutdown.cpp"),
            Object(NonMatching, "engine/gtl/gtl_sound_stream_source_part3.cpp"),
            Object(Matching, "engine/gtl/gtl_fx_delay_audio_callback.cpp"),
            Object(Matching, "engine/gtl/nw4r_fx_reverb_hi.cpp"),
            Object(NonMatching, "engine/gtl/fx_reverb_hi_dpl2.cpp"),
            Object(Matching, "engine/gtl/sound_stream_load_task_cancel.cpp"),
            Object(NonMatching, "engine/gtl/gtl_nw4r_fx_reverb_std_part1.cpp"),
            Object(Matching, "engine/gtl/fx_reverb_std_params.cpp"),
            Object(NonMatching, "engine/gtl/gtl_remote_audio_channel.cpp"),
            Object(Matching, "engine/gtl/sound_stream_data_load_task.cpp"),
            Object(Matching, "engine/gtl/nw4r_fx_reverb_std_shutdown.cpp"),
            Object(NonMatching, "engine/gtl/gtl_audio_worker_strm_player.cpp"),
            Object(NonMatching, "engine/gtl/gtl_strm_player_streaming.cpp"),
            Object(Matching, "engine/gtl/gtl_nw4r_fx_reverb_std_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_strm_player_pause.cpp"),
            Object(NonMatching, "engine/gtl/gtl_voice_and_stream_tasks.cpp"),
            Object(NonMatching, "engine/gtl/gtl_voice_control.cpp"),
            Object(NonMatching, "engine/gtl/strm_player_load_command.cpp"),
            Object(NonMatching, "engine/gtl/gtl_voice_pool.cpp"),
            Object(NonMatching, "engine/gtl/gtl_dvd_file_stream.cpp"),
            Object(Matching, "engine/gtl/gtl_dvd_file_stream_read_part2.cpp"),
            Object(Matching, "engine/gtl/dvd_file_stream_close.cpp"),
            Object(Matching, "engine/gtl/dvd_file_stream_seek_cancel.cpp"),
            Object(NonMatching, "engine/gtl/gtl_dvd_file_stream_and_device.cpp"),
            Object(Matching, "engine/gtl/file_io_free_wrapper.cpp"),
            Object(NonMatching, "engine/gtl/dvd_file_stream_read.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_open_part1.cpp"),
            Object(Matching, "engine/gtl/file_io_device_dvd_read_callbacks.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_device_root_path.cpp"),
            Object(NonMatching, "engine/gtl/gtl_dvd_file_stream_read_async_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_close_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_base.cpp"),
            Object(NonMatching, "engine/gtl/fileio_string_storage.cpp"),
            Object(Matching, "engine/gtl/gtl_dvd_file_stream_read_async_part2.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_dvd_read.cpp"),
            Object(Matching, "engine/gtl/gtl_dvd_file_stream_cancel.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_seek.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_size.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_fileinfo.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_cancel_part1.cpp"),
            Object(NonMatching, "engine/gtl/gtl_file_io_dvd_find_first.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_dvd_find_next.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_close_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_open_part2.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_dvd_async_read.cpp"),
            Object(NonMatching, "engine/gtl/file_io_device_dvd_seek.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_cancel_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_poll.cpp"),
            Object(Matching, "engine/gtl/gtl_file_io_device_dvd_query.cpp"),
            Object(NonMatching, "engine/gtl/gtl_image_decode_and_tiling.cpp"),
            Object(NonMatching, "engine/gtl/gtl_image_layout_and_sizing.cpp"),
            Object(NonMatching, "engine/gtl/gtl_image_format_dispatch.cpp"),
            Object(NonMatching, "engine/gtl/gtl_image_font_resource_adapters.cpp"),
            Object(NonMatching, "engine/gtl/gtl_generic_resource_loading.cpp"),
            Object(Matching, "engine/gtl/gtl_model_resource_storage.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_modeling_resource_draw.cpp"),
            Object(Matching, "engine/gtl/generic_modeling_resource_counts.cpp"),
            Object(NonMatching, "engine/gtl/gtl_graphics_resource_vectors.cpp"),
            Object(NonMatching, "engine/gtl/gtl_vertex_declaration_vector.cpp"),
            Object(Matching, "engine/gtl/gtl_model_resource_and_culling.cpp"),
            Object(Equivalent, "engine/gtl/gtl_generic_environment.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_renderer_scratch_clear.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_renderer_context_ctor.cpp"),
            Object(NonMatching, "engine/gtl/gtl_generic_model_graphics_load.cpp"),
            Object(NonMatching, "engine/gtl/gtl_model_culling_billboard_curve.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_node_evaluate.cpp"),
            Object(NonMatching, "engine/gtl/gtl_bitmap_font_and_anim_keys.cpp"),
            Object(NonMatching, "engine/gtl/gtl_bitmap_font_draw.cpp"),
            Object(NonMatching, "engine/gtl/gtl_dxt_decode_and_pixel_convert.cpp"),
            Object(NonMatching, "engine/gtl/gtl_transform_frustum_stopwatch_input.cpp"),
            Object(Matching, "engine/gtl/gtl_screen_pointer_raw_samples.cpp"),
            Object(NonMatching, "engine/gtl/gtl_input_ex_devices_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_input_ex_button_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_input_ex_button_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_input_ex_digital_button_part1.cpp"),
            Object(NonMatching, "engine/gtl/gtl_resource_chunk_sort.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_resource_file_chunk.cpp"),
            Object(NonMatching, "engine/gtl/gtl_generic_model_resource.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_animation_resource_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_environment_resource.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_renderer_vertex_decl.cpp"),
            Object(Matching, "engine/gtl/gtl_animation_hermite_vec3.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_model_matrices.cpp"),
            Object(Matching, "engine/gtl/gtl_inputex_device_poll.cpp"),
            Object(Matching, "engine/gtl/gtl_input_ex_button_reset.cpp"),
            Object(Matching, "engine/gtl/gtl_input_ex_analog_button.cpp"),
            Object(Matching, "engine/gtl/gtl_input_ex_digital_button_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_resource_chunk_sort3.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_modeling_resource_dtor.cpp"),
            Object(Matching, "engine/gtl/gtl_animation_resource_evaluate.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_environment_resource_apply.cpp"),
            Object(Matching, "engine/gtl/gtl_renderer_draw_queue_clear.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_bezier_interpolate.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_node_evaluation.cpp"),
            Object(NonMatching, "engine/gtl/gtl_input_ex_devices_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_screen_pointer_reset_buttons.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_modeling_resource_part1.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_animation_resource_part2.cpp"),
            Object(Matching, "engine/gtl/gtl_environment_light_count.cpp"),
            Object(NonMatching, "engine/gtl/gtl_generic_renderer_scratch_buffers.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_bezier_vec2_interp.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_transform_channels.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_modeling_resource_eval.cpp"),
            Object(Matching, "game/fx/environment_light_resource_export.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_renderer_init.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_bezier_vec3.cpp"),
            Object(NonMatching, "engine/gtl/gtl_texture_animation_matrices.cpp"),
            Object(Matching, "engine/gtl/gtl_material_animation_eval.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_renderer_context.cpp"),
            Object(NonMatching, "engine/gtl/gtl_animation_bezier_interp.cpp"),
            Object(NonMatching, "engine/gtl/generic_modeling_resource_description.cpp"),
            Object(NonMatching, "engine/gtl/gtl_generic_renderer_material.cpp"),
            Object(Matching, "engine/gtl/gtl_animation_interpolate_vec4.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_modeling_resource_clone.cpp"),
            Object(Matching, "engine/gtl/gtl_renderer_bind_vertex_streams.cpp"),
            Object(Matching, "engine/gtl/gtl_animation_interpolate_vec3.cpp"),
            Object(Matching, "engine/gtl/gtl_generic_modeling_resource_part2.cpp"),
            Object(NonMatching, "engine/gtl/gtl_vertex_blend_transforms.cpp"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
        ],
    },
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
