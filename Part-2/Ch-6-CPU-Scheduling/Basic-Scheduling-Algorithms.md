# Basic Scheduling Algorithms

Fundamental scheduling algorithms form the foundation of modern CPU scheduling systems, each addressing different performance objectives and system requirements. Understanding these basic algorithms provides essential insights into scheduling theory and practical implementation considerations.

## First-Come-First-Serve (FCFS) Scheduling

First-Come-First-Serve represents the simplest possible scheduling algorithm, executing processes in the exact order they arrive in the system. This straightforward approach mimics real-world queuing systems and provides intuitive fairness guarantees.

FCFS implementation requires only a simple FIFO queue to maintain arriving processes. When the CPU becomes available, the scheduler removes the first process from the queue and allocates the processor until the process completes or blocks for I/O operations.

The algorithm's simplicity makes it easy to implement and understand, with minimal overhead for scheduling decisions. No complex priority calculations, time slice management, or preemption mechanisms are required, resulting in efficient operation and predictable behavior.

However, FCFS suffers from the convoy effect, where short processes must wait behind long-running processes, dramatically increasing average waiting times. This problem becomes particularly severe when CPU-intensive processes arrive before I/O-intensive processes, blocking the entire queue until the long processes complete.

```plantuml
@startuml
!theme plain
title "First-Come-First-Serve Scheduling Algorithm Analysis"

package "FCFS Implementation" {
  
  class FCFSScheduler {
    + ready_queue: fifo_queue
    + current_process: process_reference
    --
    + enqueue_process(process): void
    + dequeue_next_process(): process_reference
    + calculate_waiting_times(): time_statistics
    + analyze_convoy_effect(): convoy_metrics
  }
  
  rectangle "Process Execution Timeline" {
    participant "Process P1" as p1
    participant "Process P2" as p2  
    participant "Process P3" as p3
    participant "CPU" as cpu
    
    p1 -> cpu : "Burst: 24ms"
    note over cpu : "P1 executes completely"
    
    p2 -> cpu : "Burst: 3ms"  
    note over cpu : "P2 waits 24ms, then executes"
    
    p3 -> cpu : "Burst: 3ms"
    note over cpu : "P3 waits 27ms, then executes"
    
    note bottom : "Average waiting time: (0+24+27)/3 = 17ms"
  }
}

package "Performance Characteristics" {
  
  rectangle "Convoy Effect Demonstration" {
    component [Long Process P1] as long_proc
    component [Short Process P2] as short_p2
    component [Short Process P3] as short_p3
    
    long_proc --> short_p2 : "blocks"
    short_p2 --> short_p3 : "delays"
    
    note bottom : "One long process\nblocks entire queue"
  }
  
  card "FCFS Metrics" {
    usecase "Waiting Time: Variable" as wait_var
    usecase "Turnaround Time: Poor for short jobs" as turn_poor
    usecase "Response Time: Same as waiting time" as resp_same
    usecase "Throughput: Depends on job mix" as through_dep
  }
  
  card "FCFS Advantages" {
    usecase "Simple Implementation" as simple_impl
    usecase "No Starvation" as no_starv
    usecase "Low Overhead" as low_over
    usecase "Fair Ordering" as fair_order
  }
}

FCFSScheduler --> long_proc : "executes_in_order"
wait_var --> turn_poor : "contributes_to"
simple_impl --> no_starv : "guarantees"
@enduml
```

## Shortest Job First (SJF) Scheduling

Shortest Job First scheduling selects the process with the smallest CPU burst time for execution, optimizing average waiting time through mathematical principles proven by queuing theory.

SJF algorithms can operate in non-preemptive or preemptive modes. Non-preemptive SJF runs each selected process to completion, while preemptive SJF (Shortest Remaining Time First) can interrupt running processes when shorter jobs arrive.

The algorithm's optimality for minimizing average waiting time makes it theoretically attractive. Mathematical analysis proves that SJF produces the minimum possible average waiting time for any given set of processes with known burst times.

Practical implementation faces significant challenges due to the impossibility of precisely predicting future CPU burst times. Real systems must estimate burst times using historical data, exponential averaging, or other prediction techniques, introducing uncertainty and potential performance degradation.

Starvation represents another serious concern with SJF scheduling. Long processes may wait indefinitely if short processes continue arriving, creating unfair resource allocation that violates basic scheduling equity principles.

