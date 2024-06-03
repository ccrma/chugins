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

#include "chugin.h"
#include "hashmap.h"

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
static Chuck_String *g_empty_string = NULL;

#define HM_ADD_REF(obj) (g_api->object->add_ref(obj))
#define HM_RELEASE_REF(obj) (g_api->object->release(obj))

enum HM_Type : uint8_t
{
    HM_NULL = 0,
    HM_INT,
    HM_FLOAT,
    HM_OBJ,
    HM_STR,
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
        Chuck_String *ckstr;
    } as;
    HM_Type type;
};

// key-value pair that is stored in the hashmap
struct HM_Pair
{
    HM_Entry key;
    HM_Entry value;
};

static int HM_Compare(const void *a, const void *b, void *udata)
{
    HM_Pair *ua = (HM_Pair *)a;
    HM_Pair *ub = (HM_Pair *)b;

    switch (ua->key.type)
    {
    case HM_INT:
        return ua->key.as.ckint - ub->key.as.ckint;
    case HM_OBJ:
        return (uint8_t *)ua->key.as.ckobj - (uint8_t *)ub->key.as.ckobj;
    case HM_STR:
        return strcmp(g_api->object->str(ua->key.as.ckstr), g_api->object->str(ub->key.as.ckstr));
        break;
    case HM_FLOAT: // not allowing float keys for now (due to precision issues)
    default:
        g_api->vm->em_log(1, "Hashmap: unsupported key type");
        return 0;
    }
}

static uint64_t HM_hash(const void *item, uint64_t seed0, uint64_t seed1)
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
        const char *c_str = g_api->object->str(pair->key.as.ckstr);
        return hashmap_xxhash3(c_str, strlen(c_str), seed0, seed1);
    }
    default:
        g_api->vm->em_log(1, "Hashmap: unsupported key type");
        return 0;
    }
    // return hashmap_sip(item, sizeof(int), seed0, seed1);
    // return hashmap_murmur(item, sizeof(int), seed0, seed1);
}

static void HM_ReleaseEntry(HM_Entry *entry)
{
    if (entry == NULL)
        return;

    switch (entry->type)
    {
    case HM_OBJ:
        HM_RELEASE_REF(entry->as.ckobj);
        return;
    case HM_STR:
        HM_RELEASE_REF((Chuck_Object *)entry->as.ckstr);
        return;
    default:
        return;
    }
}

// called when deleting element from hashmap
static void HM_ReleasePair(void *item)
{
    if (item == NULL)
        return;

    HM_Pair *pair = (HM_Pair *)item;

    // release chuck objects
    HM_ReleaseEntry(&pair->key);
    HM_ReleaseEntry(&pair->value);
}

static void HM_AddRefEntry(HM_Entry *entry)
{
    if (entry == NULL)
        return;

    switch (entry->type)
    {
    case HM_OBJ:
        HM_ADD_REF(entry->as.ckobj);
        return;
    case HM_STR:
        HM_ADD_REF((Chuck_Object *)entry->as.ckstr);
        return;
    default:
        return;
    }
}

static void HM_AddRefPair(HM_Pair *pair)
{
    if (pair == NULL)
        return;

    HM_AddRefEntry(&pair->key);
    HM_AddRefEntry(&pair->value);
}

t_CKINT hashitem_ptr_offset = 0;
t_CKINT hashpair_ptr_offset = 0;
t_CKINT hashmap_ptr_offset = 0;

// ==================================================================
// hashitem impl
// ==================================================================

CK_DLL_CTOR(hashitem_ctor)
{
}

CK_DLL_DTOR(hashitem_dtor)
{
}

CK_DLL_MFUN(hashitem_get_type)
{
}

CK_DLL_MFUN(hashitem_as_int)
{
}

CK_DLL_MFUN(hashitem_as_float)
{
}

CK_DLL_MFUN(hashitem_as_obj)
{
}

CK_DLL_MFUN(hashitem_as_str)
{
}

