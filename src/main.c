#include <pebble.h>
#include "main.h"
#include "count_down.h" 
#include "scheduler.h"
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_counting_down_layer;
static InverterLayer *s_time_bar_layer1;
static InverterLayer *s_time_bar_layer2;
static InverterLayer *s_time_gage_layer;
static TextLayer *s_start_time_layer;
static TextLayer *s_end_time_layer;
static TextLayer *s_schedule_desc_layer;
time_t count_down_end;
AppTimer *count_down_timer;
AppTimer *lesson_end_timer;
static struct lesson current_lesson;

//service for getting the local time in a tm structure
struct tm *get_time(){
  static time_t temp;
  temp = time(NULL); 
  return localtime(&temp);
}

//service for getting the time of day in seconds
int get_todays_time(){
  struct tm *tick_time = get_time();
  return tick_time->tm_hour*3600 + tick_time->tm_min*60 + tick_time->tm_sec;
}

//Update the watchface when no lesson is active, or when there are no more lessons in the day.
static void update_time() {
  // Get a tm structure
  struct tm *tick_time = get_time();

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, buffer);
  
  //draw countdown layer
   
}

//Update the watchface when there is an active lesson to be counting down. 
static void update_lesson(){

  int time_now = get_todays_time();
  //calculate fraction of class remaining
  int fraction_time_left = 100*(current_lesson.end_time - time_now)/(current_lesson.end_time - current_lesson.start_time);
  //redraw the time gage at top of window
  inverter_layer_destroy(s_time_gage_layer);
  s_time_gage_layer = inverter_layer_create(GRect((100-fraction_time_left)*1.36+4,8,fraction_time_left*1.35,4));
  layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_time_gage_layer));
  //update the minutes remaining in the lesson
  int difference = current_lesson.end_time-time_now;
  static char buffer[] = "00 Min";
  snprintf(buffer, sizeof(buffer), "%i Min", difference/60);
  text_layer_set_text(s_time_layer, buffer);
}

// Update the countdown for the watchface in any situation.
static void update_count_down(){
   int difference = count_down_end-time(NULL);
    if(difference < 60){
      static char buffer_2[] = "00 Sec";
      snprintf(buffer_2, sizeof(buffer_2), "%i Sec", difference);
      text_layer_set_text(s_counting_down_layer, buffer_2);
    }
    else{
      static char buffer_2[] = "00 Min";
      snprintf(buffer_2, sizeof(buffer_2), "%i Min", difference/60);
      text_layer_set_text(s_counting_down_layer, buffer_2);
    }
}

//tick_handler when lesson is active
static void tick_handler_lesson_active(struct tm *tick_time, TimeUnits units_changed) {
  if(count_down_end!=0){
    if(count_down_end-time(NULL)<120){
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler_lesson_active);
    }
    update_count_down();
  }
  update_lesson();
}


//tick_handler when lesson is pending
/*static void tick_handler_lesson_approaching(struct tm *tick_time, TimeUnits units_changed) {
  lesson_active=is_lesson_active();
  if(count_down_end!=0 && count_down_end-time(NULL)<120){
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  }
  update_lesson_approaching();
}*/

//tick handler when no lesson is active
static void tick_handler_clock(struct tm *tick_time, TimeUnits units_changed){
  if(count_down_end!=0){
    if(count_down_end-time(NULL)<120){
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler_clock);
    }
    update_count_down();
  }
  update_time();
}


//instructions for whenever a watchface is scheduled to change
static void load_watchface(){
  //vibes_long_pulse();
  if(is_lesson_active()){
      //Change tick handler to clock with count down timer in mind
    if(count_down_end!=0 && count_down_end-time(NULL)<120){
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler_lesson_active);
    }else{
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_lesson_active);
    }
     //display hidden layers
    layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer1), false);
    layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer2), false);
    layer_set_hidden(inverter_layer_get_layer(s_time_gage_layer), false); 
     //fetch lesson scheduled now
    current_lesson = get_current_lesson();
     //refresh clock
    update_count_down();
    update_lesson();
    text_layer_set_text(s_counting_down_layer, current_lesson.label);
    //display lesson start and end time
    static char buffer[] = "00:00";
    snprintf(buffer, sizeof(buffer), "%i:%02i",current_lesson.start_time/3600,current_lesson.start_time%3600/60);
    text_layer_set_text(s_start_time_layer, buffer);
    static char buffer2[] = "00:00";
    snprintf(buffer2, sizeof(buffer2), "%i:%02i",current_lesson.end_time/3600,current_lesson.end_time%3600/60);
    text_layer_set_text(s_end_time_layer, buffer2);
     //call this function again when the lesson is done
    int time_now = get_todays_time();
    lesson_end_timer = app_timer_register((current_lesson.end_time-time_now+5)*1000, (AppTimerCallback) load_watchface, NULL);
  } //
  //if lesson is pending pretend like there is a lesson for the remaining time
  else if(is_lesson_pending()){
     //Change tick handler to clock with count down timer in mind
    if(count_down_end!=0 && count_down_end-time(NULL)<120){
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler_lesson_active);
    }else{
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_lesson_active);
    }
     //display hidden layers
    layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer1), false);
    layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer2), false);
    layer_set_hidden(inverter_layer_get_layer(s_time_gage_layer), false); 
     //fetch lesson scheduled now
    current_lesson = get_pending_lesson();
     //refresh clock
    update_count_down();
    update_lesson();
    text_layer_set_text(s_counting_down_layer, current_lesson.label);
     //display lesson start and end time
    text_layer_set_text(s_start_time_layer, " Next lesson begins at");
    static char buffer2[] = "00:00";
    snprintf(buffer2, sizeof(buffer2), "%i:%02i",current_lesson.end_time/3600,current_lesson.end_time%3600/60);
    text_layer_set_text(s_end_time_layer, buffer2);
     //call this function again when the lesson is done
    int time_now = get_todays_time();
    lesson_end_timer = app_timer_register((current_lesson.end_time-time_now)*1000, (AppTimerCallback) load_watchface, NULL);
  }
  //otherwise display the time with no other
  else{
     //Change tick handler to clock with count down timer in mind
    if(count_down_end!=0 && count_down_end-time(NULL)<120){
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler_clock);
    }else{
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_clock);
    }
    //refresh screen
    update_count_down();
    update_time(); 
    //hide unneeded layers
    layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer1), true);
    layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer2), true);
    layer_set_hidden(inverter_layer_get_layer(s_time_gage_layer), true);
    text_layer_set_text(s_start_time_layer, "");
    text_layer_set_text(s_end_time_layer, "");
    text_layer_set_text(s_counting_down_layer,"");
    text_layer_set_text(s_schedule_desc_layer, "Day End");
    
  }
}

