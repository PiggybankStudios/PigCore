/*
File:   tests_clangd_main.c
Author: Taylor Robbins
Date:   02\08\2026
Description: 
	** This file is never actually compiled. Instead it is the "fake" compilation target that we tell clangd
	** about that serves a root for it to provide information from. This is important because clangd does
	** not like transient includes (#includes in the #includes) which we do a lot in PigCore.
	** This way we can get mostly accurate code completion and other LSP features without needing to cater
	** the code to clangd's limitations.
	** NOTE: "clangd" is a C\C++ Language Server that can be set up to provide most editors with advanced
	** features like auto-complete, goto definition, code actions, warnings, etc. I generally use Sublime Text
	** as the primary editor for this project and use the "LSP" and "LSP-clangd" packages to run clangd
	** as a language server and connect it to my editor. You can also get the same features with clangd
	** in VS Code with similar plugins.
*/

#include "tests/tests_main.c" // IWYU pragma: keep

// #include "base/base_all.h" // IWYU pragma: keep
#include "base/base_assert.h" // IWYU pragma: keep
#include "base/base_char.h" // IWYU pragma: keep
#include "base/base_compiler_check.h" // IWYU pragma: keep
#include "base/base_dbg_level.h" // IWYU pragma: keep
#include "base/base_debug_output.h" // IWYU pragma: keep
// #include "base/base_debug_output_impl.h" // IWYU pragma: keep
#include "base/base_defines_check.h" // IWYU pragma: keep
#include "base/base_macros.h" // IWYU pragma: keep
#include "base/base_math.h" // IWYU pragma: keep
#include "base/base_notifications.h" // IWYU pragma: keep
// #include "base/base_notifications_impl.h" // IWYU pragma: keep
#include "base/base_plex_is_struct.h" // IWYU pragma: keep
#include "base/base_typedefs.h" // IWYU pragma: keep
#include "base/base_unicode.h" // IWYU pragma: keep

// #include "dll/dll_main.c" // IWYU pragma: keep

// #include "file/file_fmt_all.h" // IWYU pragma: keep
#include "file_fmt/file_fmt_gltf.h" // IWYU pragma: keep
#include "file_fmt/file_fmt_sprite_sheet.h" // IWYU pragma: keep

// #include "gfx/gfx_all.h" // IWYU pragma: keep
#include "gfx/gfx_clay_renderer.h" // IWYU pragma: keep
#include "gfx/gfx_error.h" // IWYU pragma: keep
#include "gfx/gfx_font_flow.h" // IWYU pragma: keep
#include "gfx/gfx_font.h" // IWYU pragma: keep
#include "gfx/gfx_helpers.h" // IWYU pragma: keep
#include "gfx/gfx_image_loading.h" // IWYU pragma: keep
#include "gfx/gfx_mesh_generation.h" // IWYU pragma: keep
#include "gfx/gfx_pipeline.h" // IWYU pragma: keep
#include "gfx/gfx_shader.h" // IWYU pragma: keep
#include "gfx/gfx_system_global.h" // IWYU pragma: keep
#include "gfx/gfx_system.h" // IWYU pragma: keep
#include "gfx/gfx_texture.h" // IWYU pragma: keep
#include "gfx/gfx_vert_buffer.h" // IWYU pragma: keep
#include "gfx/gfx_vertices.h" // IWYU pragma: keep

// #include "input/input_all.h" // IWYU pragma: keep
#include "input/input_btn_state.h" // IWYU pragma: keep
#include "input/input_controller_btns.h" // IWYU pragma: keep
#include "input/input_keyboard.h" // IWYU pragma: keep
#include "input/input_keys.h" // IWYU pragma: keep
#include "input/input_mouse_btns.h" // IWYU pragma: keep
#include "input/input_mouse.h" // IWYU pragma: keep
#include "input/input_sokol.h" // IWYU pragma: keep
#include "input/input_touch.h" // IWYU pragma: keep

// #include "lib/lib_all.h" // IWYU pragma: keep
#include "lib/lib_freetype.h" // IWYU pragma: keep
// #include "lib/lib_freetype_modules.h" // IWYU pragma: keep
// #include "lib/lib_freetype_options.h" // IWYU pragma: keep
#include "lib/lib_handmade_math.h" // IWYU pragma: keep
#include "lib/lib_metadesk.h" // IWYU pragma: keep
#include "lib/lib_plutosvg.h" // IWYU pragma: keep
#include "lib/lib_sokol_app.h" // IWYU pragma: keep
// #include "lib/lib_sokol_app_impl.c" // IWYU pragma: keep
#include "lib/lib_sokol_gfx.h" // IWYU pragma: keep
#include "lib/lib_stb_image.h" // IWYU pragma: keep
#include "lib/lib_stb_image_write.h" // IWYU pragma: keep
#include "lib/lib_stb_rect_pack.h" // IWYU pragma: keep
#include "lib/lib_stb_truetype.h" // IWYU pragma: keep
#include "lib/lib_tracy.h" // IWYU pragma: keep

