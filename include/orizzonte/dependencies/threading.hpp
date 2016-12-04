#pragma once

#include <atomic>
#include <chrono>

#include <thread>
#include <mutex>
#include <condition_variable>

#if defined(ORIZZONTE_USE_MEGANZ_MINGW_THREAD)
#include <mingw.thread.h>
#include <mingw.mutex.h>
#include <mingw.condition_variable.h>
#endif
