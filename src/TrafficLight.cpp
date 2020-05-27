#include "TrafficLight.h"

#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive() {
  std::unique_lock<std::mutex> lock(_mutex);
  _condition.wait(lock, [this] { return !_queue.empty(); });

  T msg = std::move(_queue.front());
  _queue.pop_front();
  return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg) {
  std::lock_guard<std::mutex> lock(_mutex);
  _queue.emplace_back(msg);
  _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  auto msg = red;
  while (msg != green) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    msg = _queue.receive();
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(4000, 6000);
  int cycleDuration = dist(gen);

  auto lastSwitchedTime = std::chrono::system_clock::now();
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    auto tmpSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - lastSwitchedTime);

    int durationSinceSwitched = tmpSeconds.count();

    if (durationSinceSwitched >= cycleDuration) {
      _currentPhase = _currentPhase == red ? green : red;

      auto sentFuture =
          std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send,
                     &_queue, std::move(_currentPhase));
      sentFuture.wait();

      lastSwitchedTime = std::chrono::system_clock::now();
      cycleDuration = dist(gen);
    }
  }