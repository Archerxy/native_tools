#include "map.h"

typedef enum {RED, BLACK} Color;

typedef struct MapNode_st {
    Color                 color;
    struct MapNode_st    *parent;
    struct MapNode_st    *left;
    struct MapNode_st    *right;
    char                 *key;
    void                 *val;
} MapNode;

typedef struct Map_st {
    MapNode              *root;
    size_t                size;
} Map;

static MapNode * map_node_new(char *key, void *val, MapNode *p) {
    MapNode *node = (MapNode *)malloc(sizeof(MapNode));
    node->color = RED;
    node->parent = p;
    node->left = NULL;
    node->right = NULL;
    node->key = key;
    node->val = val;
    return node;
}

static void map_node_free(MapNode *node) {
    MapNode *p = node->parent;
    if(p) {
        if(node == p->left) {
            p->left = NULL;
        } else {
            p->right = NULL;
        }
    }
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    free(node);
}

static void map_node_free_with_cb(MapNode *node, map_key_val_free_cb free_cb) {
    if(free_cb) {
        free_cb(node->key, node->val);
    }
    map_node_free(node);
}

static void map_node_left_rotate(Map *map, MapNode *x) {
    MapNode *y = x->right;

    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == NULL) {
        map->root = y;  
    } else {
        if (x->parent->left == x) {
            x->parent->left = y; 
        } else {
            x->parent->right = y; 
        }
    }
    y->left = x;
    x->parent = y;
}

static void map_node_right_rotate(Map *map, MapNode *y) {
    MapNode *x = y->left;

    y->left = x->right;
    if (x->right != NULL) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (y->parent == NULL) {
        map->root = x;
    } else {
        if (y == y->parent->right) {
            y->parent->right = x;
        } else {
            y->parent->left = x; 
        }
    }
    x->right = y;
    y->parent = x;
}

static void map_node_insert_fixup(Map *map, MapNode *node) {
    MapNode *parent, *gparent;

    while ((parent = node->parent) && parent->color == RED) {
        gparent = parent->parent;
        if (parent == gparent->left) {
            MapNode *uncle = gparent->right;
            if (uncle && uncle->color == RED) {
                uncle->color = BLACK;
                parent->color = BLACK;
                gparent->color = RED;
                node = gparent;
            } else {
                if (parent->right == node) {
                    map_node_left_rotate(map, parent);
                    MapNode *tmp = parent;
                    parent = node;
                    node = tmp;
                }

                parent->color = BLACK;
                gparent->color = RED;
                map_node_right_rotate(map, gparent);
            }
        } else {
            MapNode *uncle = gparent->left;
            if (uncle && uncle->color == RED) {
                uncle->color = BLACK;
                parent->color = BLACK;
                gparent->color = RED;
                node = gparent;
                continue;
            } else {
                if (parent->left == node) {
                    map_node_right_rotate(map, parent);
                    MapNode *tmp = parent;
                    parent = node;
                    node = tmp;
                }
                parent->color = BLACK;
                gparent->color = RED;
                map_node_left_rotate(map, gparent);
            }
        }
    }
    map->root->color = BLACK;
}


static void map_node_delete_fixup(Map *map, MapNode *node, MapNode *parent) {
    MapNode *brother = NULL;
    while ((!node || node->color == BLACK) && node != map->root) {
        if (parent->left == node) {
            brother = parent->right;
            if (brother->color == RED) {
                brother->color = BLACK;
                parent->color = RED;
                map_node_left_rotate(map, parent);
                brother = parent->right;
            }
            if ((!brother->left || brother->left->color == BLACK) &&
                (!brother->right || brother->right->color == BLACK)) {
                brother->color = RED;
                node = parent;
                parent = node->parent;
            } else {
                if (!brother->right || brother->right->color == BLACK) {
                    brother->left->color = BLACK;
                    brother->color = RED;
                    map_node_right_rotate(map, brother);
                    brother = parent->right;
                }
                brother->color = parent->color;
                parent->color = BLACK;
                brother->right->color = BLACK;
                map_node_left_rotate(map, parent);
                node = map->root;
                break;
            }
        } else {
            brother = parent->left;
            if (brother->color == RED) {
                brother->color = BLACK;
                parent->color = RED;
                map_node_right_rotate(map, parent);
                brother = parent->left;
            }
            if ((!brother->left || brother->left->color == BLACK) &&
                (!brother->right || brother->right->color == BLACK)) {
                brother->color = RED;
                node = parent;
                parent = node->parent;
            } else {
                if (!brother->left || brother->left->color == BLACK) {
                    brother->right->color = BLACK;
                    brother->color = RED;
                    map_node_left_rotate(map, brother);
                    brother = parent->left;
                }
                brother->color = parent->color;
                parent->color = BLACK;
                brother->left->color = BLACK;
                map_node_right_rotate(map, parent);
                break;
            }
        }
    }
    map->root->color = BLACK;
}

