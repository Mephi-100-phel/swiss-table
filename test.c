#include "swiss_table.h"
#include <stdio.h>
#include <assert.h>
#include <omp.h>

static double
simple_insert_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  const int iter_max = 100;
  double start, end, total = 0;
  char tmp[10] = { 0 };
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    start = omp_get_wtime();
    int err = swiss_table_insert_update(tbl, tmp, tmp);
    end = omp_get_wtime();
    assert(err == NO_ERR);
    total += (end - start);
  }
  swiss_table_destroy(tbl);
  return total / iter_max;
}

static double
huge_insert_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  const int iter_max = 1000;
  double start, end, total = 0;
  char tmp[10] = { 0 };
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    start = omp_get_wtime();
    int err = swiss_table_insert_update(tbl, tmp, tmp);
    end = omp_get_wtime();
    assert(err == NO_ERR);
    total += (end - start);
  }
  swiss_table_destroy(tbl);
  return total / iter_max;
}

static double
strange_args_insert_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  double start, end, total = 0;
  int err, test_count = 0;
  start = omp_get_wtime();
  err = swiss_table_insert_update(NULL, "123", "1");
  end = omp_get_wtime();
  assert(err == INVALID_ARGS);
  total += (end - start);
  ++test_count;
  start = omp_get_wtime();
  err = swiss_table_insert_update(tbl, NULL, "22");
  end = omp_get_wtime();
  assert(err == INVALID_ARGS);
  total += (end - start);
  ++test_count;
  start = omp_get_wtime();
  err = swiss_table_insert_update(tbl, "123", NULL);
  end = omp_get_wtime();
  assert(err == INVALID_ARGS);
  total += (end - start);
  ++test_count;
  swiss_table_insert_update(tbl, "123", "123");
  start = omp_get_wtime();
  err = swiss_table_insert_update(tbl, "123", "456");
  end = omp_get_wtime();
  assert(err == UPDATED);
  total += (end - start);
  ++test_count;
  char long_str[128 * 3 + 1] = { 0 };
  for (int i = 0; i < 128; ++i) {
    strcat(long_str, "123");
  }
  start = omp_get_wtime();
  err = swiss_table_insert_update(tbl, long_str, "123");
  end = omp_get_wtime();
  assert(err == NO_ERR);
  total += (end - start);
  ++test_count;
  swiss_table_destroy(tbl);
  return total / test_count;
}

static double
simple_search_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  double start, end, total = 0;
  const int iter_max = 100;
  char tmp[10] = { 0 };
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    int err = swiss_table_insert_update(tbl, tmp, tmp);
    assert(err == NO_ERR);
  }
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    start = omp_get_wtime();
    char* res = swiss_table_get_copy(tbl, tmp);
    end = omp_get_wtime();
    assert(res);
    assert(!strcmp(tmp, res));
    free(res);
    total += (end - start);
  }
  swiss_table_destroy(tbl);
  return total / iter_max;
}

static double
huge_search_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  double start, end, total = 0;
  const int iter_max = 1000;
  char tmp[10] = { 0 };
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    int err = swiss_table_insert_update(tbl, tmp, tmp);
    assert(err == NO_ERR);
  }
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    start = omp_get_wtime();
    char* res = swiss_table_get_copy(tbl, tmp);
    end = omp_get_wtime();
    assert(res);
    assert(!strcmp(tmp, res));
    free(res);
    total += (end - start);
  }
  swiss_table_destroy(tbl);
  return total / iter_max;
}

static double
strange_args_search_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  double start, end, total = 0;
  int test_count = 0;
  char* res;
  start = omp_get_wtime();
  res = swiss_table_get_copy(NULL, "123");
  end = omp_get_wtime();
  assert(!res);
  total += (end - start);
  ++test_count;
  start = omp_get_wtime();
  res = swiss_table_get_copy(tbl, NULL);
  end = omp_get_wtime();
  assert(!res);
  total += (end - start);
  ++test_count;
  start = omp_get_wtime();
  res = swiss_table_get_copy(tbl, "123");
  end = omp_get_wtime();
  assert(!res);
  total += (end - start);
  ++test_count;
  swiss_table_destroy(tbl);
  return total / test_count;
}

static double
simple_delete_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  const int iter_max = 100;
  double start, end, total = 0;
  char tmp[10] = { 0 };
  int err;
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    err = swiss_table_insert_update(tbl, tmp, tmp);
    assert(err == NO_ERR);
  }
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    start = omp_get_wtime();
    err = swiss_table_delete(tbl, tmp);
    end = omp_get_wtime();
    assert(err == NO_ERR);
    total += (end - start);
  }
  swiss_table_destroy(tbl);
  return total / iter_max;
}

static double
huge_delete_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  const int iter_max = 1000;
  double start, end, total = 0;
  char tmp[10] = { 0 };
  int err;
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    err = swiss_table_insert_update(tbl, tmp, tmp);
    assert(err == NO_ERR);
  }
  for (int i = 0; i < iter_max; ++i) {
    sprintf(tmp, "%d", i);
    start = omp_get_wtime();
    err = swiss_table_delete(tbl, tmp);
    end = omp_get_wtime();
    assert(err == NO_ERR);
    total += (end - start);
  }
  swiss_table_destroy(tbl);
  return total / iter_max;
}

static double
strange_args_delete_test(void)
{
  swiss_table_t* tbl = swiss_table_init();
  assert(tbl);
  double start, end, total = 0;
  int test_count = 0;
  int err;
  start = omp_get_wtime();
  err = swiss_table_delete(NULL, "123");
  end = omp_get_wtime();
  assert(err == INVALID_ARGS);
  total += (end - start);
  ++test_count;
  start = omp_get_wtime();
  err = swiss_table_delete(tbl, NULL);
  end = omp_get_wtime();
  assert(err == INVALID_ARGS);
  total += (end - start);
  ++test_count;
  start = omp_get_wtime();
  err = swiss_table_delete(tbl, "123");
  end = omp_get_wtime();
  assert(err == KEY_NOT_FOUND);
  total += (end - start);
  ++test_count;
  swiss_table_destroy(tbl);
  return total / test_count;
}

int
main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  double time;
  printf("=======Tests started=======\n\n");

  time = simple_insert_test();
  printf("Simple insert test passed\nAvg. insertion time: %.15lf\n\n", time);
  time = huge_insert_test();
  printf("Huge insert test passed\nAvg. insertion time: %.15lf\n\n", time);
  time = strange_args_insert_test();
  printf("Strange argument insert test passed\nAvg. insertion time: %.15lf\n\n", time);
  time = simple_search_test();
  printf("Simple search test passed\nAvg. search time: %.15lf\n\n", time);
  time = huge_search_test();
  printf("Huge search test passed\nAvg. search time: %.15lf\n\n", time);
  time = strange_args_search_test();
  printf("Strange argument search test passed\nAvg. search time: %.15lf\n\n", time);
  time = simple_delete_test();
  printf("Stimple delete test passed\nAvg. delete time: %.15lf\n\n", time);
  time = huge_delete_test();
  printf("Huge delete test passed\nAvg. delete time: %.15lf\n\n", time);
  time = strange_args_delete_test();
  printf("Strange argument delete test passed\nAvg. delete time: %.15lf\n\n", time);

  printf("======All tests passed======\n");
  return 0;
}
