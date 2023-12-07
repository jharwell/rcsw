/**
 * \file grind.h
 * \ingroup tool
 *
 * \brief Simple statistic and metric-gathering engine.
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"
#include "rcsw/common/flags.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#if defined(RCSW_NO_GRIND)
#define RCSW_GRIND_START(the_grinder, name)
#define RCSW_GRIND_END(the_grinder, name)
#define RCSW_GRIND_COUNT(the_grinder, name)
#define RCSW_GRIND_TICK(the_grinder, name)
#else
/**
 * \brief Compile-time macro for \ref grind_capture_start()
 */
#define RCSW_GRIND_START(the_grinder, name) grind_capture_start(the_grinder, \
                                                                name)

/**
 * \brief Compile-time macro for \ref grind_capture_end()
 */
#define RCSW_GRIND_END(the_grinder, name) grind_capture_end(the_grinder, name)


/**
 * \brief Compile-time macro for \ref grind_capture_count()
 */
#define RCSW_GRIND_COUNT(the_grinder, name) grind_capture_count(the_grinder, \
                                                                name)

/**
 * \brief Compile-time macro for \ref grind_capture_tick()
 */
#define RCSW_GRIND_TICK(the_grinder, name) grind_capture_tick(the_grinder, name)

#endif

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief Compute all measurements within an absolute time interval.
 *
 * The grinder timeout will be used. \see RCSW_GRIND_AUTO_RESET for what
 * happens on a timeout.
 *
 * This behavior is useful for gathering timing metrics on a small piece of a
 * large system (e.g. how long interrupt handlers run).
 *
 * If this flag is not passed, then all statistics will be relatively computed;
 * that is regardless of the "real" amount of time (wall time) all statistics
 * take, all utilization percentages will always add to 100%.
 */
#define RCSW_GRIND_INTERVAL (1 << RCSW_MODFLAGS_START)

/**
 * \brief Automatically reset statistics for some/all grindees.
 *
 * Behavior with \ref RCSW_GRIND_INTERVAL : At the end of the timeout
 * interval, reset statistics for \p all grindees.
 *
 * Behavior without \ref RCSW_GRIND_INTERVAL : once a grindee instance
 * becomes full it will be reset upon the next request to gather stats (other
 * stat instances are not affected).
 *
 * If this flag is not passed, then stats for a grindee will never be set, and
 * once full no more stats will be gathered.
 */
#define RCSW_GRIND_RESET_AUTO (1 << (RCSW_MODFLAGS_START + 1))

/**
 * \brief Automatically report statistics for a grindee once full to stdout.
 *
 * If not passed: stat gathering on a grindee whose stats are full to block
 * until statistics for that instance are reported unless \ref
 * RCSW_GRIND_RESET_AUTO is passed.
 *
 * If both \ref RCSW_GRIND_REPORT_AUTO and \ref RCSW_GRIND_RESET_AUTO are
 * passed, then statistics for a grindee will be reported and then reset, the
 * exact nature of the reset being determined by the presence/absence of
 * \ref RCSW_GRIND_INTERVAL.
 */
#define RCSW_GRIND_REPORT_AUTO (1 << (RCSW_MODFLAGS_START + 2))

/**
 * \brief Require that statistics for any grindee to be full when reported.
 *
 * If passed: reporting statistics on grindees that are not full will fail.
 *
 * If not passed: statistics can be reported for any grindee at any time.
 *
 * \note Has no effect on whether or not statistics for a "full" grindee will be
 * reset/reported when full; it only comes into play WHEN the \ref
 * grind_report() function has been called (either automatically if \ref
 * RCSW_GRIND_REPORT_AUTO is passed, or manually by the calling application.
 */
#define RCSW_GRIND_REPORT_REQ_FULL (1 << (RCSW_MODFLAGS_START + 3))

/**
 * \brief Show collected datapoints when reporting.
 *
 * Valid in any mode.
 */
#define RCSW_GRIND_REPORT_DATAPOINTS (1 << (RCSW_MODFLAGS_START + 4))

/**
 * \brief Show histogram of datapoints when reporting.
 *
 * Valid in any mode.
 */
#define RCSW_GRIND_REPORT_HISTOGRAM (1 << (RCSW_MODFLAGS_START + 5))

/**
 * \brief Max length of all grindee names.
 */
#define RCSW_GRINDEE_NAMELEN 32

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief The type of "grinding" to do; sets the domain for a \ref grinder.
 */
enum grind_mode {
  /**
   * Enable usage of \ref grind_capture_count() to capture execution counts.
   *
   * All timing related flags are ignored in this mode. All stats must be
   * reported/cleared manually for each grindee.
   */
  ekRCSW_GRIND_COUNT,

