#ifndef CUID_H
/*
**
**     ::::::::  :::    ::: ::::::::::: :::::::::  
**    :+:    :+: :+:    :+:     :+:     :+:    :+: 
**    +:+        +:+    +:+     +:+     +:+    +:+ 
**    +#+        +#+    +:+     +#+     +#+    +:+ 
**    +#+        +#+    +#+     +#+     +#+    +#+ 
**    #+#    #+# #+#    #+#     #+#     #+#    #+# 
**     ########   ########  ########### ######### 
**
** This is an C implementation of cuid.
**
** It provides a single function `cuid()` that fills a char array with a cuid
** string. Care was taken to ensure similar outputs as the default Node JS
** implementation at https://github.com/ericelliott/cuid/
**
** All of the code is defined in this single .h file that you can include in
** your projects.
**
** An optional pure API is provided if you define the macro CUID_PURE.
**
** This implementation provides ample configuration points, most of it can be
** customized to ensure integration with current existing id schemes or other
** cuid implementations.
**
** The default implementation uses BSD functions. These include
** - `arc4random` as the 32bit number generator (can be overriden by the
**      macro `MWC_SYSTEM_RAND32` to be any other function.
** - `getpid` and `gethostname` for the fingerprint (can be overriden by
**      declaring a custom fingerprint generator function in the macro
**      CUID_GET_FINGERPRINT)
** - `time` for the timestamp (can be overriden by setting a timestamp
**      function in the macro CUID_GET_TIMESTAMP)
**
** If the pure API is used then many other points of the generation can be
** configured as well. To read more about the pure API follow through the code
** after the `#ifdef CUID_PURE` macro.
**
*/
#define CUID_H (1)
#include <stddef.h> // size_t
#include <stdint.h> // uint32_t, uint8_t, uint64_t
#include <stdio.h> // perror
/*-- MARK: Public API Implementation -----------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*
** The main `cuid()` function. This function is intended to be called as
** a standalone function. It makes use of `static` vars within its block.
**
** It calls the following functions:
** - The fingerprint function defined by `CUID_GET_FINGERPRINT`.
** - The timestamp function defined by `CUID_GET_TIMESTAMP`.
** - The random function defined by `MWC_SYSTEM_RAND32`.
**
** It is recomended that the pure interface provided below is used instead,
** however, for effortless and quick generation of cuid's this function does
** its job.
**
** It recieves the char array where the generated cuid will be placed at.
** The resulting cuid is '\0' terminated and a length of 23 chars + '\0'.
**
** This function returns the length of the final cuid string written to the
** result array.
*/
#define CUID_SIZE (24)
size_t cuid(char result[static CUID_SIZE]);

/*
** Provide your timestamp function and define it as CUID_GET_TIMESTAMP.
** This function should recieve no arguments and return 
** a `unsigned long` number.
**
** The default is `time()` from the <time.h> C standard lib.
*/
#ifndef CUID_GET_TIMESTAMP
#include <time.h> // time()

static inline unsigned long
cuid_get_timestamp(void) {
  return (unsigned long)time(0x0);
}
#define CUID_GET_TIMESTAMP cuid_get_timestamp
#endif /* CUID_GET_TIMESTAMP */

/*
** Converts a number to a base36 string.
** Places the resulting string in the array provided as argument.
** Returns the length of the generated base36 string.
*/
#define CUID_BASE36_RESULT_SIZE 16
static inline size_t
cuid_base36(uint64_t cuid_number,
            char cuid_result[static CUID_BASE36_RESULT_SIZE]) {
  // Table used to convert a number to base 36.
  static char const cuid_radix36_table[] =
    "0123456789abcdefghijklmnopqrstuvwxyz";

  char cuid_inverted_result[CUID_BASE36_RESULT_SIZE] = {0};
  size_t const base = sizeof cuid_radix36_table - 1;
  size_t cuid_length = 0;

  do {
    cuid_inverted_result[cuid_length] = cuid_radix36_table[cuid_number % base];
    cuid_number /= base;
    cuid_length++;
  } while (cuid_number > 0 && cuid_length < CUID_BASE36_RESULT_SIZE - 1);

  // Invert the result
  for (size_t i = 0; i <= cuid_length - 1; ++i) {
    cuid_result[i] = cuid_inverted_result[cuid_length - 1 - i];
  }
  cuid_result[cuid_length] = '\0';

  return cuid_length;
}

