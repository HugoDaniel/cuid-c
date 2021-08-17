#ifndef CUID_TESTS_H
#include "./munit.h" // The external unit tests framework - µnit

/*
** Test that the `cuid_t` exists
*/
static MunitResult
test_id_exists(const MunitParameter params[], void* data) {
    cuid_t id = {0};
    
    munit_assert_not_null(&id);
    return MUNIT_OK;
}

/*
** Test that the `cuid_t` has a fingerprint attribute.
*/
static MunitResult
test_fingerprint_exists(const MunitParameter params[], void* data) {
    cuid_t id = {0};
    
    munit_assert_not_null(&id.cuid_fingerprint);
    // Should have enough space for a base36 string
    munit_assert_uint(sizeof(id.cuid_fingerprint), ==, CUID_BASE36_RESULT_SIZE);
    return MUNIT_OK;
}

/*
** Test that the `cuid_t` has a counter variable.
*/
static MunitResult
test_counter_exists(const MunitParameter params[], void* data) {
    cuid_t id = {0};
    
    munit_assert_not_null(&id.cuid_counter);
    // munit_assert_size(id.cuid_counter, ==, 0);
    return MUNIT_OK;
}

/*
** Test that a number can be converted to a base36 string
*/
static MunitResult
test_can_convert_to_base36(const MunitParameter params[], void* data) {
    
    unsigned number1 = 128;
    char result1[CUID_BASE36_RESULT_SIZE] = {0};
    size_t length = cuid_base36(number1, result1);
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);
    munit_assert_string_equal(result1, "3k");
    munit_assert_size(length, ==, 2);

    char result2[CUID_BASE36_RESULT_SIZE] = {0};
    unsigned number2 = 12345;
    length = cuid_base36(number2, result2);
    munit_assert_string_equal(result2, "9ix");
    munit_assert_size(length, ==, 3);
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);

    char result3[CUID_BASE36_RESULT_SIZE] = {0};
    unsigned number3 = 0;
    length = cuid_base36(number3, result3);
    munit_assert_string_equal(result3, "0");
    munit_assert_size(length, ==, 1);
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);

    return MUNIT_OK;
}

/*
** Test that there is a padded version of the base36 function and
** that it can produce padded strings.
*/
static MunitResult
test_padded_base36(const MunitParameter params[], void* data) {
    unsigned number1 = 128;
    char result1[CUID_BASE36_RESULT_SIZE] = {0};
    size_t length = cuid_base36_pad(number1, result1, 4, 'z');
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);
    munit_assert_string_equal(result1, "zz3k");
    munit_assert_size(length, ==, 4);

    char result2[CUID_BASE36_RESULT_SIZE] = {0};
    unsigned number2 = 12345;
    length = cuid_base36_pad(number2, result2, 6, '0');
    munit_assert_string_equal(result2, "0009ix");
    munit_assert_size(length, ==, 6);
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);

    char result3[CUID_BASE36_RESULT_SIZE] = {0};
    unsigned number3 = 0;
    length = cuid_base36_pad(number3, result3, 4, '0');
    munit_assert_string_equal(result3, "0000");
    munit_assert_size(length, ==, 4);
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);

    // Padding should clip at the end (i.e. return the last n chars)
    char result4[CUID_BASE36_RESULT_SIZE] = {0};
    unsigned number4 = 1234567890;
    // Should produce the unpaded "kf12oi" string
    length = cuid_base36_pad(number4, result4, 4, '0');
    // Must clip at the bottom:
    munit_assert_string_equal(result4, "12oi");
    munit_assert_size(length, ==, 4);
    munit_logf(MUNIT_LOG_INFO, "length: %zu", length);


    return MUNIT_OK;
}

/*
** Test that a default counter implementation exists and works as
** expected.
*/
static MunitResult
test_has_counter(const MunitParameter params[], void* data) {
    
  CUID_COUNTER_T counter1 = CUID_INIT_COUNTER(CUID_CREATE_COUNTER());

  // Should start as zero
  munit_logf(MUNIT_LOG_INFO, "Should start as zero: %u",
      CUID_READ_COUNTER(counter1));
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 0);
  // Multiple reads should produce the same value
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 0);
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 0);
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 0);

  // Incrementing the counter should produce the next value:
  counter1 = CUID_INCREASE_COUNTER(counter1);
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 1);

  CUID_COUNTER_T counter2 = CUID_INIT_COUNTER(CUID_CREATE_COUNTER());
  // New counters should not change the values of previous counters
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 1);
  munit_assert_uint(CUID_READ_COUNTER(counter2), ==, 0);

  // Increasing a counter without attributing it to the variable should
  // not change its value. 
  CUID_INCREASE_COUNTER(counter1);
  CUID_INCREASE_COUNTER(counter1);
  CUID_INCREASE_COUNTER(counter2);
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 1);
  munit_assert_uint(CUID_READ_COUNTER(counter2), ==, 0);

  // Can initialize a counter that was previously increased
  counter1 = CUID_INIT_COUNTER(counter1);
  munit_assert_uint(CUID_READ_COUNTER(counter1), ==, 0);

  return MUNIT_OK;
}

