/* $Cambridge: exim/src/src/routers/queryprogram.h,v 1.5 2007/01/08 10:50:20 ph10 Exp $ */

/*************************************************
*     Exim - an Internet mail transport agent    *
*************************************************/

/* Copyright (c) University of Cambridge 1995 - 2007 */
/* See the file NOTICE for conditions of use and distribution. */


/* Private structure for the private options. */

typedef struct {
  uschar *command;
  int timeout;
  uid_t cmd_uid;
  gid_t cmd_gid;
  BOOL cmd_uid_set;
  BOOL cmd_gid_set;
  uschar *current_directory;
  uschar *expand_cmd_gid;
  uschar *expand_cmd_uid;
} queryprogram_router_options_block;

/* Data for reading the private options. */

extern optionlist queryprogram_router_options[];
extern int queryprogram_router_options_count;

/* Block containing default values. */

extern queryprogram_router_options_block queryprogram_router_option_defaults;

/* The main and initialization entry points for the router */

extern int queryprogram_router_entry(router_instance *, address_item *,
  struct passwd *, int, address_item **, address_item **,
  address_item **, address_item **);

extern void queryprogram_router_init(router_instance *);

/* End of routers/queryprogram.h */