/*
** A padded version of base36.
** Makes use of CUID_EXIT which by default is set at the `exit()` from
** `<stdlib.h>` 
*/
#ifndef CUID_EXIT
#include <stdlib.h> // exit, EXIT_FAILURE
#define CUID_EXIT exit
#endif // CUID_EXIT
static inline size_t
cuid_base36_pad(uint64_t cuid_number,
                char cuid_result[static CUID_BASE36_RESULT_SIZE],
                uint8_t const pad_length,
                char const pad_char) {
  if (pad_length >= CUID_BASE36_RESULT_SIZE) {
    perror("Using cuid_base36_pad function with a pad_length greater "
           "than the provided cuid_result array length");
    CUID_EXIT(EXIT_FAILURE);
  }
  size_t const base36_str_len = cuid_base36(cuid_number, cuid_result);

  // Check if padding adjustment is necessary
  if (base36_str_len < pad_length) {
    // Shift all characters to the right, and fill the rest with the
    // pad_char
    for (size_t i = 0; i <= pad_length; ++i) {
      cuid_result[pad_length - i] = i > base36_str_len ? pad_char
                                  : cuid_result[base36_str_len - i];
    }
  } else if (base36_str_len > pad_length) {
    // Clip at the end of the string by moving all characters to
    // the left
    size_t const diff = base36_str_len - pad_length;
    for (size_t i = 0; i <= pad_length; ++i) {
      cuid_result[i] = cuid_result[i + diff];
    }
  }
  return pad_length;
}


/*
** Provide your fingerprint generation function and define it as
** CUID_GET_FINGERPRINT.
**
** The default is the base36 string of the number made from the current pid +
** the sum of the `gethostname()` chars.
** Both the `getpid` and `gethostname` functions come from
** the <unistd.h> C lib.
*/
#define CUID_FINGERPRINT_SIZE (5)
#ifndef CUID_GET_FINGERPRINT
#include <unistd.h> // gethostname, getpid

static inline size_t
cuid_get_fingerprint(char result[static CUID_FINGERPRINT_SIZE]) {
#define CUID_HOSTNAME_LENGTH 256
  // A different array is used for the hostname to prevent `gethostname` to
  // to return the ENAMETOOLONG error. A hostname max value is 256 chars,
  // according to the man page of `gethostname`
  union {
    char str[CUID_HOSTNAME_LENGTH];
    // This union provides the number representation of a hostname
    // which is useful to sum all chars into a number used to create
    // the base36 resulting string.
    uint32_t values[CUID_HOSTNAME_LENGTH / 4];
  } hostname = {0};

  int error_code = gethostname(hostname.str, CUID_HOSTNAME_LENGTH);
  if (error_code != 0) {
    perror("Error getting the hostname");
    CUID_EXIT(EXIT_FAILURE);
  }
  // Add the hostname chars into an unsigned int to be converted to
  // a base36 string.
  // The number starts at the current process pid to
  // try to match the original implementation of cuid in
  // https://github.com/ericelliott/cuid/blob/master/lib/fingerprint.js
  uint64_t hostname_number = 0;
  for (size_t i = 0; i < CUID_HOSTNAME_LENGTH / 4; i++) {
    hostname_number += hostname.values[i];
  }
  // Convert to base36
  size_t length = cuid_base36_pad(hostname_number, result, 2, '0');

  uint64_t pid = (uint64_t)getpid();
  // Returns the length of the base36 hostname
  length += cuid_base36_pad(pid, &result[2], 2, '0');
  return length;
}

#define CUID_GET_FINGERPRINT cuid_get_fingerprint

#endif /* CUID_GET_FINGERPRINT */

