
/** Used when you want to turn debugging OFF for all modules */
#define RCSW_ER_OFF 6

/**
 * Errors, as in something bad happened in your program.
 */
#define RCSW_DBG_ERROR 5

/**
 * Warnings, as in something happened athat is somewhat
 * alarming/sub-optimal, but not in of itself catastrophic/fatal.
 */
#define RCSW_ER_WARN 4

/**
 * Information, as in that you want your program to print when everything is
 * going smoothly. According to UNIX philosophy, if your program has nothing
 * interesting to say, it should say nothing if this is the debugging level.
 */
#define RCSW_DBG_INFO 3

/**
 * Debugging information, as in additional info that a developer can use to
 * help debug a problem.
 */
#define RCSW_ER_DEBUG 2

/**
 * Tracking information, for use on tough problems when you want the use to get
 * as much information from your program as possible.
 */
#define RCSW_DBG_TRACE 1
