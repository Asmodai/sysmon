/*
 * json.c --- JSON parser implementation.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    22 Dec 2016 20:45:44
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
 * @file json.c
 * @author Paul Ward
 * @brief JSON parser implementation.
 */

#include "json.h"

#include <sys/param.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
# include "posix/stdint.h"
#endif

#ifdef BSD
extern         fprintf();
extern char   *malloc();
extern char   *realloc();
extern char   *calloc();
extern         free();
extern double  strtod();
#endif

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#define out_of_memory() do {             \
    fprintf(stderr, "Out of memory.\n"); \
    exit(EXIT_FAILURE);                  \
  } while (0)

/*
 * Type for Unicode codepoints.
 * We need our own because wchar_t might be 16 bits.
 */
typedef uint32_t uchar_t;

/*
 * Validate a single UTF-8 character starting at @s.
 * The string must be null-terminated.
 *
 * If it's valid, return its length (1 thru 4).
 * If it's invalid or clipped, return 0.
 *
 * This function implements the syntax given in RFC3629, which is
 * the same as that given in The Unicode Standard, Version 6.0.
 *
 * It has the following properties:
 *
 *  * All codepoints U+0000..U+10FFFF may be encoded,
 *    except for U+D800..U+DFFF, which are reserved
 *    for UTF-16 surrogate pair encoding.
 *  * UTF-8 byte sequences longer than 4 bytes are not permitted,
 *    as they exceed the range of Unicode.
 *  * The sixty-six Unicode "non-characters" are permitted
 *    (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF).
 */
static
int
utf8_validate_cz(const char *s)
{
	unsigned char c = *s++;
	
	if (c <= 0x7F) {                      /* 00..7F */
		return 1;
	} else if (c <= 0xC1) {               /* 80..C1 */
		/* Disallow overlong 2-byte sequence. */
		return 0;
	} else if (c <= 0xDF) {               /* C2..DF */
		/* Make sure subsequent byte is in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 2;
	} else if (c <= 0xEF) {               /* E0..EF */
		/* Disallow overlong 3-byte sequence. */
		if (c == 0xE0 && (unsigned char)*s < 0xA0)
			return 0;
		
		/* Disallow U+D800..U+DFFF. */
		if (c == 0xED && (unsigned char)*s > 0x9F)
			return 0;
		
		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 3;
	} else if (c <= 0xF4) {               /* F0..F4 */
		/* Disallow overlong 4-byte sequence. */
		if (c == 0xF0 && (unsigned char)*s < 0x90)
			return 0;
		
		/* Disallow codepoints beyond U+10FFFF. */
		if (c == 0xF4 && (unsigned char)*s > 0x8F)
			return 0;
		
		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 4;
	} else {                              /* F5..FF */
		return 0;
	}
}

/* Validate a null-terminated UTF-8 string. */
static
bool
utf8_validate(const char *s)
{
	int len;
	
	for (; *s != 0; s += len) {
		len = utf8_validate_cz(s);
		if (len == 0)
			return false;
	}
	
	return true;
}

/*
 * Read a single UTF-8 character starting at @s,
 * returning the length, in bytes, of the character read.
 *
 * This function assumes input is valid UTF-8,
 * and that there are enough characters in front of @s.
 */
static
int
utf8_read_char(const char *s, uchar_t *out)
{
	const unsigned char *c = (const unsigned char*) s;
	
	assert(utf8_validate_cz(s));

	if (c[0] <= 0x7F) {                   /* 00..7F */
		*out = c[0];
		return 1;
	} else if (c[0] <= 0xDF) {            /* C2..DF (unless input is invalid) */
		*out = ((uchar_t)c[0] & 0x1F) << 6 |
		       ((uchar_t)c[1] & 0x3F);
		return 2;
	} else if (c[0] <= 0xEF) {
		/* E0..EF */
		*out = ((uchar_t)c[0] &  0xF) << 12 |
		       ((uchar_t)c[1] & 0x3F) << 6  |
		       ((uchar_t)c[2] & 0x3F);
		return 3;
	} else {                              /* F0..F4 (unless input is invalid) */
		*out = ((uchar_t)c[0] &  0x7) << 18 |
		       ((uchar_t)c[1] & 0x3F) << 12 |
		       ((uchar_t)c[2] & 0x3F) << 6  |
		       ((uchar_t)c[3] & 0x3F);
		return 4;
	}
}