#ifdef CUID_PURE
/*
** The cuid() pure API
**
** This is an optional API that provides a set of
** functions that depend only on their arguments to create a cuid string. 
** This can be useful if you want to partially build your cuid's or integrate
** with some other language cuid library through its FFI.
**
** All functions are declared as static inline to prevent copies.
** To use this library please set -DCUID_PURE in your compiler arguments
** or define the CUID_PURE macro in the code that includes this .h file.
**
*/

/*
** The counter is split in several functions, most of them are pure and
** should have no side-effects:
**
** - CUID_CREATE_COUNTER: creates the CUID_COUNTER_T data type
**   * with type: `COUNTER_T CREATE_COUNTER(void);`
** - CUID_INIT_COUNTER: initializes the created CUID_COUNTER_T with the startup
**   value(s)
**   * with type: `CUID_COUNTER_T CUID_INIT_COUNTER(CUID_COUNTER_T);`
** - CUID_READ_COUNTER: returns the counter value (unsigned int)
**   * with type: `unsigned CUID_READ_COUNTER(CUID_COUNTER_T);`
** - CUID_INCREASE_COUNTER: increases counter value 
**   * with type: `CUID_COUNTER_T CUID_INCREASE_COUNTER(CUID_COUNTER_T);`
**
** In order to override the cuid counter with a specific implementation
** please define them all, as well as the CUID_COUNTER_T type, be sure that
** they can be called during startup as:
**
**   CUID_COUNTER_T c = CUID_INIT_COUNTER(CUID_CREATE_COUNTER())
**
** And then at each call as:
**
** CUID_COUNTER_T next_counter = CUID_INCREASE_COUNTER(c)
** unsigned value = CUID_READ_COUNTER(next_counter)
**
** A default implementation is provided which increments a simple variable as
** `c++;` // this is not subliminal
**
*/
#ifndef CUID_COUNTER_T
typedef struct cuid_counter_t {
  unsigned value;
} cuid_counter_t;
#define CUID_COUNTER_T cuid_counter_t

static inline cuid_counter_t cuid_create_counter(void) {
  return (cuid_counter_t){0};
}
#define CUID_CREATE_COUNTER cuid_create_counter

static inline cuid_counter_t
cuid_init_counter(cuid_counter_t c) {
  c.value = 0U;
  return c;
}
#define CUID_INIT_COUNTER cuid_init_counter

static inline unsigned
cuid_read_counter(cuid_counter_t const c) {
  return c.value;
}
#define CUID_READ_COUNTER cuid_read_counter

static inline cuid_counter_t
cuid_inc_counter(cuid_counter_t c) {
  c.value++;
  return c;
}
#define CUID_INCREASE_COUNTER cuid_inc_counter

#endif // CUID_COUNTER_T

/*
** The random number generator is also split in several functions, these follow
** the same logic and approach as the COUNTER_T above.
** 
** After creation the functions are intended to be pure and should have
** no side-effects:
**
** - CUID_CREATE_RANDOM: creates the CUID_RANDOM_T data type, this is not meant
**       to be a pure function.
**   * with type: `CUID_RANDOM_T CUID_CREATE_RANDOM();`
** - CUID_INIT_RANDOM: initializes the created CUID_RANDOM_T with the startup
**       value(s), this is a pure function.
**   * with type: `CUID_RANDOM_T CUID_INIT_RANDOM(CUID_RANDOM_T);`
** - CUID_READ_RANDOM: returns the random value (uint32_t), this is a pure
**       function
**   * with type: `uint32_t CUID_READ_RANDOM(CUID_RANDOM_T);`
** - CUID_NEXT_RANDOM: generates another random value, this is a pure
**       function
**   * with type: `CUID_RANDOM_T CUID_NEXT_RANDOM(CUID_RANDOM_T);`
**
** In order to override the cuid random with a specific implementation
** please define them all, as well as the CUID_RANDOM_T type, be sure that
** they can be called during startup as:
**
**   CUID_RANDOM_T r = CUID_INIT_RANDOM(CUID_CREATE_RANDOM())
**
** And then at each call as:
**
** CUID_RANDOM_T next_random = CUID_NEXT_RANDOM(r)
** uint32_t value = CUID_READ_RANDOM(next_random)
**
** A default implementation is provided which uses MWC[0] with stdlib.h rand().
**
** A random number generator must return a uint32_t.
**
** [0] - https://github.com/HugoDaniel/mwc
*/
#ifndef CUID_RANDOM_T

