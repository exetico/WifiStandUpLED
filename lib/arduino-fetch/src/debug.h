// Debugging macros
#ifndef DEBUGV
#define DEBUGV(...) \
  do                \
  {                 \
    (void)0;        \
  } while (0)
#endif
#define DEBUG 1
#if DEBUG
#define DEBUG_PORT Serial
#endif /* DEBUG */
#if DEBUG
#define DEBUG_FETCH(format, args...) \
  DEBUG_PORT.printf(("[FETCH-DEBUG] " + String(format) + "\n").c_str(), ##args)
#else
//#define DEBUG_FETCH(format, args...) \
    os_printf(("[FETCH-DEBUG] " + String(format)).c_str(), ##args)

#ifndef DEBUG_FETCH
#define DEBUG_FETCH(format, args...)
#endif
#endif /* DEBUG */
