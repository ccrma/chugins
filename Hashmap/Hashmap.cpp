//-----------------------------------------------------------------------------
// HashMap.chug
// Author: Andrew Zhu Aday (azaday)
// Date: Spring 2024
//-----------------------------------------------------------------------------
// run `chuck --chugin-probe` to probe what chugins would be loaded, and
//      from where in the chugin search paths
//
// run `chuck -v3 --loop` to see what chugins are actually loaded at runtime,
//      with more info and error reporting than with --chugin-probe
//
// other helpful chugin-related flags include:
//      --chugin:<filename>
//      --chugin-path:(path) / -G(path)
//      --chugin-load:{on/off}
//
// for more information on command-line options:
//      https://chuck.stanford.edu/doc/program/options.html
// for more information on chugins:
//      https://chuck.stanford.edu/extend/
//-----------------------------------------------------------------------------

#include <vector>
#include <algorithm>

#include "chugin.h"
#include "hashmap.h"
#include "jsmn.h"

CK_DLL_INFO(Hashmap)
{
    // the version string of this chugin, e.g., "v1.2.1"
    QUERY->setinfo(QUERY, CHUGIN_INFO_CHUGIN_VERSION, "v1.0");
    // the author(s) of this chugin, e.g., "Alice Baker & Carl Donut"
    QUERY->setinfo(QUERY, CHUGIN_INFO_AUTHORS, "Andrew Zhu Aday");
    // text description of this chugin; what is it? what does it do? who is it for?
    QUERY->setinfo(QUERY, CHUGIN_INFO_DESCRIPTION, "Generic hashmap");
    // (optional) URL of the homepage for this chugin
    QUERY->setinfo(QUERY, CHUGIN_INFO_URL, "");
    // (optional) contact email
    QUERY->setinfo(QUERY, CHUGIN_INFO_EMAIL, "azaday@ccrma.stanford.edu");
}

static Chuck_VM *g_vm = NULL;
static CK_DL_API g_api = NULL;
static Chuck_Type *g_hm_type = NULL;
static Chuck_Type *g_int_array_type = NULL;
static Chuck_Type *g_object_array_type = NULL;
static Chuck_Type *g_string_array_type = NULL;

#define HM_ADD_REF(obj) (g_api->object->add_ref(obj))
#define HM_RELEASE_REF(obj) (g_api->object->release(obj))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, lo, hi) (MIN(hi, MAX(lo, x)))

// #ifndef NDEBUG
#define ASSERT(expression)                                                      \
    {                                                                           \
        if (!(expression))                                                      \
        {                                                                       \
            printf("Assertion(%s) failed: file \"%s\", line %d\n", #expression, \
                   __FILE__, __LINE__);                                         \
        }                                                                       \
    }
// #else
// #define ASSERT(expression) NULL;
// #endif

enum HM_Type : uint8_t
{
    HM_NULL = 0,
    HM_INT,   // 1
    HM_FLOAT, // 2
    HM_OBJ,   // 3
    HM_STR,   // 4
};

const char *HM_TypeNames[] = {
    "null",
    "int",
    "float",
    "Object",
    "string",
};

// data type for key / value
struct HM_Entry
{
    union
    {
        t_CKINT ckint;
        t_CKFLOAT ckfloat; // supported for value only
        Chuck_Object *ckobj;
        const char *str_OWNED; // must be freed on delete and clear
    } as;
    HM_Type type;
};

// key-value pair that is stored in the hashmap
struct HM_Pair
{
    HM_Entry key;
    HM_Entry value;

    // returns 0 if equal.
    static int compare(const void *a, const void *b, void *udata)
    {
        HM_Entry key_a = ((HM_Pair *)a)->key;
        HM_Entry key_b = ((HM_Pair *)b)->key;

        if (key_a.type != key_b.type)
            return key_a.type - key_b.type;

        switch (key_a.type)
        {
        case HM_INT:
            return key_a.as.ckint - key_b.as.ckint;
        case HM_OBJ:
            return (uint8_t *)key_a.as.ckobj - (uint8_t *)key_b.as.ckobj;
        case HM_STR:
            return strcmp(key_a.as.str_OWNED, key_b.as.str_OWNED);
        case HM_FLOAT: // not allowing float keys for now (due to precision issues)
        default:
            g_api->vm->em_log(1, "Hashmap: unsupported key type");
            return 0;
        }
    }

    static uint64_t hash(const void *item, uint64_t seed0, uint64_t seed1)
    {
        HM_Pair *pair = (HM_Pair *)item;

        switch (pair->key.type)
        {
        case HM_INT:
            return hashmap_xxhash3(&pair->key.as.ckint, sizeof(pair->key.as.ckint), seed0, seed1);
        case HM_OBJ:
            return hashmap_xxhash3(&pair->key.as.ckobj, sizeof(pair->key.as.ckobj), seed0, seed1);
        case HM_STR:
        {
            const char *c_str = pair->key.as.str_OWNED;
            return hashmap_xxhash3(c_str, strlen(c_str), seed0, seed1);
        }
        default:
            g_api->vm->em_log(1, "Hashmap: unsupported key type");
            return 0;
        }
        // return hashmap_sip(item, sizeof(int), seed0, seed1);
        // return hashmap_murmur(item, sizeof(int), seed0, seed1);
    }