#ifndef MWC_SYSTEM_RAND32
#include <stdlib.h> // arc4random()
#define MWC_SYSTEM_RAND32 arc4random
#endif /* MWC_SYSTEM_RAND32 */

#define MWC_CYCLE 4096         // as Marsaglia recommends
#define MWC_C_MAX 809430660    // as Marsaglia recommends

typedef struct mwc_random_t {
	uint32_t mwc_q[MWC_CYCLE];
	uint32_t mwc_carry;
	unsigned mwc_current_cycle;
	uint32_t mwc_initial_carry;
	uint32_t mwc_initial_q[MWC_CYCLE];
} mwc_random_t;

#define CUID_RANDOM_T mwc_random_t

/*
** Internal function that creates a carry that is guaranteed to be < MWC_C_MAX.
** This function uses the defined MWC_SYSTEM_RAND32 system random.
*/
static inline uint32_t mwc_initial_c(void) {
  uint32_t mwc_initial_carry = 0;

  do {
		mwc_initial_carry = MWC_SYSTEM_RAND32();
  } while (mwc_initial_carry >= MWC_C_MAX);

  return mwc_initial_carry;
}

/*
** Creates a new mwc random state. 
** Keeps a copy of the initial state to allow `mwc_init()` to
** be able to reset it to the same initial state, allowing for the 
** random numbers generation sequence to be replicated again if needed.
*/
static inline mwc_random_t
mwc_create() {
  uint32_t mwc_c = mwc_initial_c();
  mwc_random_t mwc_state = {
    .mwc_initial_carry = mwc_c,
    .mwc_carry = mwc_c,
    .mwc_current_cycle = MWC_CYCLE -1,
  };

 	for (size_t i = 0; i < MWC_CYCLE; i++) {
    mwc_state.mwc_q[i] = MWC_SYSTEM_RAND32();
    mwc_state.mwc_initial_q[i] = mwc_state.mwc_q[i];
  }

  return mwc_state;
}

#define CUID_CREATE_RANDOM mwc_create

/*
** Resets to the initial state.
*/
static inline mwc_random_t
mwc_init(mwc_random_t r) {
  r.mwc_carry = r.mwc_initial_carry;
  r.mwc_current_cycle = MWC_CYCLE -1;

 	for (size_t i = 0; i < MWC_CYCLE; i++) {
    r.mwc_q[i] = r.mwc_initial_q[i];
  }

  return r;
}

#define CUID_INIT_RANDOM mwc_init

/*
** Returns the random value for the supplied mwc_random_t state.
** This is a pure function, it always returns the same value for the
** same input argument.
**
** To generate a new random number call `state = mwc_next_random(state)` before
** calling this method.
*/
static inline uint32_t
mwc_read_random(mwc_random_t const state) {
  return state.mwc_q[state.mwc_current_cycle];
}

#define CUID_READ_RANDOM mwc_read_random

/*
** This function creates a new random state for the next random number.
** 
** To retrieve the random number created, call `mwc_read_random(state)` after
** this method.
*/
static inline mwc_random_t
mwc_next_random(mwc_random_t state) {
  uint64_t const a = 18782;	// as Marsaglia recommends
	uint32_t const m = 0xfffffffe;	// as Marsaglia recommends
	uint64_t t;
	uint32_t x;

	state.mwc_current_cycle = (state.mwc_current_cycle + 1) & (MWC_CYCLE - 1);
	t = a * state.mwc_q[state.mwc_current_cycle] + state.mwc_carry;
	/* Let c = t / 0xffffffff, x = t mod 0xffffffff */
	state.mwc_carry = t >> 32;
	x = (uint32_t)t + state.mwc_carry;
	if (x < state.mwc_carry) {
		x++;
		state.mwc_carry++;
	}

  state.mwc_q[state.mwc_current_cycle] = m - x;

  return state;
}

#define CUID_NEXT_RANDOM mwc_next_random

#endif // CUID_RANDOM_T

