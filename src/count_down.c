#include <pebble.h>
#include "count_down.h"
#include "main.h"

static Window *s_count_down_window;
static TextLayer *s_count_down_layer;
AppTimer *count_down_window_timer;
int selector=1;
const int durations[10] = {0,30,60,120,300,600,900,1200,1500,1800};
const char *labels[] = {"Cancel", "30 Sec"};

static void update_countdown() {
  if(selector<2){
    text_layer_set_text(s_count_down_layer, labels[selector]);
  } else{
    static char buffer[] = "00 Min";
    snprintf(buffer, sizeof(buffer), "%i Min", durations[selector]/60);
    text_layer_set_text(s_count_down_layer, buffer);
  }
  
}

void count_down_window_timer_callback(){
		//This will push the window off the screen
		if(window_stack_contains_window(s_count_down_window)){
      window_stack_pop(s_count_down_window);
    }
}
void count_down_up_click_handler(ClickRecognizerRef recognizer, void *context){
  if (selector<9){
    selector++;
    update_countdown();
  }
  app_timer_reschedule(count_down_window_timer, 2000);
}

void count_down_down_click_handler(ClickRecognizerRef recognizer, void *context){
  if (selector>0){
    selector--;
    update_countdown();
  }
  app_timer_reschedule(count_down_window_timer, 2000);
}

void count_down_click_config_provider(void *context){
  window_single_click_subscribe(BUTTON_ID_UP, count_down_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, count_down_down_click_handler);
}

static void s_count_down_window_load(Window *window){
  // Create time TextLayer
  s_count_down_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_count_down_layer, GColorClear);
  text_layer_set_text_color(s_count_down_layer, GColorBlack);
  text_layer_set_text(s_count_down_layer, "");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_count_down_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_count_down_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_count_down_layer));
  update_countdown();
}

static void s_count_down_window_unload(Window *window){ 
  startTimer(durations[selector]);
  selector=1;  
  text_layer_destroy(s_count_down_layer);
}

void initialize_timer(){
  window_stack_push(s_count_down_window, true);
  count_down_window_timer = app_timer_register(2000, (AppTimerCallback) count_down_window_timer_callback, NULL);
}

void count_down_init(void){
  s_count_down_window = window_create();

  window_set_window_handlers(s_count_down_window, (WindowHandlers) {
    .load = s_count_down_window_load,
    .unload = s_count_down_window_unload
  });
  
  //Register click providers
  window_set_click_config_provider(s_count_down_window, count_down_click_config_provider);
}
void count_down_deinit(void) {
    window_destroy(s_count_down_window);
}