/*
** Test that a timestamp can be generated.
*/
static MunitResult
test_timestamp(const MunitParameter params[], void* data) {
    unsigned long t1 = CUID_GET_TIMESTAMP();
    munit_logf(MUNIT_LOG_INFO, "timestamp1: %zu", t1);
    munit_assert_ulong(t1, >, 0);

    // Burn some time
    for(size_t volatile i = 0; i < 1000000000; i++);

    unsigned long t2 = CUID_GET_TIMESTAMP();
    munit_logf(MUNIT_LOG_INFO, "timestamp2: %zu", t2);
    munit_assert_ulong(t1, !=, t2);

    return MUNIT_OK;
}

/*
** Test that a fingerprint can be generated.
*/
static MunitResult
test_fingerprint(const MunitParameter params[], void* data) {
    char fingerprint[CUID_FINGERPRINT_SIZE] = {0};
    size_t length = CUID_GET_FINGERPRINT(fingerprint);
    munit_logf(MUNIT_LOG_INFO, "fingerprint: %s", fingerprint);
    munit_assert_size(length, ==, 4);

    for (size_t i = length - 1; i > 0; --i) {
      munit_assert_char(fingerprint[i], >, 0);
    }

    // Fingerprint should not change in multiple executions
    char fingerprint2[CUID_FINGERPRINT_SIZE] = {0};
    CUID_GET_FINGERPRINT(fingerprint2);
    munit_logf(MUNIT_LOG_INFO, "fingerprint2: %s", fingerprint2);
    munit_assert_string_equal(fingerprint, fingerprint2);

    return MUNIT_OK;
}

/*
** Test that a default random implementation exists and works as
** expected.
*/
static MunitResult
test_has_random(const MunitParameter params[], void* data) {
    
  CUID_RANDOM_T random1 = CUID_INIT_RANDOM(CUID_CREATE_RANDOM());

  // Should not start as zero
  uint32_t rnd_value1 = CUID_READ_RANDOM(random1);
  munit_logf(MUNIT_LOG_INFO, "Should not start as zero: %u",
      rnd_value1);
  munit_assert_ulong(rnd_value1, >, 0);


  // Multiple reads should produce the same value
  munit_assert_ulong(CUID_READ_RANDOM(random1), ==, rnd_value1);
  munit_assert_ulong(CUID_READ_RANDOM(random1), ==, rnd_value1);
  munit_assert_ulong(CUID_READ_RANDOM(random1), ==, rnd_value1);

  // Generating a random number should produce a different value:
  random1 = CUID_NEXT_RANDOM(random1);
  uint32_t rnd_value2 = CUID_READ_RANDOM(random1);
  munit_assert_uint32(rnd_value2, !=, rnd_value1);
  munit_logf(MUNIT_LOG_INFO, "Should not change: %u", rnd_value2);
  munit_assert_uint32(CUID_READ_RANDOM(random1), ==, rnd_value2);

  // Can initialize a rng that was previously increased and it should
  // produce the same value again
  random1 = CUID_INIT_RANDOM(random1);
  uint32_t rnd_new_value1 = CUID_READ_RANDOM(random1);
  random1 = CUID_NEXT_RANDOM(random1);
  uint32_t rnd_new_value2 = CUID_READ_RANDOM(random1);

  munit_assert_uint32(rnd_new_value1, ==, rnd_value1);
  munit_assert_uint32(rnd_new_value2, ==, rnd_value2);

  return MUNIT_OK;
}