  /**
   * Enable usage of \ref grind_capture_start() / \ref grind_capture_end() to
   * capture duration.
   *
   * Durations are captured by (end - start) time. Execution counts of the
   * section between \ref grind_capture_start() and \ref grind_capture_end() are
   * also captured.
   */
  ekRCSW_GRIND_DURATION,

  /**
   * Enable usage of \ref grind_capture_tick() to capture ticks.
   *
   * Ticks are captured by tick= last tick - current time; this is the same as
   * the period of the thing that is ticking.  Execution counts of \ref
   * grind_capture_tick() are also captured.
   */
  ekRCSW_GRIND_PERIOD,
};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief A semantic unit for collecting timing/execution count information.
 *
 * Can be a function, a section of code, etc. VERY
 * flexible. Identified/differentiated from other grindees by name.
 */
struct grindee {
  /** Name of grindee */
  char                name[RCSW_GRINDEE_NAMELEN];

  /** How many times the section has been entered since last datapoint */
  size_t               count;

  /**
   * Location of next datapoint to insert/count of datapoints gathered thus
   * far.
   */
  size_t               tindex;

  /** Flag indicating if the stat table is full */
  bool_t               full;

  /**
   * Sample table for grindee; all data stored here.
   */
  size_t*              table;

  /** Size of sample table */
  size_t               tsize;

  /**
   * \brief Low-level implementation details for \ref grinder.
   *
   * There are currently two domains: duration and tick. See \ref grind_mode for
   * details.
   */
  union grind_mode_impl {
    struct {
      /**
       * Temporary variable for holding accumulated execution time.
       */
      size_t          accum;

      /** Duration start time. */
      struct timespec start;

      /** Duration end time. */
      struct timespec end;

      /**
       * The last call to \ref grind_capture_start() has not yet been followed
       * by a call to \ref grind_capture_end() (i.e., grinding is in progress).
       */
      bool_t          active;
    } duration;

    struct {
      /** Recorded time of current tick */
      struct timespec current;

      /** Is this the first tick capture since last reset? */
      bool_t          first;

      /**
       * Temporary variable for holding accumulated execution time.
       */
      size_t          accum;
    } tick;
  } domain;
};

/** Grinder initialization parameters */
struct grind_params {
  /**
   * Names of grindees
   */
  char**          names;

  /**
   * Number of grindee names in \ref grind_params.names.
   */
  size_t          n_inst;

  /**
   * Configuration flags. See \ref grinder.flags for valid flags.
   */
  uint32_t        flags;
  enum grind_mode mode;

  /**
   * How many collected samples will be averaged to form a single datapoint for
   * each grindee.
   */
  size_t          res;

  /** Size of stat table for each grindee (homogeneous) */
  size_t          tsize;

  /**
   * Timeout to reset statistics.
   */
  struct timespec interval;

  /**
   * Callback to return the current time for a \ref grindee. Can be NULL in
   * \ref ekRCSW_GRIND_COUNT mode.
   */
  struct timespec (*gettime)(void);
};

/**
 * Structure for tracking timing, execution count, etc. information gathered
 * about a running application.
 */
struct grinder {
  /** What is our mode of operation? */
  enum grind_mode mode;

  /** How many \ref grindee are we managing? */
  size_t                     n_inst;

  /**
   * How many counts of a grindee being executed will be averaged to form a
   * single entry in the grindee's metric table
   */
  size_t                     res;

  /** One or more grindees are ready to report */
  bool_t                     avail;

  /** That the timeout interval has begun (timing-related modes only) */
  bool_t                     in_interval;

  /**
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_GRIND_REPORT_DATAPOINTS
   * - \ref RCSW_GRIND_REPORT_HISTOGRAM
   * - \ref RCSW_GRIND_REPORT_AUTO
   * - \ref RCSW_GRIND_REPORT_REQ_FULL
   * - \ref RCSW_GRIND_RESET_AUTO
   * - \ref RCSW_GRIND_INTERVAL
   *
   * All other flags are ignored.
   */
  uint32_t                   flags;

  struct     grindee         *grindees;

  /** Timeout to reset statistics */
  struct     timespec        interval;

  /**
   * Time call to \ref grind_capture_start()/\ref grind_capture_tick() was
   * made. Used for determining if/when to reset statistics for ALL instances if
   * \ref RCSW_GRIND_INTERVAL was passed.
   */
  struct  timespec           interval_start;

  /**
   * Callback to return the current time for a \ref grindee. Can be NULL in
   * \ref ekRCSW_GRIND_COUNT mode.
   */
  struct timespec (*gettime)(void);
};


/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Initialize a \ref grinder for statistics collection.
 *
 * This function initializes the statistics global data structure to gather
 * statistics on all stat instances encountered during execution.
 *
 * \param rb_in An application allocated handle for the grinder. Can be NULL,
 *        depending on if \ref RCSW_NOALLOC_HANDLE is passed in \ref
 *        grind_params.flags.
 *
 * \return The initialized grinder, or NULL if an error occurred.
 */
