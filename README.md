# cuid-c

Cuids are "Collision-resistant ids optimized for horizontal scaling and binary search lookup performance."

This is an implementation of [cuid](https://github.com/ericelliott/cuid/) in C.

All of the code is done in the `cuid.h` file.

API
---

Two API's are provided, the simplest and more convenient one is just the single
function `cuid()`.

- `size_t cuid(char[24])`:
  * Generates a cuid string and places it in the provided array.
    The string created is '\0' terminated and has exactly 23 + 1 chars.
    The length of the string is returned.

There are macros defined for each syscall that can be overrided in order to
match intended use cases. For more information on these please read the
source code of `cuid.h`.

### Pure API

An optional pure API is provided that can create cuid strings through pure
functions that pass along a `cuid_t` state. This can be useful if you want
better control over the moving parts of the cuid creation.

Define the macro CUID_PURE to enable the following functions:

- `cuid_t cuid_create(char const[static 5])`
  * Creates the `cuid_t` state, and recieves the fingerprint string as argument.
    The same fingerprint string is used in all of the cuid's created by the
    returned `cuid_t`.
    This function should be called once at the start.
- `cuid_t cuid_init(cuid_t, unsigned long const)`
  * Initializes a `cuid_t` state by setting the initial counter and
    random number generator arrays.
    It recieves a timestamp as the second arg.
    Returns an initialized `cuid_t`.
- `void cuid_read(cuid_t, char[static 24])`
  * Returns the cuid string associated with the `cuid_t` state provided as arg.
    This function copies the cuid string chars into the char array at the
    second argument.
    It always returns the same result for the same `cuid_t`. To create a new
    different cuid string a new `cuid_t` state is needed, to create it call
    `cuid_next`.
- `cuid_t cuid_next(cuid_t, unsigned long const)`
  * Creates a new `cuid_t` based on the provided `cuid_t` argument.
    This function recieves a timestamp `unsigned long` and returns a new
    `cuid_t` state that is one cycle away from the provided one.

On top of these a lot of customization can be achieved by redefining the macros
that the pure API uses. All of the random and counter creation/initialization
and generation functions can be overriden to match very specific needs.

Read the code of `cuid.h` after the `CUID_PURE` macro is defined if you want
to know more on how to override the counter and random number generator.


Example Usage
-------------

```C
#include "cuid.h"

// Create space for a cuid string:
char my_cuid[CUID_SIZE] = {0};

// Call the `cuid()` function and pass it the space for the cuid string.
cuid(my_cuid);

// A cuid string is now available in the `my_cuid` var.
```


Here is an example of the optional pure API.

```C
#define CUID_PURE (1)
#include "cuid.h"

// Use any fingerprint function, here just a static string for example purposes
char const fingerprint[] = "ipad";

// The flow for the pure api is:
// create -> init -> read -> next -> (read -> next ->...)
cuid_t id = cuid_create(fingerprint);

// Use any specific timestamp functions, or just pass along an unsigned long
// for example purposes
unsigned long timestamp = 123456789;
id = cuid_init(id, timestamp);

// Create space for the cuid strings:
char my_first_cuid[CUID_SIZE] = {0};
char my_second_cuid[CUID_SIZE] = {0};

// Read the first cuid
cuid_read(id, my_first_cuid);

// Advance the cuid_t state (use a specific timestamping function, here
// just increasing the timestamp for example purposes).
id = cuid_next(id, ++timestamp);

// Read the second cuid
cuid_read(id, my_second_cuid);

// The cuids `my_first_cuid` and `my_secund_cuid` are different, however
// note that calling `cuid_read` multiple times without doing a `cuid_next`
// will always write the same string value to the provided array.
```

