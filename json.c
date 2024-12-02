#include "json.h"

struct JsonVal_st
{
    JsonVal *next;
    JsonVal *prev;
    JsonVal *child;

    int type;

    char *valuestring;
    int valueint;
    double valuedouble;

    char *keystring;

    char errstr[64];
};

typedef struct 
{
    char *buf;
    size_t total;
    size_t off;
} JsonStr;

static const char *json_parse_value(JsonVal *item, const char *value);
static const char *json_parse_array(JsonVal *item, const char *value);
static const char *json_parse_object(JsonVal *item, const char *value);
static void json_stringify_value(const JsonVal *item, JsonStr *out);
static void json_stringify_array(const JsonVal *item, JsonStr *out);
static void json_stringify_object(const JsonVal *item, JsonStr *out);


static void jsonstr_expand(JsonStr *out, const char *normal) {
    size_t vallen = strlen(normal);
    if(out->total - out->off < vallen) {
        int total = (out->total + vallen) < (out->total << 1) ? (out->total << 1) : (out->total + vallen); 
        out->buf = realloc(out->buf, total);
        out->total = total;
    }
    memcpy(out->buf + out->off, normal, vallen);
    out->off += vallen;
}

static void jsonstr_expand_key(JsonStr *out, const char *key) {
    size_t keylen = strlen(key);
    size_t size = 2 + keylen + 1;
    if(out->total - out->off < size) {
        int total = (out->total + size) < (out->total << 1) ? (out->total << 1) : (out->total + size); 
        out->buf = realloc(out->buf, total);
        out->total = total;
    }
    out->buf[out->off++] = '\"';
    memcpy(out->buf + out->off, key, keylen);
    out->off += keylen;
    out->buf[out->off++] = '\"';
    out->buf[out->off++] = ':';
}

static void jsonstr_expand_normal(JsonStr *out, const char *key, const char *normal) {
    size_t keylen = strlen(key), vallen = strlen(normal);
    size_t size = 2 + keylen + 1 + vallen;
    if(out->total - out->off < size) {
        int total = (out->total + size) < (out->total << 1) ? (out->total << 1) : (out->total + size); 
        out->buf = realloc(out->buf, total);
        out->total = total;
    }
    out->buf[out->off++] = '\"';
    memcpy(out->buf + out->off, key, keylen);
    out->off += keylen;
    out->buf[out->off++] = '\"';
    out->buf[out->off++] = ':';
    memcpy(out->buf + out->off, normal, vallen);
    out->off += vallen;
}

static void jsonstr_expand_int(JsonStr *out, const char *key, int num) {
    char val[16] = {0};
    sprintf(val, "%d", num);

    jsonstr_expand_normal(out, key, val);
}

static void jsonstr_expand_double(JsonStr *out, const char *key, const char *fmt, double num) {
    char val[32] = {0};
    sprintf(val, fmt, num);
    size_t off = strlen(val) - 1;
    while(val[off] == '0') {
        off--;
    }
    if(val[off] == '.') {
        val[off + 2] = 0;
    } else {
        val[off + 1] = 0; 
    }
    jsonstr_expand_normal(out, key, val);
}

static void jsonstr_expand_str(JsonStr *out, const char *key, const char *val) {
    size_t keylen = strlen(key), vallen = strlen(val);
    size_t size = 2 + keylen + 1 + 2 + vallen + 32;
    if(out->total - out->off < size) {
        int total = (out->total + size) < (out->total << 1) ? (out->total << 1) : (out->total + size); 
        out->buf = realloc(out->buf, total);
        out->total = total;
    }
    out->buf[out->off++] = '\"';
    memcpy(out->buf + out->off, key, keylen);
    out->off += keylen;
    out->buf[out->off++] = '\"';
    out->buf[out->off++] = ':';
    out->buf[out->off++] = '\"';
    
    size_t i = 0;
    while(i < vallen) {
        if(val[i] == '"') {
            out->buf[out->off++] = '\\';
            out->buf[out->off++] = val[i];
        } else {
            out->buf[out->off++] = val[i];
        }
        i++;
    }
    out->buf[out->off++] = '\"';
}

