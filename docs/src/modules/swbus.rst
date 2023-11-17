.. _modules/swbus:

Software Bus (SWBUS)
====================

A publisher subscriber system, in which any number of
tasks/threads/processes/etc. can send arbitrarily sized packets to each other
using a distributed FIFO system. There is no centralized controller, meaning
each publishing thread does the work of its publish().
