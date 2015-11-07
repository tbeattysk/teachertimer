#include <pebble.h>
#include "selector.h"
#include "scheduler.h"

#define NUM_MAIN_SELECTOR_MENU_SECTIONS 1
#define NUM_FIRST_SELECTOR_MENU_ITEMS 4

//make an 1A day for testing
struct schedule all_sched[4] ={
  {"A Day 1/3",
  {{7*3600 + 45*60,8*3600,"PCG"},
  {8*3600 + 5*60,9*3600,"Pd 1"},
  {9*3600,9*3600 + 50*60,"Pd 2"},
  {10*3600 + 15*60,11*3600 + 10*60,"Pd 3"},
  {11*3600 + 10*60,12*3600 + 5*60,"Pd 4"},
  {12*3600 + 10*60,12*3600 + 55*60,"Pd 6"},
  {13*3600 + 30*60,14*3600 + 15*60,"Pd 7"},
  {14*3600 + 20*60,15*3600 + 5*60,"Pd 8"}}},
  {"B Day 2/4",
  {{7*3600 + 45*60,8*3600,"PCG"},
  {8*3600 + 5*60,9*3600,"Pd 1"},
  {9*3600,9*3600 + 50*60,"Pd 2"},
  {10*3600 + 15*60,11*3600 + 10*60,"Pd 3"},
  {11*3600 + 10*60,12*3600 + 5*60,"Pd 4"},
  {12*3600 + 10*60,12*3600 + 55*60,"Pd 6"},
  {13*3600 + 30*60,14*3600 + 15*60,"Pd 7"},
  {14*3600 + 20*60,15*3600 + 5*60,"Pd 8"}}},
  {"A Day 5",
  {{7*3600 + 45*60,8*3600 + 45*60,"PCG"},
  {8*3600 + 50*60,9*3600 + 35*60,"Pd 1"},
  {9*3600 + 35*60,10*3600 + 20*60,"Pd 2"},
  {10*3600 + 45*60,11*3600 + 30*60,"Pd 3"},
  {11*3600 + 30*60,12*3600 + 15*60,"Pd 4"},
  {12*3600 + 55*60,13*3600 + 35*60,"Pd 6"},
  {13*3600 + 40*60,14*3600 + 20*60,"Pd 7"},
  {14*3600 + 25*60,15*3600 + 5*60,"Pd 8"}}},
  {"B Day 6",
  {{7*3600 + 45*60,8*3600 + 25*60,"PCG"},
  {8*3600 + 30*60,9*3600 + 15*60,"Pd 1"},
  {9*3600 + 15*60,10*3600 + 0*60,"Pd 2"},
  {10*3600 + 25*60,11*3600 + 10*60,"Pd 3"},
  {11*3600 + 10*60,11*3600 + 55*60,"Pd 4"},
  {12*3600 + 40*60,13*3600 + 25*60,"Pd 6"},
  {13*3600 + 30*60,14*3600 + 15*60,"Pd 7"},
  {14*3600 + 20*60,15*3600 + 5*60,"Pd 8"}}}};

static Window *s_selector_window;

static MenuLayer *selector_menu_layer;

static uint16_t selector_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data){
  return  NUM_MAIN_SELECTOR_MENU_SECTIONS;
}

static uint16_t selector_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_SELECTOR_MENU_ITEMS;

    default:
      return 0;
  }
}

static void selector_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Today's Schedule");
      break;
  }
}

static int16_t selector_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void selector_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in

  switch (cell_index->section) {
    case 0:
        menu_cell_title_draw(ctx, cell_layer, all_sched[cell_index->row].desc);
    break;
  }
}
// Here we capture when a user selects a menu item
void selector_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  define_schedule(all_sched[cell_index->row]);
  window_stack_pop(s_selector_window);
}

static void s_selector_window_load(Window *window){

  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  //   the menu layer
  selector_menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(selector_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = selector_menu_get_num_sections_callback,
    .get_num_rows = selector_menu_get_num_rows_callback,
    .get_header_height = selector_menu_get_header_height_callback,
    .draw_header = selector_menu_draw_header_callback,
    .draw_row = selector_menu_draw_row_callback,
    .select_click = selector_menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(selector_menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(selector_menu_layer));
}

static void s_selector_window_unload(Window *window){ 
 // Destroy the menu layer
  menu_layer_destroy(selector_menu_layer);
}

void select_schedule(){
  window_stack_push(s_selector_window, true);
}

void open_selector(){
   window_stack_push(s_selector_window, true);
}

void selector_init(void){
  s_selector_window = window_create();

  window_set_window_handlers(s_selector_window, (WindowHandlers) {
    .load = s_selector_window_load,
    .unload = s_selector_window_unload
  });
}

void selector_deinit(void) {
    window_destroy(s_selector_window);
}