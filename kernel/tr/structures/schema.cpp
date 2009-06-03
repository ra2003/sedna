/*
 * File:  schema.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "common/sedna.h"

#include "tr/structures/schema.h"
#include "tr/structures/nodes.h"
#include "tr/executor/base/XPathOnSchema.h"
#include "tr/idx/btree/btstruct.h"
#include "tr/idx/btree/btree.h"
#include "tr/idx/index_data.h"
#include "tr/vmm/vmm.h"
#include "tr/crmutils/node_utils.h"
#ifdef SE_ENABLE_FTSEARCH
#include "tr/ft/ft_index_data.h"
#endif
#ifdef SE_ENABLE_TRIGGERS
#include "tr/triggers/triggers_data.h"
#endif
#include "tr/cat/catstore.h"
#include "tr/locks/locks.h"

#define CAT_FOR_EACH(T, i, list) for (cat_list<T>::item * i = list.first; i != NULL; i = i->next) 

/*
xptr xmlns_hint = XNULL;

char * xmlns_alloc_concat(const char * prefix, const char * uri) 
{
    size_t n1, n2, nn;
    char * fullid;

    n1 = (prefix == NULL) ? 0 : strlen(prefix);
    n2 = (uri == NULL) ? 0 : strlen(uri);
    nn = n1 + n2 + 2;
    fullid = (char *) cat_malloc_context(CATALOG_COMMON_CONTEXT, nn);
    if (n1 > 0) memcpy(fullid, prefix, n1);
    fullid[n1] = '=';
    if (n2 > 0) memcpy(fullid + n1 + 1, uri, n2);
    fullid[n1+n2+1] = '\0';

    return fullid;
}
*/


#define XMLNS_HASH_SIZE 32

struct xmlns_hash_object {
    struct xmlns_local_object object;
    struct xmlns_hash_object *next;
};

struct xmlns_hash_object * xmlns_hash[XMLNS_HASH_SIZE];

inline int str_hash(const char * a) {
    if (a == NULL) return 0;
    unsigned int i = 0;
    while (*a != '\0') { i += (* (uint8_t *) a) * 7; a++; }
    return i % XMLNS_HASH_SIZE; 
};

inline int mystrcmp(const char * str1, const char * str2)
{
    if ((str1 == NULL) && (str2 == NULL)) return 0;
    if ((str1 == NULL) || (str2 == NULL)) return (((ptrdiff_t) str1) - ((ptrdiff_t) str2));

    return strcmp(str1, str2);
}

xmlns_ptr xmlns_touch(const char * prefix, const char * uri)
{
    if ((prefix == NULL) && (uri == NULL)) return NULL_XMLNS;

    int original_hash = str_hash(prefix);
    struct xmlns_hash_object * i = xmlns_hash[original_hash];

    while (i != NULL) {
        if ((mystrcmp(i->object.prefix, prefix) == 0) && (mystrcmp(i->object.uri, uri) == 0)) {
            return &(i->object);
        }
        i = i->next;
    }

    i = (struct xmlns_hash_object *) cat_malloc_context(CATALOG_COMMON_CONTEXT, sizeof(struct xmlns_hash_object));
    i->object.prefix = cat_strcpy(i, prefix);
    i->object.uri = cat_strcpy(i, uri);
    i->next = xmlns_hash[original_hash];
    xmlns_hash[original_hash] = i;

    return &(i->object);
}


void free_xmlns_hash()
{
    struct xmlns_hash_object *j, *i;

    for (int k = 0; k < XMLNS_HASH_SIZE; k++) {
      i = xmlns_hash[k];

      while (i != NULL) {
          j = i->next;
          cat_free(i->object.prefix);
          cat_free(i->object.uri);
          cat_free(i);
          i = j;
      }
      xmlns_hash[k] = NULL;
    }
}

/*
xmlns_ptr xmlns_touch(const char * prefix, const char * uri)
{
    xmlns_ptr res;

    if ((prefix == NULL) && (uri == NULL)) return XNULL;

    res = xml_ns_find(prefix, uri);

    if (res == XNULL) {
        res = (xml_ns_object::create(prefix, uri))->p;
        xmlns_hint = res;
    };

    return res;
};
*/