// #include "mem/mem_all.h" // IWYU pragma: keep
#include "mem/mem_arena.h" // IWYU pragma: keep
#include "mem/mem_scratch.h" // IWYU pragma: keep

// #include "misc/misc_all.h" // IWYU pragma: keep
#include "misc/misc_easing.h" // IWYU pragma: keep
#include "misc/misc_escaping.h" // IWYU pragma: keep
#include "misc/misc_file_watch.h" // IWYU pragma: keep
#include "misc/misc_hash.h" // IWYU pragma: keep
#include "misc/misc_noise.h" // IWYU pragma: keep
#include "misc/misc_parsing.h" // IWYU pragma: keep
#include "misc/misc_poly_simplify.h" // IWYU pragma: keep
#include "misc/misc_printing.h" // IWYU pragma: keep
#include "misc/misc_random.h" // IWYU pragma: keep
#include "misc/misc_regex.h" // IWYU pragma: keep
#include "misc/misc_result.h" // IWYU pragma: keep
#include "misc/misc_sorting.h" // IWYU pragma: keep
#include "misc/misc_standard_colors.h" // IWYU pragma: keep
#include "misc/misc_standard_names.h" // IWYU pragma: keep
#include "misc/misc_triangulation.h" // IWYU pragma: keep
#include "misc/misc_two_pass.h" // IWYU pragma: keep
#include "misc/misc_web.h" // IWYU pragma: keep
#include "misc/misc_zip.h" // IWYU pragma: keep

// #include "os/os_all.h" // IWYU pragma: keep
#include "os/os_atomics.h" // IWYU pragma: keep
#include "os/os_clipboard.h" // IWYU pragma: keep
#include "os/os_dll.h" // IWYU pragma: keep
#include "os/os_error.h" // IWYU pragma: keep
#include "os/os_file_dialog.h" // IWYU pragma: keep
#include "os/os_file.h" // IWYU pragma: keep
#include "os/os_font.h" // IWYU pragma: keep
#include "os/os_http.h" // IWYU pragma: keep
#include "os/os_jni.h" // IWYU pragma: keep
#include "os/os_orca.h" // IWYU pragma: keep
#include "os/os_path.h" // IWYU pragma: keep
#include "os/os_process_info.h" // IWYU pragma: keep
#include "os/os_program_args.h" // IWYU pragma: keep
#include "os/os_sleep.h" // IWYU pragma: keep
#include "os/os_threading.h" // IWYU pragma: keep
#include "os/os_thread_pool.h" // IWYU pragma: keep
#include "os/os_time.h" // IWYU pragma: keep
#include "os/os_virtual_mem.h" // IWYU pragma: keep

// #include "parse/parse_all.h" // IWYU pragma: keep
#include "parse/parse_binary.h" // IWYU pragma: keep
#include "parse/parse_c_tokenizer.h" // IWYU pragma: keep
#include "parse/parse_metadesk.h" // IWYU pragma: keep
#include "parse/parse_protobuf.h" // IWYU pragma: keep
// #include "parse/parse_proto_google_types.pb-c.h" // IWYU pragma: keep
// #include "parse/parse_proto_google_types.pb-c.c" // IWYU pragma: keep
#include "parse/parse_simple_parsers.h" // IWYU pragma: keep

// #include "phys/phys_all.h" // IWYU pragma: keep
#include "phys/phys_ode.h" // IWYU pragma: keep
#include "phys/phys_physx.h" // IWYU pragma: keep

// #include "piggen/piggen_generate.c" // IWYU pragma: keep
// #include "piggen/piggen_main.c" // IWYU pragma: keep

// #include "std/std_all.h" // IWYU pragma: keep
#include "std/std_angles.h" // IWYU pragma: keep
#include "std/std_basic_math.h" // IWYU pragma: keep
#include "std/std_includes.h" // IWYU pragma: keep
#include "std/std_malloc.h" // IWYU pragma: keep
#include "std/std_math_ex.h" // IWYU pragma: keep
#include "std/std_memset.h" // IWYU pragma: keep
#include "std/std_printf.h" // IWYU pragma: keep
#include "std/std_trig.h" // IWYU pragma: keep

