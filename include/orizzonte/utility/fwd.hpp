#pragma once

#ifndef FWD
#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)
#endif