catalog_object_header * xmlns_indb_object::create(const char* prefix, const char* uri, const xptr root, const xmlns_ptr_pers next_xmlns)
{
    xmlns_indb_object * a = new(cat_malloc(CATALOG_PERSISTENT_CONTEXT, sizeof(xmlns_indb_object)))
      xmlns_indb_object(prefix, uri, root, next_xmlns);

    return catalog_create_object(a, (root != XNULL));
}


void xmlns_indb_object::serialize_data(se_simplestream &stream)
{
    cs_set_hint(root);

    stream.write(&next_xmlns, sizeof(xmlns_ptr_pers));
    stream.write_string(prefix);
    stream.write_string(uri);
}

void xmlns_indb_object::deserialize_data(se_simplestream &stream)
{
    root = p_object; /* actually there is no need in root now */

    stream.read(&next_xmlns, sizeof(xmlns_ptr_pers));
    prefix = (char *) cat_malloc(this, stream.read_string_len());
    stream.read_string(SSTREAM_SAVED_LENGTH, prefix);
    uri = (char *) cat_malloc(this, stream.read_string_len());
    stream.read_string(SSTREAM_SAVED_LENGTH, uri);
}

void xmlns_indb_object::drop()
{
    cs_free(p_object);
    catalog_delete_object(this);
}

/**************************************
 *  Common schema node
 */


void sc_node_ref_list::add_object_tail(sc_ref obj)
{
    sc_ref_item * item;

    item = (sc_ref_item *) cat_malloc(this, sizeof(sc_ref_item));
    item->object = obj;
    item->next = NULL;

    if (this->first == NULL) {
        item->prev = NULL;
        this->first = item;
        this->last = item;
    } else {
        item->prev = this->last;
        this->last->next = item;
        this->last = item;
    }
}

sc_ref_item * sc_node_ref_list::get(int n) const
{
    for (sc_ref_item * i = this->first; i != NULL; i = i->next) {
        if (n == 0) return i;
        n--;
    }
    return NULL;
}

int sc_node_ref_list::count() const
{
    int c = 0;
    for (sc_ref_item * i = this->first; i != NULL; i = i->next) { c++; }
    return c;
}

void sc_node_ref_list::serialize(se_simplestream &stream)
{
    int count = this->count();
    stream.write(&count, sizeof(int));

    for (sc_ref_item * i = this->first; i != NULL; i = i->next) {
        stream.write(&(i->object.snode), sizeof(schema_node_xptr));
        stream.write(&(i->object.type), sizeof(t_item));
        stream.write(&(i->object.xmlns_pers), sizeof(xmlns_ptr_pers));
        stream.write_string(i->object.name);
    }
}

void sc_node_ref_list::deserialize(se_simplestream &stream)
{
    int count;
    sc_ref a;
    stream.read(&count, sizeof(int));

    while (count > 0) {
        stream.read(&(a.snode), sizeof(schema_node_xptr));
        stream.read(&(a.type), sizeof(t_item));
        stream.read(&(a.xmlns_pers), sizeof(xmlns_ptr_pers));

        if (a.name != NULL) { cat_free(a.name); }
        a.name = (char *) cat_malloc(this, stream.read_string_len());

        stream.read_string(SSTREAM_SAVED_LENGTH, a.name);
        add_object_tail(a);
        count--;
    }
}

catalog_object_header * schema_node_object::create(doc_schema_node_xptr root, xmlns_ptr xmlns, const char* name, t_item type, bool persistent)
{
    schema_node_object * a = 
      new(cat_malloc(persistent ? CATALOG_PERSISTENT_CONTEXT : CATALOG_TEMPORARY_CONTEXT, sizeof(schema_node_object)))
      schema_node_object(root, xmlns, name, type, persistent);

    if (type == virtual_root) { a->root = a->p_object; };

    return catalog_create_object(a, persistent);
}


