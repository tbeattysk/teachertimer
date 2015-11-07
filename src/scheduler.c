#include <pebble.h>
#include "scheduler.h"
#include "selector.h"
#include "main.h"
  
#define NUM_MAIN_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 8

//make an 1A day for testing
static struct lesson todays_sched[8]= {{0, 0,"None"}};

static bool has_sched = false;

static Window *s_scheduler_window;

static MenuLayer *main_menu_layer;

bool is_lesson_active(){
  int time_now = get_todays_time();
  unsigned int i;
  for(i = 0; i<sizeof(todays_sched)/sizeof(todays_sched[0]); i++){
    if(todays_sched[i].start_time <= time_now && todays_sched[i].end_time > time_now){
      return true;
    }
  }
  return false;
}

bool is_lesson_pending(){
  int time_now = get_todays_time();
  if(todays_sched[0].start_time >= time_now){
    return true;
  }
  unsigned int i;
  for(i = 0; i<sizeof(todays_sched)/sizeof(todays_sched[0])-1; i++){
    if(todays_sched[i].end_time <= time_now && todays_sched[i+1].start_time > time_now){
      return true;
    }
  }
  return false;
}

struct lesson get_current_lesson(){
  int time_now = get_todays_time();
  unsigned int i;
  for(i = 0; i<sizeof(todays_sched)/sizeof(todays_sched[0]); i++){
    if(todays_sched[i].start_time <= time_now && todays_sched[i].end_time > time_now){
      return todays_sched[i];
    }
  }
  return todays_sched[0];
}

struct lesson get_pending_lesson(){
  int time_now = get_todays_time();
  struct lesson pending_lesson;
  unsigned int i;
  for(i = 0; i<sizeof(todays_sched)/sizeof(todays_sched[0])-1; i++){
    if(todays_sched[i].end_time <= time_now && todays_sched[i+1].start_time > time_now){
      pending_lesson.start_time = todays_sched[i].end_time;
      pending_lesson.end_time = todays_sched[i+1].start_time;
      strcpy (pending_lesson.label,todays_sched[i+1].label);
      return pending_lesson;
    }
  }
  return todays_sched[0];
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data){
  return  NUM_MAIN_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;

    default:
      return 0;
  }
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Lessons");
      break;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  static char buffer[] = "00:00 - 00:00";
  switch (cell_index->section) {
    case 0:
      snprintf(buffer, sizeof(buffer), "%i:%02i - %i:%02i",
               todays_sched[cell_index->row].start_time/3600,
               todays_sched[cell_index->row].start_time%3600/60,
               todays_sched[cell_index->row].end_time/3600,
               todays_sched[cell_index->row].end_time%3600/60
              );
      
      menu_cell_basic_draw(ctx, cell_layer, buffer, todays_sched[cell_index->row].label, NULL);
    break;
  }
}
// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    // This is the menu item with the cycling icon
    case 1:
      break;
  }

}

static void s_scheduler_window_load(Window *window){

  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  //   the menu layer
  main_menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(main_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(main_menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(main_menu_layer));
}

static void s_scheduler_window_unload(Window *window){ 
 // Destroy the menu layer
  menu_layer_destroy(main_menu_layer);
}

void open_scheduler(){
  window_stack_push(s_scheduler_window, true);
  if(!has_sched){
    select_schedule();
  }
}   

void define_schedule(struct schedule this_sched){
  unsigned int i;
  for(i=0; i<sizeof(this_sched.lessons)/sizeof(this_sched.lessons[i]); i++){
    todays_sched[i]=this_sched.lessons[i];
  }
  has_sched=true;
  window_stack_pop(s_scheduler_window);
  
  set_schedule_desc(this_sched.desc);
}

void sched_init(void){
  s_scheduler_window = window_create();

  window_set_window_handlers(s_scheduler_window, (WindowHandlers) {
    .load = s_scheduler_window_load,
    .unload = s_scheduler_window_unload
  });
}

void sched_deinit(void) {
    window_destroy(s_scheduler_window);
}