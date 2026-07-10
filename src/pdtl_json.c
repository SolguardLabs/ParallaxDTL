#include "pdtl.h"

#include <stdio.h>
#include <string.h>

static void pdtl_json_indent(PdtlJson *json)
{
    int i;

    for (i = 0; i < json->depth; i++) {
        (void)fputs("  ", json->out);
    }
}

static void pdtl_json_string(PdtlJson *json, const char *value)
{
    const unsigned char *p = (const unsigned char *)value;

    (void)fputc('"', json->out);
    while (*p != '\0') {
        switch (*p) {
        case '"':
            (void)fputs("\\\"", json->out);
            break;
        case '\\':
            (void)fputs("\\\\", json->out);
            break;
        case '\b':
            (void)fputs("\\b", json->out);
            break;
        case '\f':
            (void)fputs("\\f", json->out);
            break;
        case '\n':
            (void)fputs("\\n", json->out);
            break;
        case '\r':
            (void)fputs("\\r", json->out);
            break;
        case '\t':
            (void)fputs("\\t", json->out);
            break;
        default:
            if (*p < 0x20u) {
                (void)fprintf(json->out, "\\u%04x", (unsigned int)*p);
            } else {
                (void)fputc((int)*p, json->out);
            }
            break;
        }
        p++;
    }
    (void)fputc('"', json->out);
}

static void pdtl_json_prop_prefix(PdtlJson *json, const char *name)
{
    int current = json->depth - 1;

    if (!json->first[current]) {
        (void)fputs(",\n", json->out);
    } else {
        json->first[current] = 0;
    }
    pdtl_json_indent(json);
    pdtl_json_string(json, name);
    (void)fputs(": ", json->out);
}

static void pdtl_json_array_value_prefix(PdtlJson *json)
{
    int current = json->depth - 1;

    if (!json->first[current]) {
        (void)fputs(",\n", json->out);
    } else {
        json->first[current] = 0;
    }
    pdtl_json_indent(json);
}

void pdtl_json_init(PdtlJson *json, FILE *out)
{
    memset(json, 0, sizeof(*json));
    json->out = out;
}

void pdtl_json_begin_object(PdtlJson *json)
{
    (void)fputs("{\n", json->out);
    json->kind[json->depth] = 'o';
    json->first[json->depth] = 1;
    json->depth++;
}

void pdtl_json_end_object(PdtlJson *json)
{
    if (json->depth <= 0) {
        return;
    }
    (void)fputc('\n', json->out);
    json->depth--;
    pdtl_json_indent(json);
    (void)fputc('}', json->out);
}

void pdtl_json_prop_object_begin(PdtlJson *json, const char *name)
{
    pdtl_json_prop_prefix(json, name);
    (void)fputs("{\n", json->out);
    json->kind[json->depth] = 'o';
    json->first[json->depth] = 1;
    json->depth++;
}

void pdtl_json_prop_array_begin(PdtlJson *json, const char *name)
{
    pdtl_json_prop_prefix(json, name);
    (void)fputs("[\n", json->out);
    json->kind[json->depth] = 'a';
    json->first[json->depth] = 1;
    json->depth++;
}

void pdtl_json_array_object_begin(PdtlJson *json)
{
    pdtl_json_array_value_prefix(json);
    (void)fputs("{\n", json->out);
    json->kind[json->depth] = 'o';
    json->first[json->depth] = 1;
    json->depth++;
}

void pdtl_json_end_array(PdtlJson *json)
{
    if (json->depth <= 0) {
        return;
    }
    (void)fputc('\n', json->out);
    json->depth--;
    pdtl_json_indent(json);
    (void)fputc(']', json->out);
}

void pdtl_json_prop_string(PdtlJson *json, const char *name, const char *value)
{
    pdtl_json_prop_prefix(json, name);
    pdtl_json_string(json, value == NULL ? "" : value);
}

void pdtl_json_prop_u64(PdtlJson *json, const char *name, uint64_t value)
{
    pdtl_json_prop_prefix(json, name);
    (void)fprintf(json->out, "%llu", (unsigned long long)value);
}

void pdtl_json_prop_i32(PdtlJson *json, const char *name, int value)
{
    pdtl_json_prop_prefix(json, name);
    (void)fprintf(json->out, "%d", value);
}

void pdtl_json_prop_bool(PdtlJson *json, const char *name, int value)
{
    pdtl_json_prop_prefix(json, name);
    (void)fputs(value ? "true" : "false", json->out);
}

void pdtl_json_prop_null(PdtlJson *json, const char *name)
{
    pdtl_json_prop_prefix(json, name);
    (void)fputs("null", json->out);
}
