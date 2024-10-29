/* 
* Ficheiro header de hosptime
*/
#ifndef HOSPTIME_H_GUARD
#define HOSPTIME_H_GUARD

struct timespec setTime();

char *formatted_time(struct timespec ts);

#endif
