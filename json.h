/*
 * json.h --- JSON.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 * Copyright (c) 2011 Joseph A. Adams <joeyadams3.14159@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    22 Dec 2016 20:27:54
 */
/* {{{ License: */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are  permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* }}} */
/* {{{ Commentary: */
/*
 *
 */
/* }}} */

/**
 * @file json.h
 * @author Paul Ward
 * @brief JSON.
 */

#ifndef _json_h_
#define _json_h_

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include "posix/stdbool.h"
#endif

#include <stddef.h>

typedef enum {
  JSON_NULL,
  JSON_BOOL,
  JSON_STRING,
  JSON_NUMBER,
  JSON_ARRAY,
  JSON_OBJECT,
  VALID_JSON_TAG
} json_tag_t;

typedef struct json_node_s json_node_t;

struct json_node_s {
  json_tag_t   tag;
  char        *key;
  json_node_t *parent;
  json_node_t *prev;
  json_node_t *next;

  union {
    bool    _bool;
    char   *_string;
    double  _number;

    struct {
      json_node_t *head;
      json_node_t *tail;
    } children;
  } _u;
};

char        *json_encode(const json_node_t *node);
char        *json_encode_string(const char *str);
char        *json_stringify(const json_node_t *node, const char *space);
void         json_delete(json_node_t *node);

json_node_t *json_find_element(json_node_t *array, size_t index);
json_node_t *json_find_member(json_node_t *object, const char *key);
json_node_t *json_first_child(const json_node_t *node);

#define json_foreach(i, object_or_array)         \
  for ((i) = json_first_child(object_or_array);  \
       (i) != NULL;                              \
       (i)  = (i)->next)

json_node_t *json_mknull(void);
json_node_t *json_mkbool(bool b);
json_node_t *json_mkstring(const char *str);
json_node_t *json_mknumber(double num);
json_node_t *json_mkarray(void);
json_node_t *json_mkobject(void);

void json_append_element(json_node_t *array, json_node_t *element);
void json_prepend_element(json_node_t *array, json_node_t *element);
void json_append_member(json_node_t *object, const char *key, json_node_t *val);
void json_prepend_member(json_node_t *object, const char *key, json_node_t *val);
void json_remove_from_parent(json_node_t *node);

#endif /* !_json_h_ */

/* json.h ends here. */
