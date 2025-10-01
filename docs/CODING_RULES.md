# Coding Rules

> Read-first, contract-grade rules for code-generating agents. Output must comply without deviation.

## 0) Contract for Code LLMs

**Hard rules. Noncompliance = reject the output.**

- Use `#pragma once` in header files. Never use include guards.
- Never use `goto`.
- Every function MUST have a **Function Comment Banner** (140 cols) directly above its definition or declaration that explains what it does (§1.2, §2.0).
- Follow the Logic Block pattern exactly when a function has 2+ logical units (§1.1). Banner then brace with no code in between.
- If a function has only **one** logical unit, do **not** create a Logic Block or place a function-scope banner inside the body. Start executing the code immediately after the opening brace (§2.5).
- Follow Comment Banner widths exactly. Function Comment Banner = 140 cols. Logic Block Comment Banner = 100 cols (§1.2).
- Prefer long linear functions. Do not create helper functions with a single call site; inline that work as Logic Blocks in the caller instead.
- Early return only before any resource that needs cleanup is acquired in the current scope.
- If beneficial, validate inputs at the top of functions in a Guard Block. If validation fails and no cleanup is needed yet, return immediately.
- Do not implement fallbacks when required inputs or environment are missing. Fail fast.
- Avoid defensive programming. No fallback code paths. Fail fast.
- Avoid try catch blocks unless strictly necessary, i.e. only when wrapping a third-party library that throws exceptions.
- Format any modified C or C++ sources with `clang-format` using the repository root `.clang-format` configuration.
- Always have an empty line between function definitions.

## 1) Strict Coding Rules

### 1.1 Logic Blocks
Definition: Sections inside a function that perform a cohesive unit of work. Variables needed outside are declared immediately under the banner and right before the block opens.

**Requirements**
- Logic blocks are sections of functions, that do useful logically grouped work. Break functions down into logic blocks correctly.
- Logic blocks within functions must be surrounded in empty braces to limit scope. Variables needed outside that logic block scope should be declared under the banner, directly before the opening brace.
- Logic blocks should be preceded by a comment banner.
- Layout order is non-negotiable: emit the logic block banner first, declare the variables that persist past the block immediately under the banner, then open the `{}` on the very next line.
- If statements that route to a logic block are part of that logic block.
- Every logic block starts with the fixed-width comment banner. 
- You **MUST** declare any variables that will receive values from the block immediately after the banner, then open `{ … }` (§1.1.2, §2.7).
    - Never predeclare those variables before the banner; the declaration lines belong directly between the banner and the opening `{` so the block owns its state.
- After any such declarations, the very next line must be the opening `{` for the logic block; never place an `if`, `while`, or other control flow before that brace.
- When a logic block needs a guard condition, open the block first, then put the `if` inside it (§1.1.1).
- Functions that contain a single cohesive unit of work must not include logic blocks.

**Canonical template (copy-paste):**
```c
// =================================================================================================
// =================================================================================================
// short description of the work this logic block performs.
// =================================================================================================
// =================================================================================================
size_t produced_value; // declare values needed outside the block here
{
    if (condition)
    {
        perform_action();
        perform_other_action();
        produced_value = calc_value();
    }
}
```

#### 1.1.1 Conditions are part of the Logic Block Example
**Incorrect example:**
```c
if (sigaction(SIGINT, &sa, nullptr) != 0)
{
	// =====================================================================================
	// =====================================================================================
	// Fail fast when the process cannot register the SIGINT handler needed for graceful shutdown.  
	// =====================================================================================
	// =====================================================================================
	fprintf(stderr, "Failed to install SIGINT handler\n");
	startup_ok = false;
}
```

**Correct example:**
```c
// =====================================================================================
// =====================================================================================
// Fail fast when the process cannot register the SIGINT handler needed for graceful shutdown.  
// =====================================================================================
// =====================================================================================
{
	if (sigaction(SIGINT, &sa, nullptr) != 0)
	{
		
		fprintf(stderr, "Failed to install SIGINT handler\n");
		startup_ok = false;
	}
}
```

