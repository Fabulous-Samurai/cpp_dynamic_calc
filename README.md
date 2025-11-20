🚀 The Architectural Evolution

This engine wasn't built in a day; it was forged through four distinct evolutionary phases, each tackling a specific software engineering nightmare:

Phase I: Breaking the Monolith (OCP & Dynamic Dispatch)

    The Obstacle: The classic "Switch-Case Hell." Adding a new operator meant modifying the core loop, violating the Open/Closed Principle and inviting spaghetti code.

    The Breakthrough: We moved to a Dynamic Dispatch system. Operators are no longer hardcoded logic but are treated as first-class citizens stored in std::map<string, std::function>.

    Result: You can now register new binary or unary operations at runtime without recompiling a single line of the core engine.

Phase II: Teaching Logic to the Machine (Shunting-Yard)

    The Obstacle: A simple calculator fails when faced with complexity like 3 + 5 * 2. It lacks the context of "order of operations."

    The Breakthrough: We integrated a custom Shunting-Yard Algorithm. The engine doesn't just calculate; it parses. It tokenizes input, respects mathematical precedence (PEMDAS), manages parentheses, and executes logic using a Reverse Polish Notation (RPN) stack machine.

Phase III: Taming Concurrency (Thread-Safety & SOLID)

    The Obstacle: As capabilities grew, the engine became a fragile monolith—unsafe for concurrent requests and hard to test.

    The Breakthrough:

        Bulletproof Concurrency: Adopted a Readers-Writer Lock model using std::shared_mutex. Multiple threads can calculate simultaneously, while registration remains exclusive.

        Deterministic Errors: Goodbye, try-catch. We replaced costly exceptions with std::variant and std::optional for zero-overhead, type-safe error handling.

        Strategy Pattern: The code was decoupled into specialized strategies (AlgebraicParser vs. LinearSystemParser) orchestrated by a central Context (CalcEngine).

Phase IV: The Scientific Quantum Leap

    The Obstacle: Basic arithmetic wasn't enough. We needed a tool for scientific analysis and system solving.

    The Breakthrough:

        Advanced Math: Full support for Trigonometric, Inverse Trigonometric, Hyperbolic, and Logarithmic functions.

        Linear Algebra: A powerful Gauss-Jordan Elimination solver that parses natural language equations (e.g., 2x + y = 5) and solves for N variables instantly.

🛠️ Capabilities under the Hood

🧮 Algebraic Engine

    Core: +, -, *, /, ^, %

    Functions: sqrt, abs, exp

    Trigonometry: sin, cos, tan, cot, sec, csc (Degrees)

    Transcendental: log (base 10), ln (base e), lg (base 2), plus inverse trig functions.

📐 Linear Algebra Engine

    Natural Parsing: Understands equations as strings: "2x + 3y = 10; x - y = 5"

    Solver: Optimized matrix algorithms for N-variable systems.

⚙️ System Architecture

    Zero-Overhead: Heavy reliance on C++17/20 features (std::variant, std::optional) to avoid exception overhead.

    Hot-Swapping: Switch parsing strategies (Algebraic <-> Linear) on the fly without restarting the engine.

💻 Code in Action

C++