/*
** Test that the `cuid()` function works as expected.
*/
static MunitResult
test_cuid(const MunitParameter params[], void* data) {
    char result[CUID_SIZE] = {0};
    size_t length = 0;
    length = cuid(result);
    munit_logf(MUNIT_LOG_INFO, "cuid: %s", result);
    munit_assert_size(length, >, 0);
    munit_assert_char(result[0], ==, 'c');

    char timestamp_result[CUID_BASE36_RESULT_SIZE] = {0};
    size_t timestamp_length = cuid_base36(CUID_GET_TIMESTAMP(),
                                          timestamp_result);
    // The timestamp must be available at the CUID
    munit_logf(MUNIT_LOG_INFO, "timestamp: %s", timestamp_result);
    for (size_t i = 0; i < timestamp_length; ++i) {
      munit_assert_char(result[1 + i], ==, timestamp_result[i]);
    }

    // Counter should start as 1
    char counter_result[CUID_BASE36_RESULT_SIZE] = {0};
    size_t counter_length = cuid_base36_pad(1, counter_result, 4, '0');
    for (size_t i = 0; i < counter_length; ++i) {
      munit_assert_char(result[1 + timestamp_length + i], ==, counter_result[i]);
    }

    // Fingerprint should have 4 chars
    char fingerprint_result[CUID_FINGERPRINT_SIZE] = {0};
    size_t fingerprint_length = CUID_GET_FINGERPRINT(fingerprint_result);
    munit_assert_size(fingerprint_length, ==, 4);
    munit_logf(MUNIT_LOG_INFO, "fingerprint: %s", fingerprint_result);
    // Fingerprint should be present
    for (size_t i = 0; i < fingerprint_length; ++i) {
      munit_assert_char(result[1 + timestamp_length + counter_length + i],
          ==, fingerprint_result[i]);
    }
    // The last 8 chars should be different
    char result2[CUID_SIZE] = {0};
    cuid(result2);
    munit_logf(MUNIT_LOG_INFO, "cuid1: %s; cuid2: %s", result, result2);
    // The last 8 chars should be random
    size_t sum1 = 0;
    size_t sum2 = 0;
    for (size_t i = 0; i < length; ++i) {
      if (i > length - 8) {
        sum1 += (unsigned long)result[i];
        sum2 += (unsigned long)result2[i];
      } 
    }
    munit_assert_size(sum1, !=, sum2);

    // The counter should be increased
    munit_logf(MUNIT_LOG_INFO, "counter1: %c; counter2: %c", result[1 + timestamp_length + 3], result2[1 + timestamp_length + 3]);
    munit_assert_char(result[1 + timestamp_length + 3], !=, result2[1 + timestamp_length + 3]);

    return MUNIT_OK;
}

/*
** Test that the `cuid_create()` function works as expected.
*/
static MunitResult
test_create(const MunitParameter params[], void* data) {
    char fingerprint_result[CUID_FINGERPRINT_SIZE] = {0};
    size_t fingerprint_length = CUID_GET_FINGERPRINT(fingerprint_result);
    cuid_t id = cuid_create(fingerprint_result);

    // Should set the fingerprint
    munit_logf(MUNIT_LOG_INFO, "fingerprint: %s", fingerprint_result);
    // Fingerprint should be present
    for (size_t i = 0; i < fingerprint_length; ++i) {
      munit_assert_char(id.cuid_fingerprint[i], ==, fingerprint_result[i]);
    }

    // Should start with cuid_value as 0
    for (size_t i = 0; i < CUID_SIZE; ++i) {
      munit_assert_char(id.cuid_value[i], ==, 0);
    }

    return MUNIT_OK;
}