struct grinder * grind_init(struct grinder* grind_in,
                            const struct grind_params* const params);

/**
 * \brief Shutdown grinding.
 *
 * Any further use of the API is undefined until \ref grind_init() is called
 * again.
 */
void grind_destroy(struct grinder * const the_grinder);

/**
 * \brief Mark the start of a single [begin, end] datapoint for a \ref grindee.
 *
 * Only valid with \ref ekRCSW_GRIND_DURATION.
 *
 * \return \ref status_t
 *
 */
status_t grind_capture_start(struct grinder* const the_grinder,
                             const char* const name);

/**
 * \brief Mark the end of a single [begin, end] datapoint for a \ref grindee.
 *
 * Only valid with \ref ekRCSW_GRIND_DURATION.
 *
 * \return \ref status_t
 */
status_t grind_capture_end(struct grinder * const the_grinder,
                           const char *const name);

/**
 * \brief Increment the execution count for the \ref grindee.
 *
 * Only valid with \ref ekRCSW_GRIND_COUNT.
 *
 * \return \ref status_t
 */
status_t grind_capture_count(struct grinder * const the_grinder,
                             const char *const name);

/**
 * \brief Capture the current time/tick for the \ref grindee.
 *
 * Only valid with \ref ekRCSW_GRIND_PERIOD.
 *
 * \return \ref status_t
 */
status_t grind_capture_tick(struct grinder * const the_grinder,
                            const char *const name);

/**
 * \brief Report stats for all pending instances
 *
 * \note Should only need to be called if \ref RCSW_GRIND_REPORT_AUTO was not
 * passed..
 */
void grind_report_all(const struct grinder * const the_grinder);

/**
 * \brief Report statistics for a \ref grindee by name.
 *
 * Report stats for the grindee and then flushes/resets them. The format of the
 * reported stats depends on initialization parameters.
 *
 * \return \ref status_t
 */
status_t grind_report(const struct grinder * const the_grinder,
                      struct grindee *const fm);

/**
 * \brief Report utilization results to stdout.
 *
 * Utilization reported in relative or absolute time, depending on flags, and
 * the result written to stdout via \ref DPRINTF.
 *
 * \return \ref status_t
 */
status_t grind_report_utilization(const struct grinder * const the_grinder);

/**
 * \brief Report utilization results to a buffer.
 *
 * Utilization is reported in either in relative or absolute time, depending on
 * flags, and the result written to the buffer, which MUST have enough space.
 *
 * \return # bytes written to \p buf or -1 on ERROR.
 */
int grind_report_utilization2(const struct grinder * const the_grinder,
                              char * const buf);

/**
 * \brief Reset statistics for all grindees.
*/
void grind_reset_all(struct grinder * const the_grinder);

/**
 * \brief Reset statistics for a single  \ref grindee.
 */
void grind_reset(const struct grinder * const the_grinder,
                 struct grindee * const grindee);

/**
 * \brief Given the name of a grindee, get the index it maps to.
 *
 * \return The index the grindee module name maps to, or -1 on ERROR
 */
int grindee_lookup(const struct grinder * const the_grinder,
                   const char *const name) RCSW_PURE;

/**
 * \brief Find the largest datapoint of a \ref grindee.
 *
 * This function searches through all gathered datapoints of a grindee and finds
 * the largest one. Can be called at any time.
 *
 * \return The max datapoint.
 */
size_t grindee_data_max(const struct grindee* const grindee);

/**
 * \brief Find the smallest datapoint of a \ref grindee.
 *
 * This function searches through all gathered datapoints of a grindee
 * and finds the smallest one. Can be called at any time.
 *
 * \return The min time
 */
size_t grindee_data_min(const struct grindee * const grindee);

/**
 * \brief Sum all datapoints for the \ref grindee.
 */
size_t grindee_data_sum(const struct grindee * const grindee);


/**
 * \brief Sum ALL \ref grindee datapoints
 *
 * Compute the sum of all datapoints for all grindees. It is only a true total
 * if the resolution for grindees is 1. Otherwis it is a sum of the averages
 * according to the resolution parameter.
 */
size_t grind_sum_all(const struct grinder * const the_grinder);

/**
 * \brief Get utilization for a \ref grindee.
 *
 * For timing-related grinding:
 *
 * If \ref RCSW_GRIND_INTERVAL passed: utilization is fraction of the timeout
 * interval attributed to the specified grindee. Otherwise it is the fraction of
 * the total amount of time spent by all grindees attributed to the specified
 * grindee.
 *
 * For counting relating grinding: utilization is the fraction of the total # of
 * execution counts for all grindees attributed to the specified grindee.
 *
 * \return The utilization, or -1 on ERROR.
 */
double grind_get_utilization(struct grinder * the_grinder,
                             const char * const name);