    static void releaseEntry(HM_Entry *entry)
    {
        if (entry == NULL)
            return;

        switch (entry->type)
        {
        case HM_OBJ:
            HM_RELEASE_REF(entry->as.ckobj);
            return;
        case HM_STR:
            free((char *)entry->as.str_OWNED);
            return;
        default:
            return;
        }
    }

    // called when deleting element from hashmap
    static void releaseRef(void *item)
    {
        if (item == NULL)
            return;

        HM_Pair *pair = (HM_Pair *)item;

        // release chuck objects
        HM_Pair::releaseEntry(&pair->key);
        HM_Pair::releaseEntry(&pair->value);
    }

    static void addRefEntry(HM_Entry *entry)
    {
        if (entry == NULL)
            return;

        if (entry->type == HM_OBJ)
            HM_ADD_REF(entry->as.ckobj);
    }

    static void addRef(HM_Pair *pair)
    {
        if (pair == NULL)
            return;

        HM_Pair::addRefEntry(&pair->key);
        HM_Pair::addRefEntry(&pair->value);
    }
};

// ==================================================================
// hashmap impl
// ==================================================================

t_CKINT hashmap_ptr_offset = 0;

static Chuck_Object *HM_Init(Chuck_Object *ck_obj)
{
    CK_DL_API API = g_api;
    // create ck_obj if not provided
    if (ck_obj == NULL)
    {
        ck_obj = g_api->object->create_without_shred(g_vm, g_hm_type, false); // NO add ref
    }

    int seed = time(NULL);
    OBJ_MEMBER_UINT(ck_obj, hashmap_ptr_offset) = (t_CKUINT)hashmap_new(
        sizeof(HM_Pair), 0, seed, seed, HM_Pair::hash, HM_Pair::compare, HM_Pair::releaseRef, NULL);

    return ck_obj;
}

CK_DLL_CTOR(hm_ctor)
{
    HM_Init(SELF);
}

CK_DLL_DTOR(hm_dtor)
{
    hashmap_free((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset));
    OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset) = 0;
}

CK_DLL_MFUN(hm_count)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    RETURN->v_int = hashmap_count(hm);
}

static void HM_SetImpl(Chuck_Object *ckobj, HM_Type key_type, HM_Type value_type, void *ARGS)
{
    CK_DL_API API = g_api;
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(ckobj, hashmap_ptr_offset);

    HM_Pair pair = {};
    pair.key.type = key_type;
    pair.value.type = value_type;
    // set key
    switch (key_type)
    {
    case HM_INT:
        pair.key.as.ckint = GET_NEXT_INT(ARGS);
        break;
    case HM_OBJ:
        pair.key.as.ckobj = GET_NEXT_OBJECT(ARGS);
        break;
    case HM_STR:
    {
        Chuck_String *ckstr = GET_NEXT_STRING(ARGS);
        pair.key.as.str_OWNED = strdup(g_api->object->str(ckstr));
    }
    break;
    default:
        break;
    }
    // set value
    switch (value_type)
    {
    case HM_INT:
        pair.value.as.ckint = GET_NEXT_INT(ARGS);
        break;
    case HM_FLOAT:
        pair.value.as.ckfloat = GET_NEXT_FLOAT(ARGS);
        break;
    case HM_OBJ:
        pair.value.as.ckobj = GET_NEXT_OBJECT(ARGS);
        break;
    case HM_STR:
    {
        Chuck_String *ckstr = GET_NEXT_STRING(ARGS);
        pair.value.as.str_OWNED = strdup(g_api->object->str(ckstr));
    }
    break;
    default:
        break;
    }

    HM_Pair::addRef(&pair);
    HM_Pair::releaseRef((HM_Pair *)hashmap_set(hm, &pair));
}

CK_DLL_MFUN(hm_set_int_int)
{
    HM_SetImpl(SELF, HM_INT, HM_INT, ARGS);
}

CK_DLL_MFUN(hm_set_int_float)
{
    HM_SetImpl(SELF, HM_INT, HM_FLOAT, ARGS);
}

CK_DLL_MFUN(hm_set_int_obj)
{
    HM_SetImpl(SELF, HM_INT, HM_OBJ, ARGS);
}

CK_DLL_MFUN(hm_set_int_str)
{
    HM_SetImpl(SELF, HM_INT, HM_STR, ARGS);
}

CK_DLL_MFUN(hm_set_obj_int)
{
    HM_SetImpl(SELF, HM_OBJ, HM_INT, ARGS);
}

CK_DLL_MFUN(hm_set_obj_float)
{
    HM_SetImpl(SELF, HM_OBJ, HM_FLOAT, ARGS);
}

CK_DLL_MFUN(hm_set_obj_obj)
{
    HM_SetImpl(SELF, HM_OBJ, HM_OBJ, ARGS);
}

CK_DLL_MFUN(hm_set_obj_str)
{
    HM_SetImpl(SELF, HM_OBJ, HM_STR, ARGS);
}

CK_DLL_MFUN(hm_set_str_int)
{
    HM_SetImpl(SELF, HM_STR, HM_INT, ARGS);
}