// #include "struct/struct_all.h" // IWYU pragma: keep
#include "struct/struct_bkt_array.h" // IWYU pragma: keep
#include "struct/struct_circles.h" // IWYU pragma: keep
#include "struct/struct_color.h" // IWYU pragma: keep
#include "struct/struct_directions.h" // IWYU pragma: keep
#include "struct/struct_faces.h" // IWYU pragma: keep
#include "struct/struct_font_char_range.h" // IWYU pragma: keep
#include "struct/struct_image_data.h" // IWYU pragma: keep
#include "struct/struct_lines.h" // IWYU pragma: keep
#include "struct/struct_matrices.h" // IWYU pragma: keep
#include "struct/struct_model_data.h" // IWYU pragma: keep
#include "struct/struct_pointer_remap.h" // IWYU pragma: keep
#include "struct/struct_quaternion.h" // IWYU pragma: keep
#include "struct/struct_ranges.h" // IWYU pragma: keep
#include "struct/struct_rectangles.h" // IWYU pragma: keep
#include "struct/struct_rich_string.h" // IWYU pragma: keep
#include "struct/struct_sparse_sets.h" // IWYU pragma: keep
#include "struct/struct_stream.h" // IWYU pragma: keep
#include "struct/struct_string_buffer.h" // IWYU pragma: keep
#include "struct/struct_string_error_list.h" // IWYU pragma: keep
#include "struct/struct_string.h" // IWYU pragma: keep
#include "struct/struct_typed_array.h" // IWYU pragma: keep
#include "struct/struct_var_array.h" // IWYU pragma: keep
#include "struct/struct_vectors.h" // IWYU pragma: keep
#include "struct/struct_work_subject.h" // IWYU pragma: keep

// #include "tests/main2d_shader.glsl.c" // IWYU pragma: keep
// #include "tests/main2d_shader.glsl.h" // IWYU pragma: keep
// #include "tests/main3d_shader.glsl.c" // IWYU pragma: keep
// #include "tests/main3d_shader.glsl.h" // IWYU pragma: keep
// #include "tests/simple_shader.glsl.c" // IWYU pragma: keep
// #include "tests/simple_shader.glsl.h" // IWYU pragma: keep
// #include "tests/tests_android.c" // IWYU pragma: keep
// #include "tests/tests_auto_profile.c" // IWYU pragma: keep
// #include "tests/tests_box2d.c" // IWYU pragma: keep
// #include "tests/tests_box2d_renderers.c" // IWYU pragma: keep
// #include "tests/tests_gtk.c" // IWYU pragma: keep
// #include "tests/tests_parsing.c" // IWYU pragma: keep
// #include "tests/tests_playdate.c" // IWYU pragma: keep
// #include "tests/tests_proto_types.pb-c.c" // IWYU pragma: keep
// #include "tests/tests_proto_types.pb-c.h" // IWYU pragma: keep
// #include "tests/tests_raylib.c" // IWYU pragma: keep
// #include "tests/tests_sokol.c" // IWYU pragma: keep
// #include "tests/tests_sqlite.c" // IWYU pragma: keep
// #include "tests/tests_vr.c" // IWYU pragma: keep
// #include "tests/tests_wasm_std.c" // IWYU pragma: keep

// #include "tools/tools_android_build_helpers.h" // IWYU pragma: keep
// #include "tools/tools_build_helpers.h" // IWYU pragma: keep
// #include "tools/tools_clang_flags.h" // IWYU pragma: keep
// #include "tools/tools_cli.h" // IWYU pragma: keep
// #include "tools/tools_emscripten_flags.h" // IWYU pragma: keep
// #include "tools/tools_extract_define_main.c" // IWYU pragma: keep
// #include "tools/tools_gcc_flags.h" // IWYU pragma: keep
// #include "tools/tools_msvc_flags.h" // IWYU pragma: keep
// #include "tools/tools_pdc_flags.h" // IWYU pragma: keep
// #include "tools/tools_pig_core_build_flags.h" // IWYU pragma: keep
// #include "tools/tools_shared.h" // IWYU pragma: keep
// #include "tools/tools_shdc_flags.h" // IWYU pragma: keep
// #include "tools/tools_str_array.h" // IWYU pragma: keep

