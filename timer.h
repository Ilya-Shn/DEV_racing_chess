#pragma once
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>

class Timer {
public:
    Timer();
    ~Timer();

    void setTickRate(int milliseconds);
    void start(std::function<void()> callback);
    void stop();

private:
    void timerLoop(std::function<void()> callback);

    int tick_rate_ms_;
    std::atomic<bool> running_;
    std::thread timer_thread_;
};