/*
 * Construct a UTF-16 surrogate pair given a Unicode codepoint.
 *
 * @unicode must be U+10000..U+10FFFF.
 */
static
void
to_surrogate_pair(uchar_t unicode, uint16_t *uc, uint16_t *lc)
{
	uchar_t n;
	
	assert(unicode >= 0x10000 && unicode <= 0x10FFFF);
	
	n = unicode - 0x10000;
	*uc = ((n >> 10) & 0x3FF) | 0xD800;
	*lc = (n & 0x3FF) | 0xDC00;
}

#define is_space(c) ((c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == ' ')
#define is_digit(c) ((c) >= '0' && (c) <= '9')

/* }}} */
/* ============================================================================ */

static
char *
json_strdup(const char *str)
{
  char *ret = NULL;

  if ((ret = (char *)malloc(strlen(str) + 1)) == NULL) {
    out_of_memory();
  }

  strcpy(ret, str);

  return ret;
}

typedef struct {
  char *cur;
  char *end;
  char *start;
} SB;

static
void
sb_init(SB *sb)
{
  if ((sb->start = (char *)malloc(17)) == NULL) {
    out_of_memory();
  }

  sb->cur = sb->start;
  sb->end = sb->start + 16;
}

#define sb_need(sb, need) do {          \
    if ((sb)->end - (sb)->cur < (need)) \
      sb_grow(sb, need);                \
  } while (0)

static
void
sb_grow(SB *sb, int need)
{
  size_t length = sb->cur - sb->start;
  size_t alloc  = sb->end - sb->start;

  do {
    alloc *= 2;
  } while (alloc < length + need);

  if ((sb->start = (char *)realloc(sb->start, alloc + 1)) == NULL) {
    out_of_memory();
  }

  sb->cur = sb->start + length;
  sb->end = sb->start + alloc;
}

static
void
sb_put(SB *sb, const char *bytes, int count)
{
  sb_need(sb, count);
  memcpy(sb->cur, bytes, count);
  sb->cur += count;
}

#define sb_putc(sb, c) do {                     \
    if ((sb)->cur >= (sb)->end) {               \
      sb_grow(sb, 1);                           \
    }                                           \
    *(sb)->cur++ = (c);                         \
  } while (0)

static
void
sb_puts(SB *sb, const char *str)
{
  sb_put(sb, str, strlen(str));
}

static
char *
sb_finish(SB *sb)
{
  *sb->cur = 0;

  assert(sb->start         <= sb->cur &&
         strlen(sb->start) == (size_t)(sb->cur - sb->start));

  return sb->start;
}

/*
static
void
sb_free(SB *sb)
{
  free(sb->start);
}
*/

static void emit_value(SB *out, const json_node_t *node);
static void emit_value_indented(SB                *out,
                                const json_node_t *node,
                                const char        *space,
                                int                indent_level);
static void emit_string(SB *out, const char *str);
static void emit_number(SB *out, double num);
static void emit_array(SB *out, const json_node_t *array);
static void emit_array_indented(SB                *out,
                                const json_node_t *array,
                                const char        *space,
                                int                indent_level);
static void emit_object(SB *out, const json_node_t *object);
static void emit_object_indented(SB                *out,
                                 const json_node_t *object,
                                 const char        *space,
                                 int                indent_level);

static int write_hex16(char *out, uint16_t val);

static json_node_t *mknode(json_tag_t tag);
static void         append_node(json_node_t *parent, json_node_t *child);
static void         prepend_node(json_node_t *parent, json_node_t *child);
static void         append_member(json_node_t *object,
                                  char        *key,
                                  json_node_t *value);

static bool tag_is_valid(unsigned int tag);
static bool number_is_valid(const char *num);

char *
json_encode(const json_node_t *node)
{
  return json_stringify(node, NULL);
}

char *
json_encode_string(const char *str)
{
  SB sb;

  sb_init(&sb);
  emit_string(&sb, str);

  return sb_finish(&sb);
}