// #include "ui/ui_all.h" // IWYU pragma: keep
#include "ui/ui_clay_checkbox.h" // IWYU pragma: keep
#include "ui/ui_clay.h" // IWYU pragma: keep
#include "ui/ui_clay_hoverable.h" // IWYU pragma: keep
#include "ui/ui_clay_label.h" // IWYU pragma: keep
#include "ui/ui_clay_large_textview.h" // IWYU pragma: keep
#include "ui/ui_clay_listview.h" // IWYU pragma: keep
#include "ui/ui_clay_notifications.h" // IWYU pragma: keep
#include "ui/ui_clay_resizable_split.h" // IWYU pragma: keep
#include "ui/ui_clay_textbox.h" // IWYU pragma: keep
#include "ui/ui_clay_tooltip.h" // IWYU pragma: keep
#include "ui/ui_clay_tooltip_registry.h" // IWYU pragma: keep
#include "ui/ui_clay_widget_context.h" // IWYU pragma: keep
// #include "ui/ui_imconfig.h" // IWYU pragma: keep
#include "ui/ui_imgui.h" // IWYU pragma: keep
#include "ui/ui_performance_graph.h" // IWYU pragma: keep
#include "ui/ui_system.h" // IWYU pragma: keep

#include "cross/cross_color_and_raylib.h" // IWYU pragma: keep
#include "cross/cross_color_and_vectors.h" // IWYU pragma: keep
#include "cross/cross_dbg_level_and_standard_colors.h" // IWYU pragma: keep
#include "cross/cross_easing_and_math_ex.h" // IWYU pragma: keep
#include "cross/cross_file_and_random.h" // IWYU pragma: keep
#include "cross/cross_file_and_sprite_sheet.h" // IWYU pragma: keep
#include "cross/cross_font_char_range_and_var_array.h" // IWYU pragma: keep
#include "cross/cross_gfx_system_global_and_helpers.h" // IWYU pragma: keep
#include "cross/cross_gltf_and_os_file.h" // IWYU pragma: keep
#include "cross/cross_image_loading_and_file.h" // IWYU pragma: keep
#include "cross/cross_imgui_and_gfx_system.h" // IWYU pragma: keep
#include "cross/cross_imgui_and_keys.h" // IWYU pragma: keep
#include "cross/cross_imgui_input_sokol_and_btn_state.h" // IWYU pragma: keep
#include "cross/cross_matrices_and_raylib.h" // IWYU pragma: keep
#include "cross/cross_mem_arena_and_image_data.h" // IWYU pragma: keep
#include "cross/cross_mem_arena_and_string_buffer.h" // IWYU pragma: keep
#include "cross/cross_mem_arena_and_string.h" // IWYU pragma: keep
#include "cross/cross_mem_arena_string_and_unicode.h" // IWYU pragma: keep
#include "cross/cross_mem_scratch_and_string_buffer.h" // IWYU pragma: keep
#include "cross/cross_os_font_and_gfx_font.h" // IWYU pragma: keep
#include "cross/cross_parsing_and_color.h" // IWYU pragma: keep
#include "cross/cross_parsing_and_directions.h" // IWYU pragma: keep
#include "cross/cross_parsing_and_matrices.h" // IWYU pragma: keep
#include "cross/cross_parsing_and_rectangles.h" // IWYU pragma: keep
#include "cross/cross_parsing_and_vectors.h" // IWYU pragma: keep
#include "cross/cross_printf_and_string_buffer.h" // IWYU pragma: keep
#include "cross/cross_quaternion_and_matrices.h" // IWYU pragma: keep
#include "cross/cross_quaternion_and_raylib.h" // IWYU pragma: keep
#include "cross/cross_rich_string_scratch_and_ranges.h" // IWYU pragma: keep
#include "cross/cross_scratch_and_clay.h" // IWYU pragma: keep
#include "cross/cross_scratch_and_poly_simplify.h" // IWYU pragma: keep
#include "cross/cross_scratch_and_printing.h" // IWYU pragma: keep
#include "cross/cross_shader_and_texture.h" // IWYU pragma: keep
#include "cross/cross_shader_and_vert_buffer.h" // IWYU pragma: keep
#include "cross/cross_sokol_gfx_and_matrices.h" // IWYU pragma: keep
#include "cross/cross_sorting_and_bkt_array.h" // IWYU pragma: keep
#include "cross/cross_sorting_and_font_char_range.h" // IWYU pragma: keep
#include "cross/cross_stream_and_parse_binary.h" // IWYU pragma: keep
#include "cross/cross_string_and_hash.h" // IWYU pragma: keep
#include "cross/cross_string_and_ranges.h" // IWYU pragma: keep
#include "cross/cross_string_and_typed_array.h" // IWYU pragma: keep
#include "cross/cross_string_and_unicode.h" // IWYU pragma: keep
#include "cross/cross_threading_and_mem_arena.h" // IWYU pragma: keep
#include "cross/cross_vectors_and_raylib.h" // IWYU pragma: keep
#include "cross/cross_vectors_quaternion_matrices_and_rectangles.h" // IWYU pragma: keep
#include "cross/cross_zip_and_file.h" // IWYU pragma: keep