CK_DLL_MFUN(hm_set_str_float)
{
    HM_SetImpl(SELF, HM_STR, HM_FLOAT, ARGS);
}

CK_DLL_MFUN(hm_set_str_obj)
{
    HM_SetImpl(SELF, HM_STR, HM_OBJ, ARGS);
}

CK_DLL_MFUN(hm_set_str_str)
{
    HM_SetImpl(SELF, HM_STR, HM_STR, ARGS);
}

static void HM_GetImpl(Chuck_Object *ckobj, HM_Type key_type, HM_Type value_type, void *ARGS, Chuck_VM_Shred *SHRED, Chuck_DL_Return *RETURN)
{
    CK_DL_API API = g_api;
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(ckobj, hashmap_ptr_offset);

    HM_Pair pair = {};
    pair.key.type = key_type;
    // set key
    switch (key_type)
    {
    case HM_INT:
        pair.key.as.ckint = GET_NEXT_INT(ARGS);
        break;
    case HM_OBJ:
        pair.key.as.ckobj = GET_NEXT_OBJECT(ARGS);
        break;
    case HM_STR:
    {
        // don't strdup here because this is only a lookup key
        Chuck_String *ckstr = GET_NEXT_STRING(ARGS);
        pair.key.as.str_OWNED = (char *)g_api->object->str(ckstr);
    }
    break;
    default:
        break;
    }

    // lookup
    HM_Pair *result = (HM_Pair *)hashmap_get(hm, &pair);

    // default return values if not found
    if (result == NULL)
    {
        switch (value_type)
        {
        case HM_INT:
            RETURN->v_int = 0;
            break;
        case HM_FLOAT:
            RETURN->v_float = 0.0;
            break;
        case HM_OBJ:
            RETURN->v_object = NULL;
            break;
        case HM_STR:
            RETURN->v_string = g_api->object->create_string(g_vm, "", false);
            break;
        default:
            break;
        }
        return;
    }

    // check type match
    if (result->value.type != value_type)
    {
        char err_msg[256] = {};
        snprintf(err_msg, 256,
                 "Expected Hashmap value of type %s, but type was %s",
                 HM_TypeNames[value_type],
                 HM_TypeNames[result->value.type]);
        // TODO: log error rather than crash
        g_api->vm->throw_exception("HashmapGetTypeMismatch",
                                   err_msg,
                                   SHRED);
        // thread is halted, no need to set RETURN value
        return;
    }

    // set value
    switch (value_type)
    {
    case HM_INT:
        RETURN->v_int = result->value.as.ckint;
        break;
    case HM_FLOAT:
        RETURN->v_float = result->value.as.ckfloat;
        break;
    case HM_OBJ:
        RETURN->v_object = result->value.as.ckobj;
        break;
    case HM_STR:
        RETURN->v_string = g_api->object->create_string(g_vm, result->value.as.str_OWNED, false);
        break;
    default:
        break;
    }
}

CK_DLL_MFUN(hm_get_int_int)
{
    HM_GetImpl(SELF, HM_INT, HM_INT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_int_float)
{
    HM_GetImpl(SELF, HM_INT, HM_FLOAT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_int_obj)
{
    HM_GetImpl(SELF, HM_INT, HM_OBJ, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_int_str)
{
    HM_GetImpl(SELF,
               HM_INT, HM_STR, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_int)
{
    HM_GetImpl(SELF, HM_OBJ, HM_INT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_float)
{
    HM_GetImpl(SELF, HM_OBJ, HM_FLOAT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_obj)
{
    HM_GetImpl(SELF, HM_OBJ, HM_OBJ, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_str)
{
    HM_GetImpl(SELF, HM_OBJ, HM_STR, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_int)
{
    HM_GetImpl(SELF, HM_STR, HM_INT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_float)
{
    HM_GetImpl(SELF, HM_STR, HM_FLOAT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_obj)
{
    HM_GetImpl(SELF, HM_STR, HM_OBJ, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_str)
{
    HM_GetImpl(SELF, HM_STR, HM_STR, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_hashmap)
{
    HM_GetImpl(SELF, HM_STR, HM_OBJ, ARGS, SHRED, RETURN);
    if (RETURN->v_object != NULL)
    {
        Chuck_Type *obj_type = API->object->get_type(RETURN->v_object);
        if (!API->type->isa(obj_type, g_hm_type))
        {
            g_api->vm->em_log(1, "HashMap.get(string): object at key is *not* a HashMap");
        }
    }
}

CK_DLL_MFUN(hm_has_int)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    t_CKINT key = GET_NEXT_INT(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_INT;
    pair.key.as.ckint = key;

    RETURN->v_int = hashmap_get(hm, &pair) ? 1 : 0;
}

CK_DLL_MFUN(hm_has_obj)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_Object *key = GET_NEXT_OBJECT(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_OBJ;
    pair.key.as.ckobj = key;

    RETURN->v_int = hashmap_get(hm, &pair) ? 1 : 0;
}

CK_DLL_MFUN(hm_has_str)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_String *key = GET_NEXT_STRING(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_STR;
    pair.key.as.str_OWNED = API->object->str(key);

    RETURN->v_int = hashmap_get(hm, &pair) ? 1 : 0;
}

CK_DLL_MFUN(hm_del_int)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    t_CKINT key = GET_NEXT_INT(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_INT;
    pair.key.as.ckint = key;

    HM_Pair *deleted_pair = (HM_Pair *)hashmap_delete(hm, &pair);
    HM_Pair::releaseRef(deleted_pair);
    RETURN->v_int = deleted_pair ? 1 : 0;
}

CK_DLL_MFUN(hm_del_obj)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_Object *key = GET_NEXT_OBJECT(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_OBJ;
    pair.key.as.ckobj = key;

    HM_Pair *deleted_pair = (HM_Pair *)hashmap_delete(hm, &pair);
    HM_Pair::releaseRef(deleted_pair);
    RETURN->v_int = deleted_pair ? 1 : 0;
}

CK_DLL_MFUN(hm_del_str)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_String *key = GET_NEXT_STRING(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_STR;
    pair.key.as.str_OWNED = API->object->str(key);

    HM_Pair *deleted_pair = (HM_Pair *)hashmap_delete(hm, &pair);
    HM_Pair::releaseRef(deleted_pair);
    RETURN->v_int = deleted_pair ? 1 : 0;
}

CK_DLL_MFUN(hm_clear)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    hashmap_clear(hm, true);
}

CK_DLL_MFUN(hm_type_int)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);

    HM_Pair pair = {};
    pair.key.type = HM_INT;
    pair.key.as.ckint = GET_NEXT_INT(ARGS);

    // lookup
    HM_Pair *result = (HM_Pair *)hashmap_get(hm, &pair);

    RETURN->v_int = result ? result->value.type : 0;
}

CK_DLL_MFUN(hm_type_obj)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);

    HM_Pair pair = {};
    pair.key.type = HM_OBJ;
    pair.key.as.ckobj = GET_NEXT_OBJECT(ARGS);

    // lookup
    HM_Pair *result = (HM_Pair *)hashmap_get(hm, &pair);

    RETURN->v_int = result ? result->value.type : 0;
}

CK_DLL_MFUN(hm_type_str)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);

    HM_Pair pair = {};
    pair.key.type = HM_STR;
    pair.key.as.str_OWNED = API->object->str(GET_NEXT_STRING(ARGS));

    // lookup
    HM_Pair *result = (HM_Pair *)hashmap_get(hm, &pair);

    RETURN->v_int = result ? result->value.type : 0;
}

CK_DLL_MFUN(hm_keys_int)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_ArrayInt *ck_arr = (Chuck_ArrayInt *)g_api->object->create(SHRED, g_int_array_type, false);

    size_t hashmap_idx_DONT_USE = 0;
    HM_Pair *pair = NULL;
    while (
        hashmap_iter(hm, &hashmap_idx_DONT_USE, (void **)&pair))
    {
        if (pair->key.type == HM_INT)
            g_api->object->array_int_push_back(ck_arr, pair->key.as.ckint);
    }

    RETURN->v_object = (Chuck_Object *)ck_arr;
}

CK_DLL_MFUN(hm_keys_obj)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_ArrayInt *ck_arr = (Chuck_ArrayInt *)g_api->object->create(SHRED, g_object_array_type, false);

    size_t hashmap_idx_DONT_USE = 0;
    HM_Pair *pair = NULL;
    while (
        hashmap_iter(hm, &hashmap_idx_DONT_USE, (void **)&pair))
    {
        if (pair->key.type == HM_OBJ)
            g_api->object->array_int_push_back(ck_arr, (t_CKINT)pair->key.as.ckobj);
    }

    RETURN->v_object = (Chuck_Object *)ck_arr;
}

CK_DLL_MFUN(hm_keys_str)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_ArrayInt *ck_arr = (Chuck_ArrayInt *)g_api->object->create(SHRED, g_string_array_type, false);

    size_t hashmap_idx_DONT_USE = 0;
    HM_Pair *pair = NULL;
    while (
        hashmap_iter(hm, &hashmap_idx_DONT_USE, (void **)&pair))
    {
        if (pair->key.type == HM_STR)
            g_api->object->array_int_push_back(ck_arr, (t_CKINT)g_api->object->create_string(g_vm, pair->key.as.str_OWNED, false));
    }

    RETURN->v_object = (Chuck_Object *)ck_arr;
}

void print(jsmntok_t *token)
{
    printf("type: %d, start: %d, end: %d, size: %d\n",
           token->type,
           token->start,
           token->end,
           token->size);
}

char *copyJsonString(const char *json_string, jsmntok_t *token)
{
    ASSERT(token->type == JSMN_STRING);

    // make string copy of key
    int key_strlen = token->end - token->start;
    char *key_copy = (char *)malloc(key_strlen + 1);
    key_copy[key_strlen] = '\0';
    strncpy(key_copy, json_string + token->start, key_strlen);
    return key_copy;
}

// forward decl
HM_Entry createHMValue(
    const char *json_string, jsmntok_t *token_list, int *curr_token_idx);

Chuck_Object *HM_FromJson(
    const char *json_string,
    jsmntok_t *token_list,
    int *curr_token_idx)
{
    Chuck_Object *hm_ckobj = HM_Init(NULL);

    jsmntok_t *token = &token_list[(*curr_token_idx)];
    jsmntype_t type = token->type;

    ASSERT(type == JSMN_ARRAY || type == JSMN_OBJECT);

    int expected_size = token->size;

    for (int json_elem_idx = 0; json_elem_idx < expected_size; json_elem_idx++)
    {
        HM_Pair pair = {};
        if (type == JSMN_OBJECT)
        {
            jsmntok_t *key = &token_list[++(*curr_token_idx)];
            ASSERT(key->type == JSMN_STRING && key->size == 1);

            // register as value in curr hashmap
            pair.key.type = HM_STR;
            pair.key.as.str_OWNED = copyJsonString(json_string, key);

            ++(*curr_token_idx);
            pair.value = createHMValue(
                json_string,
                token_list,
                curr_token_idx);

            // jsmntype_t value_type = token_list[*curr_token_idx].type;
            // if (value_type == JSMN_STRING || value_type == JSMN_PRIMITIVE)
            // {
            //     (*curr_token_idx)++;
            // }
        }
        else if (type == JSMN_ARRAY)
        {
            // for arrays, key is always the integer index
            pair.key.type = HM_INT;
            pair.key.as.ckint = json_elem_idx;

            ++(*curr_token_idx);
            pair.value = createHMValue(
                json_string,
                token_list,
                curr_token_idx);

            // jsmntype_t value_type = token_list[*curr_token_idx].type;
            // if (value_type == JSMN_STRING || value_type == JSMN_PRIMITIVE)
            // {
            //     (*curr_token_idx)++;
            // }
        }
        else
        {
            ASSERT(false);
        }

        // reference count and add to hashmap
        CK_DL_API API = g_api;
        hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(hm_ckobj, hashmap_ptr_offset);
        HM_Pair::addRef(&pair);
        HM_Pair::releaseRef((HM_Pair *)hashmap_set(hm, &pair));
    }

    return hm_ckobj;
}

HM_Entry createHMValue(
    const char *json_string, jsmntok_t *token_list,
    int *curr_token_idx)
{
    jsmntok_t *token = &token_list[*curr_token_idx];

    HM_Entry value = {};
    switch (token->type)
    {
    case JSMN_OBJECT:
    case JSMN_ARRAY:
    {
        Chuck_Object *nested_hm = HM_FromJson(
            json_string,
            token_list,
            curr_token_idx);

        value.type = HM_OBJ;
        value.as.ckobj = nested_hm;
    }
    break;
    case JSMN_STRING:
    {
        value.type = HM_STR;
        value.as.str_OWNED = copyJsonString(json_string, token);
    }
    break;
    case JSMN_PRIMITIVE:
    {
        char prim = json_string[token->start];

        if (prim == 't' || prim == 'f')
        { // boolean
            value.type = HM_INT;
            value.as.ckint = (prim == 't') ? 1 : 0;
        }
        else if (prim == 'n')
        { // null
            value.type = HM_OBJ;
            value.as.ckobj = NULL;
        }
        else if (prim == '-' || (prim >= '0' && prim <= '9'))
        { // number
            // check for decimal
            bool is_float = false;
            for (int i = token->start; i < token->end; i++)
            {
                if (json_string[i] == '.')
                {
                    is_float = true;
                    break;
                }
            }
            if (is_float)
            {
                value.type = HM_FLOAT;
                value.as.ckfloat = atof(&json_string[token->start]);
            }
            else
            {
                value.type = HM_INT;
                value.as.ckint = atoi(&json_string[token->start]);
            }
        }
        else
        {
            ASSERT(false)
        }
    }
    break;
    default:
    {
        ASSERT(false);
    }
    }
    return value;
}

// (void) name( Chuck_Type * TYPE, void * ARGS, Chuck_DL_Return * RETURN, Chuck_VM * VM, Chuck_VM_Shred * SHRED, CK_DL_API API )
CK_DLL_SFUN(hm_from_json)
{
    Chuck_String *ckstr = GET_NEXT_STRING(ARGS);
    const char *json_str = API->object->str(ckstr);

    RETURN->v_object = NULL;

    int curr_token_idx = 0;

    // determine number of tokens needed
    jsmn_parser parser;
    jsmn_init(&parser);
    int num_tokens = jsmn_parse(&parser, json_str, strlen(json_str), NULL, 0);
    jsmn_init(&parser);
    jsmntok_t *tokens = (jsmntok_t *)malloc(sizeof(jsmntok_t) * num_tokens);
    int parse_result = jsmn_parse(&parser, json_str, strlen(json_str), tokens, num_tokens);

    // error handling
    if (parse_result < 0)
    {
        switch (parse_result)
        {
        case JSMN_ERROR_NOMEM:
        {
            g_api->vm->em_log(1, "HashMap.fromJson: Out of memory, json too large");
        }
        break;
        case JSMN_ERROR_INVAL:
        case JSMN_ERROR_PART:
        {
            g_api->vm->em_log(1, "HashMap.fromJson: Invalid JSON");
        }
        break;
        }
        return;
    }
    else
    {
        ASSERT(parse_result == num_tokens);
    }

    // else parsed succesfully
    RETURN->v_object = HM_FromJson(json_str, tokens, &curr_token_idx);

    // free
    free(tokens);
}

/*
Exceptions
- if there are *only* int keys, treat as JSON array
- if there are *any* string keys, ignore all int keys and treat as JSON object
- ignore all object keys
- if value is a HashMap, we will parse that too.
- other object values are converted to their classname string
*/

struct DynamicString
{
    char *buff;
    int cap;
    int curr;

    void init(int size)
    {
        this->buff = (char *)malloc(size);
        this->cap = size;
    }

    void release()
    {
        free(this->buff);
    }

    void append(const char *s)
    {
        int len = strlen(s);

        if (len + this->curr > cap)
        {
            int newsize = MAX(len + this->curr + 1, cap * 2);
            this->buff = (char *)realloc(this->buff, newsize);
            this->cap = newsize;
        }

        strncpy(this->buff + this->curr, s, len + 1);
        this->curr += len;
    }
};

char *itoa(int i)
{
    static char str_buff[32];
    memset(str_buff, 0, sizeof(str_buff));
    snprintf(str_buff, sizeof(str_buff), "%d", i);
    return str_buff;
}

char *ftoa(double d)
{
    static char str_buff[32];
    memset(str_buff, 0, sizeof(str_buff));
    snprintf(str_buff, sizeof(str_buff), "%lf", d);
    return str_buff;
}

void HM_ToJson(Chuck_Object *hm_ckobj, DynamicString *d);

void valueToJson(DynamicString *d, HM_Entry value)
{
    // append value
    switch (value.type)
    {
    case HM_INT:
    {
        d->append(itoa(value.as.ckint));
    }
    break;
    case HM_FLOAT:
    {
        // // Extract integer part
        // int ipart = (int)value.as.ckint;
        // // Extract floating part
        // float fpart = value.as.ckint - (float)ipart;
        d->append(ftoa(value.as.ckfloat));
    }
    break;
    case HM_OBJ:
    {
        if (value.as.ckobj == NULL)
        {
            d->append("null");
        }
        else
        {
            Chuck_Type *obj_type = g_api->object->get_type(value.as.ckobj);
            bool is_hashmap = g_api->type->isa(obj_type, g_hm_type);
            if (is_hashmap)
            {
                HM_ToJson(value.as.ckobj, d);
            }
            else
            {
                d->append("\"");
                d->append(g_api->type->name(obj_type));
                d->append("\"");
            }
        }
    }
    break;
    case HM_STR:
    {
        d->append("\"");
        d->append(value.as.str_OWNED);
        d->append("\"");
    }
    break;
    default:
        ASSERT(false);
    }
}

void HM_ToJson(Chuck_Object *hm_ckobj, DynamicString *d)
{
    CK_DL_API API = g_api;
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(hm_ckobj, hashmap_ptr_offset);

    size_t hashmap_idx_DONT_USE = 0;
    HM_Pair *pair = NULL;

    std::vector<int> int_keys;
    int num_string_keys = 0;

    while (
        hashmap_iter(hm, &hashmap_idx_DONT_USE, (void **)&pair))
    {
        // if (pair->key.type == HM_OBJ)
        //     g_api->object->array_int_push_back(ck_arr, (t_CKINT)pair->key.as.ckobj);
        switch (pair->key.type)
        {
        case HM_INT:
        {
            int_keys.push_back(pair->key.as.ckint);
        }
        break;
        case HM_OBJ:
        {
            g_api->vm->em_log(1, "HashMap.toJson(): object key will not be converted to json");
        }
        break;
        case HM_STR:
        {
            ++num_string_keys;
        }
        break;
        case HM_FLOAT: // not allowing float keys for now (due to precision issues)
        default:
        {
            g_api->vm->em_log(1, "Hashmap: unsupported key type");
        }
        }
    }

    // determine # contiguous int keys
    bool is_empty_obj = (int_keys.size() == 0 && num_string_keys == 0);
    bool is_json_obj = (num_string_keys > 0);
    bool is_json_arr = false;
    int contiguous_int_keys = 0;
    if (!is_json_obj && !is_empty_obj)
    {
        std::sort(int_keys.begin(), int_keys.end());
        for (int i = 0; i < int_keys.size(); i++)
        {
            if (i == int_keys[i])
                contiguous_int_keys++;
            else
                break;
        }
    }
    is_json_arr = (contiguous_int_keys > 0 && num_string_keys == 0);

    ASSERT(!(is_empty_obj && is_json_arr && is_json_obj));

    if (is_empty_obj)
    {
        d->append("{}");
    }

    if (is_json_arr)
    {
        // process JSON array
        d->append("[");
        for (int i = 0; i < contiguous_int_keys; i++)
        {
            // get
            HM_Pair lookup = {};
            lookup.key.type = HM_INT;
            lookup.key.as.ckint = i;

            HM_Pair *result = (HM_Pair *)hashmap_get(hm, &lookup);
            valueToJson(d, result->value);

            if (i < contiguous_int_keys - 1)
                d->append(", ");
        }
        d->append("]");
    }

    if (is_json_obj)
    {
        d->append("{");
        // process JSON obj
        hashmap_idx_DONT_USE = 0;
        pair = NULL;
        int curr_str_key = 0;
        while (hashmap_iter(hm, &hashmap_idx_DONT_USE, (void **)&pair))
        {
            if (pair->key.type != HM_STR)
                continue;

            // for all string keys
            d->append("\"");
            d->append(pair->key.as.str_OWNED);
            d->append("\": ");

            valueToJson(d, pair->value);

            // logic for comma
            if (++curr_str_key < num_string_keys)
            {
                d->append(", ");
            }
        }

        d->append("}");
    }
}

CK_DLL_MFUN(hm_to_json)
{
    DynamicString d = {};
    d.init(256);
    HM_ToJson(SELF, &d);
    RETURN->v_string = API->object->create_string(VM, d.buff, false);
    d.release();
}

CK_DLL_QUERY(Hashmap)
{
    // set globals
    g_api = QUERY->ck_api(QUERY);
    g_vm = QUERY->ck_vm(QUERY);

    QUERY->setname(QUERY, "HashMapChugin");

    // Hash Type Enum ----------------------------------------------
    static t_CKINT HASH_TYPE_NULL = HM_NULL;
    static t_CKINT HASH_TYPE_INT = HM_INT;
    static t_CKINT HASH_TYPE_FLOAT = HM_FLOAT;
    static t_CKINT HASH_TYPE_OBJ = HM_OBJ;
    static t_CKINT HASH_TYPE_STR = HM_STR;

    { // HashMap ----------------------------------------------
        QUERY->begin_class(QUERY, "HashMap", "Object");
        hashmap_ptr_offset = QUERY->add_mvar(QUERY, "int", "@hashmap_ptr", false);

        QUERY->add_svar(QUERY, "int", "Type_None", true, &HASH_TYPE_NULL);
        QUERY->add_svar(QUERY, "int", "Type_Int", true, &HASH_TYPE_INT);
        QUERY->add_svar(QUERY, "int", "Type_Float", true, &HASH_TYPE_FLOAT);
        QUERY->add_svar(QUERY, "int", "Type_Obj", true, &HASH_TYPE_OBJ);
        QUERY->add_svar(QUERY, "int", "Type_Str", true, &HASH_TYPE_STR);

        QUERY->add_ctor(QUERY, hm_ctor);
        QUERY->add_dtor(QUERY, hm_dtor);

        QUERY->add_mfun(QUERY, hm_count, "int", "size");
        QUERY->doc_func(QUERY, "Get the number of elements in the hashmap");

        // ----- setters ----------------------------------------
        // for now setters return void
        // alternative is to allocate new hashitem and return previously stored value
        QUERY->add_mfun(QUERY, hm_set_int_int, "void", "set");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->add_arg(QUERY, "int", "value");
        QUERY->doc_func(QUERY, "Set an int key to an int value");

        QUERY->add_mfun(QUERY, hm_set_int_float, "void", "set");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->add_arg(QUERY, "float", "value");
        QUERY->doc_func(QUERY, "Set an int key to a float value");

        QUERY->add_mfun(QUERY, hm_set_int_obj, "void", "set");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->add_arg(QUERY, "Object", "value");
        QUERY->doc_func(QUERY, "Set an int key to an Object value");

        QUERY->add_mfun(QUERY, hm_set_int_str, "void", "set");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->add_arg(QUERY, "string", "value");
        QUERY->doc_func(QUERY, "Set an int key to a string value");

        QUERY->add_mfun(QUERY, hm_set_obj_int, "void", "set");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->add_arg(QUERY, "int", "value");
        QUERY->doc_func(QUERY, "Set an Object key to an int value");

        QUERY->add_mfun(QUERY, hm_set_obj_float, "void", "set");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->add_arg(QUERY, "float", "value");
        QUERY->doc_func(QUERY, "Set an Object key to a float value");

        QUERY->add_mfun(QUERY, hm_set_obj_obj, "void", "set");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->add_arg(QUERY, "Object", "value");
        QUERY->doc_func(QUERY, "Set an Object key to an Object value");

        QUERY->add_mfun(QUERY, hm_set_obj_str, "void", "set");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->add_arg(QUERY, "string", "value");
        QUERY->doc_func(QUERY, "Set an Object key to a string value");

        QUERY->add_mfun(QUERY, hm_set_str_int, "void", "set");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->add_arg(QUERY, "int", "value");
        QUERY->doc_func(QUERY, "Set a string key to an int value");

        QUERY->add_mfun(QUERY, hm_set_str_float, "void", "set");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->add_arg(QUERY, "float", "value");
        QUERY->doc_func(QUERY, "Set a string key to a float value");

        QUERY->add_mfun(QUERY, hm_set_str_obj, "void", "set");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->add_arg(QUERY, "Object", "value");
        QUERY->doc_func(QUERY, "Set a string key to an Object value");

        QUERY->add_mfun(QUERY, hm_set_str_str, "void", "set");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->add_arg(QUERY, "string", "value");
        QUERY->doc_func(QUERY, "Set a string key to a string value");

        // ----- getters ----------------------------------------
        QUERY->add_mfun(QUERY, hm_get_int_int, "int", "getInt");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Get an int value from an int key (default 0)");

        QUERY->add_mfun(QUERY, hm_get_int_float, "float", "getFloat");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Get a float value from an int key (default 0.0)");

        QUERY->add_mfun(QUERY, hm_get_int_obj, "Object", "getObj");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Get an Object value from an int key (default NULL)");

        QUERY->add_mfun(QUERY, hm_get_int_str, "string", "getStr");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Get a string value from an int key (default \"\")");

        QUERY->add_mfun(QUERY, hm_get_obj_int, "int", "getInt");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Get an int value from an Object key (default 0)");

        QUERY->add_mfun(QUERY, hm_get_obj_float, "float", "getFloat");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Get a float value from an Object key (default 0.0)");

        QUERY->add_mfun(QUERY, hm_get_obj_obj, "Object", "getObj");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Get an Object value from an Object key (default NULL)");

        QUERY->add_mfun(QUERY, hm_get_obj_str, "string", "getStr");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Get a string value from an Object key (default \"\")");

        QUERY->add_mfun(QUERY, hm_get_str_int, "int", "getInt");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Get an int value from a string key (default 0)");

        QUERY->add_mfun(QUERY, hm_get_str_float, "float", "getFloat");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Get a float value from a string key (default 0.0)");

        QUERY->add_mfun(QUERY, hm_get_str_obj, "Object", "getObj");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Get an Object value from a string key (default NULL)");

        QUERY->add_mfun(QUERY, hm_get_str_str, "string", "getStr");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Get a string value from a string key (default \"\")");

        QUERY->add_mfun(QUERY, hm_get_str_hashmap, "HashMap", "get");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Get a HashMap from a string key. Will warn if the object type is not HashMap");

        // ----- find ----------------------------------------
        QUERY->add_mfun(QUERY, hm_has_int, "int", "has");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Check if an int key exists in the hashmap.");

        QUERY->add_mfun(QUERY, hm_has_obj, "int", "has");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Check if an Object key exists in the hashmap.");

        QUERY->add_mfun(QUERY, hm_has_str, "int", "has");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Check if a string key exists in the hashmap.");

        // ----- delete ----------------------------------------
        QUERY->add_mfun(QUERY, hm_del_int, "int", "del");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Delete an int key from the hashmap. Returns 1 if key was found, 0 otherwise.");

        QUERY->add_mfun(QUERY, hm_del_obj, "int", "del");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Delete an Object key from the hashmap. Returns 1 if key was found, 0 otherwise.");

        QUERY->add_mfun(QUERY, hm_del_str, "int", "del");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Delete a string key from the hashmap. Returns 1 if key was found, 0 otherwise.");

        // ----- type ----------------------------------------
        QUERY->add_mfun(QUERY, hm_type_int, "int", "type");
        QUERY->add_arg(QUERY, "int", "key");
        QUERY->doc_func(QUERY, "Get the type of the value that the given key maps to. Will return one of the HashMap.Type_XXX values. Returns 0 if key not found");

        QUERY->add_mfun(QUERY, hm_type_obj, "int", "type");
        QUERY->add_arg(QUERY, "Object", "key");
        QUERY->doc_func(QUERY, "Get the type of the value that the given key maps to. Will return one of the HashMap.Type_XXX values. Returns 0 if key not found");

        QUERY->add_mfun(QUERY, hm_type_str, "int", "type");
        QUERY->add_arg(QUERY, "string", "key");
        QUERY->doc_func(QUERY, "Get the type of the value that the given key maps to. Will return one of the HashMap.Type_XXX values. Returns 0 if key not found");

        // ----- keys -----------------------------------------
        QUERY->add_mfun(QUERY, hm_keys_int, "int[]", "intKeys");
        QUERY->doc_func(QUERY, "Get the integer keys of this map");

        QUERY->add_mfun(QUERY, hm_keys_str, "string[]", "strKeys");
        QUERY->doc_func(QUERY, "Get the string keys of this map");

        QUERY->add_mfun(QUERY, hm_keys_obj, "Object[]", "objKeys");
        QUERY->doc_func(QUERY, "Get the Object keys of this map");

        // ----- json -----------------------------------------
        QUERY->add_sfun(QUERY, hm_from_json, "HashMap", "fromJson");
        QUERY->add_arg(QUERY, "string", "json");
        QUERY->doc_func(QUERY, "convert the given JSON string to a HashMap");

        QUERY->add_mfun(QUERY, hm_to_json, "string", "toJson");
        QUERY->doc_func(QUERY, "print out a JSON string representation of this HashMap");

        // ----- clear ----------------------------------------
        QUERY->add_mfun(QUERY, hm_clear, "void", "clear");
        QUERY->doc_func(QUERY, "Clear all elements from the hashmap.");

        QUERY->end_class(QUERY);
    } // HashMap

    // set type globals
    g_hm_type = g_api->type->lookup(g_vm, "HashMap");
    g_int_array_type = g_api->type->lookup(g_vm, "int[]");
    g_object_array_type = g_api->type->lookup(g_vm, "Object[]");
    g_string_array_type = g_api->type->lookup(g_vm, "string[]");

    return TRUE;
}