char *
json_stringify(const json_node_t *node, const char *space)
{
  SB sb;

  sb_init(&sb);

  if (space != NULL) {
    emit_value_indented(&sb, node, space, 0);
  } else {
    emit_value(&sb, node);
  }

  return sb_finish(&sb);
}

void
json_delete(json_node_t *node)
{
  if (node != NULL) {
    json_remove_from_parent(node);

    switch (node->tag) {
      case JSON_STRING:
        free(node->_u._string);
        break;

      case JSON_ARRAY:
      case JSON_OBJECT:
        {
          json_node_t *child = NULL;
          json_node_t *next  = NULL;

          for (child = node->_u.children.head; child != NULL; child = next) {
            next = child->next;
            json_delete(child);
          }

          break;
        }

      default:;
    }

    free(node);
  }
}

json_node_t *
json_find_element(json_node_t *array, size_t index)
{
  json_node_t *element = NULL;
  int i = 0;

  if (array == NULL || array->tag != JSON_ARRAY) {
    return NULL;
  }

  json_foreach(element, array) {
    if (i == index) {
      return element;
    }

    i++;
  }

  return NULL;
}

json_node_t *
json_find_member(json_node_t *object, const char *name)
{
  json_node_t *member;

  if (object == NULL || object->tag != JSON_OBJECT) {
    return NULL;
  }


  json_foreach(member, object) {
    if (strcmp(member->key, name) == 0) {
      return member;
    }
  }

  return NULL;
}

json_node_t *
json_first_child(const json_node_t *node)
{
  if (node != NULL && (node->tag == JSON_ARRAY || node->tag == JSON_OBJECT)) {
    return node->_u.children.head;
  }

  return NULL;
}

static
json_node_t *
mknode(json_tag_t tag)
{
  json_node_t *ret = NULL;

  if ((ret = (json_node_t *)calloc(1, sizeof(json_node_t))) == NULL) {
    out_of_memory();
  }

  ret->tag = tag;

  return ret;
}

json_node_t *
json_mknull(void)
{
  return mknode(JSON_NULL);
}

json_node_t *
json_mkbool(bool b)
{
  json_node_t *ret = NULL;

  ret           = mknode(JSON_BOOL);
  ret->_u._bool = b;

  return ret;
}

static
json_node_t *
mkstring(char *s)
{
  json_node_t *ret = NULL;

  ret             = mknode(JSON_STRING);
  ret->_u._string = s;

  return ret;
}

json_node_t *
json_mkstring(const char *s)
{
  return mkstring(json_strdup(s));
}

json_node_t *
json_mknumber(double n)
{
  json_node_t *ret = NULL;

  ret             = mknode(JSON_NUMBER);
  ret->_u._number = n;

  return ret;
}

json_node_t *
json_mkarray(void)
{
  return mknode(JSON_ARRAY);
}

json_node_t *
json_mkobject(void)
{
  return mknode(JSON_OBJECT);
}

static
void
append_node(json_node_t *parent, json_node_t *child)
{
  child->parent = parent;
  child->prev   = parent->_u.children.tail;
  child->next   = NULL;

  if (parent->_u.children.tail != NULL) {
    parent->_u.children.tail->next = child;
  } else {
    parent->_u.children.head = child;
  }

  parent->_u.children.tail = child;
}

static
void
prepend_node(json_node_t *parent, json_node_t *child)
{
  child->parent = parent;
  child->prev   = NULL;
  child->next   = parent->_u.children.head;

  if (parent->_u.children.head != NULL) {
    parent->_u.children.head->prev = child;
  } else {
    parent->_u.children.tail = child;
  }

  parent->_u.children.head = child;
}

static
void
append_member(json_node_t *object, char *key, json_node_t *value)
{
  value->key = key;
  append_node(object, value);
}

void
json_append_element(json_node_t *array, json_node_t *element)
{
  assert(array->tag == JSON_ARRAY);
  assert(element->parent == NULL);

  append_node(array, element);
}

void
json_prepend_element(json_node_t *array, json_node_t *element)
{
  assert(array->tag == JSON_ARRAY);
  assert(element->parent == NULL);

  prepend_node(array, element);
}