//for updating watchface when mid-lesson or when no lesson is present, needed to choose tick/timer
static void update_watchface(){
  if(is_lesson_active() || is_lesson_pending()){
    if(count_down_end!=0){
      if(count_down_end-time(NULL)<120){
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler_lesson_active);
      }
      update_count_down();
    }
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_lesson_active);
    update_lesson();
    
  }else{
    if(count_down_end!=0){
      if(count_down_end-time(NULL)<120){
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler_clock);
      }
      update_count_down();
    }
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_clock);
    update_time();
  }
}

//count down timer completion 
void count_down_timer_callback(){
		//This will push the window off the screen
	vibes_long_pulse();
  count_down_end=0;
  text_layer_set_text(s_counting_down_layer, current_lesson.label);
  update_watchface();
}

//click handler for count down timer
void up_click_handler(ClickRecognizerRef recognizer, void *context){
  app_timer_cancel(count_down_timer);
  initialize_timer();
}

//click handler for menu window
void select_click_handler(ClickRecognizerRef recognizer, void *context){
  open_scheduler();
}

void down_click_handler(ClickRecognizerRef recognizer, void *context){
  load_watchface();
}


void click_config_provider(void *context){
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void main_window_load(Window *window) {
  count_down_end = 0;
  
  //Create time bar background
  s_time_bar_layer1 = inverter_layer_create(GRect(2,5,140,10));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_time_bar_layer1));
  layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer1), true);
  //create timebar center
  s_time_bar_layer2 = inverter_layer_create(GRect(4,7,136,6));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_time_bar_layer2));
  layer_set_hidden(inverter_layer_get_layer(s_time_bar_layer2), true);
  //initialize time gage
  s_time_gage_layer = inverter_layer_create(GRect(4,8,135,4));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_time_gage_layer));
  layer_set_hidden(inverter_layer_get_layer(s_time_gage_layer), true);
  
  //start time layer
  s_start_time_layer = text_layer_create(GRect(2, 15, 142, 15));
  text_layer_set_background_color(s_start_time_layer, GColorClear);
  text_layer_set_text_color(s_start_time_layer, GColorBlack);
  text_layer_set_text(s_start_time_layer, "");
  text_layer_set_font(s_start_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_start_time_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_start_time_layer));
  
  //end time layer
  s_end_time_layer = text_layer_create(GRect(112, 15, 30, 15));
  text_layer_set_background_color(s_end_time_layer, GColorClear);
  text_layer_set_text_color(s_end_time_layer, GColorBlack);
  text_layer_set_text(s_end_time_layer, "");
  text_layer_set_font(s_end_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_end_time_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_end_time_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 45, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  //count down and class label
  s_counting_down_layer = text_layer_create(GRect(0, 20, 144, 26));
  text_layer_set_background_color(s_counting_down_layer, GColorClear);
  text_layer_set_text_color(s_counting_down_layer, GColorBlack);
  text_layer_set_text(s_counting_down_layer, "");
  text_layer_set_font(s_counting_down_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_counting_down_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_counting_down_layer));
  
   //schedule desc
  s_schedule_desc_layer = text_layer_create(GRect(0, 90, 144, 26));
  text_layer_set_background_color(s_schedule_desc_layer, GColorClear);
  text_layer_set_text_color(s_schedule_desc_layer, GColorBlack);
  text_layer_set_text(s_schedule_desc_layer, "No Schedule");
  text_layer_set_font(s_schedule_desc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_schedule_desc_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_schedule_desc_layer));
  // Make sure the time is displayed from the start
  
  update_time();

}

static void main_window_unload(Window *window) {
    // Destroy TextLayers
  text_layer_destroy(s_time_layer); 
  text_layer_destroy(s_counting_down_layer);
  text_layer_destroy(s_start_time_layer);
  text_layer_destroy(s_end_time_layer);
  text_layer_destroy(s_schedule_desc_layer);
  inverter_layer_destroy(s_time_bar_layer1);
  inverter_layer_destroy(s_time_bar_layer2);
  inverter_layer_destroy(s_time_gage_layer);
}

void startTimer(time_t duration){
  count_down_end = time(NULL)+duration;
  count_down_timer = app_timer_register(duration*1000, (AppTimerCallback) count_down_timer_callback, NULL);
  //for timers less than 2 minutes display seconds only
  update_watchface();
}

void set_schedule_desc(char *desc){
  static char buffer4[10];
  strcpy(buffer4,desc);
  text_layer_set_text(s_schedule_desc_layer, buffer4);
  load_watchface();
}

void main_init(void) {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  //Register click providers
  window_set_click_config_provider(s_main_window, click_config_provider);
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_clock);
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

}

void main_deinit(void) {
    window_destroy(s_main_window);
}