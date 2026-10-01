/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup tool
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/tool/grind.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "rcsw/al/clock.h"
#define RCSW_ER_MODID LOG4CL_GRIND
#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "profile", "grind")
#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define GRINDER_TYPE(the_grinder) \
  ((RCSW_GRIND_MODE_COUNT == (the_grinder)->mode) ? "counting" : "timing")

/*******************************************************************************
 * Private API
 ******************************************************************************/
/**
 * \brief Find the largest datapoint of a \ref grindee.
 *
 * This function searches through all gathered datapoints of a grindee and finds
 * the largest one. Can be called at any time.
 *
 * \return The max datapoint.
 */
static uint64_t grindee_data_max(const struct grindee* const grindee) {
  RCSW_FPC_NV(0, NULL != grindee, grindee->tindex > 0);

  uint64_t max = 0;

  for (size_t i = 0; i < grindee->tindex; ++i) {
    max = RCSW_MAX(max, grindee->table[i]);
  }
  return max;
}

/**
 * \brief Find the smallest datapoint of a \ref grindee.
 *
 * This function searches through all gathered datapoints of a grindee
 * and finds the smallest one. Can be called at any time.
 *
 * \return The min time
 */
static uint64_t grindee_data_min(const struct grindee* const grindee) {
  RCSW_FPC_NV(0, NULL != grindee, grindee->tindex > 0);

  uint64_t min = UINT64_MAX;

  for (size_t i = 0; i < grindee->tindex; ++i) {
    min = RCSW_MIN(min, grindee->table[i]);
  }
  return min;
}

/**
 * \brief Sum all datapoints for the \ref grindee.
 */
static uint64_t grindee_data_sum(const struct grindee* const grindee) {
  RCSW_FPC_NV(0, NULL != grindee);

  uint64_t sum = 0;
  for (size_t i = 0; i < grindee->tindex; ++i) {
    sum += grindee->table[i];
  }
  return sum;
}

/**
 * \brief Report basic statistics for a \ref grindee.
 */
static void grind_report_stats(const struct grinder* the_grinder,
                               const struct grindee* grindee) {
  DPRINTF("----------------------------------------\n");
  DPRINTF("               STATISTICS               \n");
  DPRINTF("----------------------------------------\n");

  /* show statistics metadata */
  DPRINTF("Execution count    : %zu + %zu = %zu\n",
          grindee->tsize * the_grinder->res,
          grindee->count,
          (grindee->tsize * the_grinder->res) + grindee->count);
  DPRINTF("Datapoints         : %zu\n", grindee->tindex);

  /* get minimum time and maximum times */
  uint64_t min = grindee_data_min(grindee);
  uint64_t max = grindee_data_max(grindee);

  DPRINTF("Maximum            : %" PRIu64 " ns\n", max);
  DPRINTF("Minimum            : %" PRIu64 " ns\n", min);

  /* get average time */
  uint64_t sum  = grindee_data_sum(grindee);
  double   mean = (double)sum / (double)grindee->tindex;

  DPRINTF("Mean               : %.8e ns\n", mean);

  /* calculate standard deviation and variance */
  double std_dev  = 0.0;
  double variance = 0.0;
  for (size_t i = 0; i < grindee->tindex; i++) {
    variance += pow((double)grindee->table[i] - mean, 2.0);
  }

  variance /= (double)grindee->tindex;
  std_dev = sqrt(variance);

  DPRINTF("Variance           : %.8e\n", variance);
  DPRINTF("Standard Deviation : %.8e\n", std_dev);

  DPRINTF("\n\n");
} /* grind_report_stats() */

/**
 *  \brief Create and print a histogram for a grindee
 */
