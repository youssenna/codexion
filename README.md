*This project has been created as part of the 42 curriculum by yousenna.*

# Codexion - Dynamic Multithreaded Resource Synchronization

## Description

**Codexion** is a high-performance multithreading and concurrency simulation project developed as part of the 42 curriculum. Inspired by Dijkstra's classic *Dining Philosophers Problem*, Codexion simulates a dynamic environment where multiple threads (**coders**) contend for shared resources (**dongles**) to complete iterative workflows consisting of **compiling**, **debugging**, and **refactoring**.

The main objective of Codexion is to manage dynamic thread synchronization, enforce strict scheduling policies (**FIFO** and **EDF - Earliest Deadline First**), prevent race conditions, avoid resource starvation, and prevent system deadlocks under extreme high-concurrency loads. Additionally, a dedicated **Monitor Thread** continuously audits thread state to detect burnout conditions with sub-10ms precision.

---

## Instructions

### Compilation

The project includes a `Makefile` supporting standard rules. Build the project using `make`:

```bash
make
```

To recompile cleanly or clean build objects:

```bash
make clean
make fclean
make re
```

For concurrency debugging and data race validation, compile with ThreadSanitizer:

```bash
cc -fsanitize=thread -g main.c parser.c simulation_routine.c create_resources.c simulation_utiles.c codexion_utiles.c -pthread -o codexion
```

### Execution

Run the compiled executable with 8 mandatory command-line arguments:

```bash
./codexion <nb_coders> <burnout_time> <compile_time> <debug_time> <refactor_time> <compile_nb> <dongle_cooldown> <scheduler>
```

#### Arguments Breakdown:

1. `nb_coders`: Number of coders (and matching number of dongles).
2. `burnout_time`: Time (in ms) a coder can go without starting a compile before burning out.
3. `compile_time`: Duration (in ms) required to complete compilation.
4. `debug_time`: Duration (in ms) spent debugging after compilation.
5. `refactor_time`: Duration (in ms) spent refactoring after debugging.
6. `compile_nb`: Number of compile-debug-refactor loops each coder must perform.
7. `dongle_cooldown`: Cooldown duration (in ms) a dongle must rest after being released before it can be reused.
8. `scheduler`: Scheduling policy: `fifo` (First-In, First-Out) or `edf` (Earliest Deadline First).

#### Usage Examples:

* **3 Coders FIFO Simulation**:
  ```bash
  ./codexion 3 1000 200 50 50 5 0 fifo
  ```

* **4 Coders EDF Policy with Dongle Cooldown**:
  ```bash
  ./codexion 4 3300 90 60 60 10 600 edf
  ```

* **Single Coder Burnout Test**:
  ```bash
  ./codexion 1 1000 200 50 50 1 0 fifo
  ```

---

## Blocking Cases Handled

Codexion addresses all major concurrency hazards by strictly managing Coffman's four deadlock conditions and resource contention pitfalls:

### 1. Deadlock Prevention & Coffman's Conditions

* **Mutual Exclusion**: Guaranteed per dongle using individual `pthread_mutex_t` locks.
* **Hold and Wait Prevention**: Dongles are acquired **atomically**. A coder never holds one dongle locked while waiting indefinitely for an adjacent dongle. If both dongles are not ready simultaneously, the coder releases all locks, waits, and retries.
* **No Preemption**: Dongles cannot be forcibly confiscated from an active coder.
* **Circular Wait Prevention**: All dongles are acquired in strict **resource ordering** ($d_1 \text{ ID} < d_2 \text{ ID}$). Even in circular table arrangements, this strict ascending index acquisition breaks circular wait graphs across the system.

### 2. Starvation Prevention & Work-Conserving Priority

* **Priority Queues**: Each dongle maintains a priority queue (`queue[2]`).
* **FIFO Policy**: Coders are prioritized based on request timestamps (`get_curr_t()`).
* **EDF Policy**: Coders with the closest burnout deadline (`last_compile + burnout_time`) are moved to the front of the queue to prevent starvation under tight deadlines.
* **Work-Conserving Execution**: If a top-priority coder is blocked on an adjacent dongle, lower-priority coders whose required dongles are completely free are permitted to run, ensuring CPU throughput is never stalled.

### 3. Dongle Cooldown Handling

* Each dongle tracks `last_usage` timestamps upon being released.
* `take_both_dongles()` evaluates elapsed time (`get_curr_t() - d->last_usage`). A dongle is marked ready only when the elapsed time satisfies $\ge \text{dongle\_cooldown}$.

### 4. Precise Burnout Detection

* A dedicated background **Monitor Thread** polls coder timestamps with sub-millisecond sleeping intervals (`usleep(500)`).
* Burnout is detected immediately when $\text{current\_time} - \text{last\_compile} \ge \text{burnout\_time}$.
* Upon burnout detection, a global flag `run_simulation = 0` is broadcast to immediately halt all working threads within $< 10\text{ms}$.