schema_node_object::schema_node_object(
    const doc_schema_node_xptr _root, 
    xmlns_ptr _xmlns, 
    const char * _name, 
    t_item _type, 
    bool _persistent
) :  
    xmlns_pers(XNULL),
    xmlns_local(_xmlns),
    persistent(_persistent),
    name(cat_strcpy(this, _name)),
    root(_root), 
    parent(XNULL), 
    type(_type),
    bblk(XNULL), bblk_indir(XNULL), 
    nodecnt(0), blockcnt(0), extnids(0), indir_blk_cnt(0), textcnt(0), 
    lastnode_ind(XNULL)
{
//    if ((this->get_xmlns() != XNULL) && (this->persistent)) xml_ns_inc_ref(this->get_xmlns());

    U_ASSERT((root != NULL) || (_xmlns == NULL));

    if (root != NULL) { xmlns_pers = root->xmlns_register(_xmlns); } 
};

schema_node_object::~schema_node_object() {
    cat_free(name);
//    if ((this->get_xmlns() != XNULL) && (this->persistent)) xml_ns_dec_ref(this->get_xmlns());
};


void schema_node_object::serialize_data(se_simplestream &stream)
{
    if (root != XNULL) { cs_set_hint(root); }

    stream.write(&persistent, sizeof(bool));
    stream.write_string(name);
    stream.write(&root, sizeof(doc_schema_node_xptr));
    stream.write(&parent, sizeof(schema_node_xptr));
    stream.write(&type, sizeof(t_item));
    stream.write(&xmlns_pers, sizeof(xmlns_ptr_pers));
    stream.write(&bblk, sizeof(xptr));
    stream.write(&bblk_indir, sizeof(xptr));

    children.serialize(stream);

    stream.write(&nodecnt, sizeof(unsigned int));
    stream.write(&blockcnt, sizeof(unsigned int));
    stream.write(&extnids, sizeof(unsigned int));
    stream.write(&indir_blk_cnt, sizeof(unsigned int));
    stream.write(&textcnt, sizeof(__int64));
    stream.write(&lastnode_ind, sizeof(xptr));

    index_list.serialize(stream);
#ifdef SE_ENABLE_FTSEARCH
    ft_index_list.serialize(stream);
#endif
#ifdef SE_ENABLE_TRIGGERS
    trigger_list.serialize(stream);
#endif
};

void schema_node_object::deserialize_data(se_simplestream &stream)
{
    stream.read(&persistent, sizeof(bool));
    name = (char *) cat_malloc(this, stream.read_string_len());
    stream.read_string(SSTREAM_SAVED_LENGTH, name);
    stream.read(&root, sizeof(doc_schema_node_xptr));
    stream.read(&parent, sizeof(schema_node_xptr));
    stream.read(&type, sizeof(t_item));
    stream.read(&xmlns_pers, sizeof(xmlns_ptr_pers));
    stream.read(&bblk, sizeof(xptr));
    stream.read(&bblk_indir, sizeof(xptr));

    children.deserialize(stream);

    stream.read(&nodecnt, sizeof(unsigned int));
    stream.read(&blockcnt, sizeof(unsigned int));
    stream.read(&extnids, sizeof(unsigned int));
    stream.read(&indir_blk_cnt, sizeof(unsigned int));
    stream.read(&textcnt, sizeof(__int64));
    stream.read(&lastnode_ind, sizeof(xptr));

    index_list.deserialize(stream);
#ifdef SE_ENABLE_FTSEARCH
    ft_index_list.deserialize(stream);
#endif
#ifdef SE_ENABLE_TRIGGERS
    trigger_list.deserialize(stream);
#endif
};


schema_node_xptr schema_node_object::get_first_child(const xmlns_ptr xmlns, const char * name, t_item type) const 
{
    CAT_FOR_EACH(sc_ref, i, this->children) {
        if (i->object.same_node(xmlns, name, type)) return i->object.snode;
    }

    return XNULL;
}

#define MAX_ELEMENT_CHILDREN ((int) ((PAGE_SIZE \
 /* block header    */    - sizeof(node_blk_hdr) \
 /* indirection rec */    - sizeof(xptr) \
 /* node header     */    - sizeof(e_dsc) \
        ) / sizeof(xptr)))

#define MAX_DOCUMENT_CHILDREN ((int) ((PAGE_SIZE \
 /* block header    */    - sizeof(node_blk_hdr) \
 /* indirection rec */    - sizeof(xptr) \
 /* node header     */    - sizeof(d_dsc) \
        ) / sizeof(xptr)))

