#include "file_editor.h"

#include "app/building/construction.h"
#include "app/building/menu.h"
#include "app/building/storage.h"
#include "app/city/data.h"
#include "app/city/message.h"
#include "app/city/victory.h"
#include "app/city/view.h"
#include "app/core/image.h"
#include "app/core/image_group_editor.h"
#include "app/empire/empire.h"
#include "app/empire/object.h"
#include "app/figure/enemy_army.h"
#include "app/figure/figure.h"
#include "app/figure/formation.h"
#include "app/figure/name.h"
#include "app/figure/route.h"
#include "app/figure/trader.h"
#include "app/figuretype/editor.h"
#include "app/figuretype/water.h"
#include "app/game/animation.h"
#include "app/game/file_io.h"
#include "app/game/state.h"
#include "app/game/time.h"
#include "app/map/aqueduct.h"
#include "app/map/building.h"
#include "app/map/desirability.h"
#include "app/map/elevation.h"
#include "app/map/figure.h"
#include "app/map/image.h"
#include "app/map/image_context.h"
#include "app/map/natives.h"
#include "app/map/property.h"
#include "app/map/random.h"
#include "app/map/road_network.h"
#include "app/map/routing_terrain.h"
#include "app/map/soldier_strength.h"
#include "app/map/sprite.h"
#include "app/map/terrain.h"
#include "app/map/tiles.h"
#include "app/scenario/distant_battle.h"
#include "app/scenario/editor.h"
#include "app/scenario/empire.h"
#include "app/scenario/invasion.h"
#include "app/scenario/map.h"
#include "app/scenario/property.h"
#include "app/sound/city.h"
#include "app/sound/music.h"

void game_file_editor_clear_data(void)
{
    city_victory_reset();
    building_construction_clear_type();
    city_data_init();
    city_data_init_scenario();
    city_message_init_scenario();
    game_state_init();
    game_animation_init();
    sound_city_init();
    building_menu_enable_all();
    building_clear_all();
    building_storage_clear_all();
    figure_init_scenario();
    enemy_armies_clear();
    figure_name_init();
    formations_clear();
    figure_route_clear_all();
    traders_clear();
    game_time_init(2098);
    scenario_invasion_clear();
}

static void clear_map_data(void)
{
    map_image_clear();
    map_building_clear();
    map_terrain_clear();
    map_aqueduct_clear();
    map_figure_clear();
    map_property_clear();
    map_sprite_clear();
    map_random_clear();
    map_desirability_clear();
    map_elevation_clear();
    map_soldier_strength_clear();
    map_road_network_clear();

    map_image_context_init();
    map_terrain_init_outside_map();
    map_random_init();
    map_property_init_alternate_terrain();
}

static void create_blank_map(int size)
{
    scenario_editor_create(size);
    scenario_map_init();
    clear_map_data();
    map_image_init_edges();
    city_view_set_camera(76, 152);
    city_view_reset_orientation();
}

static void prepare_map_for_editing(void)
{
    image_load_climate(scenario_property_climate(), 1, 0);

    empire_load(1, scenario_empire_id());
    empire_object_init_cities();

    figure_init_scenario();
    figure_create_editor_flags();
    figure_create_flotsam();

    map_tiles_update_all_elevation();
    map_tiles_update_all_water();
    map_tiles_update_all_earthquake();
    map_tiles_update_all_rocks();
    map_tiles_update_all_empty_land();
    map_tiles_update_all_meadow();
    map_tiles_update_all_roads();
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);
    map_natives_init_editor();
    map_routing_update_all();

    city_view_init();
    game_state_unpause();
}

void game_file_editor_create_scenario(int size)
{
    create_blank_map(size);
    prepare_map_for_editing();
}

int game_file_editor_load_scenario(const char *scenario_file)
{
    clear_map_data();
    if (!game_file_io_read_scenario(scenario_file)) {
        return 0;
    }
    scenario_map_init();

    prepare_map_for_editing();
    return 1;
}

int game_file_editor_write_scenario(const char *scenario_file)
{
    scenario_editor_set_native_images(
        image_group(GROUP_EDITOR_BUILDING_NATIVE),
        image_group(GROUP_EDITOR_BUILDING_NATIVE) + 2,
        image_group(GROUP_EDITOR_BUILDING_CROPS)
    );
    scenario_distant_battle_set_roman_travel_months();
    scenario_distant_battle_set_enemy_travel_months();

    return game_file_io_write_scenario(scenario_file);
}