static void json_seterr(JsonVal *c, const char *str) {
    size_t end = strlen(str);
    end = end > 64 ? 64 : end;
    memcpy(c->errstr, str, end);
}

static void json_cpyerr(JsonVal *p, const JsonVal *c) {
    memcpy(p->errstr, c->errstr, 64);
}

static int json_strcasecmp(const char *s1, const char *s2)
{
    if (!s1)
    {
        return (s1 == s2) ? 0 : 1;
    }
    if (!s2)
    {
        return 1;
    }
    for(; tolower(*(const unsigned char *)s1) == tolower(*(const unsigned char *)s2); ++s1, ++s2)
    {
        if (*s1 == 0)
        {
            return 0;
        }
    }

    return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

static const char *json_parse_number(JsonVal *item, const char *num)
{
    const char *start = num;
    int dot = 0, opt = 0;
    while((*num >= '0' && *num <= '9') || *num == '+' || *num == '-' || *num == '.') {
        if(*num == '-' || *num == '+') {
            if(opt) {
                json_seterr(item, start);
                return 0;
            }
            opt = 1;
        }
        if(*num == '.') {
            if(dot) {
                json_seterr(item, start);
                return 0;
            }
            dot = 1;
        }
        num++;
    }
    
    int len = num - start;
    char numstr[len +1];
    memcpy(numstr, start, len);
    numstr[len] = 0;
    if(dot) {
        item->valuedouble = atof(numstr);
        item->valueint = (int) item->valuedouble;
        item->type = json_Double;
    } else {
        item->valueint = atoi(numstr);
        item->valuedouble = item->valueint;
        item->type = json_Integer;
    }

    return num;
}

static unsigned json_parse_hex4(const char *str)
{
    unsigned h = 0;
    if ((*str >= '0') && (*str <= '9'))
    {
        h += (*str) - '0';
    }
    else if ((*str >= 'A') && (*str <= 'F'))
    {
        h += 10 + (*str) - 'A';
    }
    else if ((*str >= 'a') && (*str <= 'f'))
    {
        h += 10 + (*str) - 'a';
    }
    else 
    {
        return 0;
    }

    h = h << 4;
    str++;
    if ((*str >= '0') && (*str <= '9'))
    {
        h += (*str) - '0';
    }
    else if ((*str >= 'A') && (*str <= 'F'))
    {
        h += 10 + (*str) - 'A';
    }
    else if ((*str >= 'a') && (*str <= 'f'))
    {
        h += 10 + (*str) - 'a';
    }
    else 
    {
        return 0;
    }

    h = h << 4;
    str++;
    if ((*str >= '0') && (*str <= '9'))
    {
        h += (*str) - '0';
    }
    else if ((*str >= 'A') && (*str <= 'F'))
    {
        h += 10 + (*str) - 'A';
    }
    else if ((*str >= 'a') && (*str <= 'f'))
    {
        h += 10 + (*str) - 'a';
    }
    else 
    {
        return 0;
    }

    h = h << 4;
    str++;
    if ((*str >= '0') && (*str <= '9'))
    {
        h += (*str) - '0';
    }
    else if ((*str >= 'A') && (*str <= 'F'))
    {
        h += 10 + (*str) - 'A';
    }
    else if ((*str >= 'a') && (*str <= 'f'))
    {
        h += 10 + (*str) - 'a';
    }
    else 
    {
        return 0;
    }

    return h;
}

static const unsigned char json_1st_byte_mark[7] =
{
    0x00,
    0x00, 
    0xC0, 
    0xE0,
    0xF0,
    0xF8,
    0xFC
};

static const char *json_parse_string(JsonVal *item, const char *str)
{
    const char *ptr = str + 1;
    const char *end_ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc;
    unsigned uc2;

    if (*str != '\"')
    {
        json_seterr(item, str);
        return 0;
    }

    while ((*end_ptr != '\"') && *end_ptr && ++len)
    {
        if (*end_ptr++ == '\\')
        { 
            if (*end_ptr == '\0')
            {
                json_seterr(item, end_ptr-4);
                return 0;
            }
            end_ptr++;
        }
    }

    out = (char*)malloc(len + 1);
    if (!out)
    {
        json_seterr(item, "out of memory");
        return 0;
    }
    item->valuestring = out; 
    item->type = json_String;

    ptr = str + 1;
    ptr2 = out;
    while (ptr < end_ptr)
    {
        if (*ptr != '\\')
        {
            *ptr2++ = *ptr++;
        }
        else
        {
            ptr++;
            switch (*ptr)
            {
                case 'b':
                    *ptr2++ = '\b';
                    break;
                case 'f':
                    *ptr2++ = '\f';
                    break;
                case 'n':
                    *ptr2++ = '\n';
                    break;
                case 'r':
                    *ptr2++ = '\r';
                    break;
                case 't':
                    *ptr2++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *ptr2++ = *ptr;
                    break;
                case 'u':
                    uc = json_parse_hex4(ptr + 1); 
                    ptr += 4;
                    if (ptr >= end_ptr)
                    {
                        json_seterr(item, ptr);
                        return 0;
                    }
                    if (((uc >= 0xDC00) && (uc <= 0xDFFF)) || (uc == 0))
                    {
                        json_seterr(item, ptr);
                        return 0;
                    }

                    if ((uc >= 0xD800) && (uc<=0xDBFF))
                    {
                        if ((ptr + 6) > end_ptr)
                        {
                            json_seterr(item, ptr);
                            return 0;
                        }
                        if ((ptr[1] != '\\') || (ptr[2] != 'u'))
                        {
                            json_seterr(item, ptr);
                            return 0;
                        }
                        uc2 = json_parse_hex4(ptr + 3);
                        ptr += 6; 
                        if ((uc2 < 0xDC00) || (uc2 > 0xDFFF))
                        {
                            json_seterr(item, ptr);
                            return 0;
                        }
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }

                    len = 4;
                    if (uc < 0x80)
                    {
                        len = 1;
                    }
                    else if (uc < 0x800)
                    {
                        len = 2;
                    }
                    else if (uc < 0x10000)
                    {
                        len = 3;
                    }
                    ptr2 += len;

                    switch (len) {
                        case 4:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 3:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 2:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 1:
                            *--ptr2 = (uc | json_1st_byte_mark[len]);
                    }
                    ptr2 += len;
                    break;
                default:
                    json_seterr(item, ptr);
                    return 0;
            }
            ptr++;
        }
    }
    *ptr2 = '\0';
    if (*ptr == '\"')
    {
        ptr++;
    }

    return ptr;
}

static const char *skip(const char *in)
{
    while (in && *in && ((unsigned char)*in<=32))
    {
        in++;
    }

    return in;
}

static const char *json_parse_value(JsonVal *item, const char *value)
{
    if (!value)
    {
        json_seterr(item, "empty content");
        return 0;
    }

    if (!strncmp(value, "null", 4))
    {
        item->type = json_NULL;
        return value + 4;
    }
    if (!strncmp(value, "false", 5))
    {
        item->type = json_False;
        return value + 5;
    }
    if (!strncmp(value, "true", 4))
    {
        item->type = json_True;
        item->valueint = 1;
        return value + 4;
    }
    if (*value == '\"')
    {
        return json_parse_string(item, value);
    }
    if ((*value == '-') || (*value == '+') || ((*value >= '0') && (*value <= '9')))
    {
        return json_parse_number(item, value);
    }
    if (*value == '[')
    {
        return json_parse_array(item, value);
    }
    if (*value == '{')
    {
        return json_parse_object(item, value);
    }
    json_seterr(item, value);
    return 0;
}

static const char *json_parse_array(JsonVal *item,const char *value)
{
    JsonVal *child;
    if (*value != '[')
    {
        json_seterr(item, value);
        return 0;
    }

    item->type = json_Array;
    value = skip(value + 1);
    if (*value == ']')
    {
        return value + 1;
    }

    item->child = child = json_new();
    if (!item->child)
    {
        json_seterr(item, "out of memory");
        return 0;
    }
    value = skip(json_parse_value(child, skip(value)));
    if (!value)
    {
        json_cpyerr(item, child);
        return 0;
    }

    while (*value == ',')
    {
        JsonVal *new_item;
        if (!(new_item = json_new()))
        {
            json_seterr(item, "out of memory");
            return 0;
        }
        child->next = new_item;
        new_item->prev = child;
        child = new_item;

        value = skip(json_parse_value(child, skip(value + 1)));
        if (!value)
        {
            json_cpyerr(item, child);
            return 0;
        }
    }

    if (*value == ']')
    {
        return value + 1;
    }
    json_seterr(item, value);
    return 0;
}

static const char *json_parse_object(JsonVal *item, const char *value)
{
    JsonVal *child;
    if (*value != '{')
    {   
        json_seterr(item, value);
        return 0;
    }

    item->type = json_Object;
    value = skip(value + 1);
    if (*value == '}')
    {
        return value + 1;
    }

    child = json_new();
    item->child = child;
    if (!item->child)
    {
        json_seterr(item, "out of memory");
        return 0;
    }
    value = skip(json_parse_string(child, skip(value)));
    if (!value)
    {
        json_cpyerr(item, child);
        return 0;
    }
    child->keystring = child->valuestring;
    child->valuestring = 0;

    if (*value != ':')
    {
        json_seterr(item, value);
        return 0;
    }
    value = skip(json_parse_value(child, skip(value + 1)));
    if (!value)
    {
        json_cpyerr(item, child);
        return 0;
    }

    while (*value == ',')
    {
        JsonVal *new_item;
        if (!(new_item = json_new()))
        {
            json_seterr(item, "out of memory");
            return 0;
        }
        child->next = new_item;
        new_item->prev = child;

        child = new_item;
        value = skip(json_parse_string(child, skip(value + 1)));
        if (!value)
        {
            json_cpyerr(item, child);
            return 0;
        }
        child->keystring = child->valuestring;
        child->valuestring = 0;

        if (*value != ':')
        {
            json_seterr(item, value);
            return 0;
        }
        value = skip(json_parse_value(child, skip(value + 1)));
        if (!value)
        {
            json_cpyerr(item, child);
            return 0;
        }
    }
    if (*value == '}')
    {
        return value + 1;
    }
    json_seterr(item, value);
    return 0;
}

static JsonVal *json_detach_item_from_array(JsonVal *array, int which) {
    JsonVal *c = array->child;
    while (c && (which > 0))
    {
        c = c->next;
        which--;
    }
    if (!c)
    {
        return 0;
    }
    if (c->prev)
    {
        c->prev->next = c->next;
    }
    if (c->next)
    {
        c->next->prev = c->prev;
    }
    if (c==array->child)
    {
        array->child = c->next;
    }
    c->prev = c->next = 0;

    return c;
}


static void json_stringify_array(const JsonVal *item, JsonStr *out) {
    JsonVal *child = item->child;

    if (!child) {
        jsonstr_expand(out, "[]");
        return ;
    }

    int count = 0;
    while(child) {
        child = child->next;
        count++;
    }
    child = item->child;
    jsonstr_expand(out, "[");
    while(child) {
        json_stringify_value(child, out);
        if(count > 1) {
            jsonstr_expand(out, ",");
        }
        count--;
        child = child->next;
    }
    jsonstr_expand(out, "]");
}

static void json_stringify_object(const JsonVal *item, JsonStr *out) {
    int count = 0;
    JsonVal *child = item->child;
    if (!child) {
        jsonstr_expand(out, "{}");
        return ;
    }

    while (child) {
        child = child->next;
        count++;
    }

    child = item->child;
    jsonstr_expand(out, "{");
    while(child) {
        json_stringify_value(child, out);
        if(count > 1) {
            jsonstr_expand(out, ",");
        }

        count--;
        child = child->next;
    }
    jsonstr_expand(out, "}");
}


static void json_stringify_value(const JsonVal *item, JsonStr *out) {
    if (!item) {
        return ;
    }
    switch ((item->type) & 0xFF)
    {
        case json_NULL:
            jsonstr_expand_normal(out, item->keystring, "null");
            break;
        case json_False:
            jsonstr_expand_normal(out, item->keystring, "false");
            break;
        case json_True:
            jsonstr_expand_normal(out, item->keystring, "true");
            break;
        case json_Integer:
            jsonstr_expand_int(out, item->keystring, item->valueint);
            break;
        case json_Double:
            if ((item->valuedouble * 0) != 0) {
                jsonstr_expand_normal(out, item->keystring, "null");
            } else {
                jsonstr_expand_double(out, item->keystring, "%lf", item->valuedouble);
            }
            break;
        case json_String:
            jsonstr_expand_str(out, item->keystring, item->valuestring);
            break;
        case json_Array:
            jsonstr_expand_key(out, item->keystring);
            json_stringify_array(item, out);
            break;
        case json_Object:
            jsonstr_expand_key(out, item->keystring);
            json_stringify_object(item, out);
            break;
    }
}



JsonVal *json_new(void) {
    JsonVal* node = (JsonVal*)malloc(sizeof(JsonVal));
    if (node)
    {
        memset(node, 0, sizeof(JsonVal));
    }

    return node;
}
int json_get_type(JsonVal *c) {
    return c->type;
}
void json_delete(JsonVal *c) {
    JsonVal *next;
    while (c)
    {
        next = c->next;
        if (c->child)
        {
            json_delete(c->child);
        }
        if (c->valuestring)
        {
            free(c->valuestring);
        }
        if (c->keystring)
        {
            free(c->keystring);
        }
        free(c);
        c = next;
    }
}
void json_minify(char *json){
    char *into = json;
    while (*json)
    {
        if (*json == ' ')
        {
            json++;
        }
        else if (*json == '\t')
        {
            json++;
        }
        else if (*json == '\r')
        {
            json++;
        }
        else if (*json=='\n')
        {
            json++;
        }
        else if ((*json == '/') && (json[1] == '/'))
        {
            while (*json && (*json != '\n'))
            {
                json++;
            }
        }
        else if ((*json == '/') && (json[1] == '*'))
        {
            while (*json && !((*json == '*') && (json[1] == '/')))
            {
                json++;
            }
            json += 2;
        }
        else if (*json == '\"')
        {
            *into++ = *json++;
            while (*json && (*json != '\"'))
            {
                if (*json == '\\')
                {
                    *into++=*json++;
                }
                *into++ = *json++;
            }
            *into++ = *json++;
        }
        else
        {
            *into++ = *json++;
        }
    }
    *into = '\0';
}

void json_parse(JsonVal *c, const char *value) {  
    const char *end = json_parse_value(c, skip(value));
    if (!end)
    {
        json_seterr(c, end);
    }
}
char *json_stringify(const JsonVal *item)
{
    if (!item) {
        return NULL;
    }

    JsonStr *out = (JsonStr *)malloc(sizeof(JsonStr));
    out->off = 0;
    out->total = 256;
    out->buf = malloc(out->total);
    switch ((item->type) & 0xFF)
    {
        case json_NULL:
            jsonstr_expand(out, "null");
            break;
        case json_False:
            jsonstr_expand(out, "false");
            break;
        case json_True:
            jsonstr_expand(out, "true");
            break;
        case json_Integer: {
            char val[16] = {0};
            sprintf(val, "%d", 0);
            jsonstr_expand(out, val);
            break;
        }
        case json_Double: {
            char val[32] = {0};
            if ((item->valuedouble * 0) != 0) {
                memcpy(val, "null", 4);
            } else {
                sprintf(val, "%lf", item->valuedouble);
            }
            jsonstr_expand(out, val);
            break;
        }
        case json_String:
            jsonstr_expand(out, item->valuestring);
            break;
        case json_Array:
            json_stringify_array(item, out);
            break;
        case json_Object:
            json_stringify_object(item, out);
            break;
    }
    
    char *str = malloc(out->off + 1);
    memcpy(str, out->buf, out->off);
    str[out->off] = '\0';
    free(out->buf);
    out->buf = NULL;
    free(out);
    return str;
}
const char *json_get_errstr(JsonVal *c) {
    return c->errstr;
}

JsonVal *json_get_array_item(const JsonVal *array, int index) {
    JsonVal *c = array ? array->child : 0;
    while (c && index > 0)
    {
        index--;
        c = c->next;
    }
    return c;
}
JsonVal *json_get_object_item(const JsonVal *object, const char *key) {
    JsonVal *c = object ? object->child : 0;
    while (c && json_strcasecmp(c->keystring, key))
    {
        c = c->next;
    }
    return c;
}

void json_add_item_to_object(JsonVal *object, const char *key, JsonVal *item) {
    int i = 0;
    JsonVal *c = object->child, *last = NULL;
    if(!c) {
        object->child = item;
        return ;
    }
    while(c && json_strcasecmp(c->keystring, key))
    {
        i++;
        c = c->next;
        last = c;
    }
    if(item->keystring) {
        free(item->keystring);
    }
    size_t klen = strlen(key);
    item->keystring = malloc(klen + 1);
    memcpy(item->keystring, key, klen);
    item->keystring[klen] = 0;

    if(!c) {
        last->next = item;
        item->prev = c;
        return ;
    }
        
    item->next = c->next;
    item->prev = c->prev;
    if (item->next)
    {
        item->next->prev = item;
    }
    if (c == object->child)
    {
        object->child = item;
    }
    else
    {
        item->prev->next = item;
    }
    c->next = c->prev = 0;
    json_delete(c);
}
void json_add_item_to_array(JsonVal *array, JsonVal *item) {
    JsonVal *c = array->child;
    if (!item)
    {
        return;
    }
    if (!c)
    {
        array->child = item;
    }
    else
    {
        while (c->next)
        {
            c = c->next;
        }
        
        c->next = item;
        item->prev = c;
    }
}

void json_delete_item_from_array(JsonVal *array, int which) {
    json_delete(json_detach_item_from_array(array, which));
}
void json_delete_item_from_object(JsonVal *object, const char *key) {
    int i = 0;
    JsonVal *c = object->child;
    while (c && json_strcasecmp(c->keystring,key))
    {
        i++;
        c = c->next;
    }
    if (c)
    {
        json_delete(json_detach_item_from_array(object, i));
    }
}

void json_add_null_to_object(JsonVal *object, const char *key) {
    JsonVal *item = json_new();
    if(item)
    {
        item->type = json_NULL;
        json_add_item_to_object(object, key, item);
    }
}
void json_add_true_to_object(JsonVal *object, const char *key) {
    JsonVal *item = json_new();
    if(item)
    {
        item->type = json_True;
        json_add_item_to_object(object, key, item);
    }
}
void json_add_false_to_object(JsonVal *object, const char *key) {
    JsonVal *item = json_new();
    if(item)
    {
        item->type = json_False;
        json_add_item_to_object(object, key, item);
    }
} 
void json_add_bool_to_object(JsonVal *object, const char *key, int bool) {
    JsonVal *item = json_new();
    if(item)
    {   
        if(bool) {
            item->type = json_True;
        } else {
            item->type = json_False;
        }
        json_add_item_to_object(object, key, item);
    }
} 
void json_add_int_to_object(JsonVal *object, const char *key, int num) {
    JsonVal *item = json_new();
    if(item)
    {
        item->type = json_Integer;
        item->valuedouble = num;
        item->valueint = num;
        json_add_item_to_object(object, key, item);
    }
} 
void json_add_double_to_object(JsonVal *object, const char *key, double num) {
    JsonVal *item = json_new();
    if(item)
    {
        item->type = json_Double;
        item->valuedouble = num;
        item->valueint = (int)num;
        json_add_item_to_object(object, key, item);
    }
} 
void json_add_string_to_object(JsonVal *object, const char *key, const char *str) {
    JsonVal *item = json_new();
    if(item)
    {
        item->type = json_String;
    
        size_t vlen = strlen(str);
        item->valuestring = malloc(vlen + 1);
        if(!item->valuestring)
        {
            json_delete(item);
            return ;
        }
        memcpy(item->valuestring, str, vlen);
        item->valuestring[vlen] = 0;

        json_add_item_to_object(object, key, item);
    }
}

int json_get_int(JsonVal *c) {
    if(c->type != json_Integer && c->type != json_Double) {
        return 0;
    }
    return c->valueint;
}
double json_get_double(JsonVal *c) {
    if(c->type != json_Integer && c->type != json_Double) {
        return 0;
    }
    return c->valuedouble;
}
const char * json_get_string(JsonVal *c) {
    if(c->type != json_String) {
        return NULL;
    }
    return c->valuestring;
}
