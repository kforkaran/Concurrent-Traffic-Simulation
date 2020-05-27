#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <condition_variable>
#include <deque>
#include <mutex>

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

template <class T>
class MessageQueue {
 public:
  void send(T &&msg);
  T receive();

 private:
  std::deque<T> _queue;
  std::condition_variable _condition;
  std::mutex _mutex;
};

enum TrafficLightPhase { red, green };

class TrafficLight : TrafficObject {
 public:
  TrafficLight();
  void waitForGreen();
  void simulate();

  TrafficLightPhase getCurrentPhase();

 private:
  std::condition_variable _condition;
  std::mutex _mutex;

  void cycleThroughPhases();
  TrafficLightPhase _currentPhase;
  MessageQueue<TrafficLightPhase> _queue;
};

#endif