schema_node_xptr schema_node_object::add_child(const xmlns_ptr xmlns, const char * name, t_item type) 
{
    if (((schema_node_object *) this->modify_self()) != this) {
        return ((schema_node_object *) this->modify_self())->add_child(xmlns, name, type);
    }

    U_ASSERT(this->type == element || this->type == document || this->type == virtual_root);

    if (!this->index_list.empty() && type==element)
        throw USER_EXCEPTION(SE2032); // Trying to create mixed content in the element whose value is used as key

     if ((this->children.count() + 1) > ((this->type == element) ? MAX_ELEMENT_CHILDREN : MAX_DOCUMENT_CHILDREN))
        throw USER_EXCEPTION(SE2040); // Too many childs by schema

    schema_node_cptr new_node(schema_node_object::create(this->root, xmlns, name, type, this->persistent));

    children.add_object_tail(sc_ref(new_node.ptr(), name, root->xmlns_register(xmlns), type));

    new_node->parent = this->p_object;

    if (new_node->persistent) {
        for (cat_list<index_cell_xptr>::item * i = this->root->full_index_list.first; i != NULL; i = i->next) {
            t_scmnodes vec = i->object->fits_to_index_as_key(new_node);
            t_scmnodes::iterator ii;
            for (ii = vec.begin(); ii != vec.end(); ii++) {
                new_node->index_list.add(index_ref(i->object, *ii));
            }
        }

        #ifdef SE_ENABLE_FTSEARCH
        CAT_FOR_EACH(ft_index_cell_xptr, i, this->root->full_ft_index_list)
            if (i->object->fits_to_index(new_node))
                new_node->ft_index_list.add(i->object);
        #endif
        #ifdef SE_ENABLE_TRIGGERS
        CAT_FOR_EACH(trigger_cell_xptr, i, this->root->full_trigger_list)
            if (i->object->fits_to_trigger(new_node))
                new_node->trigger_list.add(i->object);
        #endif
    }

    return ((schema_node_xptr) new_node.ptr());
};

bool schema_node_object::first_child_has_data (const xmlns_ptr xmlns, const char * name, t_item type)
{
    schema_node_cptr ch(this->get_first_child(xmlns, name, type));

    return (ch.found() && ch->nodecnt != 0);
};


int schema_node_object::find_first_child (const xmlns_ptr xmlns, const char * name, t_item type) const 
{
    int c = 0;

    for (cat_list<sc_ref>::item * i = this->children.first; i != NULL; i = i->next) {
        if (i->object.same_node(xmlns, name, type)) return c;
        c++;
    }

    return -1;
};

const sc_ref * schema_node_object::get_first_child_ref(const xmlns_ptr xmlns, const char * name, t_item type) const 
{
    cat_list<sc_ref>::item * i;

    for (i = this->children.first; i != NULL; i = i->next) 
        if (i->object.same_node(xmlns, name, type)) return &(i->object);

    return NULL;
};


bool schema_node_object::is_ancestor_or_self (schema_node_cptr node)
{
    if (this->p_object == node.ptr()) return true;

    CAT_FOR_EACH(sc_ref, i, children) {
        if (i->object.snode->is_ancestor_or_self(node)) return true;
    }

    return false;
}

void schema_node_object::drop()
{
    cat_list<sc_ref>::item * i;
    for (i = this->children.first; i != NULL; i = i->next) 
        i->object.snode->drop();

    cs_free(p_object);
    catalog_delete_object(this);
}

void schema_node_object::remove_index(const index_cell_xptr &c)
{
    if (((schema_node_object *) this->modify_self()) != this) {
        ((schema_node_object *) this->modify_self())->remove_index(c);
    }

    struct cat_list<index_ref>::item *j, * i = index_list.first;

    while (i != NULL) {
        j = i;
        i = i->next;
        if (j->object.index == c) index_list.remove_object(j);
    }

    CAT_FOR_EACH(sc_ref, z, children) { z->object.snode->remove_index(c); }
}

#ifdef SE_ENABLE_TRIGGERS