static void grind_report_hist(const struct grinder* const the_grinder,
                              const struct grindee* const grindee) {
  DPRINTF("----------------------------------------\n");
  DPRINTF("               HISTOGRAM                \n");
  DPRINTF("----------------------------------------\n");

  /* get minimum time and maximum times */
  uint64_t min = grindee_data_min(grindee);
  uint64_t max = grindee_data_max(grindee);

  /* compute size of each of the 50 bins for histogram */
  double binsize = ((double)(max - min)) / (50);

  /* fill histogram array */
  size_t hist_arr[50];
  memset(hist_arr, 0, sizeof(hist_arr));

  size_t bin_count_max = 0;
  for (size_t i = 0; i < grindee->tindex; ++i) {
    for (size_t j = 1; j <= 50; ++j) {
      bool_t match =
        (j == 50) ? RCSW_IS_BETWEENC((double)grindee->table[i],
                                     (double)min + ((double)(j - 1) * binsize),
                                     (double)min + ((double)j * binsize))
                  : RCSW_IS_BETWEENHO((double)grindee->table[i],
                                      (double)min + ((double)(j - 1) * binsize),
                                      (double)min + ((double)j * binsize));
      if (match) {
        hist_arr[j - 1]++;
        bin_count_max = RCSW_MAX(bin_count_max, hist_arr[j - 1]);
      }
    } /* for(j...) */
  } /* for(i..) */

  size_t xmax = RCSW_MIN(bin_count_max, (size_t)40);

  /* print histogram */
  for (size_t i = 0; i < 50; ++i) {
    if (RCSW_GRIND_MODE_COUNT == the_grinder->mode) {
      DPRINTF("%8zu | ", i);
    } else {
      uint64_t lower = (uint64_t)((double)min + ((double)i * binsize));
      DPRINTF("%8" PRIu64 ".%09" PRIu64 " sec | ",
              lower / RCSW_E9,
              lower % RCSW_E9);
    }
    double scale =
      (bin_count_max > 0) ? (double)hist_arr[i] / (double)bin_count_max : 0.0;
    size_t fill = (size_t)(scale * (double)xmax);
    for (size_t j = 0; j < fill; ++j) {
      DPRINTF("*");
    } /* for(j..) */
    DPRINTF("\n");
    int rc = fflush(NULL);
    ER_ASSERT(rc == 0, "Failed to flush");
  }
  DPRINTF("\n\n");
} /* grind_report_hist() */

/**
 * \brief Report the collected datapoints for a grindee to stdout.
 */
static void grind_report_datapoints(const struct grinder*       the_grinder,
                                    const struct grindee* const grindee) {
  DPRINTF("----------------------------------------\n");
  DPRINTF("               DATAPOINTS               \n");
  DPRINTF("----------------------------------------\n");

  DPRINTF("    Index        Value\n");
  DPRINTF("-------------+--------------------------\n");

  for (size_t i = 0; i < grindee->tindex; ++i) {
    if (RCSW_GRIND_MODE_COUNT == the_grinder->mode) {
      DPRINTF("%8zu            %08" PRIu64 "\n", i, grindee->table[i]);
    } else {
      struct timespec ts = utils_monons2ts(grindee->table[i]);
      DPRINTF("%8zu            %8" PRIu64 ".%09ld sec\n",
              i,
              (uint64_t)ts.tv_sec,
              (long)ts.tv_nsec);
    }
  } /* for() */
}

/**
 * \brief Report timing info for a \ref grindee.
 */
static void grind_report_time(const struct grinder* const the_grinder,
                              struct grindee* const       grindee) {
  grind_report_stats(the_grinder, grindee);

  if (the_grinder->flags & RCSW_GRIND_REPORT_DATAPOINTS) {
    grind_report_datapoints(the_grinder, grindee);
  }
  if (the_grinder->flags & RCSW_GRIND_REPORT_HISTOGRAM) {
    grind_report_hist(the_grinder, grindee);
  }
}

/**
 * \brief Report execution count info for a \ref grindee.
 */
