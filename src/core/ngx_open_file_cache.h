
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

// 这组头文件和实现文件实现了缓存文件fd功能的相关函数。
// 缓存fd一方面可以减少打开文件的次数，一方面可以减少进程使用的fd数量。
// 会用到红黑树和双向链表

#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_OPEN_FILE_CACHE_H_INCLUDED_
#define _NGX_OPEN_FILE_CACHE_H_INCLUDED_


#define NGX_OPEN_FILE_DIRECTIO_OFF  NGX_MAX_OFF_T_VALUE


typedef struct {
    ngx_fd_t                 fd;
    ngx_file_uniq_t          uniq;
    time_t                   mtime;
    off_t                    size;
    off_t                    fs_size;
    off_t                    directio;
    size_t                   read_ahead;

    ngx_err_t                err;
    char                    *failed;

    time_t                   valid;

    ngx_uint_t               min_uses;

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 test_dir:1;
    unsigned                 test_only:1;
    unsigned                 log:1;
    unsigned                 errors:1;
    unsigned                 events:1;

    unsigned                 is_dir:1;
    unsigned                 is_file:1;
    unsigned                 is_link:1;
    unsigned                 is_exec:1;
    unsigned                 is_directio:1;
} ngx_open_file_info_t;


typedef struct ngx_cached_open_file_s  ngx_cached_open_file_t;

// 一个被缓存的fd对应一个ngx_cached_open_file_t对象
struct ngx_cached_open_file_s {
    // 使用了红黑树为了快速某个文件名对应的ngx_cached_open_file_t对象
    ngx_rbtree_node_t        node;
    ngx_queue_t              queue;

    u_char                  *name;
    time_t                   created;
    time_t                   accessed;

    ngx_fd_t                 fd;
    ngx_file_uniq_t          uniq;
    time_t                   mtime;
    off_t                    size;
    ngx_err_t                err;

    uint32_t                 uses;

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 count:24;
    unsigned                 close:1;
    unsigned                 use_event:1;

    unsigned                 is_dir:1;
    unsigned                 is_file:1;
    unsigned                 is_link:1;
    unsigned                 is_exec:1;
    unsigned                 is_directio:1;

    ngx_event_t             *event;
};


typedef struct {
    // 使用了红黑树为了快速某个文件名对应的ngx_cached_open_file_t对象
    // 红黑树
    ngx_rbtree_t             rbtree;
    // 红黑树哨兵节点
    ngx_rbtree_node_t        sentinel;
    // 双向链表用于取出最先过期的元素
    ngx_queue_t              expire_queue;

    // 已缓存的fd数量
    ngx_uint_t               current;
    // 最大可以缓存的fd数量
    ngx_uint_t               max;
    // 一个fd多就没有使用后就可以被关闭
    time_t                   inactive;
} ngx_open_file_cache_t;


typedef struct {
    ngx_open_file_cache_t   *cache;
    ngx_cached_open_file_t  *file;
    ngx_uint_t               min_uses;
    ngx_log_t               *log;
} ngx_open_file_cache_cleanup_t;


typedef struct {

    /* ngx_connection_t stub to allow use c->fd as event ident */
    void                    *data;
    ngx_event_t             *read;
    ngx_event_t             *write;
    ngx_fd_t                 fd;

    ngx_cached_open_file_t  *file;
    ngx_open_file_cache_t   *cache;
} ngx_open_file_cache_event_t;


ngx_open_file_cache_t *ngx_open_file_cache_init(ngx_pool_t *pool,
    ngx_uint_t max, time_t inactive);
ngx_int_t ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool);


#endif /* _NGX_OPEN_FILE_CACHE_H_INCLUDED_ */