```plantuml
@startuml
!theme plain
title "Shortest Job First Scheduling Analysis and Prediction"

package "SJF Algorithm Implementation" {
  
  class SJFScheduler {
    + ready_queue: priority_queue_by_burst_time
    + burst_time_predictor: prediction_engine
    + historical_data: burst_history
    --
    + predict_next_burst(process): predicted_time
    + select_shortest_job(): process_reference
    + update_prediction_accuracy(): accuracy_metrics
    + handle_starvation_prevention(): mitigation_actions
  }
  
  class BurstTimePredictor {
    + exponential_average: averaging_engine
    + historical_bursts: time_series
    + prediction_accuracy: accuracy_tracker
    --
    + exponential_averaging(alpha, history): predicted_burst
    + adaptive_prediction(): dynamic_prediction
    + evaluate_prediction_quality(): quality_metrics
  }
}

package "SJF Execution Example" {
  
  rectangle "Process Set Analysis" {
    component [Process A: 6ms] as proc_a
    component [Process B: 8ms] as proc_b
    component [Process C: 7ms] as proc_c
    component [Process D: 3ms] as proc_d
    
    proc_d --> proc_a : "Execute first (shortest)"
    proc_a --> proc_c : "Execute second"
    proc_c --> proc_b : "Execute last (longest)"
    
    note bottom : "Execution order: D(3), A(6), C(7), B(8)\nAverage waiting: (0+3+9+16)/4 = 7ms"
  }
  
  rectangle "Comparison with FCFS" {
    card "SJF Scheduling" {
      usecase "Average Wait: 7ms" as sjf_wait
      usecase "Optimal for given burst times" as sjf_opt
    }
    
    card "FCFS Alternative" {
      usecase "Average Wait: 10.25ms" as fcfs_wait
      usecase "Order: A(6), B(8), C(7), D(3)" as fcfs_order
    }
    
    sjf_wait --> fcfs_wait : "46% improvement"
  }
}

package "Prediction Mechanisms" {
  
  class ExponentialAveraging {
    + alpha: smoothing_factor
    + previous_prediction: time_value
    + actual_burst: time_value
    --
    + calculate_next_prediction(): predicted_time
    + formula: "T(n+1) = α * t(n) + (1-α) * T(n)"
  }
  
  rectangle "Prediction Accuracy Analysis" {
    component [Short-term Accuracy] as short_acc
    component [Long-term Trends] as long_trends
    component [Burst Variation] as burst_var
    
    short_acc --> long_trends : "trades_off_with"
    burst_var --> short_acc : "affects"
    
    note bottom : "α controls responsiveness\nvs stability trade-off"
  }
}

SJFScheduler --> BurstTimePredictor : "uses"
BurstTimePredictor --> ExponentialAveraging : "implements"
proc_d --> sjf_wait : "contributes_to_optimal"
@enduml
```

## Round Robin Scheduling

Round Robin scheduling allocates CPU time in fixed time slices or quanta, cycling through ready processes in circular fashion. This approach combines the fairness of FCFS with improved responsiveness for interactive systems.

The time quantum represents the critical parameter in Round Robin scheduling, determining the balance between responsiveness and overhead. Short quanta provide better response times but increase context switching overhead, while long quanta reduce overhead but may harm responsiveness.

Process execution continues until the time quantum expires or the process blocks for I/O operations. When the quantum expires, the scheduler preempts the running process, places it at the end of the ready queue, and allocates the CPU to the next process in the queue.

Round Robin provides good response time characteristics for interactive systems because each process receives regular CPU access regardless of its total execution requirements. No process can monopolize the CPU for extended periods, preventing the convoy effect that plagues FCFS scheduling.

