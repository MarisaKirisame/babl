/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "babl-internal.h"
#include "babl-db.h"
#include "assert.h"
#include <string.h>
#include <stdarg.h>

static int 
each_babl_conversion_destroy (Babl *babl,
                              void *data)
{
  babl_free (babl->instance.name);
  babl_free (babl);
  return 0;  /* continue iterating */
}

static Babl *
conversion_new (const char        *name,
                int                id,
                Babl              *source,
                Babl              *destination,
                int                time_cost,
                int                loss,
                BablFuncLinear     linear,
                BablFuncPlanar     planar,
                BablFuncPlanarBit  planar_bit)
{
  Babl *babl = NULL;

  /* destination is of same type as source */ 
  switch (source->class_type)
    {
      case BABL_TYPE:
        if (linear)
          {
            babl = babl_calloc (sizeof (BablConversionType), 1);
            babl->class_type      = BABL_CONVERSION_TYPE;
            babl->conversion.function.linear = linear;
          }
        else if (planar)
          {
            babl = babl_calloc (sizeof (BablConversionTypePlanar), 1);
            babl->class_type = BABL_CONVERSION_TYPE_PLANAR;
            babl->conversion.function.planar = planar;
          }
        else if (planar_bit)
          {
            babl_log ("%s(): planar_bit support not implemented yet",
                      __FUNCTION__);
          }
        break;
      case BABL_SAMPLING:
        babl_log ("%s(): sampling conversions not implemented yet\n",
                  __FUNCTION__);
        break;
      case BABL_COMPONENT:
        babl_log ("%s(): component conversions do not make sense (except perhaps for gamma correction)\n",
                  __FUNCTION__);
        break;
      case BABL_MODEL:
        if (linear)
          {
            babl_log ("%s(): linear support for model conversion not supported",
                      __FUNCTION__);
          }
        else if (planar)
          {
            babl = babl_calloc (sizeof (BablConversionModelPlanar), 1);
            babl->class_type = BABL_CONVERSION_MODEL_PLANAR;
            babl->conversion.function.planar = planar;
          }
        else if (planar_bit)
          {
            babl_log ("%s(): planar_bit support for model conversion not supported",
                      __FUNCTION__);
          }
        break;

      case BABL_PIXEL_FORMAT:
        if (linear)
          {
            babl = babl_calloc (sizeof (BablConversionPixelFormat), 1);
            babl->class_type = BABL_CONVERSION_PIXEL_FORMAT;
            babl->conversion.function.linear = linear;
          }
        else if (planar)
          {
            babl = babl_calloc (sizeof (BablConversionPixelFormatPlanar), 1);
            babl->class_type = BABL_CONVERSION_PIXEL_FORMAT_PLANAR;
            babl->conversion.function.planar = planar;
          }
        else if (planar_bit)
          {
            babl_log ("%s(): planar_bit support for pixelformat conversion not supported",
                      __FUNCTION__);
          }
        break;
      default:
        break;
    }
  if (!babl)
    {
      babl_log ("%s(name='%s', ...): creation failed", __FUNCTION__, name);
      return NULL;
    }

  babl->instance.id            = id;
  babl->instance.name          = babl_strdup (name);
  babl->conversion.source      = (union Babl*)source;
  babl->conversion.destination = (union Babl*)destination;
  babl->conversion.time_cost   = time_cost;
  babl->conversion.loss        = loss;

  babl_add_ptr_to_list ((void ***)&(source->type.from), babl);
  babl_add_ptr_to_list ((void ***)&(destination->type.to), babl);
  
  return babl;
}

Babl *
babl_conversion_new (const char *name,
                     ...)
{
  va_list            varg;
  Babl              *babl;

  int                id          = 0;
  Babl              *source      = NULL;
  Babl              *destination = NULL;
  int                time_cost   = 0;
  int                loss        = 0;
  BablFuncLinear     linear      = NULL;
  BablFuncPlanar     planar      = NULL;
  BablFuncPlanarBit  planar_bit  = NULL;

  int                got_func    = 0;
  const char        *arg         = name;

  va_start (varg, name);
  
  while (1)
    {
      arg = va_arg (varg, char *);
      if (!arg)
        break;
     
      else if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }
      
      else if (!strcmp (arg, "linear"))
        {
          if (got_func++)
            {
              babl_log ("%s(): already got a conversion func, registration of multiple might be possible later\n",
              __FUNCTION__); 
            }
          linear = va_arg (varg, BablFuncLinear);
        }

      else if (!strcmp (arg, "planar"))
        {
          if (got_func++)
            {
              babl_log ("%s(): already got a conversion func, registration of multiple might be possible later\n",
              __FUNCTION__); 
            }
          planar = va_arg (varg, BablFuncPlanar);
        }

      else if (!strcmp (arg, "planar-bit"))
        {
          if (got_func++)
            {
              babl_log ("%s(): already got a conversion func, registration of multiple might be possible later\n",
              __FUNCTION__); 
            }
          planar_bit = va_arg (varg, BablFuncPlanarBit);
        }

      else if (!strcmp (arg, "time-cost"))
        {
          time_cost = va_arg (varg, int);
        }
      else if (!strcmp (arg, "loss"))
        {
          loss = va_arg (varg, int);
        }
      else if (!strcmp (arg, "source"))
        {
          source = va_arg (varg, Babl*);
        }
      else if (!strcmp (arg, "destination"))
        {
          destination = va_arg (varg, Babl*);
        }
      else
        {
          babl_log ("%s(): unhandled parameter '%s' for pixel_format '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);

  assert (source);
  assert (destination);

  babl = conversion_new (name, id, source, destination, time_cost, loss, linear,
                         planar, planar_bit);

  if (db_insert (babl) == babl)
    {
      return babl;
    }
  else
    {
      each_babl_conversion_destroy (babl, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE (babl_conversion)
