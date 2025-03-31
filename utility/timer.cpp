#include "timer.h"
#include <iostream>

Timer::Timer() : tick_rate_ms_(100), running_(false) {
}

Timer::~Timer() {
    stop();
}

void Timer::setTickRate(int milliseconds) {
    tick_rate_ms_ = milliseconds;
}

void Timer::start(std::function<void()> callback) {
    if (running_) {
        stop();
    }

    running_ = true;
    timer_thread_ = std::thread(&Timer::timerLoop, this, callback);
}

void Timer::stop() {
    if (running_) {
        running_ = false;
        if (timer_thread_.joinable()) {
            timer_thread_.join();
        }
    }
}

void Timer::timerLoop(std::function<void()> callback) {
    while (running_) {
        auto start = std::chrono::steady_clock::now();

        callback();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();

        int sleep_time = tick_rate_ms_ - static_cast<int>(elapsed);
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
}