/*
** Test that the `cuid_init()` function works as expected.
*/
static MunitResult
test_init(const MunitParameter params[], void* data) {
    char fingerprint_result[CUID_FINGERPRINT_SIZE] = {0};
    CUID_GET_FINGERPRINT(fingerprint_result);
    cuid_t id = cuid_create(fingerprint_result);

    unsigned long const timestamp = 123456789;
    // Set a value, to make sure it is cleared after
    id.cuid_value[0] = 'c';

    id = cuid_init(id, timestamp);

    // Counter should start at 0
    unsigned int counter_value = CUID_READ_COUNTER(id.cuid_counter);
    munit_assert_uint(counter_value, ==, 0);

    // Random should return a pure random number
    uint32_t rnd_value = CUID_READ_RANDOM(id.cuid_rnd1);
    munit_assert_uint32(rnd_value, >, 0);
    uint32_t rnd_value2 = CUID_READ_RANDOM(id.cuid_rnd1);
    munit_assert_uint32(rnd_value, ==, rnd_value2);
    uint32_t rnd_value3 = CUID_READ_RANDOM(id.cuid_rnd2);
    munit_assert_uint32(rnd_value, >, 0);
    uint32_t rnd_value4 = CUID_READ_RANDOM(id.cuid_rnd2);
    munit_assert_uint32(rnd_value3, ==, rnd_value4);

    // Timestamp should be set as a base36 string of the provided number
    char expected_timestamp[] = "21i3v9";
    munit_logf(MUNIT_LOG_INFO, "timestamp: %s", id.cuid_timestamp);
    for (size_t i = 0; i < CUID_TIMESTAMP_LENGTH; ++i) {
      munit_assert_char(id.cuid_timestamp[i], ==, expected_timestamp[i]);
    }

    // Value should have the new string
    munit_logf(MUNIT_LOG_INFO, "cuid: %s", id.cuid_value);
    for (size_t i = 0; i < CUID_SIZE - 1; ++i) {
      munit_assert_char(id.cuid_value[i], !=, 0);
    }
    // First chars should be "c21i3v90000"
    munit_assert_char(id.cuid_value[0], ==, 'c');

    size_t fingerprint_length = CUID_GET_FINGERPRINT(fingerprint_result);
    munit_assert_size(fingerprint_length, ==, 4);
    munit_logf(MUNIT_LOG_INFO, "fingerprint: %s", fingerprint_result);
    // Fingerprint should be present
    for (size_t i = 0; i < fingerprint_length; ++i) {
      munit_assert_char(id.cuid_fingerprint[i], ==, fingerprint_result[i]);
    }

    return MUNIT_OK;
}

/*
** Test that the `cuid_read()` function works as expected.
*/
static MunitResult
test_read(const MunitParameter params[], void* data) {
    cuid_t id = cuid_create("fing");

    id = cuid_init(id, 123456789);
    // Read the first value
    char cuid1[CUID_SIZE] = {0};
    cuid_read(id, cuid1);
    munit_logf(MUNIT_LOG_INFO, "cuid1: %s", cuid1);
    char expected_cuid1[] = "c21i3v90000fing";
    for (size_t i = 0; i < sizeof(expected_cuid1) - 1; ++i) {
      munit_assert_char(cuid1[i], ==, expected_cuid1[i]);
    }

    // Should not change in multiple read calls
    char cuid1_equals[CUID_SIZE] = {0};
    cuid_read(id, cuid1_equals);
    munit_assert_string_equal(cuid1, cuid1_equals);

    // Should produce a new value when the cuid is advanced with next()
    id = cuid_next(id, 223456789);
    char cuid2[CUID_SIZE] = {0};
    cuid_read(id, cuid2);
    munit_logf(MUNIT_LOG_INFO, "cuid2: %s", cuid2);
    char expected_cuid2[] = "c3p1gd10001fing";
    for (size_t i = 0; i < sizeof(expected_cuid2) - 1; ++i) {
      munit_assert_char(cuid2[i], ==, expected_cuid2[i]);
    }

    return MUNIT_OK;
}

/*
** The main() function is included to be able to run the cuid tests directly in
** the CLI. This function is the unit tests entry-point.
*/
int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    // The array of tests to run
    static MunitTest all_tests[] = {
        { (char*) "test_id_exists", test_id_exists,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_fingerprint_exists", test_fingerprint_exists,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_fingerprint",
          test_fingerprint,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_counter_exists",
          test_counter_exists,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_can_convert_to_base36",
          test_can_convert_to_base36,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_padded_base36",
          test_padded_base36,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_has_counter",
          test_has_counter,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_timestamp",
          test_timestamp,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_has_random",
          test_has_random,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_cuid",
          test_cuid,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_create",
          test_create,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_init",
          test_init,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { (char*) "test_read",
          test_read,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    };
    // Single test:
    static MunitTest single_test[] = {
        { (char*) "test_init",
            test_init,
            0, 0, MUNIT_TEST_OPTION_NONE, 0 },
        { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    };
    const MunitSuite test_suite = {
        // This string will be prepended to all test names in this suite
        (char*) "cuid/",
        // The array of test suites.
        // single_test,
        all_tests,
        // Suites can contain other test suites. This isn't necessary for
        // cuid
        NULL,
        // Number of iterations of each test
        1,
        // Use the munit default settings
        MUNIT_SUITE_OPTION_NONE
    };
    return munit_suite_main(&test_suite, (void*) "munit", argc, argv);
}
#endif /* CUID_TESTS_H */