```plantuml
@startuml
!theme plain
title "Round Robin Scheduling Implementation and Analysis"

package "Round Robin Scheduler" {
  
  class RoundRobinScheduler {
    + ready_queue: circular_queue
    + time_quantum: quantum_duration
    + quantum_timer: countdown_timer
    + context_switch_overhead: overhead_measurement
    --
    + allocate_time_slice(): void
    + handle_quantum_expiry(): void
    + calculate_optimal_quantum(): quantum_value
    + measure_response_times(): response_statistics
  }
  
  rectangle "Quantum Selection Analysis" {
    component [Small Quantum (1ms)] as small_q
    component [Medium Quantum (10ms)] as med_q
    component [Large Quantum (100ms)] as large_q
    
    small_q --> med_q : "Better responsiveness\nHigher overhead"
    med_q --> large_q : "Balanced approach\nModerate overhead"
    large_q : "Lower overhead\nWorse responsiveness"
    
    note bottom : "Optimal quantum depends on\nworkload and context switch cost"
  }
}

package "Round Robin Execution Timeline" {
  
  participant "Process P1" as p1
  participant "Process P2" as p2
  participant "Process P3" as p3
  participant "CPU Scheduler" as scheduler
  
  == First Round (Quantum = 4ms) ==
  scheduler -> p1 : "Allocate CPU for 4ms"
  p1 -> scheduler : "Quantum expired, remaining: 20ms"
  
  scheduler -> p2 : "Allocate CPU for 4ms"
  p2 -> scheduler : "Quantum expired, remaining: 2ms"
  
  scheduler -> p3 : "Allocate CPU for 4ms"
  p3 -> scheduler : "Quantum expired, remaining: 4ms"
  
  == Second Round ==
  scheduler -> p1 : "Allocate CPU for 4ms"
  p1 -> scheduler : "Quantum expired, remaining: 16ms"
  
  scheduler -> p2 : "Allocate CPU for 2ms"
  p2 -> scheduler : "Process completed"
  
  scheduler -> p3 : "Allocate CPU for 4ms"
  p3 -> scheduler : "Process completed"
  
  note over scheduler : "P1 continues until completion\nResponse times: P1=0, P2=4, P3=8"
}

package "Performance Analysis Framework" {
  
  class QuantumOptimizer {
    + workload_characteristics: workload_profile
    + context_switch_cost: overhead_measurement
    + response_time_requirements: time_constraints
    --
    + analyze_workload_patterns(): workload_analysis
    + calculate_optimal_quantum(): quantum_recommendation
    + evaluate_quantum_performance(): performance_metrics
  }
  
  rectangle "Trade-off Analysis" {
    card "Small Quantum Effects" {
      usecase "Excellent Response Time" as excel_resp
      usecase "High Context Switch Overhead" as high_overhead
      usecase "CPU Cache Pollution" as cache_poll
    }
    
    card "Large Quantum Effects" {
      usecase "Low Context Switch Overhead" as low_overhead
      usecase "Poor Response Time" as poor_resp
      usecase "Approaches FCFS Behavior" as fcfs_like
    }
    
    excel_resp --> high_overhead : "trade_off"
    low_overhead --> poor_resp : "trade_off"
  }
}

RoundRobinScheduler --> small_q : "configures"
QuantumOptimizer --> excel_resp : "evaluates"
p1 --> scheduler : "preempted_by_quantum"
@enduml
```

## Priority Scheduling

Priority scheduling assigns each process a priority value and allocates CPU time based on these priorities, ensuring that more important processes receive preferential treatment. This approach enables fine-grained control over resource allocation based on process importance, user requirements, or system policies.

Static priority assignment sets process priorities at creation time and maintains them throughout execution. This approach provides predictable behavior and simplifies implementation but cannot adapt to changing process characteristics or system conditions.

Dynamic priority adjustment modifies process priorities during execution based on various factors such as aging, resource usage, or performance feedback. This flexibility enables better system adaptation but introduces additional complexity and potential instability.

Priority scheduling can operate in preemptive or non-preemptive modes. Preemptive priority scheduling immediately interrupts lower-priority processes when higher-priority processes become ready, while non-preemptive priority scheduling waits for the current process to complete or block.

The major challenge with priority scheduling is indefinite postponement or starvation, where low-priority processes may never execute if high-priority processes continuously arrive. Aging mechanisms address this problem by gradually increasing the priority of waiting processes.

