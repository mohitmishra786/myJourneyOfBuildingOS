const bookConfig = {
    title: "Operating Systems: Architecture, Design, and Implementation",
    author: "OS Learning Journey",
    description: "A comprehensive guide to operating system fundamentals, architecture patterns, and implementation details.",
    
    chapters: [
        {
            title: "Table of Contents",
            pages: [
                {
                    title: "Index of the Book",
                    path: "indexOfBook.md"
                }
            ]
        },
        {
            title: "Introduction to Operating System",
            pages: [
                {
                    title: "Deep Dive Into Evolution And Architecture",
                    path: "Part-1/Ch-1-Introduction-to-Operating-System/Deep-Dive-Into-Evolution-And-Architecture.md"
                },
                {
                    title: "Deep Dive Into System Calls And API Interfaces",
                    path: "Part-1/Ch-1-Introduction-to-Operating-System/Deep-Dive-Into-System-Calls-And-API-Interfaces.md"
                }
            ]
        },
        {
            title: "OS Architecture Patterns",
            pages: [
                {
                    title: "Introduction to OS Architecture",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Introduction-to-OS-Architecture.md"
                },
                {
                    title: "Monolithic Kernel",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Monolithic-Kernel.md"
                },
                {
                    title: "Case Study: Traditional Unix Architecture", 
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/CaseStudies/Traditional-Unix-Architecture.md"
                },
                {
                    title: "Microkernels",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Microkernels.md"
                },
                {
                    title: "Case Study: Mach Kernel Architecture",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/CaseStudies/Mach-Kernel-Architecture.md"
                },
                {
                    title: "Hybrid Kernel Architecture",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Hybrid-Kernel-Arch.md"
                },
                {
                    title: "Case Study: Windows NT Architecture",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/CaseStudies/Windows_NT.md"
                },
                {
                    title: "Exokernel",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Exokernel.md"
                },
                {
                    title: "Unikernels",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Unikernels.md"
                },
                {
                    title: "Virtual Machines as an OS Pattern",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Virtual-Machines-as-an-OS-Pattern.md"
                },
                {
                    title: "Operating System Layer Architecture",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Layered-Systems.md"
                },
                {
                    title: "Real Time Operating Systems",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/RTOS.md"
                },
                {
                    title: "Adaptive and Reflective Architectures",
                    path: "Part-1/Ch-2-OS-Architecture-Patterns/Adaptive-And-Reflective-Architecture.md"
                }
            ]
        },
        {
            title: "Computer System Architecture",
            pages: [
                {
                    title: "Introduction to Computer System Architecture",
                    path: "Part-1/Ch-3-Computer-System-Architecture/Introduction-to-Computer-System-Architecture.md"
                },
                {
                    title: "Von Neumann Architecture",
                    path: "Part-1/Ch-3-Computer-System-Architecture/Von-Neumann-Architecture.md"
                },
                {
                    title: "Instruction Execution Cycle",
                    path: "Part-1/Ch-3-Computer-System-Architecture/Instruction-Execution-Cycle.md"
                },
                {
                    title: "Privilege Levels and Protection Rings",
                    path: "Part-1/Ch-3-Computer-System-Architecture/Privilege-Levels-Protection-Rings.md"
                },
                {
                    title: "Memory Hierarchy",
                    path: "Part-1/Ch-3-Computer-System-Architecture/Memory-Hierarchy.md"
                },
                {
                    title: "I/O Subsystems",
                    path: "Part-1/Ch-3-Computer-System-Architecture/IO-Subsystems.md"
                },
                {
                    title: "Interrupt Handling and DMA",
                    path: "Part-1/Ch-3-Computer-System-Architecture/Interrupt-Handling-DMA.md"
                }
            ]
        },
        {
            title: "Process Management",
            pages: [
                {
                    title: "Process Concepts and States",
                    path: "Part-1/Ch-4-Process-Management/Process-Concepts-and-States.md"
                },
                {
                    title: "Process Control Block (PCB)",
                    path: "Part-1/Ch-4-Process-Management/Process-Control-Block-PCB.md"
                },
                {
                    title: "Context Switching",
                    path: "Part-1/Ch-4-Process-Management/Context-Switching.md"
                },
                {
                    title: "Process Scheduling Algorithms",
                    path: "Part-1/Ch-4-Process-Management/Process-Scheduling-Algorithms.md"
                },
                {
                    title: "Inter-Process Communication (IPC)",
                    path: "Part-1/Ch-4-Process-Management/Inter-Process-Communication-IPC.md"
                },
                {
                    title: "POSIX Process Management APIs",
                    path: "Part-1/Ch-4-Process-Management/POSIX-Process-Management-APIs.md"
                }
            ]
        },
        {
            title: "Thread Management",
            pages: [
                {
                    title: "Thread Models and Implementation",
                    path: "Part-1/Ch-5-Thread-Management/Thread-Models-and-Implementation.md"
                },
                {
                    title: "User-level vs Kernel-level Threads",
                    path: "Part-1/Ch-5-Thread-Management/User-level-vs-Kernel-level-Threads.md"
                },
                {
                    title: "Thread Scheduling",
                    path: "Part-1/Ch-5-Thread-Management/Thread-Scheduling.md"
                },
                {
                    title: "Thread Synchronization",
                    path: "Part-1/Ch-5-Thread-Management/Thread-Synchronization.md"
                },
                {
                    title: "Thread Pools",
                    path: "Part-1/Ch-5-Thread-Management/Thread-Pools.md"
                },
                {
                    title: "Pthread Programming",
                    path: "Part-1/Ch-5-Thread-Management/Pthread-Programming.md"
                }
            ]
        },
        {
            title: "CPU Scheduling",
            pages: [
                {
                    title: "Scheduling Criteria and Metrics",
                    path: "Part-2/Ch-6-CPU-Scheduling/Scheduling-Criteria-and-Metrics.md"
                },
                {
                    title: "Preemptive vs Non-preemptive Scheduling",
                    path: "Part-2/Ch-6-CPU-Scheduling/Preemptive-vs-Non-preemptive-Scheduling.md"
                },
                {
                    title: "Basic Scheduling Algorithms",
                    path: "Part-2/Ch-6-CPU-Scheduling/Basic-Scheduling-Algorithms.md"
                },
                {
                    title: "Advanced Scheduling Algorithms",
                    path: "Part-2/Ch-6-CPU-Scheduling/Advanced-Scheduling-Algorithms.md"
                },
                {
                    title: "Real-time Scheduling",
                    path: "Part-2/Ch-6-CPU-Scheduling/Real-time-Scheduling.md"
                },
                {
                    title: "Multiprocessor Scheduling",
                    path: "Part-2/Ch-6-CPU-Scheduling/Multiprocessor-Scheduling.md"
                }
            ]
        },
        {
            title: "Process Synchronization",
            pages: [
                {
                    title: "Race Conditions and Critical Section Problem",
                    path: "Part-2/Ch-7-Process-Synchronization/Race-Conditions-and-Critical-Section-Problem.md"
                },
                {
                    title: "Peterson's Solution and Hardware Support",
                    path: "Part-2/Ch-7-Process-Synchronization/Petersons-Solution-and-Hardware-Support.md"
                },
                {
                    title: "Semaphores and Mutex",
                    path: "Part-2/Ch-7-Process-Synchronization/Semaphores-and-Mutex.md"
                },
                {
                    title: "Monitors and Condition Variables",
                    path: "Part-2/Ch-7-Process-Synchronization/Monitors-and-Condition-Variables.md"
                },
                {
                    title: "Classical Synchronization Problems",
                    path: "Part-2/Ch-7-Process-Synchronization/Classical-Synchronization-Problems.md"
                },
                {
                    title: "Dining Philosophers and Deadlock Handling",
                    path: "Part-2/Ch-7-Process-Synchronization/Dining-Philosophers-and-Deadlock-Handling.md"
                }
            ]
        }
    ]
};