static void map_node_free_all(MapNode *node, map_key_val_free_cb free_cb) {
    if(node->left) {
        map_node_free_all(node->left, free_cb);
    }
    if(node->right) {
        map_node_free_all(node->right, free_cb);
    }
    map_node_free_with_cb(node, free_cb);
}

static void map_node_foreach(MapNode *node,  map_foreach_cb fn) {
    if(node->left) {
        map_node_foreach(node->left, fn);
    }
    if(node->right) {
        map_node_foreach(node->right, fn);
    }
    if(fn) {
        fn(node->key, node->val);
    }
}

Map * map_new() {
    Map *map = (Map *)malloc(sizeof(Map));
    map->root = NULL;
    map->size = 0;

    return map;
}

void map_free(Map *map, map_key_val_free_cb free_cb) {
    if(!map || !map->root) {
        return ;
    }
    map_node_free_all(map->root, free_cb);
    map->root = NULL;
    free(map);
}

size_t map_size(Map *map) {
    return map->size;
}

void map_foreach(Map *map, map_foreach_cb fn) {
    if(!map || !map->root) {
        return ;
    }
    map_node_foreach(map->root, fn);
}

void* map_put(Map *map, char *key, void *val) {
    if(!map) {
        return NULL;
    }
    if(!map->root) {
        map->root = map_node_new(key, val, NULL);
        map->root->color = BLACK;
        map->size = 1;
        return NULL;

    }
    MapNode *cur = map->root;
    size_t key_len = strlen(key), len = 0;
    while(1) {
        len = strlen(cur->key);
        if(cur->key == key || (key_len == len && strncmp(cur->key, key, key_len) == 0)) {
            void *old = cur->val;
            cur->val = val;
            return old;
        }
        if(((uint64_t)cur->key) > (uint64_t) key) {
            if(!cur->left) {
                cur->left = map_node_new(key, val, cur);
                cur->left->parent = cur;
                cur = cur->left;
                break ;
            } else {
                cur = cur->left;
            }
        } else {
            if(!cur->right) {
                cur->right = map_node_new(key, val, cur);
                cur->right->parent = cur;
                cur = cur->right;
                break ;
            } else {
                cur = cur->right;
            }
        }
    }
    
    map->size++;
    map_node_insert_fixup(map, cur);
    return NULL;
}

void * map_get(Map *map, char *key) {
    if(!map || !map->root) {
        return NULL;
    }
    MapNode *cur = map->root;
    size_t key_len = strlen(key), len = 0;
    while(1) {
        len = strlen(cur->key);
        if(cur->key == key || (key_len == len && strncmp(cur->key, key, key_len) == 0)) {
            break ;
        }
        if(((uint64_t)cur->key) > (uint64_t) key) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }
        if(cur == NULL) {
            return NULL;
        }
    }
    return cur->val;
}

void * map_remove(Map *map, char *key) {
    if(!map || !map->root) {
        return NULL;
    }
    MapNode *cur = map->root;
    size_t key_len = strlen(key), len = 0;
    while(1) {
        len = strlen(cur->key);
        if(cur->key == key || (key_len == len && strncmp(cur->key, key, key_len) == 0)) {
            break ;
        }
        if(((uint64_t)cur->key) > (uint64_t) key) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }
        if(cur == NULL) {
            return NULL;
        }
    }
    map->size--;
    MapNode *child = NULL, *parent = NULL;
    Color color;

    if (cur->left != NULL && cur->right != NULL) {
        MapNode *replace = cur->right;
        while (replace->left != NULL) {
            replace = replace->left;
        }
        if (cur->parent) {
            if (cur->parent->left == cur) {
                cur->parent->left = replace;
            } else {
                cur->parent->right = replace;
            }
        } else {
            map->root = replace;
        }
        child = replace->right;
        parent = replace->parent;
        color = replace->color;

        if (replace->parent == cur) {
            parent = replace;
        } else {
            if (child) {
                child->parent = parent;
            }
            parent->left = child;
            replace->right = cur->right;
            cur->right->parent = replace;
        }

        replace->parent = cur->parent;
        replace->color = cur->color;
        replace->left = cur->left;
        cur->left->parent = replace;

        if (color == BLACK) {
            map_node_delete_fixup(map, child, parent);
        }
        void *old = cur->val;
        map_node_free(cur);
        return old;
    }

    if (cur->left !=NULL) {
        child = cur->left;
    } else {
        child = cur->right;
    }
    parent = cur->parent;
    color = cur->color;

    if (child) {
        child->parent = parent;
    }
    if (parent) {
        if (parent->left == cur) {
            parent->left = child;
        } else {
            parent->right = child;
        }
    } else {
        map->root = child;
    }
    if (color == BLACK) {
        map_node_delete_fixup(map, child, parent);
    }
    void *old = cur->val;
    map_node_free(cur);
    return old;
}

