
#ifndef _JSON_H_
#define _JSON_H_

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>

#ifndef json_False
#define json_False  (1 << 0)
#endif
#ifndef json_True
#define json_True   (1 << 1)
#endif
#ifndef json_NULL
#define json_NULL   (1 << 2)
#endif
#ifndef json_Integer
#define json_Integer (1 << 3)
#endif
#ifndef json_Double
#define json_Double (1 << 4)
#endif
#ifndef json_String
#define json_String (1 << 5)
#endif
#ifndef json_Array
#define json_Array  (1 << 6)
#endif
#ifndef json_Object
#define json_Object (1 << 7)
#endif

struct JsonVal_st;
typedef struct JsonVal_st JsonVal;


JsonVal *json_new(void);
int json_get_type(JsonVal *c);
void json_delete(JsonVal *c);
void json_minify(char *json);

void json_parse(JsonVal *c, const char *value);
char *json_stringify(const JsonVal *item);
const char *json_get_errstr(JsonVal *c);

JsonVal *json_get_array_item(const JsonVal *array, int index);
JsonVal *json_get_object_item(const JsonVal *object, const char *key);

void json_add_item_to_object(JsonVal *object, const char *key, JsonVal *item);
void json_add_item_to_array(JsonVal *array, JsonVal *item);

void json_delete_item_from_array(JsonVal *array, int which);
void json_delete_item_from_object(JsonVal *object, const char *key);

void json_add_null_to_object(JsonVal *object, const char *key);
void json_add_true_to_object(JsonVal *object, const char *key);
void json_add_false_to_object(JsonVal *object, const char *key);
void json_add_bool_to_object(JsonVal *object, const char *key, int bool);
void json_add_int_to_object(JsonVal *object, const char *key, int num);
void json_add_double_to_object(JsonVal *object, const char *key, double num);
void json_add_string_to_object(JsonVal *object, const char *key, const char *str);

int json_get_int(JsonVal *c);
double json_get_double(JsonVal *c);
const char * json_get_string(JsonVal *c);

#endif
