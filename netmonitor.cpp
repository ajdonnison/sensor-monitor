/*
 * Sensor network monitor using the nRF2401+ radio.
 *
 * TODO:
 * - Change from file to socket-based comms with web client
 * - Store configs in database
 * - Allow pacing of messages (message queueing)
 */
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "message.h"
#include "msgqueue.h"
#include <fcntl.h>
#include <signal.h>
#include <queue>


using namespace std;
// CE Pin, CSN Pin, SPI Speed
RF24 radio(RPI_BPLUS_GPIO_J8_15,RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);

RF24Network network(radio);

// Address of our node
const uint16_t this_node = 0;

const unsigned long interval = 2000; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already
int sensor_list[] = { 2, 3, -1 };    // This should be dynamic

queue<MessageQueueItem> message_queue;

void
queueMessage(int sensor_id, int msg_type, message_t &msg)
{
  MessageQueueItem item(sensor_id, msg_type, msg);
  message_queue.push(item);
}

void
set_value(int node, int item, int value)
{
  message_t response;
  response.payload.config.item = item;
  response.payload.config.value = value;
  
  queueMessage(node, 'c', response);
}

void
set_time(int node)
{
  set_value(node, 't', time(NULL));
}

void
set_value_str(int node, string item, int value)
{
  if (item == "low_point") {
    set_value(node, 'l', value);
  } else if (item == "high_point") {
    set_value(node, 'h', value);
  } else if (item == "low_time") {
    set_value(node, 's', value);
  } else if (item == "high_time") {
    set_value(node, 'e', value);
  } else if (item == "reference") {
    set_value(node, 'r', value);
  } else if (item == "mode") {
    set_value(node, 'm', value);
  } else if (item == "time") {
    set_time(node);
  }
}

void
write_status(int node, message_t *msg)
{
  ostringstream ofs;
  ofs << "sensor." << node << ".status" << ends;
  ofstream ofile(ofs.str().c_str(), ios::out | ios::trunc);
  ofile << "low_value " << msg->payload.sensor.value / 10.0 << endl;
  ofile << "high_value " << msg->payload.sensor.value_2 / 10.0 << endl;
  ofile << "light " << msg->payload.sensor.value_3 << endl;
  ofile << "heater " << msg->payload.sensor.value_4 << endl;
  ofile.close();
}

void roll_log(int sig)
{
  int outfd;
  outfd = open("netmonitor.log",
    O_WRONLY | O_APPEND | O_CREAT,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
  );
  close(1);
  close(2);
  dup2(outfd, 1);
  dup2(outfd, 2);
  close(outfd);
}

void
handleMessage(void)
{
  RF24NetworkHeader header;
  message_t msg;

  network.read(header,&msg,sizeof(msg));
  switch (header.type) {
    case 's': // Sensor
      cout << "Temp: " << msg.payload.sensor.value / 10.0
      << "C, " << msg.payload.sensor.value_2 / 10.0 << "C "
      << "LIGHT(" << (msg.payload.sensor.value_3 ? "ON" : "OFF")
      << ") HEATER(" << (msg.payload.sensor.value_4 ? "ON" : "OFF")
      << ")" << endl;
      // Write out a sensor status file
      write_status(header.from_node, &msg);
      break;
    case 't': // Time
      cout << setw(4) << msg.payload.time.year
      << '-' << setfill('0') << setw(2) << msg.payload.time.month
      << '-' << setfill('0') << setw(2) << msg.payload.time.day
      << ' ' << setfill(' ') << setw(2) << msg.payload.time.hour
      << ':' << setfill('0') << setw(2) << msg.payload.time.minute
      << ':' << setfill('0') << setw(2) << msg.payload.time.second
      << endl;
      break;
    case 'C': // Current config
      // 
      switch (msg.payload.config.item) {
	case 'l':
	  cout << "Low Temp: " << msg.payload.config.value << "C" << endl;
	  break;
	case 'h':
	  cout << "High Temp: " << msg.payload.config.value << "C" << endl;
	  break;
	case 'r':
	  cout << "Reference: " << msg.payload.config.value << endl;
	  break;
	case 's':
	  cout << "Start Time: " << msg.payload.config.value << endl;
	  break;
	case 'e':
	  cout << "End Time: " << msg.payload.config.value << endl;
	  break;
      }
      break;
    case 'c': // Request config
      cout << "Remote requested config: " << msg.payload.config.item << endl;
      switch (msg.payload.config.item) {
	case 'a': // All
	  set_time(header.from_node);
	  // need to grab the other details.
	  break;
	case 't': // Time
	  set_time(header.from_node);
	  break;
	case 'h': // High point
	  break;
	case 'l':
	  break;
	case 'r':
	  break;
	case 's':
	  break;
	case 'e':
	  break;
      }
    break;
  }
}

void
checkForConfig(void)
{
  struct stat st;
  // Check for an update file
  if (stat("sensor.cfg", &st) == 0) {
    string line;
    string item;
    int value;
    int node = -1;
    ifstream fh("sensor.cfg");
    if (fh.is_open()) {
      while (getline(fh, line)) {
	stringstream _l(line);
	_l >> item >> value;
	if (item == "node") {
	  node = value;
	} else {
	  if (node != -1) {
	    set_value_str(node, item, value);
	  }
	}
      }
      fh.close();
    }
    unlink("sensor.cfg");
  }
}

void
requestAllConfig(void)
{
  message_t msg;
  for (int i = 0; sensor_list[i] != -1; i++) {
    queueMessage(sensor_list[i], 'r', msg);
  }
}

void
checkRequests(void)
{
}

void
sendPendingMessages(void)
{
  if (message_queue.empty()) {
    return;
  }
  MessageQueueItem item = message_queue.front();
  RF24NetworkHeader hdr(item.get_id(), item.get_type());
  network.write(hdr, item.get_message(), item.get_size());
  cout << "Sending to node " << item.get_id() << " type="  << item.get_type() << " msgtype=" << item.get_message()->payload.config.item << endl;
  message_queue.pop();
}

int main(int argc, char** argv) 
{
  bool startup = false;
  struct sigaction newsig;


  close(0);
  roll_log(0);
  
  newsig.sa_handler = roll_log;
  sigaction(SIGHUP, &newsig, NULL);

  daemon(1, 1);


  radio.begin();
  
  delay(5);
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  radio.printDetails();

  // Grab the list of sensors and send out requests for config
  while(1)
  {
    network.update();
    while ( network.available() ) {     // Is there anything ready for us?
      handleMessage();
    }		  
    checkForConfig();
    if (!startup) {
      startup = true;
      requestAllConfig();
    }
    checkRequests();
    sendPendingMessages();
    delay(10);
  }

  return 0;
}
// vi:set ai sw=2:
