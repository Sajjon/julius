cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

if(VITA_BUILD AND SWITCH_BUILD)
    message(FATAL_ERROR "Cannot build with -DVITA_BUILD=ON and -DSWITCH_BUILD=ON at the same time")
endif()

if((VITA_BUILD OR SWITCH_BUILD) AND ANDROID_BUILD)
    message(FATAL_ERROR "Cannot build with -DVITA_BUILD=ON or -DSWITCH_BUILD=ON and -DANDROID_BUILD=ON at the same time")
endif()

option(DRAW_FPS "Draw FPS on the top left corner of the window." OFF)
option(SYSTEM_LIBS "Use system libraries when available." ON)
option(EMSCRIPTEN_LOAD_SDL_PORTS "Load SDL and SDL_mixer emscripten ports instead of compiling them" OFF)
option(LINK_MPG123 "Link mpg123 statically to Julius instead of relying on a library." OFF)

cmake_dependent_option(VITA_BUILD "Build for the PlayStation Vita handheld game console." OFF "NOT MSVC" OFF)
cmake_dependent_option(SWITCH_BUILD "Build for the Nintendo Switch handheld game console." OFF "NOT MSVC; NOT VITA_BUILD" OFF)
cmake_dependent_option(ANDROID_BUILD "Build for the Android environment." OFF "NOT MSVC; NOT VITA_BUILD; NOT SWITCH_BUILD" OFF)
cmake_dependent_option(EMESCRIPTEN_BUILD "Build for the Emscripten environment." OFF "NOT VITA_BUILD; NOT SWITCH_BUILD; NOT ANDROID_BUILD" OFF)