void
json_append_member(json_node_t *object, const char *key, json_node_t *value)
{
  assert(object->tag == JSON_OBJECT);
  assert(value->parent == NULL);

  append_member(object, json_strdup(key), value);
}

void
json_prepend_member(json_node_t *object, const char *key, json_node_t *value)
{
  assert(object->tag == JSON_OBJECT);
  assert(value->parent == NULL);

  value->key = json_strdup(key);
  prepend_node(object, value);
}

void
json_remove_from_parent(json_node_t *node)
{
  json_node_t *parent = node->parent;

  if (parent != NULL) {
    if (node->prev != NULL) {
      node->prev->next = node->next;
    } else {
      parent->_u.children.head = node->next;
    }

    if (node->next != NULL) {
      node->next->prev = node->prev;
    } else {
      parent->_u.children.tail = node->prev;
    }

    free(node->key);

    node->parent = NULL;
    node->prev   = node->next = NULL;
    node->key    = NULL;
  }
}

static
void
emit_value(SB *out, const json_node_t *node)
{
  assert(tag_is_valid(node->tag));

  switch (node->tag) {
    case JSON_NULL:   sb_puts(out, "null");                            break;
    case JSON_BOOL:   sb_puts(out, node->_u._bool ? "true" : "false"); break;
    case JSON_STRING: emit_string(out, node->_u._string);              break;
    case JSON_NUMBER: emit_number(out, node->_u._number);              break;
    case JSON_ARRAY:  emit_array(out, node);                           break;
    case JSON_OBJECT: emit_object(out, node);                          break;
    default:          assert(false);
  }
}

void
emit_value_indented(SB                *out,
                    const json_node_t *node,
                    const char        *space,
                    int                ilevel)
{
  assert(tag_is_valid(node->tag));

  switch (node->tag) {
    case JSON_NULL:   sb_puts(out, "null");                              break;
    case JSON_BOOL:   sb_puts(out, node->_u._bool ? "true" : "false");   break;
    case JSON_STRING: emit_string(out, node->_u._string);                break;
    case JSON_NUMBER: emit_number(out, node->_u._number);                break;
    case JSON_ARRAY:  emit_array_indented(out, node, space, ilevel);     break;
    case JSON_OBJECT: emit_object_indented(out, node, space, ilevel);    break;
    default:          assert(false);
  }
}

static
void
emit_array(SB *out, const json_node_t *array)
{
  const json_node_t *element = NULL;

  sb_putc(out, '[');

  json_foreach(element, array) {
    emit_value(out, element);

    if (element->next != NULL) {
      sb_putc(out, ',');
    }
  }

  sb_putc(out, ']');
}

static
void
emit_array_indented(SB                *out,
                    const json_node_t *array,
                    const char        *space,
                    int                ilevel)
{
  const json_node_t *element = array->_u.children.head;
  int i;

  if (element == NULL) {
    sb_puts(out, "[]");
    return;
  }

  sb_puts(out, "[\n");
  while (element != NULL) {
    for (i = 0; i < ilevel + 1; i++) {
      sb_puts(out, space);
    }
    emit_value_indented(out, element, space, ilevel + 1);

    element = element->next;
    sb_puts(out, element != NULL ? ",\n" : "\n");
  }

  for (i = 0; i < ilevel; i++) {
    sb_puts(out, space);
  }

  sb_putc(out, ']');
}

static
void
emit_object(SB *out, const json_node_t *object)
{
  const json_node_t *member;

  sb_putc(out, '{');

  json_foreach(member, object) {
    emit_string(out, member->key);
    sb_putc(out, ':');
    emit_value(out, member);

    if (member->next != NULL) {
      sb_putc(out, ',');
    }
  }

  sb_putc(out, '}');
}

static
void
emit_object_indented(SB                *out,
                     const json_node_t *object,
                     const char        *space,
                     int                ilevel)
{
  const json_node_t *member = object->_u.children.head;
  int i = 0;

  if (member == NULL) {
    sb_puts(out, "{}");
    return;
  }

  sb_puts(out, "{\n");

  while (member != NULL) {
    for (i = 0; i < ilevel + 1; i++) {
      sb_puts(out, space);
    }

    emit_string(out, member->key);
    sb_puts(out, ": ");
    emit_value_indented(out, member, space, ilevel + 1);

    member = member->next;
    sb_puts(out, member != NULL ? ",\n" : "\n");
  }

  for (i = 0; i < ilevel; i++) {
    sb_puts(out, space);
  }

  sb_putc(out, '}');
}

