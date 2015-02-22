#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#include "message.h"

class MessageQueueItem {
  private:
    int id;
    int type;
    message_t message;

  public:
    MessageQueueItem(int id, int type) : id(id), type(type) {};
    MessageQueueItem(int id, int type, message_t &msg) : id(id), type(type), message(msg) {} ;
    void setMessage(message_t &msg) { message = msg; };
    int get_id(void) { return id; };
    int get_type(void) { return type; };
    message_t * get_message(void) { return (message_t *)(&message); };
    int get_size(void) { return sizeof(message); };
};

#endif
