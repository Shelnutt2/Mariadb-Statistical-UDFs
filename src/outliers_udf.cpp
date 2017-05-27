

/*

  Copyright Seth Shelnutt 2017-05-26
  Licensed Under the GPL v3 or later

  returns the mean and stddev with outliers removed

  input parameters:
  data (real)
 
  output:
  stddev value of the distribution (real)

  registering the function:
  CREATE AGGREGATE FUNCTION stddev_no_outliers RETURNS REAL SONAME 'liboutliers_udf.so';
  CREATE AGGREGATE FUNCTION mean_no_outliers RETURNS REAL SONAME 'liboutliers_udf.so';

  getting rid of the function:
  DROP FUNCTION stddev_no_outliers;
  DROP FUNCTION mean_no_outliers;
*/


#ifdef STANDARD
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#endif
#include <vector>
#include <iostream>
#include <algorithm>
#include <mysql.h>
#include <m_ctype.h>
#include <m_string.h>
#include <mad.hpp>
#include <outliers.hpp>

extern "C" {
my_bool mean_no_outliers_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void mean_no_outliers_deinit( UDF_INIT* initid );
void mean_no_outliers_clear( UDF_INIT* initid, char* is_null, char *error );
void mean_no_outliers_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void mean_no_outliers_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double mean_no_outliers( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
/*long long mean_no_outliers( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );*/
}

struct mean_no_outliers_data
{
    std::vector<double> *double_values;
    std::vector<long long> *int_values;
};

my_bool mean_no_outliers_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count != 1)
  {
    strcpy(message,"wrong number of arguments: mean_no_outliers() requires one argument");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT && args->arg_type[0]!=INT_RESULT && args->arg_type[0]!=DECIMAL_RESULT)
  {
    if (args->arg_type[0] == STRING_RESULT)
      strcpy(message,"mean_no_outliers() requires a real, decimal, double or integer as parameter 1, received STRING");
    else
      strcpy(message,"mean_no_outliers() requires a real, decimal, double or integer as parameter 1, received Decimal");
    return 1;
  }

  initid->decimals = NOT_FIXED_DEC;
  initid->maybe_null = 1;

  mean_no_outliers_data *buffer = new mean_no_outliers_data;
  buffer->double_values = NULL;
  buffer->int_values = NULL;
  initid->ptr = (char*)buffer;

  return 0;
}


void mean_no_outliers_deinit( UDF_INIT* initid )
{
  mean_no_outliers_data *buffer = (mean_no_outliers_data*)initid->ptr;

  if (buffer->double_values != NULL)
  {
    free(buffer->double_values);
    buffer->double_values=NULL;
  }
  if (buffer->int_values != NULL)
  {
    free(buffer->int_values);
    buffer->int_values=NULL;
  }
  delete initid->ptr;
}


void mean_no_outliers_clear( UDF_INIT* initid, char* is_null, char* is_error )
{
  mean_no_outliers_data *buffer = (mean_no_outliers_data*)initid->ptr;
  *is_null = 0;
  *is_error = 0;

  if (buffer->double_values != NULL)
  {
    free(buffer->double_values);
    buffer->double_values=NULL;
  }
  if (buffer->int_values != NULL)
  {
    free(buffer->int_values);
    buffer->int_values=NULL;
  }

  buffer->double_values = new std::vector<double>;
  buffer->int_values = new std::vector<long long>;

}


void mean_no_outliers_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  mean_no_outliers_clear(initid, is_null, is_error);
  mean_no_outliers_add( initid, args, is_null, is_error );
}


void mean_no_outliers_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  if (args->args[0]!=NULL)
  {
    mean_no_outliers_data *buffer = (mean_no_outliers_data*)initid->ptr;
    if (args->arg_type[0]==REAL_RESULT || args->arg_type[0]==DECIMAL_RESULT)
    {
      buffer->double_values->push_back(*((double*)args->args[0]));
    }
    else if (args->arg_type[0]==INT_RESULT)
    {
      buffer->int_values->push_back(*((long long*)args->args[0]));
    }
  }
}

double mean_no_outliers( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  mean_no_outliers_data* buffer = (mean_no_outliers_data*)initid->ptr;

  if (buffer->double_values != NULL && buffer->double_values->size() > 0) {
    std::vector<double> outliers_removed = remove_outlier(*buffer->double_values);
    return avg(outliers_removed);
  } else if (buffer->int_values != NULL && buffer->int_values->size() > 0) {
    std::vector<long long> outliers_removed = remove_outlier(*buffer->int_values);
    return avg(outliers_removed);
  }
  std::cerr << "mean_no_outliers() internal error, all vectors were null in computation" << std::endl;
  *is_error = 1;
  return 0;
}

/*long long mean_no_outliers( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  mean_no_outliers_data* buffer = (mean_no_outliers_data*)initid->ptr;
  return mean_no_outliers(buffer->int_values->begin(), buffer->int_values->end());
}*/


/* #endif */