#### 1.1.2 Values Produced By Logic Blocks Example
**Incorrect example:**
```c
const char* params[3]      = {nullptr, nullptr, nullptr};
ExampleResult* exec_result = nullptr;
bool statement_success     = false;
ExampleStatus status       = EXAMPLE_STATUS_OK;

// =============================================================================================
// =============================================================================================
// Prepare positional parameters for the INSERT statement.
// =============================================================================================
// =============================================================================================
{
    params[0] = input->first_value;
    params[1] = input->second_value;
    params[2] = input->third_value;
}

// =============================================================================================
// =============================================================================================
// Execute the insert and capture the generated identifier for diagnostics.
// =============================================================================================
// =============================================================================================
{
    exec_result = example_execute_statement(
        connection->handle,
        "INSERT INTO example_table (first_value, second_value, third_value) "
        "VALUES ($1, $2, $3) "
        "RETURNING id",
        3,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (!exec_result)
    {
        return example_set_error(connection, EXAMPLE_ERR_EXECUTION, example_last_error(connection->handle));
    }

    statement_success = example_result_status(exec_result) == EXAMPLE_RESULT_ROWS && example_row_count(exec_result) == 1;

    if (!statement_success)
    {
        status = example_set_error(connection, EXAMPLE_ERR_EXECUTION, example_last_error(connection->handle));
    }
}
```

**Correct example:**
```c
// =============================================================================================
// =============================================================================================
// Prepare positional parameters for the INSERT statement.
// =============================================================================================
// =============================================================================================
const char* params[3] = {nullptr, nullptr, nullptr};
{
    params[0] = input->first_value;
    params[1] = input->second_value;
    params[2] = input->third_value;
}

// =============================================================================================
// =============================================================================================
// Execute the insert and capture the generated identifier for diagnostics.
// =============================================================================================
// =============================================================================================
ExampleResult* exec_result;
bool statement_success;
ExampleStatus status = EXAMPLE_STATUS_OK;
{
    exec_result = example_execute_statement(
        connection->handle,
        "INSERT INTO example_table (first_value, second_value, third_value) "
        "VALUES ($1, $2, $3) "
        "RETURNING id",
        3,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (!exec_result)
    {
        return example_set_error(connection, EXAMPLE_ERR_EXECUTION, example_last_error(connection->handle));
    }

    statement_success = example_result_status(exec_result) == EXAMPLE_RESULT_ROWS && example_row_count(exec_result) == 1;

    if (!statement_success)
    {
        status = example_set_error(connection, EXAMPLE_ERR_EXECUTION, example_last_error(connection->handle));
    }
}
```

**Never forget:** If a logic block assigns or mutates values that will be read outside the block, declare those variables directly underneath the banner that introduces the block, then open the brace. Do not scatter related declarations earlier in the function; keep them adjacent so §1.1.2 is satisfied every time.


### 1.2 Comment Banners and Function Headers

Two fixed widths:
- **Function Comment Banner**: 140 columns.
- **Logic Block Comment Banner**: 100 columns.

General rule:
1. Every banner line must be the exact fixed width (140 or 100), counting indent.
2. Prefix = `// ` (3 chars).
3. Padding lines use `=` repeated **Width - Indent - 3** times.
4. Text lines must be padded so they reach the exact width; never exceed the banner width.
5. Always use 2 padding lines above and below the text section.

Formulas:
- Function Comment Banner: `E = 140 - I - 3`, `T = 140 - I - 3`.
- Logic Block Comment Banner: `E = 100 - I - 3`, `T = 100 - I - 3`.

### 1.3 Functions
- Long functions are preferred.
- Prioritise keeping execution linear.
- Never keep a helper function that is only called once; inline that code as a Logic Block in the caller function.
- Functions should ideally either return early, or return at the end. Avoid returning in the middle of a long function.
- Functions should, if beneficial, validate their input params and inherited state in a guard block at the start of the function, and return early if invalid.
- Functions are allowed to return early as long as they haven't yet created the need to cleanup resources.
- Functions that need to cleanup resources (e.g. freeing pointers, or closing libraries) should try to do that cleanup work in one section of the function. 
  - Maintaining booleans across a function can help with this.

### 1.4 Strategy
- If a program is an executable, it should begin by immediately validating all systemd credentials, environment variables, cli args, etc, that it will depend on.
- Avoid implementing fallback code paths. If something doesn't work as expected, or isn't present as expected, exit the program as a failure.
- Cleanup Clause: Early returns are strictly only acceptable as long as no resources which need cleanup have been initialised in this scope yet.
- Guard Blocks: At the top of each function, validate required inputs and runtime prerequisites. If any validation fails, and the cleanup clause is valid, return immediately; do not defer
  the failure or gate later logic with additional flags.
- Control Flow: When a fatal precondition isn’t met (e.g., missing environment variables, init failures), and the cleanup clause is valid, return directly from the function rather than
  setting state for later checks.

## 2) Templates to study and mimic

### 2.0 Function Header Block (required above every function)
```c
// =========================================================================================================================================
// =========================================================================================================================================
// can explain any quirks about the function here
// =========================================================================================================================================
// =========================================================================================================================================
int function_name(/* params */);
```

