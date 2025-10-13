# TissOS Project Structure

This document outlines the directory structure of the TissOS operating system and describes the core components that form the foundation of the TissU ecosystem.

## Directory Structure

The TissOS source code is organized into the following top-level directories within `tisslm/tissos/`:

-   `kernel/`: Contains the core source code for the TissOS microkernel.
-   `drivers/`: Houses the device drivers for various hardware components.
-   `toolchain/`: Includes scripts and tools required for building the operating system.
-   `readme.md`: The project roadmap, outlining the development phases.

### Kernel

The `kernel/` directory is further subdivided into the following components:

-   `arch/`: Contains architecture-specific code.
    -   `x86_64/`: The initial target architecture for TissOS.
-   `include/`: Public kernel headers that define the API for user-space applications and internal kernel modules.
-   `init/`: The kernel's entry point and initialization code.
-   `ipc/`: Implementation of the Inter-Process Communication (IPC) mechanism, which is fundamental to the microkernel design.
-   `mem/`: The memory manager, responsible for virtual memory, paging, and memory protection.
-   `sched/`: The process and thread scheduler.

### Drivers

The `drivers/` directory contains drivers for hardware support:

-   `storage/`: Drivers for storage devices (e.g., AHCI).
-   `input/`: Drivers for input devices (e.g., PS/2, USB).

### Toolchain

The `toolchain/` directory contains the build scripts and toolchain configuration.

-   `build.sh`: The main build script for TissOS.

## Core Components for the TissU Ecosystem

The TissOS architecture is designed to be a core part of the broader TissU ecosystem. The following components are essential for this integration:

-   **Microkernel Design:** The microkernel provides a minimal set of services, enhancing security and stability. Core OS services like filesystems, networking stacks, and drivers run as separate user-space processes.
-   **Message-Passing IPC:** All communication between components (including kernel services and user applications) is done via a secure and efficient message-passing IPC. This is a cornerstone of the TissOS design and is crucial for the modularity and extensibility required by the TissU ecosystem.
-   **TissDB Integration:** As outlined in the long-term roadmap, TissDB will be deeply integrated into TissOS. This will provide a system-wide data management layer, enabling powerful features for applications and the OS itself.
-   **Component-Based Architecture:** The entire OS is designed as a collection of independent components that communicate through well-defined interfaces. This allows for parts of the OS to be updated or replaced without rebooting the entire system, a key feature for a resilient and dynamic ecosystem.
