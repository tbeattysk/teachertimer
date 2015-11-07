#pragma once

struct lesson{
  int start_time; //seconds from beginning of day
  int end_time; //seconds from beginning of day
  //int reminder_interval; //minutes between alarms 0 indicates no reminders
  //int warning_headway;// minutes before end of class to set alarm 0 indicates no warning
  //char class;
  char label[5];
}; 


struct schedule{
  char desc[10];
  struct lesson lessons[10];
  //default reminder_interval
  //default warning_headway
};

void sched_init(void);
void sched_deinit(void);

void open_scheduler();

bool is_lesson_active();
bool is_lesson_pending();
struct lesson get_current_lesson();
struct lesson get_pending_lesson();
void define_schedule(struct schedule this_sched);