static void grind_report_count(const struct grinder* const the_grinder,
                               struct grindee* const       grindee) {
  /* show statistics metadata */
  DPRINTF("Total count        : %zu\n", grindee->count);

  /* get minimum and maximum counts */
  uint64_t min = grindee_data_min(grindee);
  uint64_t max = grindee_data_max(grindee);

  DPRINTF("Maximum            : %" PRIu64 "\n", max);
  DPRINTF("Minimum            : %" PRIu64 "\n", min);

  /* get average time */
  double mean = 0;
  for (size_t i = 0; i < grindee->tindex; ++i) {
    mean += (double)grindee->table[i];
  }
  mean /= (double)grindee->tindex;

  DPRINTF("Mean               : %.8f\n", mean);

  /* calculate standard deviation and variance */
  double std_dev  = 0;
  double variance = 0;
  for (size_t i = 0; i < grindee->tindex; i++) {
    variance += pow((double)grindee->table[i] - mean, 2);
  }

  variance /= (double)grindee->tindex;
  std_dev = sqrt(variance);

  DPRINTF("Variance           : %.8f\n", variance);
  DPRINTF("Standard Deviation : %.8f\n", std_dev);

  if (the_grinder->flags & RCSW_GRIND_REPORT_DATAPOINTS) {
    grind_report_datapoints(the_grinder, grindee);
  }
  if (the_grinder->flags & RCSW_GRIND_REPORT_HISTOGRAM) {
    grind_report_hist(the_grinder, grindee);
  }
}

/**
 * \brief Capture the current time for a grindee.
 */
static void grind_ts_capture(const struct grinder* const the_grinder,
                             struct grindee* const       grindee) {
  struct timespec ts = the_grinder->gettime();

  if (RCSW_GRIND_MODE_DURATION == the_grinder->mode) {
    if (grindee->domain.duration.active) {
      grindee->domain.duration.end    = ts;
      grindee->domain.duration.active = false;
    } else {
      grindee->domain.duration.start  = ts;
      grindee->domain.duration.active = true;
    }
  } else if (RCSW_GRIND_MODE_PERIOD == the_grinder->mode) {
    grindee->domain.tick.current = ts;
  }
}

static status_t grind_housekeeping_pre_capture(struct grinder* const the_grinder,
                                               struct grindee* const grindee) {
  /*
   * If stats for grindee are currently full, AND we are not using
   * absolute timing, BUT we have reset enabled, reset.
   */
  /*
   * A full grindee is reset automatically if requested: immediately without
   * RCSW_GRIND_INTERVAL; with it, the interval check below handles resets.
   */
  if (grindee->full && (the_grinder->flags & RCSW_GRIND_RESET_AUTO)) {
    ER_DEBUG("Reset statistics for '%s'", grindee->name);
    grind_reset(the_grinder, grindee);
  } else if (grindee->full) {
    ER_ERR("'%s' full: cannot start grind", grindee->name);
    return ERROR;
  }

  struct timespec curr_time = the_grinder->gettime();

  /*
   * If this is the first call since initialization/last reset, set start time
   * for timeout interval calculation later. Set interval started flag for use
   * in delta computations.
   */
  if ((the_grinder->flags & RCSW_GRIND_INTERVAL) && !the_grinder->in_interval &&
      !grindee->full) {
    ER_TRACE("Interval start=%" PRIu64 ".%09ld/%" PRIu64,
             (uint64_t)curr_time.tv_sec,
             (long)curr_time.tv_nsec,
             utils_ts2monons(&curr_time));
    the_grinder->interval_start = curr_time;
    the_grinder->in_interval    = true;
  }

  /*
   * If GRIND_TIMING_ABS and GRIND_RESET_ENABLE were passed, and the
   * predefined interval has elapsed, reset statistics for all
   * grindees.
   */
  if ((the_grinder->flags & RCSW_GRIND_INTERVAL) &&
      (the_grinder->flags & RCSW_GRIND_RESET_AUTO)) {
    struct timespec diff;
    utils_ts_diff(&the_grinder->interval_start, &curr_time, &diff);
    if (utils_ts_cmp(&diff, &the_grinder->interval) >= 0) {
      grind_reset_all(the_grinder);
      ER_TRACE("Reset statistics for all grindees: %zu.%zu seconds elapsed",
               (size_t)the_grinder->interval.tv_sec,
               (size_t)the_grinder->interval.tv_nsec);
    }
  }
  return OK;
}

static status_t grind_housekeeping_post_capture(struct grinder* const the_grinder,
                                                struct grindee* const grindee) {
  /*
   * Report available statistics before resetting, if the timeout interval
   * has already expired.
   */
  if (grindee->full && (the_grinder->flags & RCSW_GRIND_REPORT_AUTO)) {
    grind_report(the_grinder, grindee);
  }
  if (RCSW_GRIND_MODE_DURATION == the_grinder->mode) {
    grindee->domain.duration.active = false;
  }

  if (the_grinder->flags & RCSW_GRIND_RESET_AUTO) {
    /*
     * If GRIND_TIMING_ABS and GRIND_RESET_ENABLE were passed, and the
     * predefined interval has elapsed, reset statistics for all grindees.
     */
    if (the_grinder->flags & RCSW_GRIND_INTERVAL) {
      struct timespec curr_time = the_grinder->gettime();
      struct timespec diff;
      utils_ts_diff(&the_grinder->interval_start, &curr_time, &diff);

      if (utils_ts_cmp(&diff, &the_grinder->interval) >= 0) {
        grind_reset_all(the_grinder);
        ER_TRACE("Reset statistics all grindees: %zu.%zu seconds elapsed.",
                 (size_t)the_grinder->interval.tv_sec,
                 (size_t)the_grinder->interval.tv_nsec);
      }
      /*
       * Using relative timing, so will reset upon next call to
       * grind_start(). If you reset now, how will the software ever get any
       * useful information from the grindee?
       */
    } else {
    }
  }
  return OK;
}

static struct timespec grind_gettime(void) { return clock_realtime(); }

/*******************************************************************************
 * Public API
 ******************************************************************************/
struct grinder* grind_init(struct grinder*                  grind_in,
                           const struct grind_config* const config) {
  RCSW_FPC_NV(NULL,
              NULL != config,
              NULL != config->names,
              config->n_inst > 0,
              config->res > 0,
              config->tsize > 0);

  RCSW_ER_MODULE_INIT();

  struct grinder* the_grinder =
    rcsw_alloc(grind_in,
               sizeof(struct grinder),
               config->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  RCSW_CHECK_PTR(the_grinder);

  the_grinder->interval = config->interval;
  the_grinder->n_inst   = config->n_inst;
  the_grinder->flags    = config->flags;
  the_grinder->res      = config->res;
  the_grinder->avail    = 0;
  the_grinder->mode     = config->mode;
  the_grinder->gettime =
    (NULL != config->gettime) ? config->gettime : grind_gettime;
  the_grinder->in_interval    = false;
  the_grinder->interval_start = (struct timespec){.tv_sec = 0, .tv_nsec = 0};

  /* the array of grindees; zeroed so a failed init can be destroyed safely */
  the_grinder->grindees =
    rcsw_alloc(NULL, config->n_inst * sizeof(struct grindee), RCSW_ZALLOC);
  RCSW_CHECK_PTR(the_grinder->grindees);

  /* initialize the table for each grindee */
  for (size_t i = 0; i < config->n_inst; ++i) {
    the_grinder->grindees[i].table =
      rcsw_alloc(NULL, config->tsize * sizeof(uint64_t), RCSW_NONE);
    RCSW_CHECK_PTR(the_grinder->grindees[i].table);
    (void)snprintf(the_grinder->grindees[i].name,
                   sizeof(the_grinder->grindees[i].name),
                   "%s",
                   config->names[i]);
    the_grinder->grindees[i].count  = 0;
    the_grinder->grindees[i].tindex = 0;
    the_grinder->grindees[i].full   = 0;
    memset(&the_grinder->grindees[i].domain, 0, sizeof(union grind_mode_impl));

    if (RCSW_GRIND_MODE_PERIOD == the_grinder->mode) {
      the_grinder->grindees[i].domain.tick.first = true;
    }
    the_grinder->grindees[i].tsize = config->tsize;
  } /* for(i..) */

  ER_DEBUG("Configured for %s\n", GRINDER_TYPE(the_grinder));
  ER_DEBUG("Grindees (%zu total):\n", the_grinder->n_inst);

  for (size_t i = 0; i < the_grinder->n_inst; i++) {
    ER_DEBUG("  %s: table=%zu datapoints\n",
             the_grinder->grindees[i].name,
             the_grinder->grindees[i].tsize);
  } /* for() */

  return the_grinder;

error:
  grind_destroy(the_grinder);
  return NULL;
} /* grind_init() */

void grind_destroy(struct grinder* the_grinder) {
  RCSW_FPC_V(NULL != the_grinder);

  /* free each grindee's table (the array may not exist if init failed) */
  if (NULL != the_grinder->grindees) {
    for (size_t i = 0; i < the_grinder->n_inst; i++) {
      rcsw_free(the_grinder->grindees[i].table, RCSW_NONE);
    } /* for() */
  }

  /* free array of grindees */
  rcsw_free(the_grinder->grindees, RCSW_NONE);

  /* free grinder structure */
  rcsw_free(the_grinder, the_grinder->flags & RCSW_NOALLOC_HANDLE);
} /* grind_destroy() */

status_t grind_capture_start(struct grinder* const the_grinder,
                             const char* const     name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find grindee */
  int index = grind_lookup(the_grinder, name);
  if (-1 == index) {
    errno = ENOENT;
  }
  ER_CHECK(-1 != index, "'%s' not found: cannot start grind", name);

  struct grindee* grindee = &the_grinder->grindees[index];

  RCSW_CHECK(OK == grind_housekeeping_pre_capture(the_grinder, grindee));

  /* Set current time for grindee and mark grindee as in-progress (grinding) */
  grind_ts_capture(the_grinder, grindee);

  return OK;

error:
  return ERROR;
} /* grind_start */

status_t grind_capture_end(struct grinder* const the_grinder,
                           const char* const     name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find grindee */
  int index = grind_lookup(the_grinder, name);

  if (-1 == index) {
    errno = ENOENT;
  }
  ER_CHECK(-1 != index, "'%s' not found: cannot finish grind", name);

  struct grindee* grindee = &the_grinder->grindees[index];
  ER_CHECK(grindee->domain.duration.active,
           "'%s' not active--cannot finish grind",
           grindee->name);

  if (!grindee->full) {
    grind_ts_capture(the_grinder, grindee);

    /*
     * If you don't have enough samples for a new datapoint, accumulate the
     * captured sample.
     */
    if (grindee->count < the_grinder->res) {
      struct timespec rel;
      utils_ts_diff(&grindee->domain.duration.start,
                    &grindee->domain.duration.end,
                    &rel);

      ER_TRACE("'%s': %zu/%zu samples for duration datapoint %zu/%zu",
               name,
               grindee->count,
               the_grinder->res,
               grindee->tindex,
               grindee->tsize);

      grindee->domain.duration.accum += utils_ts2monons(&rel);
      grindee->count++;
    }

    /*
     * We have enough samples--add a new data point as the average of the
     * collected samples.
     */
    if (grindee->count == the_grinder->res) {
      uint64_t avg = grindee->domain.duration.accum / the_grinder->res;
      ER_TRACE("'%s': add duration datapoint %zu/%zu: %" PRIu64 "=%" PRIu64
               "/%zu",
               name,
               grindee->tindex,
               grindee->tsize,
               avg,
               grindee->domain.duration.accum,
               the_grinder->res);

      grindee->table[grindee->tindex++] = avg;
      grindee->count                    = 0;
      grindee->domain.duration.accum    = 0;
      grindee->full                     = (grindee->tindex == grindee->tsize);

      if (grindee->full) {
        ER_DEBUG("'%s' duration statistics available", name);
        the_grinder->avail = true;
      }
    }
  }

  RCSW_CHECK(OK == grind_housekeeping_post_capture(the_grinder, grindee));
  return OK;

error:
  return ERROR;
}

status_t grind_capture_tick(struct grinder* const the_grinder,
                            const char* const     name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find grindee */
  int index = grind_lookup(the_grinder, name);
  if (-1 == index) {
    errno = ENOENT;
  }
  ER_CHECK(-1 != index, "'%s' not found: cannot capture", name);

  struct grindee* grindee = &the_grinder->grindees[index];

  RCSW_CHECK(OK == grind_housekeeping_pre_capture(the_grinder, grindee));

  if (grindee->full) {
    RCSW_CHECK(OK == grind_housekeeping_post_capture(the_grinder, grindee));
    return OK;
  }

  struct timespec previous = grindee->domain.tick.current;
  grind_ts_capture(the_grinder, grindee);

  /*
   * We need the first tick as a reference point, to avoid large jumps in
   * computed samples which happen otherwise; return after capturing to avoid
   * erroneous processing.
   */
  if (grindee->domain.tick.first) {
    grindee->domain.tick.first = false;
    return OK;
  }

  /*
   * If you don't have enough samples for a new datapoint, accumulate the
   * captured sample.
   */
  if (grindee->count < the_grinder->res) {
    struct timespec rel;
    utils_ts_diff(&previous, &grindee->domain.tick.current, &rel);
    grindee->domain.tick.accum += utils_ts2monons(&rel);
    grindee->count++;

    ER_TRACE(
      "'%s': %zu/%zu samples for period datapoint %zu/%zu: "
      "accum=%" PRIu64 ",rel=%" PRIu64 ".%09ld",
      name,
      grindee->count,
      the_grinder->res,
      grindee->tindex,
      grindee->tsize,
      grindee->domain.tick.accum,
      (uint64_t)rel.tv_sec,
      (long)rel.tv_nsec);
  }

  /*
   * We have enough samples--add a new data point as the average of the
   * collected samples.
   */
  if (grindee->count == the_grinder->res) {
    ER_ASSERT(the_grinder->res > 0, "Resolution must be > 0");
    uint64_t avg = grindee->domain.tick.accum / the_grinder->res;
    ER_TRACE("'%s': add period datapoint %zu/%zu: %" PRIu64 "=%" PRIu64 "/%zu",
             name,
             grindee->tindex,
             grindee->tsize,
             avg,
             grindee->domain.tick.accum,
             the_grinder->res);

    grindee->table[grindee->tindex++] = avg;
    grindee->count                    = 0;
    grindee->domain.tick.accum        = 0;
    grindee->full                     = (grindee->tindex == grindee->tsize);

    if (grindee->full) {
      ER_DEBUG("'%s' period statistics available", name);
      the_grinder->avail = true;
    }
  }

  RCSW_CHECK(OK == grind_housekeeping_post_capture(the_grinder, grindee));
  return OK;

error:
  return ERROR;
}

status_t grind_capture_count(struct grinder* const the_grinder,
                             const char* const     name) {
  RCSW_FPC_NV(ERROR, the_grinder != NULL, NULL != name);

  int index = grind_lookup(the_grinder, name);
  if (-1 == index) {
    errno = ENOENT;
  }
  ER_CHECK(-1 != index, "'%s' not found: cannot start grind", name);

  struct grindee* grindee = &the_grinder->grindees[index];
  RCSW_CHECK(OK == grind_housekeeping_pre_capture(the_grinder, grindee));

  if (grindee->full) {
    RCSW_CHECK(OK == grind_housekeeping_post_capture(the_grinder, grindee));
    return OK;
  }

  /*
   * If you don't have enough samples for a new datapoint, accumulate the
   * captured sample.
   */
  if (grindee->count < the_grinder->res) {
    ER_TRACE("'%s': %zu/%zu samples for counting datapoint %zu/%zu",
             name,
             grindee->count,
             the_grinder->res,
             grindee->tindex,
             grindee->tsize);
    grindee->count++;
  }

  /*
   * We have enough samples--add a new data point as the average of the
   * collected samples.
   */
  if (grindee->count == the_grinder->res) {
    ER_ASSERT(the_grinder->res > 0, "Resolution must be > 0");
    ER_TRACE("'%s': %zu/%zu counting datapoints gathered",
             name,
             grindee->tindex,
             grindee->tsize);
    /*
     * If we are using intervals, then it makes sense to divide by the
     * specified resolution; if not, it doesn't because the count will always
     * equal the resolution, and the result will always be 1.
     */
    uint64_t avg =
      grindee->count /
      ((the_grinder->flags & RCSW_GRIND_INTERVAL) ? the_grinder->res : 1);
    grindee->table[grindee->tindex++] = avg;
    grindee->count                    = 0;
    grindee->full                     = (grindee->tindex == grindee->tsize);

    if (grindee->full) {
      ER_DEBUG("'%s' counting statistics available", name);
      the_grinder->avail = true;
    }
  }
  RCSW_CHECK(OK == grind_housekeeping_post_capture(the_grinder, grindee));
  return OK;

error:
  return ERROR;
}

void grind_report_all(const struct grinder* const the_grinder) {
  RCSW_FPC_V(NULL != the_grinder);

  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    grind_report(the_grinder, &the_grinder->grindees[i]);
  }
} /* grind_report_all() */

status_t grind_report(const struct grinder* const the_grinder,
                      struct grindee* const       grindee) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != grindee);

  if ((the_grinder->flags & RCSW_GRIND_REPORT_REQ_FULL)) {
    ER_CHECK(grindee->full,
             "'%s' stats not full yet--will not report.",
             grindee->name);
  }
  DPRINTF(
    "**********************************************************************"
    "**********\n");
  DPRINTF("\nReport for grindee '%s':\n\n", grindee->name);

  if (RCSW_GRIND_MODE_COUNT == the_grinder->mode) {
    grind_report_count(the_grinder, grindee);
  } else {
    grind_report_time(the_grinder, grindee);
  }
  DPRINTF(
    "**********************************************************************"
    "**********\n");
  return OK;

error:
  return ERROR;
} /* grind_report() */

/**
 * \brief The denominator for utilization: the configured interval if \ref
 * RCSW_GRIND_INTERVAL was passed, otherwise the sum over all grindees. Both are
 * in the same units as the datapoints.
 */
static double grind_utilization_divisor(const struct grinder* const the_grinder) {
  if (the_grinder->flags & RCSW_GRIND_INTERVAL) {
    return ((double)the_grinder->interval.tv_sec * RCSW_E9) +
           (double)the_grinder->interval.tv_nsec;
  }
  uint64_t sum = grind_sum_all(the_grinder);
  return (double)sum;
}

static double grind_utilization_pct(uint64_t total, double divisor) {
  return (divisor > 0.0) ? ((double)total / divisor) * 100.0 : 0.0;
}

#define GRIND_UTIL_HEADER                                   \
  "     Name               Total             Utilization\n" \
  "+-----------------+--------------------+-------------+\n"
#define GRIND_UTIL_ROW "  %-15.15s  %-20" PRIu64 "  %6.2f%%\n"

int grind_report_utilization_buf(const struct grinder* const the_grinder,
                                 char* const                 buf,
                                 size_t                      len) {
  RCSW_FPC_NV(-1, NULL != the_grinder, NULL != buf || 0 == len);

  double divisor = grind_utilization_divisor(the_grinder);
  size_t total   = 0;

  /*
   * snprintf() semantics: keep counting once the buffer is full so the caller
   * learns how large it needs to be.
   */
#define GRIND_APPEND(...)                         \
  do {                                            \
    int n_ = snprintf(buf + RCSW_MIN(total, len), \
                      len - RCSW_MIN(total, len), \
                      __VA_ARGS__);               \
    if (n_ < 0) {                                 \
      return -1;                                  \
    }                                             \
    total += (size_t)n_;                          \
  } while (0)

  GRIND_APPEND("Interval: %.8f\n", divisor);
  GRIND_APPEND(GRIND_UTIL_HEADER);
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    const struct grindee* grindee    = &the_grinder->grindees[i];
    uint64_t              inst_total = grindee_data_sum(grindee);
    GRIND_APPEND(GRIND_UTIL_ROW,
                 grindee->name,
                 inst_total,
                 grind_utilization_pct(inst_total, divisor));
  }
#undef GRIND_APPEND

  if (total > INT_MAX) {
    errno = EOVERFLOW;
    return -1;
  }
  return (int)total;
} /* grind_report_utilization_buf() */

status_t grind_report_utilization(const struct grinder* const the_grinder) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder);

  double divisor = grind_utilization_divisor(the_grinder);

  DPRINTF(
    "----------------------------------------"
    "Utilization"
    "----------------------------------------\n");
  DPRINTF("Interval: %.8f\n", divisor);
  DPRINTF(GRIND_UTIL_HEADER);
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    const struct grindee* grindee    = &the_grinder->grindees[i];
    uint64_t              inst_total = grindee_data_sum(grindee);
    DPRINTF(GRIND_UTIL_ROW,
            grindee->name,
            inst_total,
            grind_utilization_pct(inst_total, divisor));
  }

  return OK;
} /* grind_report_utilization() */

double grind_get_utilization(struct grinder*   the_grinder,
                             const char* const name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find the grindee */
  int index = grind_lookup(the_grinder, name);
  if (-1 == index) {
    errno = ENOENT;
  }
  ER_CHECK(-1 != index, "'%s' not found: cannot compute utilization", name);
  struct grindee* grindee = the_grinder->grindees + index;

  if (grindee->tindex == 0 && grindee->count == 0) {
    ER_ERR("grindee %s has no data for utilization calculation", name);
    return -1;
  }

  return grind_utilization_pct(grindee_data_sum(grindee),
                               grind_utilization_divisor(the_grinder));

error:
  return -1;
} /* grind_get_utilization() */

void grind_reset_all(struct grinder* const the_grinder) {
  RCSW_FPC_V(NULL != the_grinder);

  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    grind_reset(the_grinder, &the_grinder->grindees[i]);
  }
  the_grinder->in_interval = false;
} /* grind_reset_all() */

void grind_reset(struct grinder* const the_grinder,
                 struct grindee* const grindee) {
  RCSW_FPC_V(NULL != the_grinder, NULL != grindee);
  /* reset grindees for instance */
  memset(grindee->table, 0, grindee->tsize * sizeof(uint64_t));
  grindee->tindex = 0;
  grindee->count  = 0;
  grindee->full   = false;
  if (RCSW_GRIND_MODE_PERIOD == the_grinder->mode) {
    grindee->domain.tick.first = true;
    memset(&grindee->domain.tick.current, 0, sizeof(struct timespec));
  }
} /* grind_reset() */

int grind_lookup(const struct grinder* const the_grinder,
                 const char* const           name) {
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    if (strcmp(the_grinder->grindees[i].name, name) == 0) {
      return (int)i;
    }
  }

  return -1;
}

uint64_t grind_sum_all(const struct grinder* const the_grinder) {
  RCSW_FPC_NV(0, NULL != the_grinder);

  uint64_t sum = 0;
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    sum += grindee_data_sum(the_grinder->grindees + i);
  }
  return sum;
}
