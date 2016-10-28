/*
 * Copyright © 2013-2014 Collabora, Ltd.
 * Copyright © 2016 DENSO CORPORATION
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

#include "waltham-connection.h"
#include "waltham-private.h"
#include "waltham-util.h"

#include "waltham-object.h"

struct wth_object *
wth_object_new_with_id(struct wth_connection *connection, uint32_t id)
{
	struct wth_object *proxy = NULL;

	proxy = malloc(sizeof *proxy);
	if (proxy == NULL)
		return NULL;

	memset(proxy, 0, sizeof *proxy);

	proxy->id = id;
	proxy->connection = connection;

	wth_connection_insert_object_with_id(connection, proxy);

	return proxy;
}

struct wth_object *
wth_object_new(struct wth_connection *connection)
{
	struct wth_object *proxy = NULL;

	proxy = malloc(sizeof *proxy);
	if (proxy == NULL)
		return NULL;

	memset(proxy, 0, sizeof *proxy);

	proxy->connection = connection;

	wth_connection_insert_new_object(connection, proxy);

	return proxy;
}

WTH_EXPORT void
wth_object_delete(struct wth_object *object)
{
	wth_connection_remove_object(object->connection, object);

	free(object);
}

WTH_EXPORT void
wth_object_set_listener(struct wth_object *obj,
			void (**listener)(void), void *user_data)
{
	if (obj->vfunc)
		wth_debug("vfunc table already set!");

	obj->vfunc = listener;
	obj->user_data = user_data;
}

WTH_EXPORT void *
wth_object_get_user_data(struct wth_object *obj)
{
	return obj->user_data;
}

/* XXX: Copied from walthan-server.h */
void
wth_display_send_error(struct wth_display * wth_display, struct wth_object * object_id, uint32_t code, const char * message);

WTH_EXPORT void
wth_object_post_error(struct wth_object *obj,
		      uint32_t code,
		      const char *fmt, ...)
{
	struct wth_connection *conn = obj->connection;
	struct wth_display *disp;
	char str[256];
	va_list ap;

	ASSERT_SERVER_SIDE(conn);

	va_start(ap, fmt);
	vsnprintf(str, sizeof str, fmt, ap);
	va_end(ap);

	disp = wth_connection_get_display(conn);
	wth_display_send_error(disp, obj, code, str);

	wth_connection_set_protocol_error(conn, obj->id, "unknown", code);
}