if(VITA_BUILD AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

if(SWITCH_BUILD AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{DEVKITPRO})
        set(DEVKITPRO $ENV{DEVKITPRO})
        set(CMAKE_TOOLCHAIN_FILE "./cmake/switch.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define DEVKITPRO to point to your SDK path!")
    endif()
endif()

if(EMSCRIPTEN_BUILD AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{EMSDK})
        set(EMSDK $ENV{EMSDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define EMSDK to point to your Emscripten SDK path!")
    endif()
endif()

set(SHORT_NAME julius)
set(USER_FRIENDLY_NAME Julius)
project(${SHORT_NAME} C)

if (VITA_BUILD)
    include("${VITASDK}/share/vita.cmake" REQUIRED)
    set(VITA_TITLEID  "JULIUS001")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_VERSION VERSION_LESS 3.1)
    if (CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
    endif()
else()
    set(CMAKE_C_STANDARD 99)
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration")

    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0)
        set(CMAKE_C_FLAGS "-Werror=incompatible-pointer-types -Werror=int-conversion -Wstrict-prototypes ${CMAKE_C_FLAGS}")
    endif()
endif()

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 6)
set(PROJECT_VERSION_PATCH 0)
set(IS_RELEASE_VERSION FALSE)

if(IS_RELEASE_VERSION)
    set(PROJECT_VERSION_TWEAK 0)
    set(VERSION_REVISION "")
elseif (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    find_package(Git)
    if(Git_FOUND)
        execute_process(
            COMMAND git rev-list --count HEAD ^tags/v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE TWEAK_RESULT
            OUTPUT_VARIABLE PROJECT_VERSION_TWEAK
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND git rev-parse --short --verify HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE VERSION_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND git diff-index --quiet HEAD --
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE VERSION_COMMIT_DIRTY
        )
        if(VERSION_COMMIT_DIRTY)
            set(DIRTY_POSTFIX "-dirty")
        else()
            set(DIRTY_POSTFIX "")
        endif()
        if(NOT TWEAK_RESULT EQUAL 0)
            # Happens on a shallow git clone, like Travis does. Append date to version ref.
            set(PROJECT_VERSION_TWEAK 0)
            string(TIMESTAMP TODAY "%Y%m%d")
            set(VERSION_REVISION "-${TODAY}-${VERSION_COMMIT_HASH}${DIRTY_POSTFIX}")
        else()
            set(VERSION_REVISION ".${PROJECT_VERSION_TWEAK}-${VERSION_COMMIT_HASH}${DIRTY_POSTFIX}")
        endif()
    else()
        set(PROJECT_VERSION_TWEAK 0)
        set(VERSION_REVISION " unknown development version")
    endif()
else()
    set(PROJECT_VERSION_TWEAK 0)
    set(VERSION_REVISION " unknown development version")
endif()

if(VITA_BUILD)
    string(LENGTH "${PROJECT_VERSION_MAJOR}" NUM_DIGITS)
    if(NUM_DIGITS EQUAL 1)
        set(VITA_VERSION_MAJOR "0${PROJECT_VERSION_MAJOR}")
    elseif(NUM_DIGITS EQUAL 2)
        set(VITA_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    else()
        message(FATAL_ERROR "On Vita, major version number can only have one or two digits, found ${NUM_DIGITS} digits instead")
    endif()

    string(LENGTH "${PROJECT_VERSION_MINOR}" NUM_DIGITS)
    if(NUM_DIGITS EQUAL 1)
        set(VITA_VERSION_MINOR "0${PROJECT_VERSION_MINOR}")
    elseif(NUM_DIGITS EQUAL 2)
        set(VITA_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    else()
        message(FATAL_ERROR "On Vita, minor version number can only have one or two digits, found ${NUM_DIGITS} digits instead")
    endif()

    set(VITA_VERSION "${VITA_VERSION_MAJOR}.${VITA_VERSION_MINOR}")
endif()

if (SWITCH_BUILD)
    string(SUBSTRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${VERSION_REVISION}" 0 15 SWITCH_VERSION)
endif()

configure_file(${PROJECT_SOURCE_DIR}/gen/version.c.in ${PROJECT_SOURCE_DIR}/src/app/platform/version.c)
configure_file(${PROJECT_SOURCE_DIR}/gen/version.rc.in ${PROJECT_SOURCE_DIR}/res/version.rc)
configure_file(${PROJECT_SOURCE_DIR}/gen/version.txt.in ${PROJECT_SOURCE_DIR}/res/version.txt)

if(EMSCRIPTEN_BUILD)
    configure_file(${PROJECT_SOURCE_DIR}/gen/shell.html.in ${PROJECT_SOURCE_DIR}/res/shell.html)
endif()

if(DRAW_FPS)
  add_definitions(-DDRAW_FPS)
endif()

set(TINYFD_FILES
    ext/tinyfiledialogs/tinyfiledialogs.c
)

set(PNG_FILES
    ext/png/png.c
    ext/png/pngerror.c
    ext/png/pngmem.c
    ext/png/pngset.c
    ext/png/pngwio.c
    ext/png/pngwrite.c
    ext/png/pngwutil.c
)

set(ZLIB_FILES
    ext/zlib/adler32.c
    ext/zlib/crc32.c
    ext/zlib/deflate.c
    ext/zlib/trees.c
    ext/zlib/zutil.c
)

set(PLATFORM_FILES
    ${PROJECT_SOURCE_DIR}/src/julius.c
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/arguments.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/cursor.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/file_manager.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/file_manager_cache.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/joystick.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/keyboard_input.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/log.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/mouse.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/platform.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/prefs.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/screen.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/sound_device.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/touch.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/version.c
    ${PROJECT_SOURCE_DIR}/src/app/platform/virtual_keyboard.c
)

if (VITA_BUILD)
    set(PLATFORM_FILES
        ${PLATFORM_FILES}
        ${PROJECT_SOURCE_DIR}/src/app/platform/vita/vita.c
        ${PROJECT_SOURCE_DIR}/src/app/platform/vita/vita_keyboard.c
    )
elseif (SWITCH_BUILD)
    set(PLATFORM_FILES
        ${PLATFORM_FILES}
        ${PROJECT_SOURCE_DIR}/src/app/platform/switch/switch.c
    )
elseif (ANDROID_BUILD)
    set(PLATFORM_FILES
        ${PLATFORM_FILES}
        ${PROJECT_SOURCE_DIR}/src/app/platform/android/android.c
    )
endif()

set(CORE_FILES
    ${PROJECT_SOURCE_DIR}/src/app/core/backtrace.c
    ${PROJECT_SOURCE_DIR}/src/app/core/buffer.c
    ${PROJECT_SOURCE_DIR}/src/app/core/calc.c
    ${PROJECT_SOURCE_DIR}/src/app/core/config.c
    ${PROJECT_SOURCE_DIR}/src/app/core/dir.c
    ${PROJECT_SOURCE_DIR}/src/app/core/encoding.c
    ${PROJECT_SOURCE_DIR}/src/app/core/encoding_korean.c
    ${PROJECT_SOURCE_DIR}/src/app/core/encoding_simp_chinese.c
    ${PROJECT_SOURCE_DIR}/src/app/core/encoding_trad_chinese.c
    ${PROJECT_SOURCE_DIR}/src/app/core/file.c
    ${PROJECT_SOURCE_DIR}/src/app/core/hotkey_config.c
    ${PROJECT_SOURCE_DIR}/src/app/core/image.c
    ${PROJECT_SOURCE_DIR}/src/app/core/io.c
    ${PROJECT_SOURCE_DIR}/src/app/core/lang.c
    ${PROJECT_SOURCE_DIR}/src/app/core/locale.c
    ${PROJECT_SOURCE_DIR}/src/app/core/random.c
    ${PROJECT_SOURCE_DIR}/src/app/core/smacker.c
    ${PROJECT_SOURCE_DIR}/src/app/core/speed.c
    ${PROJECT_SOURCE_DIR}/src/app/core/string.c
    ${PROJECT_SOURCE_DIR}/src/app/core/time.c
    ${PROJECT_SOURCE_DIR}/src/app/core/zip.c
)

set(BUILDING_FILES
    ${PROJECT_SOURCE_DIR}/src/app/building/animation.c
    ${PROJECT_SOURCE_DIR}/src/app/building/barracks.c
    ${PROJECT_SOURCE_DIR}/src/app/building/building.c
    ${PROJECT_SOURCE_DIR}/src/app/building/building_state.c
    ${PROJECT_SOURCE_DIR}/src/app/building/clone.c
    ${PROJECT_SOURCE_DIR}/src/app/building/construction.c
    ${PROJECT_SOURCE_DIR}/src/app/building/construction_building.c
    ${PROJECT_SOURCE_DIR}/src/app/building/construction_clear.c
    ${PROJECT_SOURCE_DIR}/src/app/building/construction_routed.c
    ${PROJECT_SOURCE_DIR}/src/app/building/construction_warning.c
    ${PROJECT_SOURCE_DIR}/src/app/building/count.c
    ${PROJECT_SOURCE_DIR}/src/app/building/destruction.c
    ${PROJECT_SOURCE_DIR}/src/app/building/dock.c
    ${PROJECT_SOURCE_DIR}/src/app/building/figure.c
    ${PROJECT_SOURCE_DIR}/src/app/building/government.c
    ${PROJECT_SOURCE_DIR}/src/app/building/granary.c
    ${PROJECT_SOURCE_DIR}/src/app/building/house.c
    ${PROJECT_SOURCE_DIR}/src/app/building/house_evolution.c
    ${PROJECT_SOURCE_DIR}/src/app/building/house_population.c
    ${PROJECT_SOURCE_DIR}/src/app/building/house_service.c
    ${PROJECT_SOURCE_DIR}/src/app/building/industry.c
    ${PROJECT_SOURCE_DIR}/src/app/building/list.c
    ${PROJECT_SOURCE_DIR}/src/app/building/maintenance.c
    ${PROJECT_SOURCE_DIR}/src/app/building/market.c
    ${PROJECT_SOURCE_DIR}/src/app/building/menu.c
    ${PROJECT_SOURCE_DIR}/src/app/building/model.c
    ${PROJECT_SOURCE_DIR}/src/app/building/properties.c
    ${PROJECT_SOURCE_DIR}/src/app/building/storage.c
    ${PROJECT_SOURCE_DIR}/src/app/building/warehouse.c
)
set(CITY_FILES
    ${PROJECT_SOURCE_DIR}/src/app/city/buildings.c
    ${PROJECT_SOURCE_DIR}/src/app/city/culture.c
    ${PROJECT_SOURCE_DIR}/src/app/city/data.c
    ${PROJECT_SOURCE_DIR}/src/app/city/data_private.c
    ${PROJECT_SOURCE_DIR}/src/app/city/emperor.c
    ${PROJECT_SOURCE_DIR}/src/app/city/entertainment.c
    ${PROJECT_SOURCE_DIR}/src/app/city/festival.c
    ${PROJECT_SOURCE_DIR}/src/app/city/figures.c
    ${PROJECT_SOURCE_DIR}/src/app/city/finance.c
    ${PROJECT_SOURCE_DIR}/src/app/city/gods.c
    ${PROJECT_SOURCE_DIR}/src/app/city/health.c
    ${PROJECT_SOURCE_DIR}/src/app/city/houses.c
    ${PROJECT_SOURCE_DIR}/src/app/city/labor.c
    ${PROJECT_SOURCE_DIR}/src/app/city/map.c
    ${PROJECT_SOURCE_DIR}/src/app/city/message.c
    ${PROJECT_SOURCE_DIR}/src/app/city/migration.c
    ${PROJECT_SOURCE_DIR}/src/app/city/military.c
    ${PROJECT_SOURCE_DIR}/src/app/city/mission.c
    ${PROJECT_SOURCE_DIR}/src/app/city/population.c
    ${PROJECT_SOURCE_DIR}/src/app/city/ratings.c
    ${PROJECT_SOURCE_DIR}/src/app/city/resource.c
    ${PROJECT_SOURCE_DIR}/src/app/city/sentiment.c
    ${PROJECT_SOURCE_DIR}/src/app/city/sound.c
    ${PROJECT_SOURCE_DIR}/src/app/city/trade.c
    ${PROJECT_SOURCE_DIR}/src/app/city/victory.c
    ${PROJECT_SOURCE_DIR}/src/app/city/view.c
    ${PROJECT_SOURCE_DIR}/src/app/city/warning.c
)
set(EMPIRE_FILES
    ${PROJECT_SOURCE_DIR}/src/app/empire/city.c
    ${PROJECT_SOURCE_DIR}/src/app/empire/empire.c
    ${PROJECT_SOURCE_DIR}/src/app/empire/object.c
    ${PROJECT_SOURCE_DIR}/src/app/empire/trade_prices.c
    ${PROJECT_SOURCE_DIR}/src/app/empire/trade_route.c
)
set(FIGURE_FILES
    ${PROJECT_SOURCE_DIR}/src/app/figure/action.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/combat.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/enemy_army.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/figure.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/formation.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/formation_enemy.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/formation_herd.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/formation_layout.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/formation_legion.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/image.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/movement.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/name.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/phrase.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/properties.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/route.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/service.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/sound.c
    ${PROJECT_SOURCE_DIR}/src/app/figure/trader.c
)
set(FIGURETYPE_FILES
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/animal.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/cartpusher.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/crime.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/docker.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/editor.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/enemy.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/entertainer.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/maintenance.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/market.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/migrant.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/missile.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/native.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/service.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/soldier.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/trader.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/wall.c
    ${PROJECT_SOURCE_DIR}/src/app/figuretype/water.c
)
set(GAME_FILES
    ${PROJECT_SOURCE_DIR}/src/app/game/animation.c
    ${PROJECT_SOURCE_DIR}/src/app/game/cheats.c
    ${PROJECT_SOURCE_DIR}/src/app/game/difficulty.c
    ${PROJECT_SOURCE_DIR}/src/app/game/file.c
    ${PROJECT_SOURCE_DIR}/src/app/game/file_editor.c
    ${PROJECT_SOURCE_DIR}/src/app/game/file_io.c
    ${PROJECT_SOURCE_DIR}/src/app/game/game.c
    ${PROJECT_SOURCE_DIR}/src/app/game/mission.c
    ${PROJECT_SOURCE_DIR}/src/app/game/orientation.c
    ${PROJECT_SOURCE_DIR}/src/app/game/resource.c
    ${PROJECT_SOURCE_DIR}/src/app/game/settings.c
    ${PROJECT_SOURCE_DIR}/src/app/game/speed.c
    ${PROJECT_SOURCE_DIR}/src/app/game/state.c
    ${PROJECT_SOURCE_DIR}/src/app/game/tick.c
    ${PROJECT_SOURCE_DIR}/src/app/game/time.c
    ${PROJECT_SOURCE_DIR}/src/app/game/tutorial.c
    ${PROJECT_SOURCE_DIR}/src/app/game/undo.c
)
set(INPUT_FILES
    ${PROJECT_SOURCE_DIR}/src/app/input/cursor.c
    ${PROJECT_SOURCE_DIR}/src/app/input/hotkey.c
    ${PROJECT_SOURCE_DIR}/src/app/input/input.c
    ${PROJECT_SOURCE_DIR}/src/app/input/joystick.c
    ${PROJECT_SOURCE_DIR}/src/app/input/keyboard.c
    ${PROJECT_SOURCE_DIR}/src/app/input/keys.c
    ${PROJECT_SOURCE_DIR}/src/app/input/mouse.c
    ${PROJECT_SOURCE_DIR}/src/app/input/scroll.c
    ${PROJECT_SOURCE_DIR}/src/app/input/touch.c
)
set(MAP_FILES
    ${PROJECT_SOURCE_DIR}/src/app/map/aqueduct.c
    ${PROJECT_SOURCE_DIR}/src/app/map/bookmark.c
    ${PROJECT_SOURCE_DIR}/src/app/map/bridge.c
    ${PROJECT_SOURCE_DIR}/src/app/map/building.c
    ${PROJECT_SOURCE_DIR}/src/app/map/building_tiles.c
    ${PROJECT_SOURCE_DIR}/src/app/map/desirability.c
    ${PROJECT_SOURCE_DIR}/src/app/map/elevation.c
    ${PROJECT_SOURCE_DIR}/src/app/map/figure.c
    ${PROJECT_SOURCE_DIR}/src/app/map/grid.c
    ${PROJECT_SOURCE_DIR}/src/app/map/image.c
    ${PROJECT_SOURCE_DIR}/src/app/map/image_context.c
    ${PROJECT_SOURCE_DIR}/src/app/map/natives.c
    ${PROJECT_SOURCE_DIR}/src/app/map/orientation.c
    ${PROJECT_SOURCE_DIR}/src/app/map/point.c
    ${PROJECT_SOURCE_DIR}/src/app/map/property.c
    ${PROJECT_SOURCE_DIR}/src/app/map/random.c
    ${PROJECT_SOURCE_DIR}/src/app/map/ring.c
    ${PROJECT_SOURCE_DIR}/src/app/map/road_access.c
    ${PROJECT_SOURCE_DIR}/src/app/map/road_aqueduct.c
    ${PROJECT_SOURCE_DIR}/src/app/map/road_network.c
    ${PROJECT_SOURCE_DIR}/src/app/map/routing.c
    ${PROJECT_SOURCE_DIR}/src/app/map/routing_data.c
    ${PROJECT_SOURCE_DIR}/src/app/map/routing_path.c
    ${PROJECT_SOURCE_DIR}/src/app/map/routing_terrain.c
    ${PROJECT_SOURCE_DIR}/src/app/map/soldier_strength.c
    ${PROJECT_SOURCE_DIR}/src/app/map/sprite.c
    ${PROJECT_SOURCE_DIR}/src/app/map/terrain.c
    ${PROJECT_SOURCE_DIR}/src/app/map/tiles.c
    ${PROJECT_SOURCE_DIR}/src/app/map/water.c
    ${PROJECT_SOURCE_DIR}/src/app/map/water_supply.c
)
set(SCENARIO_FILES
    ${PROJECT_SOURCE_DIR}/src/app/scenario/building.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/criteria.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/demand_change.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/distant_battle.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/earthquake.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/editor.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/editor_events.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/editor_map.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/emperor_change.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/empire.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/gladiator_revolt.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/invasion.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/map.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/price_change.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/property.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/random_event.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/request.c
    ${PROJECT_SOURCE_DIR}/src/app/scenario/scenario.c
)
set(GRAPHICS_FILES
    ${PROJECT_SOURCE_DIR}/src/app/graphics/arrow_button.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/button.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/font.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/generic_button.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/graphics.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/image.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/image_button.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/lang_text.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/menu.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/panel.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/rich_text.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/screen.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/screenshot.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/scrollbar.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/text.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/tooltip.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/video.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/warning.c
    ${PROJECT_SOURCE_DIR}/src/app/graphics/window.c
)
set(SOUND_FILES
    ${PROJECT_SOURCE_DIR}/src/app/sound/city.c
    ${PROJECT_SOURCE_DIR}/src/app/sound/effect.c
    ${PROJECT_SOURCE_DIR}/src/app/sound/music.c
    ${PROJECT_SOURCE_DIR}/src/app/sound/speech.c
    ${PROJECT_SOURCE_DIR}/src/app/sound/system.c
)
set(WIDGET_FILES
    ${PROJECT_SOURCE_DIR}/src/app/widget/city.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_bridge.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_building_ghost.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_figure.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_overlay_education.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_overlay_entertainment.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_overlay_health.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_overlay_other.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_overlay_risks.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_with_overlay.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/city_without_overlay.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/input_box.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/map_editor.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/map_editor_tool.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/minimap.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/scenario_minimap.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/top_menu.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/top_menu_editor.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/sidebar/city.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/sidebar/common.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/sidebar/editor.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/sidebar/extra.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/sidebar/military.c
    ${PROJECT_SOURCE_DIR}/src/app/widget/sidebar/slide.c
)
set(WINDOW_FILES
    ${PROJECT_SOURCE_DIR}/src/app/window/advisors.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/chief.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/education.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/entertainment.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/financial.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/health.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/imperial.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/labor.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/military.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/population.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/ratings.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/religion.c
    ${PROJECT_SOURCE_DIR}/src/app/window/advisor/trade.c
    ${PROJECT_SOURCE_DIR}/src/app/window/build_menu.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building_info.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/common.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/culture.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/distribution.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/figures.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/government.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/house.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/industry.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/military.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/terrain.c
    ${PROJECT_SOURCE_DIR}/src/app/window/building/utility.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/allowed_buildings.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/attributes.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/build_menu.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/demand_changes.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/edit_demand_change.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/edit_invasion.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/edit_price_change.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/edit_request.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/empire.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/invasions.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/map.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/price_changes.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/requests.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/special_events.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/start_year.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/starting_conditions.c
    ${PROJECT_SOURCE_DIR}/src/app/window/editor/win_criteria.c
    ${PROJECT_SOURCE_DIR}/src/app/window/cck_selection.c
    ${PROJECT_SOURCE_DIR}/src/app/window/city.c
    ${PROJECT_SOURCE_DIR}/src/app/window/config.c
    ${PROJECT_SOURCE_DIR}/src/app/window/difficulty_options.c
    ${PROJECT_SOURCE_DIR}/src/app/window/display_options.c
    ${PROJECT_SOURCE_DIR}/src/app/window/donate_to_city.c
    ${PROJECT_SOURCE_DIR}/src/app/window/empire.c
    ${PROJECT_SOURCE_DIR}/src/app/window/file_dialog.c
    ${PROJECT_SOURCE_DIR}/src/app/window/gift_to_emperor.c
    ${PROJECT_SOURCE_DIR}/src/app/window/hold_festival.c
    ${PROJECT_SOURCE_DIR}/src/app/window/hotkey_config.c
    ${PROJECT_SOURCE_DIR}/src/app/window/hotkey_editor.c
    ${PROJECT_SOURCE_DIR}/src/app/window/intermezzo.c
    ${PROJECT_SOURCE_DIR}/src/app/window/intro_video.c
    ${PROJECT_SOURCE_DIR}/src/app/window/labor_priority.c
    ${PROJECT_SOURCE_DIR}/src/app/window/logo.c
    ${PROJECT_SOURCE_DIR}/src/app/window/main_menu.c
    ${PROJECT_SOURCE_DIR}/src/app/window/message_dialog.c
    ${PROJECT_SOURCE_DIR}/src/app/window/message_list.c
    ${PROJECT_SOURCE_DIR}/src/app/window/military_menu.c
    ${PROJECT_SOURCE_DIR}/src/app/window/mission_briefing.c
    ${PROJECT_SOURCE_DIR}/src/app/window/mission_end.c
    ${PROJECT_SOURCE_DIR}/src/app/window/mission_selection.c
    ${PROJECT_SOURCE_DIR}/src/app/window/new_career.c
    ${PROJECT_SOURCE_DIR}/src/app/window/numeric_input.c
    ${PROJECT_SOURCE_DIR}/src/app/window/overlay_menu.c
    ${PROJECT_SOURCE_DIR}/src/app/window/plain_message_dialog.c
    ${PROJECT_SOURCE_DIR}/src/app/window/popup_dialog.c
    ${PROJECT_SOURCE_DIR}/src/app/window/resource_settings.c
    ${PROJECT_SOURCE_DIR}/src/app/window/select_list.c
    ${PROJECT_SOURCE_DIR}/src/app/window/set_salary.c
    ${PROJECT_SOURCE_DIR}/src/app/window/sound_options.c
    ${PROJECT_SOURCE_DIR}/src/app/window/speed_options.c
    ${PROJECT_SOURCE_DIR}/src/app/window/trade_opened.c
    ${PROJECT_SOURCE_DIR}/src/app/window/trade_prices.c
    ${PROJECT_SOURCE_DIR}/src/app/window/victory_dialog.c
    ${PROJECT_SOURCE_DIR}/src/app/window/victory_video.c
)
set(EDITOR_FILES
    ${PROJECT_SOURCE_DIR}/src/app/editor/editor.c
    ${PROJECT_SOURCE_DIR}/src/app/editor/tool.c
    ${PROJECT_SOURCE_DIR}/src/app/editor/tool_restriction.c
)
set(TRANSLATION_FILES
    ${PROJECT_SOURCE_DIR}/src/app/translation/english.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/french.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/german.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/italian.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/korean.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/polish.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/portuguese.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/russian.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/simplified_chinese.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/spanish.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/swedish.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/traditional_chinese.c
    ${PROJECT_SOURCE_DIR}/src/app/translation/translation.c
)

set(MACOSX_FILES "")
if(APPLE)
    set(MACOSX_FILES ${PROJECT_SOURCE_DIR}/res/julius.icns)
endif()

set(EMSCRIPTEN_FILES "")
if(EMSCRIPTEN_BUILD)
    set(EMSCRIPTEN_FILES ${PROJECT_SOURCE_DIR}/res/shell.html)
endif()

set(SOURCE_FILES
    ${PLATFORM_FILES}
    ${CORE_FILES}
    ${BUILDING_FILES}
    ${CITY_FILES}
    ${EMPIRE_FILES}
    ${FIGURE_FILES}
    ${FIGURETYPE_FILES}
    ${GAME_FILES}
    ${INPUT_FILES}
    ${MAP_FILES}
    ${SCENARIO_FILES}
    ${GRAPHICS_FILES}
    ${SOUND_FILES}
    ${WIDGET_FILES}
    ${WINDOW_FILES}
    ${EDITOR_FILES}
    ${TRANSLATION_FILES}
    ${TINYFD_FILES}
    ${PROJECT_SOURCE_DIR}/res/julius.rc
    ${MACOSX_FILES}
    ${EMSCRIPTEN_FILES}
)

if(EMSCRIPTEN_BUILD AND EMSCRIPTEN_LOAD_SDL_PORTS)
    set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
else()
    function(GET_SDL_EXT_DIR result module)
        if(NOT module STREQUAL "")
            set(module "_${module}")
        endif()
        set(SDL_LOCATION ${PROJECT_SOURCE_DIR}/ext/SDL2)
        file(GLOB children
            RELATIVE ${SDL_LOCATION}
            CONFIGURE_DEPENDS
            ${SDL_LOCATION}/SDL${module}
            ${SDL_LOCATION}/SDL2${module}
            ${SDL_LOCATION}/SDL${module}-*
            ${SDL_LOCATION}/SDL2${module}-*
        )
        foreach(child ${children})
            if(IS_DIRECTORY "${SDL_LOCATION}/${child}")
                set(${result} "${SDL_LOCATION}/${child}" PARENT_SCOPE)
                break()
            endif()
        endforeach()
    endfunction()

    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    if(EMSCRIPTEN_BUILD)
        set(LINK_MPG123 true)
    endif()
endif()

if(LINK_MPG123)
    find_package(MPG123 REQUIRED)
endif()

if(ANDROID_BUILD)
    add_library(${SHORT_NAME} SHARED ${SDL2_ANDROID_HOOK} ${SOURCE_FILES})
else()
    add_executable(${SHORT_NAME} WIN32 ${SOURCE_FILES})
endif()

if(EMSCRIPTEN_BUILD)
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    set_target_properties(${SHORT_NAME} PROPERTIES LINK_FLAGS "-lidbfs.js -s SINGLE_FILE=1 -s DEMANGLE_SUPPORT=1 -s DYNCALLS=1 --shell-file ${PROJECT_SOURCE_DIR}/res/shell.html -s INITIAL_MEMORY=268435456 -s ALLOW_MEMORY_GROWTH=1 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 -s SAFE_HEAP=1 -s INVOKE_RUN=0 -s 'EXPORTED_FUNCTIONS=[\"_main\"]' -s 'EXPORTED_RUNTIME_METHODS=[\"callMain\",\"FS\"]'  --bind")
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(APPLE)
    # generating a macOS icns file (see https://stackoverflow.com/a/20703594)
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/res/julius.icns
        COMMAND mkdir -p julius.iconset
        COMMAND sips -z 16 16    julius_256.png --out julius.iconset/icon_16x16.png
        COMMAND sips -z 32 32    julius_256.png --out julius.iconset/icon_16x16@2x.png
        COMMAND sips -z 32 32    julius_256.png --out julius.iconset/icon_32x32.png
        COMMAND sips -z 64 64    julius_256.png --out julius.iconset/icon_32x32@2x.png
        COMMAND sips -z 128 128  julius_256.png --out julius.iconset/icon_128x128.png
        COMMAND sips -z 256 256  julius_256.png --out julius.iconset/icon_128x128@2x.png
        COMMAND sips -z 256 256  julius_256.png --out julius.iconset/icon_256x256.png
        COMMAND sips -z 512 512  julius_512.png --out julius.iconset/icon_256x256@2x.png
        COMMAND iconutil -c icns julius.iconset
        COMMAND rm -R julius.iconset
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/res)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/res/julius.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")

    # setting variables that will populate Info.plist
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.bvschaik.julius")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${USER_FRIENDLY_NAME})
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(MACOSX_BUNDLE_ICON_FILE "julius.icns")
    set(MACOSX_BUNDLE_BUNDLE_VERSION
        "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${VERSION_REVISION}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MACOSX_BUNDLE_BUNDLE_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MACOSX_BUNDLE_BUNDLE_VERSION})

    set_target_properties(${SHORT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/res/Info.plist")

    set(DIRS "")
    set(LIBS "")

    # if SDL2 library is a framework, we need to indicate to CMake
    # the path to its dependencies. SDL2_LIBRARY contains two parts.
    # Example: /Library/Frameworks/SDL2.framework;-framework Cocoa
    # So strip out everything after the ';'
    string(REGEX REPLACE ";.*$" "" SDL2_LIB_DIR "${SDL2_LIBRARY}")
    if(EXISTS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
        list(APPEND DIRS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
    endif()

    # if SDL2_mixer library is a framework, we need to indicate to CMake
    # the path to its dependencies (Ogg.framework etc):
    if(EXISTS "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks")
        list(APPEND DIRS "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks")
    endif()

    # when installing, "fixup" automatically copies libraries inside the
    # bundle and links the binary against them
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app \"${LIBS}\" \"${DIRS}\")
    " BUNDLE DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(SDL2_INCLUDE_DIR)
    include_directories(${SDL2_INCLUDE_DIR})
endif()
if(SDL2_MIXER_INCLUDE_DIR)
    include_directories(${SDL2_MIXER_INCLUDE_DIR})
endif()

if(SYSTEM_LIBS)
    find_package(PNG)
endif()
if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${PNG_LIBRARIES})
else()
    if(SYSTEM_LIBS)
        message(STATUS "PNG was not found but that's ok: falling back to internal version")
    endif()
    include_directories("ext/png")
    target_sources(${SHORT_NAME} PRIVATE "${PNG_FILES}" "${ZLIB_FILES}")
endif()

if(VITA_BUILD)
    include_directories(
        $ENV{VITASDK}/arm-vita-eabi/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
elseif (SWITCH_BUILD)
    include_directories(
        ${DEVKITPRO}/portlibs/switch/include/SDL2
        ${DEVKITPRO}/libnx/include
        ${DEVKITPRO}/portlibs/switch/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

include_directories(ext)
include_directories(src)
if(MSVC)
    include_directories(ext/dirent)
endif()

if (VITA_BUILD)
    target_link_libraries (${SHORT_NAME}
        ${SDL2_MIXER_LIBRARY}
        ${SDL2_LIBRARY}
        vorbisfile
        vorbis
        mikmod
        mpg123
        FLAC
        ogg
        m
        SceAppUtil_stub
        SceAudio_stub
        SceCommonDialog_stub
        SceCtrl_stub
        SceDisplay_stub
        SceGxm_stub
        SceHid_stub
        SceMotion_stub
        SceIofilemgr_stub
        SceMotion_stub
        SceSysmodule_stub
        SceTouch_stub
        ScePower_stub
        ScePgf_stub
        SceAppMgr_stub
    )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__vita__")
    # this setting enables larger heap memory sizes on Vita, up to ~330 MB
    # useful for pre-loading videos into memory
    set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")
    vita_create_self(${SHORT_NAME}.self ${SHORT_NAME} UNSAFE UNCOMPRESSED)
    vita_create_vpk(${SHORT_NAME}.vpk ${VITA_TITLEID} ${SHORT_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${SHORT_NAME}
        FILE res/vita/icon0.png sce_sys/icon0.png
        FILE res/vita/bg.png sce_sys/livearea/contents/bg.png
        FILE res/vita/startup.png sce_sys/livearea/contents/startup.png
        FILE res/vita/template.xml sce_sys/livearea/contents/template.xml
    )
elseif(SWITCH_BUILD)
    target_link_libraries (${SHORT_NAME}
        SDL2_mixer
        SDL2
        EGL
        GLESv2
        glapi
        drm_nouveau
        vorbisidec
        modplug
        mpg123
        FLAC
        opusfile
        opus
        ogg
        m
        z
        nx
        stdc++
    )

    set(FLAGS
        -fPIE
        -L${DEVKITPRO}/portlibs/switch/lib
        -L${DEVKITPRO}/libnx/lib
    )

    foreach(flag ${FLAGS})
        set(FLAGS_GENERAL "${FLAGS_GENERAL} ${flag}")
    endforeach(flag ${FLAGS})

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS_GENERAL}")

    target_compile_options(${SHORT_NAME} PRIVATE ${FLAGS})
    add_custom_command(TARGET ${SHORT_NAME}
        POST_BUILD
        COMMAND ${DEVKITPRO}/tools/bin/nacptool --create "${SHORT_NAME}" "rsn8887" ${SWITCH_VERSION} ${SHORT_NAME}.nacp
        COMMAND ${DEVKITPRO}/tools/bin/elf2nro ${SHORT_NAME} ${SHORT_NAME}.nro --icon=${CMAKE_SOURCE_DIR}/res/switch/icon0.jpg --nacp=${SHORT_NAME}.nacp
    )
else()
    if (UNIX AND NOT APPLE AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang"))
        target_link_libraries(${SHORT_NAME} m)
    endif()
    if(ANDROID_BUILD)
        find_library(LOG_LIB log)
        target_link_libraries(${SHORT_NAME} ${LOG_LIB})
    endif()
    if(LINK_MPG123)
        target_link_libraries(${SHORT_NAME} ${MPG123_LIBRARY})
    endif()
    target_link_libraries(${SHORT_NAME} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
    if(NOT APPLE AND NOT ANDROID_BUILD)
        install(TARGETS ${SHORT_NAME} RUNTIME DESTINATION bin)
    endif()

    if(UNIX AND NOT APPLE)
        install(FILES "res/julius.desktop" DESTINATION "share/applications" RENAME "com.github.bvschaik.julius.desktop")
        install(FILES "res/julius.metainfo.xml" DESTINATION "share/metainfo" RENAME "com.github.bvschaik.julius.metainfo.xml")
        install(FILES "res/julius_32.png" DESTINATION "share/icons/hicolor/32x32/apps" RENAME "com.github.bvschaik.julius.png")
        install(FILES "res/julius_48.png" DESTINATION "share/icons/hicolor/48x48/apps" RENAME "com.github.bvschaik.julius.png")
        install(FILES "res/julius_256.png" DESTINATION "share/icons/hicolor/256x256/apps" RENAME "com.github.bvschaik.julius.png")
        install(FILES "res/julius_512.png" DESTINATION "share/icons/hicolor/512x512/apps" RENAME "com.github.bvschaik.julius.png")
    endif()

    # Unit tests
    if(NOT VITA_BUILD AND NOT SWITCH_BUILD AND NOT ANDROID_BUILD AND NOT EMSCRIPTEN_BUILD)
        enable_testing()
        add_subdirectory(test)
    endif()
endif()
