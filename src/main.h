#pragma once

void main_init(void);
void main_deinit(void);

void startTimer(time_t duration);
void set_schedule_desc(char *desc);
struct tm *get_time();
int get_todays_time();