/*
** The data type for the state of the cuid pure API.
*/
typedef struct cuid_t {
  // Fingerprint, limited to 4 chars
  char cuid_fingerprint[CUID_BASE36_RESULT_SIZE];
  // Counter, limited to 4 chars
  CUID_COUNTER_T cuid_counter;
  // Temporary array to store the base36 string result for the counter
  char cuid_counter_str[CUID_BASE36_RESULT_SIZE];
  // Random values, limited to 4 chars, it uses two rng's that get
  // merged in the final 8 chars of the value.
  CUID_RANDOM_T cuid_rnd1;
  CUID_RANDOM_T cuid_rnd2;
  // Temporary arrays to store the base36 string results for the RNGs
  char cuid_rnd1_str[CUID_BASE36_RESULT_SIZE];
  char cuid_rnd2_str[CUID_BASE36_RESULT_SIZE];
  // The cuid string generated
  char cuid_value[CUID_SIZE];
  // The timestamp is limited to 6 chars, it uses a padded base36
  // string produced from an unsigned long int.
#define CUID_TIMESTAMP_LENGTH 6
  char cuid_timestamp[CUID_BASE36_RESULT_SIZE];
  // Each block of the cuid is made of 4 chars
#define CUID_BLOCK_LENGTH 4
} cuid_t;

/*
** Creates a `cuid_t` data type by calling the *_create functions
** for each of its attributes that need them to be called.
*/ 
static inline cuid_t
cuid_create(char const fingerprint[static CUID_FINGERPRINT_SIZE]) {
  cuid_t id = {
    .cuid_counter = CUID_CREATE_COUNTER(),
    .cuid_rnd1 = CUID_CREATE_RANDOM(),
    .cuid_rnd2 = CUID_CREATE_RANDOM(),
  };
  // Copy the fingerprint from the argument
  for (size_t i = 0; i < CUID_FINGERPRINT_SIZE; ++i) {
    id.cuid_fingerprint[i] = fingerprint[i];
  }
  return id;
}

/*
** Internal method that returns a cuid_t with a value string set
** from its state.
** This is used to generate a cuid in the value array, ready to be read,
** at the init and next functions.
*/
static inline cuid_t
cuid_gen_value_string(cuid_t id) {
  // Letter
  id.cuid_value[0] = 'c';
  // Timestamp
  for (size_t i = 0; i < CUID_TIMESTAMP_LENGTH; ++i) {
    id.cuid_value[1 + i] = id.cuid_timestamp[i];
  }
  // Counter
  cuid_base36_pad(
      CUID_READ_COUNTER(id.cuid_counter),
      id.cuid_counter_str,
      CUID_BLOCK_LENGTH,
      '0');
  // Copy the base36 counter string
  for (size_t i = 0; i < CUID_BLOCK_LENGTH; ++i) {
    id.cuid_value[i + CUID_TIMESTAMP_LENGTH + 1] = id.cuid_counter_str[i];
  }
  // Fingerprint (4 chars)
  for (size_t i = 0; i < CUID_BLOCK_LENGTH; ++i) {
    id.cuid_value[i + CUID_BLOCK_LENGTH + CUID_TIMESTAMP_LENGTH + 1] =
      id.cuid_fingerprint[i];
  }
  // Random block 1 (4 chars)
  cuid_base36_pad(
    CUID_READ_RANDOM(id.cuid_rnd1),
    id.cuid_rnd1_str,
    CUID_BLOCK_LENGTH,
    '0');
  // Copy the base36 random string
  for (size_t i = 0; i < CUID_BLOCK_LENGTH; ++i) {
    id.cuid_value[i + 2*CUID_BLOCK_LENGTH + CUID_TIMESTAMP_LENGTH + 1] =
      id.cuid_rnd1_str[i];
  }
  // Random block 2 (4 chars)
  cuid_base36_pad(
    CUID_READ_RANDOM(id.cuid_rnd2),
    id.cuid_rnd2_str,
    CUID_BLOCK_LENGTH,
    '0');
  // Copy the base36 random string
  for (size_t i = 0; i < CUID_BLOCK_LENGTH; ++i) {
    id.cuid_value[i + 3*CUID_BLOCK_LENGTH + CUID_TIMESTAMP_LENGTH + 1] =
      id.cuid_rnd2_str[i];
  }

  return id;
}

