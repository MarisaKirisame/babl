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

#ifndef _BABL_IMAGE_H
#define _BABL_IMAGE_H

#include "babl-classes.h"

void        babl_image_init       (void);
void        babl_image_introspect (void);
void        babl_image_each       (BablEachFunction  each_fun,
                                   void             *user_data);
void        babl_image_destroy    (void);    
BablImage * babl_image_new        (void *first_component,
                                   ...);

/* create a new BablImage based on a packed BablPixelFormat (or BablModel assumed to
 * use only doubles).
 */
BablImage *
babl_image_new_from_linear (void *buffer,
                            Babl *format);
#endif