```plantuml
@startuml
!theme plain
title "Priority Scheduling Implementation and Starvation Prevention"

package "Priority Scheduling System" {
  
  class PriorityScheduler {
    + priority_queues: multi_level_queue
    + aging_mechanism: aging_engine
    + priority_inheritance: inheritance_handler
    + starvation_detector: starvation_monitor
    --
    + select_highest_priority(): process_reference
    + apply_aging_algorithm(): void
    + handle_priority_inversion(): void
    + prevent_starvation(): mitigation_actions
  }
  
  class AgingMechanism {
    + aging_rate: priority_increment
    + aging_interval: time_interval
    + maximum_priority: priority_limit
    --
    + increment_waiting_priorities(): void
    + calculate_age_based_priority(wait_time): priority_value
    + reset_priority_after_execution(): void
  }
  
  class PriorityInheritanceHandler {
    + resource_ownership: ownership_tracking
    + blocking_relationships: dependency_graph
    --
    + detect_priority_inversion(): inversion_events
    + inherit_priority(holder, requester): void
    + restore_original_priority(): void
  }
}

package "Priority Queue Structure" {
  
  rectangle "Multi-level Priority Queues" {
    component [Real-time Priority (0-99)] as rt_priority
    component [Normal Priority (100-139)] as normal_priority
    component [Idle Priority (140+)] as idle_priority
    
    rt_priority --> normal_priority : "Higher precedence"
    normal_priority --> idle_priority : "Higher precedence"
    
    note bottom : "Lower numbers indicate\nhigher priority"
  }
  
  rectangle "Priority Assignment Examples" {
    card "System Processes" {
      usecase "Kernel threads: 0-20" as kernel_threads
      usecase "Real-time tasks: 21-99" as rt_tasks
    }
    
    card "User Processes" {
      usecase "Interactive: 100-119" as interactive
      usecase "Batch: 120-139" as batch
    }
    
    card "Background Tasks" {
      usecase "System daemons: 140+" as daemons
    }
    
    kernel_threads --> rt_tasks : "system_critical"
    interactive --> batch : "user_responsive"
  }
}

package "Starvation Prevention Mechanisms" {
  
  class StarvationPrevention {
    + wait_time_tracking: time_tracker
    + priority_boost_threshold: time_threshold
    + fairness_metrics: fairness_monitor
    --
    + monitor_process_waiting(): monitoring_status
    + apply_priority_boost(): boost_action
    + ensure_eventual_execution(): guarantee_mechanism
  }
  
  rectangle "Aging Algorithm Implementation" {
    component [Monitor Wait Times] as monitor_wait
    component [Calculate Age Factor] as calc_age
    component [Boost Priority] as boost_priority
    component [Reset After Execution] as reset_priority
    
    monitor_wait --> calc_age : "periodically"
    calc_age --> boost_priority : "when_threshold_exceeded"
    boost_priority --> reset_priority : "after_execution"
    
    note bottom : "Gradual priority increase\nprevents indefinite postponement"
  }
}

PriorityScheduler --> AgingMechanism : "uses"
PriorityScheduler --> PriorityInheritanceHandler : "coordinates_with"
rt_priority --> StarvationPrevention : "may_cause_starvation_of"
monitor_wait --> boost_priority : "triggers"

note bottom of PriorityScheduler : "Balances priority\nrespect with fairness"
@enduml
```

## Algorithm Comparison and Selection Criteria

Selecting appropriate scheduling algorithms requires understanding the trade-offs between different approaches and matching algorithm characteristics to system requirements and workload patterns.

Performance characteristics vary significantly across algorithms. FCFS provides excellent throughput for long-running batch jobs but poor response time for interactive applications. SJF optimizes average waiting time but requires burst time prediction and may cause starvation. Round Robin provides good response time but introduces quantum-related overhead. Priority scheduling enables fine-grained control but requires careful starvation prevention.

Implementation complexity ranges from the simple FIFO queue required for FCFS to the sophisticated multi-level priority queues and aging mechanisms needed for priority scheduling. This complexity affects development effort, debugging difficulty, and runtime overhead.

Fairness guarantees differ substantially between algorithms. FCFS provides perfect ordering fairness but poor performance fairness. Round Robin ensures temporal fairness through regular CPU allocation. Priority scheduling may sacrifice fairness for importance-based resource allocation.

Predictability characteristics influence suitability for different application domains. FCFS and non-preemptive algorithms provide deterministic behavior valuable for real-time systems, while preemptive algorithms offer better responsiveness at the cost of timing predictability.

Modern systems typically combine multiple algorithms in hierarchical or adaptive frameworks, using different algorithms for different process classes or adjusting algorithm behavior based on system conditions. Understanding these fundamental algorithms provides the foundation for designing and implementing such sophisticated scheduling systems.

The choice of basic scheduling algorithm depends on system objectives, workload characteristics, implementation constraints, and performance requirements. Interactive systems favor Round Robin or priority scheduling for responsiveness, batch systems may prefer FCFS or SJF for efficiency, and real-time systems often require priority scheduling with careful timing analysis. No single algorithm optimizes all performance metrics simultaneously, making algorithm selection a critical design decision that significantly impacts overall system behavior. 