/*
** Initializes a cuid_t data type.
** Clears the cuid value and sets the timestamp to be the base36 string of the
** provided argument.
** Calls the initializes of each of the cuid_t attributes that need them to be
** called.
**
** Returns an initialized cuid_t.
*/
static inline cuid_t
cuid_init(cuid_t id, unsigned long const timestamp) {
  // Initialize the counter
  id.cuid_counter = CUID_INIT_COUNTER(id.cuid_counter);
  // Initialize the PRNG's
  id.cuid_rnd1 = CUID_INIT_RANDOM(id.cuid_rnd1);
  id.cuid_rnd2 = CUID_INIT_RANDOM(id.cuid_rnd2);
  // Set the timestamp
  cuid_base36_pad(timestamp, id.cuid_timestamp, CUID_TIMESTAMP_LENGTH, '0');
  // Clear the value
  for (size_t i = 0; i < CUID_SIZE; ++i) {
    id.cuid_value[i] = '\0';
  }
  // Generate a new value string and return the id
  return cuid_gen_value_string(id);
}

/*
** Reads the cuid string value from the provided cuid_t.
**
** The value is copied into the `destination` char array passed as arg.
*/
static inline void
cuid_read(cuid_t const id, char destination[static CUID_SIZE]) {
  for (size_t i = 0; i < CUID_SIZE; ++i) {
    destination[i] = id.cuid_value[i];
  }
}

/*
** Advances the provided cuid_t into the next state.
** This function creates the new random values and sets the timestamp string
** into the cuid value string.
**
** It returns a new cuid_t with the new state to be used to read the cuid
** string.
**
** A new cuid string can then be read from the returned id with `cuid_read`.
** 
*/
static inline cuid_t
cuid_next(cuid_t id, unsigned long const timestamp) {
  // Increase the counter
  id.cuid_counter = CUID_INCREASE_COUNTER(id.cuid_counter);
  // and the PRNGs,
  id.cuid_rnd1 = CUID_NEXT_RANDOM(id.cuid_rnd1);
  id.cuid_rnd2 = CUID_NEXT_RANDOM(id.cuid_rnd2);
  // and set the timestamp
  cuid_base36_pad(timestamp, id.cuid_timestamp, CUID_TIMESTAMP_LENGTH, '0');

  // Generate a new value string into the id, and return it
  return cuid_gen_value_string(id);
}

#endif // CUID_PURE



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CUID_H */
/*-- MARK: Internal Implementation -------------------------------------------*/
#if defined(CUID_IMPL) && !defined(CUID_IMPL_INCLUDED)
#define CUID_IMPL_INCLUDED (1)

/*
** Implements CUID as the string made of
** letter + timestamp + counter + fingerprint + random;
*/
size_t cuid(char result[static CUID_SIZE]) {
  static uint32_t counter = 0;
  size_t length = 1;
  // Letter
  result[0] = 'c';
  // Timestamp (padded and limited at 6 chars)
  length += cuid_base36_pad(CUID_GET_TIMESTAMP(), &result[1], 6, '0');
  // Counter (4 chars)
  length += cuid_base36_pad(++counter, &result[length], 4, '0');
  // Fingerprint (4 chars, 2 PID + 2 Hostname)
  length += CUID_GET_FINGERPRINT(&result[length]);
  // Random block 1 (4 chars)
  length += cuid_base36_pad(MWC_SYSTEM_RAND32(), &result[length], 4, '0');
  // Random block 2 (4 chars)
  length += cuid_base36_pad(MWC_SYSTEM_RAND32(), &result[length], 4, '0');

  result[CUID_SIZE - 1] = '\0';

  return length;
}

#endif // CUID_IMPL

/*-- MARK: Tests -------------------------------------------------------------*/
/*
** Include the unit tests framework if shapeaux is being built for tests
**/
#ifdef CUID_TESTS

#include "./tests/cuid_tests.h"
#endif /* CUID_TESTS */