void
emit_string(SB *out, const char *str)
{
  bool        escape_unicode = false;
  const char *s              = str;
  char *b = NULL;

  assert(utf8_validate(str));

  sb_need(out, 14);
  b = out->cur;

  *b++ = '"';

  while (*s != 0) {
    unsigned char c = *s++;

    switch (c) {
      case '"':  *b++ = '\\'; *b++ = '"';  break;
      case '\\': *b++ = '\\'; *b++ = '\\'; break;
      case '\b': *b++ = '\\'; *b++ = 'b';  break;
      case '\f': *b++ = '\\'; *b++ = 'f';  break;
      case '\n': *b++ = '\\'; *b++ = 'n';  break;
      case '\r': *b++ = '\\'; *b++ = 'r';  break;
      case '\t': *b++ = '\\'; *b++ = 't';  break;

      default: {
        int len = 0;

        s--;
        len = utf8_validate_cz(s);

        if (len == 0) {
          assert(false);
          if (escape_unicode) {
            strcpy(b, "\\uFFD");
            b += 6;
          } else {
            *b++ = (char)0xEF;
            *b++ = (char)0xBF;
            *b++ = (char)0xBD;
          }
          s++;
        } else if (c < 0x1F || (c >= 0x80 && escape_unicode)) {
          uint32_t unicode;

          s += utf8_read_char(s, &unicode);

          if (unicode <= 0xFFFF) {
            *b++ = '\\';
            *b++ = 'u';
            b += write_hex16(b, unicode);
          } else {
            uint16_t uc, lc;
            assert(unicode <= 0x10FFFF);

            to_surrogate_pair(unicode, &uc, &lc);
            *b++ = '\\';
            *b++ = 'u';
            b += write_hex16(b, uc);
            *b++ = '\\';
            *b++ = 'u';
            b += write_hex16(b, lc);
          }
        } else {
          while (len--) {
            *b++ = *s++;
          }
        }

        break;
      }
    }

    out->cur = b;
    sb_need(out, 14);
    b = out->cur;
  }

  *b++ = '"';

  out->cur = b;
}

static
void
emit_number(SB *out, double num)
{
  char buf[64];
  sprintf(buf, "%.16g", num);

  if (number_is_valid(buf)) {
    sb_puts(out, buf);
  } else {
    sb_puts(out, "null");
  }
}

static
bool
tag_is_valid(unsigned int tag)
{
  return (tag < VALID_JSON_TAG);
}

bool
parse_number(const char **sp, double *out)
{
  const char *s = *sp;

	if (*s == '-') {
		s++;
  }

	if (*s == '0') {
		s++;
	} else {
		if (!is_digit(*s)) {
			return false;
    }

		do {
			s++;
		} while (is_digit(*s));
	}

	if (*s == '.') {
		s++;

		if (!is_digit(*s)) {
			return false;
    }

		do {
			s++;
		} while (is_digit(*s));
	}

	if (*s == 'E' || *s == 'e') {
		s++;

		if (*s == '+' || *s == '-') {
			s++;
    }

		if (!is_digit(*s)) {
			return false;
    }

		do {
			s++;
		} while (is_digit(*s));
	}

	if (out) {
		*out = strtod(*sp, NULL);
  }

	*sp = s;
	return true;  
}

static
bool
number_is_valid(const char *num)
{
  return (parse_number(&num, NULL) && *num == '\0');
}

static
int
write_hex16(char *out, uint16_t val)
{
	const char *hex = "0123456789ABCDEF";
	
	*out++ = hex[(val >> 12) & 0xF];
	*out++ = hex[(val >> 8)  & 0xF];
	*out++ = hex[(val >> 4)  & 0xF];
	*out++ = hex[ val        & 0xF];
	
	return 4;
}

/* }}} */
/* ============================================================================ */




/* json.c ends here. */