### 5. Log Serialization

* All status updates (`is compiling`, `is debugging`, `is refactoring`, `has taken a dongle`, `is burnout`) are routed through `print_coder_mesage()` protected by `prog_mutex`.
* This guarantees clean, non-interleaved, strictly chronological timestamp outputs across all concurrent threads.

---

## Thread Synchronization Mechanisms

Codexion relies on POSIX threading primitives (`pthread_mutex_t`, `pthread_cond_t`) organized into a decoupled multithreaded architecture.

```
                  +-----------------------------------+
                  |          Monitor Thread           |
                  +-----------------------------------+
                                    |
            Audits last_compile     | Sets run_simulation = 0
            timestamps periodically | Wakes sleeping threads
                                    v
     +-------------------------------------------------------------+
     |                    Shared Program State                     |
     |  (prog_mutex, run_simulation, start_sim, finished_compile) |
     +-------------------------------------------------------------+
               ^                                   ^
               | Synchronizes                      | Synchronizes
               v                                   v
    +--------------------+               +--------------------+
    |   Coder Thread 1   |               |   Coder Thread N   |
    +--------------------+               +--------------------+
      | d1->mutex          \           /   | dN->mutex
      | d1->cond            \         /    | dN->cond
      v                      v       v     v
    +---------------------------------------------------------+
    |                    Dongle Array                         |
    |      (Dongle 1, Dongle 2, Dongle 3 ... Dongle N)        |
    +---------------------------------------------------------+
```

### 1. POSIX Mutexes (`pthread_mutex_t`)

* **`dongle->mutex`**: Protects the state of each individual dongle (`is_hold`, `last_usage`, `queue_len`, `queue[2]`).
* **`prog_info->prog_mutex`**: Protects global simulation flags (`run_simulation`, `start_sim`, `finished_compile`), timestamp reads, and log serialization.

### 2. POSIX Condition Variables (`pthread_cond_t`)

* **`prog_info->prog_cond`**: Acts as a startup barrier. All coder threads wait on `prog_cond` until `create_coders()` finishes spawning all threads, guaranteeing simultaneous thread execution.
* **`dongle->cond`**: Used to wake up waiting threads whenever a dongle is released (`put_dongle()`).

### 3. Race Condition Prevention Examples

* **Atomic State Modification**: Changing `last_compile` occurs strictly within `prog_mutex` critical sections to prevent data races between coder threads and the Monitor Thread.
* **Double-Check Lock Pattern**: Before taking dongles or entering task sleeps (`my_sleep()`), threads verify `run_simulation` under `prog_mutex` lock to ensure immediate thread termination if a burnout occurs elsewhere.

---

## Resources

### Documentation & References

* **POSIX Threads (pthreads)**: [IEEE Std 1003.1-2017 Thread Specifications](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html)
* **Dijkstra's Dining Philosophers Problem**: E. W. Dijkstra, *Cooperating Sequential Processes*, 1965.
* **Coffman Deadlock Conditions**: E. G. Coffman Jr., M. J. Elphick, A. Shoshani, *System Deadlocks*, ACM Computing Surveys, 1971.
* **Earliest Deadline First (EDF) Scheduling**: C. L. Liu, J. W. Layland, *Scheduling Algorithms for Multiprogramming in a Hard-Real-Time Environment*, JACM, 1973.

### AI Attribution & Usage

Artificial Intelligence (Google Antigravity AI Assistant) was used during this project for the following specific tasks:

1. **Understanding Complex Concepts**: Explaining new and complex multithreading concepts, including Coffman deadlock conditions, atomic resource acquisition, lock order hierarchies, and work-conserving priority scheduling.
2. **Debugging Critical Errors**: Assisting in analyzing and resolving critical errors such as startup race conditions, lock order inversions, and condition variable sleep gaps.
3. **Generating Project Documentation**: Helping structure and generate this `README.md` file according to 42 curriculum requirements.




## 👨‍💻 Authors

<table align="center">
  <tr>
    <td align="center">
      <a href="https://github.com/youssenna">
        <img src="https://github.com/youssenna.png?size=150" width="200px" height="200px" alt="yousenna"/>
        <br />
        <sub><b>Youssef Ennajar</b></sub>
      </a>
      <br />
      <a href="https://github.com/youssenna" title="GitHub">
        <img src="https://img.shields.io/badge/GitHub-100000?style=flat&logo=github&logoColor=white" />
      </a>
      <a href="https://www.linkedin.com/in/youssef-ennajar-213985253/" title="LinkedIn">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=flat&logo=linkedin&logoColor=white" />
      </a>
      <a href="https://www.youtube.com/@codingwithmoljlaba" title="YouTube">
        <img src="https://img.shields.io/badge/YouTube-FF0000?style=flat&logo=youtube&logoColor=white" />
    </td>
  </tr>
</table>

---

<p align="center">
  Made with ❤️ at 1337 bengurir
</p>




---