// ==================================================================
// hashpair impl
// ==================================================================

// ==================================================================
// hashmap impl
// ==================================================================

// basic
CK_DLL_MFUN(hm_set);
CK_DLL_MFUN(hm_get);
CK_DLL_MFUN(hm_delete);
CK_DLL_MFUN(hm_clear);

// iteration
CK_DLL_MFUN(hm_iter);

CK_DLL_CTOR(hm_ctor)
{
    int seed = time(NULL);
    hashmap *map = hashmap_new(sizeof(HM_Pair), 0, seed, seed,
                               HM_hash, HM_Compare, HM_ReleasePair, NULL);
    OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset) = (t_CKUINT)map;
}

CK_DLL_DTOR(hm_dtor)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    hashmap_free(hm);
    OBJ_MEMBER_INT(SELF, hashmap_ptr_offset) = 0;
}

CK_DLL_MFUN(hm_count)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    RETURN->v_int = hashmap_count(hm);
}

static void HM_SetImpl(hashmap *hm, HM_Type key_type, HM_Type value_type, void *ARGS)
{
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
        pair.key.as.ckstr = GET_NEXT_STRING(ARGS);
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
        pair.value.as.ckstr = GET_NEXT_STRING(ARGS);
        break;
    default:
        break;
    }

    HM_AddRefPair(&pair);
    HM_ReleasePair((HM_Pair *)hashmap_set(hm, &pair));
}

CK_DLL_MFUN(hm_set_int_int)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_INT, ARGS);
}

CK_DLL_MFUN(hm_set_int_float)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_FLOAT, ARGS);
}

CK_DLL_MFUN(hm_set_int_obj)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_OBJ, ARGS);
}

CK_DLL_MFUN(hm_set_int_str)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_STR, ARGS);
}

CK_DLL_MFUN(hm_set_obj_int)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_INT, ARGS);
}

CK_DLL_MFUN(hm_set_obj_float)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_FLOAT, ARGS);
}

CK_DLL_MFUN(hm_set_obj_obj)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_OBJ, ARGS);
}

CK_DLL_MFUN(hm_set_obj_str)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_STR, ARGS);
}

CK_DLL_MFUN(hm_set_str_int)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_INT, ARGS);
}

CK_DLL_MFUN(hm_set_str_float)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_FLOAT, ARGS);
}

CK_DLL_MFUN(hm_set_str_obj)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_OBJ, ARGS);
}

CK_DLL_MFUN(hm_set_str_str)
{
    HM_SetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_STR, ARGS);
}

static void HM_GetImpl(hashmap *hm, HM_Type key_type, HM_Type value_type, void *ARGS, Chuck_VM_Shred *SHRED, Chuck_DL_Return *RETURN)
{
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
        pair.key.as.ckstr = GET_NEXT_STRING(ARGS);
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
            RETURN->v_string = g_empty_string;
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
        RETURN->v_string = result->value.as.ckstr;
        break;
    default:
        break;
    }
}

CK_DLL_MFUN(hm_get_int_int)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_INT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_int_float)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_FLOAT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_int_obj)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_OBJ, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_int_str)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_INT, HM_STR, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_int)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_INT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_float)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_FLOAT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_obj)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_OBJ, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_obj_str)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_OBJ, HM_STR, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_int)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_INT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_float)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_FLOAT, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_obj)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_OBJ, ARGS, SHRED, RETURN);
}

CK_DLL_MFUN(hm_get_str_str)
{
    HM_GetImpl((hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset),
               HM_STR, HM_STR, ARGS, SHRED, RETURN);
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
    pair.key.as.ckstr = key;

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
    HM_ReleasePair(deleted_pair);
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
    HM_ReleasePair(deleted_pair);
    RETURN->v_int = deleted_pair ? 1 : 0;
}

