#include <math.h>
#include <errno.h>
#include <unistd.h>

#include "ruby.h"
#include "rubyspec.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_RB_THREAD_ALONE
static VALUE thread_spec_rb_thread_alone() {
  return rb_thread_alone() ? Qtrue : Qfalse;
}
#endif

#ifdef HAVE_RB_THREAD_BLOCKING_REGION
/* This is unblocked by unblock_func(). */
static VALUE blocking_func(void* data) {
  int rfd = (int)(size_t)data;
  char dummy;
  ssize_t rv;

  do {
    rv = read(rfd, &dummy, 1);
  } while (rv == -1 && errno == EINTR);

  return (rv == 1) ? Qtrue : Qfalse;
}

static void unblock_func(void *data) {
  int wfd = (int)(size_t)data;
  char dummy = 0;
  ssize_t rv;

  do {
    rv = write(wfd, &dummy, 1);
  } while (rv == -1 && errno == EINTR);
}

/* Returns true if the thread is interrupted. */
static VALUE thread_spec_rb_thread_blocking_region(VALUE self) {
  int fds[2];
  VALUE ret;

  if (pipe(fds) == -1) {
    return Qfalse;
  }
  SUPPRESS_DIAGNOSTIC("-Wdeprecated-declarations",
    ret = rb_thread_blocking_region(blocking_func, (void*)(size_t)fds[0],
                                    unblock_func, (void*)(size_t)fds[1]);
  )
  close(fds[0]);
  close(fds[1]);
  return ret;
}

/* This is unblocked by a signal. */
static VALUE blocking_func_for_udf_io(void *data) {
  int rfd = (int)(size_t)data;
  char dummy;

  if (read(rfd, &dummy, 1) == -1 && errno == EINTR) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

/* Returns true if the thread is interrupted. */
static VALUE thread_spec_rb_thread_blocking_region_with_ubf_io(VALUE self) {
  int fds[2];
  VALUE ret;

  if (pipe(fds) == -1) {
    return Qfalse;
  }

  SUPPRESS_DIAGNOSTIC("-Wdeprecated-declarations",
    ret = rb_thread_blocking_region(blocking_func_for_udf_io,
                                    (void*)(size_t)fds[0], RUBY_UBF_IO, 0);
  )
  close(fds[0]);
  close(fds[1]);
  return ret;
}
#endif

#ifdef HAVE_RB_THREAD_CURRENT
static VALUE thread_spec_rb_thread_current() {
  return rb_thread_current();
}
#endif

#ifdef HAVE_RB_THREAD_LOCAL_AREF
static VALUE thread_spec_rb_thread_local_aref(VALUE self, VALUE thr, VALUE sym) {
  return rb_thread_local_aref(thr, SYM2ID(sym));
}
#endif

#ifdef HAVE_RB_THREAD_LOCAL_ASET
static VALUE thread_spec_rb_thread_local_aset(VALUE self, VALUE thr, VALUE sym, VALUE value) {
  return rb_thread_local_aset(thr, SYM2ID(sym), value);
}
#endif

#ifdef HAVE_RB_THREAD_SELECT
static VALUE thread_spec_rb_thread_select_fd(VALUE self, VALUE fd_num, VALUE msec) {
  int fd = NUM2INT(fd_num);
  struct timeval tv;
  int n;

  fd_set read;
  FD_ZERO(&read);
  FD_SET(fd, &read);
  tv.tv_sec = 0;
  tv.tv_usec = NUM2INT(msec);

  SUPPRESS_DIAGNOSTIC("-Wdeprecated-declarations",
    n = rb_thread_select(fd + 1, &read, NULL, NULL, &tv);
  )
  if(n == 1 && FD_ISSET(fd, &read)) return Qtrue;
  return Qfalse;
}

static VALUE thread_spec_rb_thread_select(VALUE self, VALUE msec) {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = NUM2INT(msec);
  SUPPRESS_DIAGNOSTIC("-Wdeprecated-declarations",
    rb_thread_select(0, NULL, NULL, NULL, &tv);
  )
  return Qnil;
}
#endif

#ifdef HAVE_RB_THREAD_WAKEUP
static VALUE thread_spec_rb_thread_wakeup(VALUE self, VALUE thr) {
  return rb_thread_wakeup(thr);
}
#endif

#ifdef HAVE_RB_THREAD_WAIT_FOR
static VALUE thread_spec_rb_thread_wait_for(VALUE self, VALUE s, VALUE ms) {
  struct timeval tv;
  tv.tv_sec = NUM2INT(s);
  tv.tv_usec = NUM2INT(ms);
  rb_thread_wait_for(tv);
  return Qnil;
}
#endif

void Init_thread_spec() {
  VALUE cls;
  cls = rb_define_class("CApiThreadSpecs", rb_cObject);

#ifdef HAVE_RB_THREAD_ALONE
  rb_define_method(cls, "rb_thread_alone", thread_spec_rb_thread_alone, 0);
#endif

#ifdef HAVE_RB_THREAD_BLOCKING_REGION
  rb_define_method(cls, "rb_thread_blocking_region", thread_spec_rb_thread_blocking_region, 0);
  rb_define_method(cls, "rb_thread_blocking_region_with_ubf_io", thread_spec_rb_thread_blocking_region_with_ubf_io, 0);
#endif

#ifdef HAVE_RB_THREAD_CURRENT
  rb_define_method(cls, "rb_thread_current", thread_spec_rb_thread_current, 0);
#endif

#ifdef HAVE_RB_THREAD_LOCAL_AREF
  rb_define_method(cls, "rb_thread_local_aref", thread_spec_rb_thread_local_aref, 2);
#endif

#ifdef HAVE_RB_THREAD_LOCAL_ASET
  rb_define_method(cls, "rb_thread_local_aset", thread_spec_rb_thread_local_aset, 3);
#endif

#ifdef HAVE_RB_THREAD_SELECT
  rb_define_method(cls, "rb_thread_select_fd", thread_spec_rb_thread_select_fd, 2);
  rb_define_method(cls, "rb_thread_select", thread_spec_rb_thread_select, 1);
#endif

#ifdef HAVE_RB_THREAD_WAKEUP
  rb_define_method(cls,  "rb_thread_wakeup", thread_spec_rb_thread_wakeup, 1);
#endif

#ifdef HAVE_RB_THREAD_WAIT_FOR
  rb_define_method(cls,  "rb_thread_wait_for", thread_spec_rb_thread_wait_for, 2);
#endif
}

#ifdef __cplusplus
}
#endif