void schema_node_object::remove_trigger(const trigger_cell_xptr &c)
{
    if (((schema_node_object *) this->modify_self()) != this) {
        ((schema_node_object *) this->modify_self())->remove_trigger(c);
    }

    struct cat_list<trigger_cell_xptr>::item *j, * i = trigger_list.first;

    while (i != NULL) {
        j = i;
        i = i->next;
        if (j->object == c) trigger_list.remove_object(j);
    }

    CAT_FOR_EACH(sc_ref, z, children) { z->object.snode->remove_trigger(c); }
}

#endif // SE_ENABLE_TRIGGERS

#ifdef SE_ENABLE_FTSEARCH

void schema_node_object::remove_ft_index(const ft_index_cell_xptr &c)
{
    if (((schema_node_object *) this->modify_self()) != this) {
        ((schema_node_object *) this->modify_self())->remove_ft_index(c);
    }

    struct cat_list<ft_index_cell_xptr>::item *j, * i = ft_index_list.first;

    while (i != NULL) {
        j = i;
        i = i->next;
        if (j->object == c) ft_index_list.remove_object(j);
    }

    CAT_FOR_EACH(sc_ref, z, children) { z->object.snode->remove_ft_index(c); }
}

#endif // SE_ENABLE_FTSEARCH


/**************************************
 *  Document schema node
 */


catalog_object_header * doc_schema_node_object::create(bool persistent)
{
    doc_schema_node_object * a = 
      new(cat_malloc(persistent ? CATALOG_PERSISTENT_CONTEXT : CATALOG_TEMPORARY_CONTEXT, sizeof(doc_schema_node_object))) 
      doc_schema_node_object(persistent);
    catalog_object_header * b = catalog_create_object(a, persistent);

    a->root = a->p_object;

    return b;
}

catalog_object_header * doc_schema_node_object::create_virtual_root()
{
    doc_schema_node_object * a = 
      new(cat_malloc(CATALOG_TEMPORARY_CONTEXT, sizeof(doc_schema_node_object))) doc_schema_node_object(false);
    catalog_object_header * b = catalog_create_object(a, false);

    a->type = virtual_root;
    a->root = a->p_object;

    return b;
}

void doc_schema_node_object::serialize_data(se_simplestream &stream)
{
    schema_node_object::serialize_data(stream);

    stream.write(&ext_nids_block, sizeof(xptr));
    stream.write(&total_ext_nids, sizeof(__int64));

    full_index_list.serialize(stream);
#ifdef SE_ENABLE_FTSEARCH
    full_ft_index_list.serialize(stream);
#endif
#ifdef SE_ENABLE_TRIGGERS
    full_trigger_list.serialize(stream);
#endif
};

void doc_schema_node_object::deserialize_data(se_simplestream &stream)
{
    schema_node_object::deserialize_data(stream);

    stream.read(&ext_nids_block, sizeof(xptr));
    stream.read(&total_ext_nids, sizeof(__int64));

    full_index_list.deserialize(stream);
#ifdef SE_ENABLE_FTSEARCH
    full_ft_index_list.deserialize(stream);
#endif
#ifdef SE_ENABLE_TRIGGERS
    full_trigger_list.deserialize(stream);
#endif
};

/* 
 * If _doc_drop_mode is set, substructure deletion (index, for example) does not cause 
 * its deletion from the schema node list.
 */
volatile bool _doc_drop_mode = false;

void doc_schema_node_object::delete_index(index_cell_xptr c) 
{
    if (_doc_drop_mode) { return; }

    if (((doc_schema_node_object *) this->modify_self()) != this) {
        ((doc_schema_node_object *) this->modify_self())->delete_index(c);
    }

    this->remove_index(c);
    this->full_index_list.remove(c);
};

#ifdef SE_ENABLE_TRIGGERS

void doc_schema_node_object::delete_trigger(trigger_cell_xptr c) 
{
    if (_doc_drop_mode) { return; }

    if (((doc_schema_node_object *) this->modify_self()) != this) {
        ((doc_schema_node_object *) this->modify_self())->delete_trigger(c);
    }

    this->remove_trigger(c);
    this->full_trigger_list.remove(c);
};

#endif

#ifdef SE_ENABLE_FTSEARCH

void doc_schema_node_object::delete_ftindex(ft_index_cell_xptr c)
{
    if (_doc_drop_mode) { return; }

    if (((doc_schema_node_object *) this->modify_self()) != this) {
        ((doc_schema_node_object *) this->modify_self())->delete_ftindex(c);
    }

    this->remove_ft_index(c);
    this->full_ft_index_list.remove(c);
};