### 2.1 Typical Function Example (Multiple Logic Blocks)
```c
// =========================================================================================================================================
// =========================================================================================================================================
// do_thing: Does one cohesive operation. Inputs: a,b. Returns 0 on success.
// Side effects: writes to fd if provided.
// =========================================================================================================================================
// =========================================================================================================================================
int do_thing(int a, int b)
{
    // =============================================================================================
    // =============================================================================================
	// Function-level context for the single operation performed.
    // =============================================================================================
    // =============================================================================================
    {
		if (a <= 0 || b <= 0)
		{
			return -1;
		}
	}

    // =============================================================================================
    // =============================================================================================
    // =============================================================================================
    // =============================================================================================
	{
    	// perform the single operation directly here
	}
	
	return 0;
}
```

### 2.2 Guard Block at Function Start
```c
int run_service(const char* conninfo, int port)
{
    // =============================================================================================
    // =============================================================================================
	// Guard: Validate required inputs and runtime prerequisites.
    // =============================================================================================
    // =============================================================================================
	{
		if (conninfo == nullptr || port <= 0)
		{
			return -1; // invalid input; can return early as no resources acquired yet
		}
	}

    // =============================================================================================
    // =============================================================================================
    // =============================================================================================
    // =============================================================================================
    bool init_ok;
	void* resource_ptr;
	{
    	// ... acquire resources
	}

    // =============================================================================================
    // =============================================================================================
	// Cleanup section: release resources once at the end.
    // =============================================================================================
    // =============================================================================================
	{
		if (!init_ok)
		{
			// release any partially acquired resources
			free(resource_ptr);
		}
	}

    // =============================================================================================
    // =============================================================================================
    // =============================================================================================
    // =============================================================================================
    return init_ok ? 0 : -1;
}
```

### 2.3 Logic Block for Config Resolution Example
```c
// =============================================================================================
// =============================================================================================
// Load config strictly from environment. Fail fast if any required key is missing.
// =============================================================================================
// =============================================================================================
{
    const char* port_s = getenv("EXAMPLE_PORT");
    const char* service_endpoint = getenv("EXAMPLE_SERVICE_ENDPOINT");
    if (!port_s || !service_endpoint)
    {
		fprintf(stderr, "Missing env EXAMPLE_PORT or EXAMPLE_SERVICE_ENDPOINT\n");
        return EXIT_FAILURE; // Allowed by cleanup clause if no resources yet
    }
}
```

### 2.4 File-scope Header Block example
```c
// =========================================================================================================================================
// =========================================================================================================================================
// example_service: placeholder REST server exposing GET /resource and GET /resource/{id}.
// =========================================================================================================================================
// =========================================================================================================================================
```

### 2.5 Function with a single cohesive operation doesn't need a Logic Block Example
```c
// =========================================================================================================================================
// =========================================================================================================================================
// do_thing: Does one cohesive operation.
// =========================================================================================================================================
// =========================================================================================================================================
int do_thing(int a, int b)
{
    if (a <= 0 || b <= 0)
    {
        return -1;
    }

    return a * b + b + a;
}
```

### 2.6 Single-Unit Helper with Side Effects Example
```c
// =========================================================================================================================================
// =========================================================================================================================================
// restore_env: Restore an environment variable to its saved value after test mutation.
// =========================================================================================================================================
// =========================================================================================================================================
static void restore_env(const char* key, const char* saved)
{
    if (saved)
    {
        setenv(key, saved, 1);
        free((void*)saved);
    }
    else
    {
        unsetenv(key);
    }
}
```

### 2.7 Logic Block that produces values Example
```c
// =================================================================================================
// =================================================================================================
// description of logic block
// =================================================================================================
// =================================================================================================
bool init_ok;
void* acquired_ptr;
{
    param = acquire_param();
    init_ok = acquire_thing(param, &acquired_ptr);
}

// =================================================================================================
// =================================================================================================
// another logic block that uses the values produced earlier
// =================================================================================================
// =================================================================================================
{
    if (!init_ok) {
        do_something_else();
    }
}
```

## 3) Language-Specific Rules

### 3.1 C23
- Use `nullptr`, `bool`, and `static_assert`.

### 3.2 C++26
- Write mostly C-style C++. You can use namespaces, std::string, std::vector, std::unordered_map, or other simple C++ features.

### 3.3 Rust
- Format with ```cargo +nightly fmt```.
- Prioritise simplicity and reducing lines of code.
- Never introduce `type` aliases (Rust typedefs); use explicit structs or enums when new types are required.

### 3.4 TypeScript (Bun)
- Strictly use the Bun runtime.
- React JSX/TSX doesn't need formatting.
- Strictly use snake_case.