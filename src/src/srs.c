/* $Cambridge: exim/src/src/srs.c,v 1.3 2004/12/17 14:52:44 ph10 Exp $ */

/*************************************************
*     Exim - an Internet mail transport agent    *
*************************************************/

/* SRS - Sender rewriting scheme support
  �2004 Miles Wilton <miles@mirtol.com>
  License: GPL */

#include "exim.h"
#ifdef EXPERIMENTAL_SRS

#include <srs_alt.h>
#include "srs.h"

srs_t    *srs                   = NULL;
uschar   *srs_db_forward        = NULL;
uschar   *srs_db_reverse        = NULL;


/* srs_init just initialises libsrs and creates (if necessary)
   an srs object to use for all srs calls in this instance */
   
int eximsrs_init()
{
  int co;
  uschar *list = srs_config;
  char secret_buf[SRS_MAX_SECRET_LENGTH];
  char *secret;
  char sbuf[4];
  char *sbufp;
  int hashlen, maxage;

  
  if(!srs)
  {
    /* Check config */
    if(!srs_config)
    {
      log_write(0, LOG_MAIN | LOG_PANIC,
          "SRS Configuration Error");
      return DEFER;
    }
    
    /* Get config */
    co = 0;
    if((secret = string_nextinlist(&list, &co, secret_buf,
                                   SRS_MAX_SECRET_LENGTH)) == NULL)
    {
      log_write(0, LOG_MAIN | LOG_PANIC,
          "SRS Configuration Error: No secret specified");
      return DEFER;
    }
    
    if((sbufp = string_nextinlist(&list, &co, sbuf, sizeof(sbuf))) == NULL)
      maxage = 31;
    else
      maxage = atoi(sbuf);
    if(maxage < 0 || maxage > 365)
    {
      log_write(0, LOG_MAIN | LOG_PANIC,
          "SRS Configuration Error: Invalid maximum timestamp age");
      return DEFER;
    }

    if((sbufp = string_nextinlist(&list, &co, sbuf, sizeof(sbuf))) == NULL)
      hashlen = 6;
    else
      hashlen = atoi(sbuf);
    if(hashlen < 1 || hashlen > 20)
    {
      log_write(0, LOG_MAIN | LOG_PANIC,
          "SRS Configuration Error: Invalid hash length");
      return DEFER;
    }
    
    
    if((srs = srs_open(secret, strnlen(secret, SRS_MAX_SECRET_LENGTH),
                      maxage, hashlen, hashlen)) == NULL)
    {
      log_write(0, LOG_MAIN | LOG_PANIC,
          "Failed to allocate SRS memory");
      return DEFER;
    }


    if((sbufp = string_nextinlist(&list, &co, sbuf, sizeof(sbuf))) != NULL)
      srs_set_option(srs, SRS_OPTION_USETIMESTAMP, atoi(sbuf));
    
    if((sbufp = string_nextinlist(&list, &co, sbuf, sizeof(sbuf))) != NULL)
      srs_set_option(srs, SRS_OPTION_USEHASH, atoi(sbuf));

    DEBUG(D_any)
      debug_printf("SRS initialized\n");
  }

  return OK;
}


int eximsrs_done()
{
  if(srs)
    srs_close(srs);
  
  srs = NULL;

  return OK;
}


int eximsrs_forward(uschar **result, uschar *orig_sender, uschar *domain)
{
  char res[512];
  int n;

  if((n = srs_forward(srs, orig_sender, domain, res, sizeof(res))) & SRS_RESULT_FAIL)
  {
    DEBUG(D_any)
      debug_printf("srs_forward failed (%s, %s): %s\n", orig_sender, domain, srs_geterrormsg(n));
    return DEFER;
  }

  *result = string_copy(res);
  return OK;
}


int eximsrs_reverse(uschar **result, uschar *address)
{
  char res[512];
  int n;

  if((n = srs_reverse(srs, address, res, sizeof(res))) & SRS_RESULT_FAIL)
  {
    DEBUG(D_any)
      debug_printf("srs_reverse failed (%s): %s\n", address, srs_geterrormsg(n));
    if(n == SRS_RESULT_NOTSRS || n == SRS_RESULT_BADSRS)
      return DECLINE;
    if(n == SRS_RESULT_BADHASH || n == SRS_RESULT_BADTIMESTAMP || n == SRS_RESULT_TIMESTAMPEXPIRED)
      return FAIL;
    return DEFER;
  }

  *result = string_copy(res);
  return OK;
}


int eximsrs_db_set(BOOL reverse, uschar *srs_db)
{
  if(reverse)
    srs_db_reverse = string_copy(srs_db);
  else
    srs_db_forward = string_copy(srs_db);
    
  if(srs_set_db_functions(srs, eximsrs_db_insert, eximsrs_db_lookup) * SRS_RESULT_FAIL)
    return DEFER;
  
  return OK;
}


srs_result eximsrs_db_insert(srs_t *srs, char *data, uint data_len, char *result, uint result_len)
{
  uschar *res;
  char buf[64];

  srs_db_address = string_copyn(data, data_len);
  if(srs_generate_unique_id(srs, srs_db_address, buf, 64) & SRS_RESULT_FAIL)
    return DEFER;
  
  srs_db_key = string_copyn(buf, 16);
  
  if((res = expand_string(srs_db_forward)) == NULL)
    return SRS_RESULT_DBERROR;
  
  if(result_len < 17)
    return SRS_RESULT_DBERROR;
    
  strncpy(result, srs_db_key, result_len);

  return SRS_RESULT_OK;
}


srs_result eximsrs_db_lookup(srs_t *srs, char *data, uint data_len, char *result, uint result_len)
{
  uschar *res;

  srs_db_key = string_copyn(data, data_len);
  if((res = expand_string(srs_db_reverse)) == NULL)
    return SRS_RESULT_DBERROR;
  
  if(Ustrlen(res) >= result_len)
    return SRS_RESULT_ADDRESSTOOLONG;
    
  strncpy(result, res, result_len);

  return SRS_RESULT_OK;
}


#endif