CK_DLL_MFUN(hm_del_str)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    Chuck_String *key = GET_NEXT_STRING(ARGS);

    HM_Pair pair = {};
    pair.key.type = HM_STR;
    pair.key.as.ckstr = key;

    HM_Pair *deleted_pair = (HM_Pair *)hashmap_delete(hm, &pair);
    HM_ReleasePair(deleted_pair);
    RETURN->v_int = deleted_pair ? 1 : 0;
}

CK_DLL_MFUN(hm_clear)
{
    hashmap *hm = (hashmap *)OBJ_MEMBER_UINT(SELF, hashmap_ptr_offset);
    hashmap_clear(hm, true);
}

CK_DLL_QUERY(Hashmap)
{
    // set globals
    g_api = QUERY->ck_api(QUERY);
    g_vm = QUERY->ck_vm(QUERY);
    g_empty_string = g_api->object->create_string(g_vm, "", true);

    QUERY->setname(QUERY, "HashMapChugin");

    // HashItem ----------------------------------------------
    static t_CKINT HASH_ITEM_NULL = HM_NULL;
    static t_CKINT HASH_ITEM_INT = HM_INT;
    static t_CKINT HASH_ITEM_FLOAT = HM_FLOAT;
    static t_CKINT HASH_ITEM_OBJ = HM_OBJ;
    static t_CKINT HASH_ITEM_STR = HM_STR;

    // immutable generic return type
    // QUERY->begin_class(QUERY, "HashItem", "Object");
    // hashitem_ptr_offset = QUERY->add_mvar(QUERY, int", "@hashitem_ptr", false);

    // QUERY->add_svar(QUERY, "int", "HM_NULL", true, &HASH_ITEM_NULL);
    // QUERY->add_svar(QUERY, "int", "HM_INT", true, &HASH_ITEM_INT);
    // QUERY->add_svar(QUERY, "int", "HM_FLOAT", true, &HASH_ITEM_FLOAT);
    // QUERY->add_svar(QUERY, "int", "HM_OBJ", true, &HASH_ITEM_OBJ);
    // QUERY->add_svar(QUERY, "int", "HM_STR", true, &HASH_ITEM_STR);

    // QUERY->add_ctor(QUERY, hashitem_ctor);
    // QUERY->add_dtor(QUERY, hashitem_dtor);

    // QUERY->add_mfun(QUERY, hashitem_get_type, "int", "type");
    // QUERY->doc_func(QUERY, "Get the type of the hashitem. Returns one of HashItem.NULL, HashItem.INT, HashItem.FLOAT, HashItem.OBJ, HashItem.STR");

    // QUERY->add_mfun(QUERY, hashitem_as_int, "int", "asInt");
    // QUERY->doc_func(QUERY, "Get the value of the HashItem as an integer");

    // QUERY->add_mfun(QUERY, hashitem_as_float, "float", "asFloat");
    // QUERY->doc_func(QUERY, "Get the value of the HashItem as a float");

    // QUERY->add_mfun(QUERY, hashitem_as_obj, "Object", "asObj");
    // QUERY->doc_func(QUERY, "Get the value of the HashItem as an Object");

    // QUERY->add_mfun(QUERY, hashitem_as_str, "string", "asStr");
    // QUERY->doc_func(QUERY, "Get the value of the HashItem as a string");

    // QUERY->end_class(QUERY);

    // HashPair ----------------------------------------------
    // QUERY->begin_class(QUERY, "HashPair", "Object");
    // hashpair_ptr_offset = QUERY->add_mvar(QUERY, "int", "@hashpair_ptr", false);

    // TODO: just contains 2 hashitems

    // QUERY->end_class(QUERY);

    // HashMap ----------------------------------------------

    QUERY->begin_class(QUERY, "HashMap", "Object");
    hashmap_ptr_offset = QUERY->add_mvar(QUERY, "int", "@hashmap_ptr", false);

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

    // ----- clear ----------------------------------------
    QUERY->add_mfun(QUERY, hm_clear, "void", "clear");
    QUERY->doc_func(QUERY, "Clear all elements from the hashmap.");

    QUERY->end_class(QUERY);

    return TRUE;
}