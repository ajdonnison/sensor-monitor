#ifndef _MESSAGES_H
#define _MESSAGES_H

/**
 * Common message structure
 */
typedef struct _sensor_msg_t {
  uint16_t type;
  uint16_t value;
  uint16_t value_2;
  uint16_t value_3;
  uint16_t value_4;
  uint16_t adjust;
} sensor_msg_t;

typedef struct _time_msg_t {
  uint16_t year;
  uint16_t month;
  uint16_t day;
  uint16_t hour;
  uint16_t minute;
  uint16_t second;
} time_msg_t;

typedef struct _config_msg_t {
  uint32_t item;
  uint32_t value;
} config_msg_t;

typedef struct _message_t {
  uint32_t id;
  union _payload {
    time_msg_t time;
    sensor_msg_t sensor;
    config_msg_t config;
  } payload;
} message_t;

#endif
// vim:set ai sw=2:
