/**
 * \file grind.h
 * \ingroup profile
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/profile/grind.h"

#include <math.h>

#include "rcsw/al/clock.h"
#define RCSW_ER_MODID ekLOG4CL_GRIND
#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "profile", "grind")
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/time.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define GRINDER_TYPE(the_grinder) \
  ((ekRCSW_GRIND_COUNT == the_grinder->mode) ? "counting" : "timing")

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
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
          grindee->tsize * the_grinder->res + grindee->count);
  DPRINTF("Datapoints         : %zu\n", grindee->tindex);

  /* get minimum time and maximum times */
  size_t min = grindee_data_min(grindee);
  size_t max = grindee_data_max(grindee);

  DPRINTF("Maximum            : %zu ns\n", max);
  DPRINTF("Minimum            : %zu ns\n", min);

  /* get average time */
  double mean = grindee_data_sum(grindee) / (double)grindee->tsize;

  DPRINTF("Mean               : %.8e ns\n", mean);

  /* calculate standard deviation and variance */
  double RCSW_UNUSED std_dev = 0.0;
  double variance = 0.0;
  for (size_t i = 0; i < grindee->tsize; i++) {
    variance += pow(grindee->table[i] - mean, 2.0);
  }

  variance /= grindee->tsize;
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
  size_t min = grindee_data_min(grindee);
  size_t max = grindee_data_max(grindee);

  /* compute size of each of the 50 bins for histogram */
  double binsize = ((double)(max - min)) / (50);

  /* fill histogram array */
  size_t hist_arr[50];
  memset(hist_arr, 0, sizeof(hist_arr));

  size_t bin_count_max = 0;
  for (size_t i = 0; i < grindee->tsize; ++i) {
    for (size_t j = 1; j <= 50; ++j) {
      if (RCSW_IS_BETWEENHO(
              grindee->table[i], min + (j - 1) * binsize, min + j * binsize)) {
        hist_arr[j - 1]++;
        bin_count_max = RCSW_MAX(bin_count_max, hist_arr[j - 1]);
      }
    } /* for(j...) */
  } /* for(i..) */

  size_t xmax = RCSW_MIN(bin_count_max, (size_t)40);

  /* print histogram */
  for (size_t i = 0; i < 50; ++i) {
    if (ekRCSW_GRIND_COUNT == the_grinder->mode) {
      DPRINTF("%8zu | ", i);
    } else {
      DPRINTF("%8zu.%08zu sec | ",
              (size_t)(min + i * binsize) / ONEE9,
              (size_t)(min + i * binsize) % ONEE9);
    }
    double scale = hist_arr[i] / (double)bin_count_max;
    size_t fill = (size_t)(scale * xmax);
    for (size_t j = 0; j < fill; ++j) {
      DPRINTF("*");
    } /* for(j..) */
    DPRINTF("\n");
    fflush(NULL);
  }
  DPRINTF("\n\n");
} /* grind_report_hist() */

/**
 * \brief Report the collected datapoints for a grindee to stdout.
 */
static void grind_report_datapoints(const struct grinder* the_grinder,
                                    const struct grindee* const grindee) {
  DPRINTF("----------------------------------------\n");
  DPRINTF("               DATAPOINTS               \n");
  DPRINTF("----------------------------------------\n");

  DPRINTF("    Index        Value\n");
  DPRINTF("-------------+--------------------------\n");

  for (size_t i = 0; i < grindee->tsize; ++i) {
    if (ekRCSW_GRIND_COUNT == the_grinder->mode) {
      DPRINTF("%8zu            %08zu\n", i, grindee->table[i]);
    } else {
      struct timespec ts = time_monons2ts(grindee->table[i]);
      DPRINTF("%8zu            %8zu.%08zu sec\n",
              i,
              (size_t)ts.tv_sec,
              (size_t)ts.tv_nsec);
    }
  } /* for() */
}

/**
 * \brief Report timing info for a \ref grindee.
 */
static void grind_report_time(const struct grinder* const the_grinder,
                              struct grindee* const grindee) {
  grind_report_stats(the_grinder, grindee);

  if (the_grinder->flags & RCSW_GRIND_REPORT_DATAPOINTS) {
    grind_report_datapoints(the_grinder, grindee);
  }
  if (the_grinder->flags & RCSW_GRIND_REPORT_HISTOGRAM) {
    grind_report_hist(the_grinder, grindee);
  }

  /* reset stats for instance */
  grind_reset(the_grinder, grindee);
}

/**
 * \brief Report execution count info for a \ref grindee.
 */
static void grind_report_count(const struct grinder* const the_grinder,
                               struct grindee* const grindee) {
  /* show statistics metadata */
  DPRINTF("Total count        : %zu\n", grindee->count);

  /* get minimum and maximum counts */
  size_t min = grindee_data_min(grindee);
  size_t max = grindee_data_max(grindee);

  DPRINTF("Maximum            : %zu\n", max);
  DPRINTF("Minimum            : %zu\n", min);

  /* get average time */
  double mean = 0;
  for (size_t i = 0; i < grindee->tsize; ++i) {
    mean += grindee->table[i];
  }
  mean /= grindee->tsize;

  DPRINTF("Mean               : %.8f\n", mean);

  /* calculate standard deviation and variance */
  RCSW_UNUSED double std_dev = 0;
  double variance = 0;
  for (size_t i = 0; i < grindee->tsize; i++) {
    variance += pow(grindee->table[i] - mean, 2);
  }

  variance /= grindee->tsize;
  std_dev = sqrt(variance);

  DPRINTF("Variance           : %.8f\n", variance);
  DPRINTF("Standard Deviation : %.8f\n", std_dev);

  if (the_grinder->flags & RCSW_GRIND_REPORT_DATAPOINTS) {
    grind_report_datapoints(the_grinder, grindee);
  }
  if (the_grinder->flags & RCSW_GRIND_REPORT_HISTOGRAM) {
    grind_report_hist(the_grinder, grindee);
  }
  /* reset stats for instance */
  grind_reset(the_grinder, grindee);
}

/**
 * \brief Capture the current time for a grindee.
 */
static void grind_ts_capture(const struct grinder* const the_grinder,
                             struct grindee* const grindee) {
  struct timespec ts = the_grinder->gettime();

  if (ekRCSW_GRIND_DURATION == the_grinder->mode) {
    if (grindee->domain.duration.active) {
      grindee->domain.duration.end = ts;
      grindee->domain.duration.active = false;
    } else {
      grindee->domain.duration.start = ts;
      grindee->domain.duration.active = true;
    }
  } else if (ekRCSW_GRIND_PERIOD == the_grinder->mode) {
    grindee->domain.tick.current = ts;
  }
}

static status_t grind_housekeeping_pre_capture(struct grinder* const the_grinder,
                                               struct grindee* const grindee) {
  /*
   * If stats for grindee are currently full, AND we are not using
   * absolute timing, BUT we have reset enabled, reset.
   */
  if (grindee->full && (the_grinder->flags & RCSW_GRIND_RESET_AUTO) &&
      (the_grinder->flags & RCSW_GRIND_INTERVAL)) {
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
    ER_TRACE("Interval start=%zu.%zu/%zu",
             (size_t)curr_time.tv_sec,
             (size_t)curr_time.tv_nsec,
             time_ts2monons(&curr_time));
    the_grinder->interval_start = curr_time;
    the_grinder->in_interval = true;
  }

  /*
   * If GRIND_TIMING_ABS and GRIND_RESET_ENABLE were passed, and the
   * predefined interval has elapsed, reset statistics for all
   * grindees.
   */
  if ((the_grinder->flags & RCSW_GRIND_INTERVAL) &&
      (the_grinder->flags & RCSW_GRIND_RESET_AUTO)) {
    struct timespec diff;
    time_ts_diff(&the_grinder->interval_start, &curr_time, &diff);
    if (time_ts_cmp(&diff, &the_grinder->interval) >= 0) {
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
   * Report available statistics before reseting, if the timeout interval
   * has already expired.
   */
  if (grindee->full && (the_grinder->flags & RCSW_GRIND_REPORT_AUTO)) {
    grind_report(the_grinder, grindee);
  }
  if (ekRCSW_GRIND_DURATION == the_grinder->mode) {
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
      time_ts_diff(&the_grinder->interval_start, &curr_time, &diff);

      if (time_ts_cmp(&diff, &the_grinder->interval) >= 0) {
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

static struct timespec grind_gettime(void) {
  return clock_realtime();
}

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct grinder* grind_init(struct grinder* grind_in,
                           const struct grind_params* const params) {
  RCSW_FPC_NV(NULL,
              NULL != params,
              NULL != params->names,
              params->n_inst > 0,
              params->res > 0);

  RCSW_ER_MODULE_INIT();

  struct grinder* the_grinder = rcsw_alloc(grind_in,
                                           sizeof(struct grinder),
                                           params->flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(the_grinder);

  the_grinder->interval = params->interval;
  the_grinder->n_inst = params->n_inst;
  the_grinder->flags = params->flags;
  the_grinder->res = params->res;
  the_grinder->avail = 0;
  the_grinder->mode = params->mode;
  the_grinder->gettime = (NULL != params->gettime) ? params->gettime
                                                   : grind_gettime;

  /* the array of grindees */
  the_grinder->grindees = rcsw_alloc(NULL,
                                     params->n_inst * sizeof(struct grindee),
                                     RCSW_NONE);
  RCSW_CHECK_PTR(the_grinder->grindees);

  /* initialize the table for each grindee */
  for (size_t i = 0; i < params->n_inst; ++i) {
    the_grinder->grindees[i].table = rcsw_alloc(NULL,
                                                params->tsize * sizeof(size_t),
                                                RCSW_NONE);
    RCSW_CHECK_PTR(the_grinder->grindees[i].table);
    snprintf(the_grinder->grindees[i].name,
             sizeof(the_grinder->grindees[i].name),
             "%s",
             params->names[i]);
    the_grinder->grindees[i].count = 0;
    the_grinder->grindees[i].tindex = 0;
    the_grinder->grindees[i].full = 0;
    memset(&the_grinder->grindees[i].domain, 0, sizeof(union grind_mode_impl));

    if (ekRCSW_GRIND_PERIOD == the_grinder->mode) {
      the_grinder->grindees[i].domain.tick.first = true;
    }
    the_grinder->grindees[i].tsize = params->tsize;
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

  /* free each grindee's table */
  for (size_t i = 0; i < the_grinder->n_inst; i++) {
    rcsw_free(the_grinder->grindees[i].table, RCSW_NONE);
  } /* for() */

  /* free array of grindees */
  rcsw_free(the_grinder->grindees, RCSW_NONE);

  /* free grinder structure */
  rcsw_free(the_grinder, the_grinder->flags & RCSW_NOALLOC_HANDLE);

  return;
} /* grind_destroy() */

status_t grind_capture_start(struct grinder* const the_grinder,
                             const char* const name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find grindee */
  int index = grindee_lookup(the_grinder, name);
  ER_CHECK(-1 != index, "'%s' not found: cannnot start grind", name);

  struct grindee* grindee = &the_grinder->grindees[index];

  RCSW_CHECK(OK == grind_housekeeping_pre_capture(the_grinder, grindee));

  /* Set current time for grindee and mark grindee as in-progress (grinding) */
  grind_ts_capture(the_grinder, grindee);

  return OK;

error:
  return ERROR;
} /* grind_start */

status_t grind_capture_end(struct grinder* const the_grinder,
                           const char* const name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find grindee */
  int index = grindee_lookup(the_grinder, name);

  ER_CHECK(-1 != index, "'%s' not found: cannnot finish grind", name);

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
      time_ts_diff(
          &grindee->domain.duration.start, &grindee->domain.duration.end, &rel);

      ER_TRACE("'%s': %zu/%zu samples for duration datapoint %zu/%zu",
               name,
               grindee->count,
               the_grinder->res,
               grindee->tindex,
               grindee->tsize);

      grindee->domain.duration.accum += time_ts2monons(&rel);
      grindee->count++;
    }

    /*
     * We have enough samples--add a new data point as the average of the
     * collected samples.
     */
    if (grindee->count == the_grinder->res) {
      size_t avg = grindee->domain.duration.accum / the_grinder->res;
      ER_TRACE("'%s': add duration datapoint %zu/%zu: %zu=%zu/%zu",
               name,
               grindee->tindex,
               grindee->tsize,
               avg,
               grindee->domain.duration.accum,
               the_grinder->res);

      grindee->table[grindee->tindex++] = avg;
      grindee->count = 0;
      grindee->domain.duration.accum = 0;
      grindee->full = (grindee->tindex == grindee->tsize);

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
                            const char* const name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find grindee */
  int index = grindee_lookup(the_grinder, name);
  ER_CHECK(-1 != index, "'%s' not found: cannnot capture", name);

  struct grindee* grindee = &the_grinder->grindees[index];

  RCSW_CHECK(OK == grind_housekeeping_pre_capture(the_grinder, grindee));

  if (!grindee->full) {
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
      time_ts_diff(&previous, &grindee->domain.tick.current, &rel);
      grindee->domain.tick.accum += time_ts2monons(&rel);
      grindee->count++;

      ER_TRACE("'%s': %zu/%zu samples for period datapoint %zu/%zu: "
               "accum=%zu,rel=%zu.%zu",
               name,
               grindee->count,
               the_grinder->res,
               grindee->tindex,
               grindee->tsize,
               grindee->domain.tick.accum,
               (size_t)rel.tv_sec,
               (size_t)rel.tv_nsec);
    }

    /*
     * We have enough samples--add a new data point as the average of the
     * collected samples.
     */
    if (grindee->count == the_grinder->res) {
      size_t avg = grindee->domain.tick.accum / the_grinder->res;
      ER_TRACE("'%s': add period datapoint %zu/%zu: %zu=%zu/%zu",
               name,
               grindee->tindex,
               grindee->tsize,
               avg,
               grindee->domain.duration.accum,
               the_grinder->res);

      grindee->table[grindee->tindex++] = avg;
      grindee->count = 0;
      grindee->domain.tick.accum = 0;
      grindee->full = (grindee->tindex == grindee->tsize);

      if (grindee->full) {
        ER_DEBUG("'%s' period statistics available", name);
        the_grinder->avail = true;
      }
    }
  }

  RCSW_CHECK(OK == grind_housekeeping_post_capture(the_grinder, grindee));
  return OK;

error:
  return ERROR;
}

status_t grind_capture_count(struct grinder* const the_grinder,
                             const char* const name) {
  RCSW_FPC_NV(ERROR, the_grinder != NULL, NULL != name);

  int index = grindee_lookup(the_grinder, name);
  ER_CHECK(-1 != index, "'%s' not found: cannnot start grind", name);

  struct grindee* grindee = &the_grinder->grindees[index];
  RCSW_CHECK(OK == grind_housekeeping_pre_capture(the_grinder, grindee));

  if (!grindee->full) {
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
      ER_TRACE("'%s': %zu/%zu counting datapoints gathered",
               name,
               grindee->tindex,
               grindee->tsize);
      /*
       * If we are using intervals, then it makes sense to divide by the
       * specified resolution; if not, it doesn't because the count will always
       * equal the resolution, and the result will always be 1.
       */
      size_t avg = grindee->count / (the_grinder->flags & RCSW_GRIND_INTERVAL)
                       ? the_grinder->res
                       : 1;
      grindee->table[grindee->tindex++] = avg;
      grindee->count = 0;
      grindee->full = (grindee->tindex == grindee->tsize);

      if (grindee->full) {
        ER_DEBUG("'%s' counting statistics available", name);
        the_grinder->avail = true;
      }
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
                      struct grindee* const grindee) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != grindee);

  if ((the_grinder->flags & RCSW_GRIND_REPORT_REQ_FULL)) {
    ER_CHECK(grindee->full,
             "'%s' stats not full yet--will not report.",
             grindee->name);
  }
  DPRINTF("**********************************************************************"
          "**********\n");
  DPRINTF("\nReport for grindee '%s':\n\n", grindee->name);

  if (ekRCSW_GRIND_COUNT == the_grinder->mode) {
    grind_report_count(the_grinder, grindee);
  } else {
    grind_report_time(the_grinder, grindee);
  }
  DPRINTF("**********************************************************************"
          "**********\n");
  return OK;

error:
  return ERROR;
} /* grind_report() */

int grind_report_utilization2(const struct grinder* const the_grinder,
                              char* const buf) {
  RCSW_FPC_NV(-1, NULL != the_grinder, NULL != buf);

  size_t inst_total = 0;
  struct grindee* grindee;
  double divisor;
  char* buf_ptr = buf;

  if (the_grinder->flags & RCSW_GRIND_INTERVAL) {
    divisor = the_grinder->interval.tv_sec +
              (the_grinder->interval.tv_nsec / 1000000000.0);
  } else { /* use the cumulative total time of ALL grindees */
    divisor = grind_sum_all(the_grinder);
  }

  buf_ptr += sprintf(buf_ptr, "Interval: %.8f", divisor);
  buf_ptr += sprintf(buf_ptr,
                     "     Name           Time         Utilization"
                     "+---------------+--------------+---------------+");
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    grindee = &the_grinder->grindees[i];
    inst_total = grindee_data_sum(grindee);
    buf_ptr += sprintf(buf_ptr,
                       "  %-15.15s  %-18zu   %3.2f",
                       grindee->name,
                       inst_total,
                       (inst_total / divisor) * 100.0);
  }
  return buf_ptr - buf;
} /* grind_report_utilization2() */

status_t grind_report_utilization(const struct grinder* const the_grinder) {
  RCSW_FPC_NV(-1, NULL != the_grinder);

  DPRINTF("----------------------------------------"
          "Utilization"
          "----------------------------------------");
  RCSW_UNUSED size_t inst_total = 0;
  struct grindee* grindee;
  double divisor;

  if (the_grinder->flags & RCSW_GRIND_INTERVAL) {
    divisor = the_grinder->interval.tv_sec * ONEE9 +
              the_grinder->interval.tv_nsec / 1000000000;
  } else { /* use the cumulative total time of ALL grindees */
    divisor = grind_sum_all(the_grinder);
  }

  DPRINTF("Interval: %.8f", divisor);
  DPRINTF("     Name           Time         Utilization"
          "+---------------+--------------+---------------+");
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    grindee = &the_grinder->grindees[i];
    inst_total = grindee_data_sum(grindee);
    DPRINTF("%-15.15s   %-18zu    %3.2f%%",
            grindee->name,
            inst_total,
            (inst_total / divisor) * 100.0);
  }

  return OK;
} /* grind_report_utilization() */

double grind_get_utilization(struct grinder* the_grinder,
                             const char* const name) {
  RCSW_FPC_NV(ERROR, NULL != the_grinder, NULL != name);

  /* find the grindee */
  int index = grindee_lookup(the_grinder, name);
  ER_CHECK(-1 != index, "'%s' not found: cannnot compute utilization", name);
  struct grindee* grindee = the_grinder->grindees + index;

  if (grindee->tindex == 0 && grindee->count == 0) {
    ER_ERR("grindee %s has no data for utilization calculation", name);
    return -1;
  }

  size_t inst_total = 0;
  double divisor;

  inst_total = grindee_data_sum(grindee);

  if (the_grinder->flags & RCSW_GRIND_INTERVAL) {
    divisor = the_grinder->interval.tv_sec * ONEE9 +
              the_grinder->interval.tv_nsec / ONEE9;
  } else { /* use the cumulative total time of ALL grindees */
    divisor = grind_sum_all(the_grinder);
  }

  return (inst_total / divisor) * 100.0;

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

void grind_reset(const struct grinder* const the_grinder,
                 struct grindee* const grindee) {
  RCSW_FPC_V(NULL != the_grinder, NULL != grindee);
  /* reset grindees for instance */
  memset(grindee->table, 0, grindee->tsize * sizeof(size_t));
  grindee->tindex = 0;
  grindee->count = 0;
  grindee->full = false;
  if (ekRCSW_GRIND_PERIOD == the_grinder->mode) {
    grindee->domain.tick.first = true;
    memset(&grindee->domain.tick.current, 0, sizeof(struct timespec));
  }
} /* grind_reset() */

int grindee_lookup(const struct grinder* const the_grinder,
                   const char* const name) {
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    if (strcmp(the_grinder->grindees[i].name, name) == 0) {
      return i;
      break;
    }
  }

  return -1;
} /* grind_name2index() */

size_t grindee_data_max(const struct grindee* const grindee) {
  RCSW_FPC_NV(0, NULL != grindee, grindee->tindex > 0);

  size_t max = 0;

  for (size_t i = 0; i < grindee->tindex; ++i) {
    max = RCSW_MAX(max, grindee->table[i]);
  }
  return max;
}

size_t grindee_data_min(const struct grindee* const grindee) {
  RCSW_FPC_NV(0, NULL != grindee, grindee->tindex > 0);

  size_t min = SIZE_MAX;

  for (size_t i = 0; i < grindee->tindex; ++i) {
    min = RCSW_MIN(min, grindee->table[i]);
  }
  return min;
}

size_t grindee_data_sum(const struct grindee* const grindee) {
  RCSW_FPC_NV(0, NULL != grindee);

  size_t sum = 0;
  for (size_t i = 0; i < grindee->tindex; ++i) {
    sum += grindee->table[i];
  }
  return sum;
}

size_t grind_sum_all(const struct grinder* const the_grinder) {
  RCSW_FPC_NV(0, NULL != the_grinder);

  size_t sum = 0;
  for (size_t i = 0; i < the_grinder->n_inst; ++i) {
    sum += grindee_data_sum(the_grinder->grindees + i);
  }
  return sum;
}