#endif


void doc_schema_node_object::drop()
{
    _doc_drop_mode = true;

    CAT_FOR_EACH(index_cell_xptr, i, this->root->full_index_list) { local_lock_mrg->put_lock_on_index(i->object->index_title); }
    CAT_FOR_EACH(index_cell_xptr, i, this->root->full_index_list) { i->object->drop(); }

    #ifdef SE_ENABLE_FTSEARCH
    CAT_FOR_EACH(ft_index_cell_xptr, i, this->root->full_ft_index_list) { local_lock_mrg->put_lock_on_index(i->object->index_title); }
    CAT_FOR_EACH(ft_index_cell_xptr, i, this->root->full_ft_index_list)  { i->object->drop(); }
    #endif

    #ifdef SE_ENABLE_TRIGGERS
    CAT_FOR_EACH(trigger_cell_xptr, i, this->root->full_trigger_list) { local_lock_mrg->put_lock_on_trigger(i->object->trigger_title); }
    CAT_FOR_EACH(trigger_cell_xptr, i, this->root->full_trigger_list)  { i->object->drop(); }
    #endif

    schema_node_object::drop();

    _doc_drop_mode = false;
}


xmlns_ptr_pers doc_schema_node_object::xmlns_register(xmlns_ptr xmlns)
{
    if (xmlns == NULL) { return XNULL; };

    xmlns_ptr_pers i = this->xmlns_list;

    while ((i != XNULL) && 
        ~((mystrcmp(xmlns->uri, i->uri) == 0) && (mystrcmp(xmlns->prefix, i->prefix) == 0))) {
        i = i->next_xmlns;
    }

    if (i == XNULL) {
        if (((doc_schema_node_object *) this->modify_self()) != this) {
            return ((doc_schema_node_object *) this->modify_self())->xmlns_register(xmlns);
        }

        catalog_cptr_template<xmlns_indb_object> a(xmlns_indb_object::create(xmlns->prefix, xmlns->uri, (persistent ? this->p_object : XNULL), this->xmlns_list));
        i = this->xmlns_list = a.ptr();
    }

    return i;
}


/**************************************
 *  Collection schema node
 */

catalog_object_header * col_schema_node_object::create()
{
    col_schema_node_object * a = 
      new(cat_malloc(CATALOG_PERSISTENT_CONTEXT, sizeof(col_schema_node_object)))
      col_schema_node_object();
    catalog_object_header * b = catalog_create_object(a);
  
    a->root = a->p_object;
    a->metadata = bt_create(xs_string);

    return b;
};

void col_schema_node_object::serialize_data(se_simplestream &stream)
{
    doc_schema_node_object::serialize_data(stream);

    stream.write(&eblk, sizeof(xptr));
    stream.write(&metadata, sizeof(xptr));
};

void col_schema_node_object::deserialize_data(se_simplestream &stream)
{
    doc_schema_node_object::deserialize_data(stream);

    stream.read(&eblk, sizeof(xptr));
    stream.read(&metadata, sizeof(xptr));
};



xptr col_schema_node_object::find_document(const char * doc_name)
{
    bt_key key;
    key.setnew(doc_name);
    return removeIndirection(bt_find(metadata, key).bt_next_obj());
};

void col_schema_node_object::delete_document(const char * doc_name)
{
    xptr new_btree = metadata;
    bt_key key; 
    key.setnew(doc_name);
    bt_delete(new_btree, key);

    if (new_btree != metadata) ((col_schema_node_object *) this->modify_self())->metadata = new_btree;
};

void col_schema_node_object::insert_document(const char * doc_name, xptr node)
{
    xptr new_btree = metadata;
    bt_key key;
    key.setnew(doc_name);
    bt_insert(new_btree, key, node);

    if (new_btree != metadata) ((col_schema_node_object *) this->modify_self())->metadata = new_btree;
};

void col_schema_node_object::drop()
{
    bt_drop(metadata);
    doc_schema_node_object::drop();
}

/* returns the name of atomic type
char* convertTypeToName(xmlscm_type i)
{return "";}
*/
