# TissOS Directory Structure

This document outlines the proposed directory structure for the TissOS source code. This structure is designed to be modular and scalable, supporting the microkernel architecture and separation of components as described in the TissOS roadmap.

/
|-- boot/           # Bootloader code and configuration
|   |-- grub/       # Configuration for GRUB or other bootloaders
|
|-- kernel/         # The TissOS microkernel
|   |-- arch/       # Architecture-specific code (e.g., x86_64, arm)
|   |   |-- x86_64/
|   |   |   |-- boot/       # Assembly boot code
|   |   |   |-- mm/         # Memory management specifics (paging, etc.)
|   |-- core/       # Core, architecture-independent kernel components
|   |   |-- ipc/        # Inter-Process Communication
|   |   |-- sched/      # Process and thread scheduler
|   |   |-- mm/         # Virtual memory management
|   |-- include/    # Public kernel headers for system calls and services
|
|-- drivers/        # User-space device drivers
|   |-- net/        # Network interface card drivers
|   |-- storage/    # Storage device drivers (AHCI, NVMe)
|   |-- input/      # Input device drivers (keyboard, mouse)
|   |-- pci/        # PCI bus driver/server
|   |-- gpu/        # Graphics drivers
|
|-- services/       # User-space system servers
|   |-- fs/         # Filesystem server
|   |-- net/        # Networking stack server
|   |-- init/       # The init process responsible for starting other services
|
|-- libs/           # System libraries
|   |-- libc/       # Standard C library (port or implementation)
|   |-- libm/       # Math library
|   |-- libgui/     # GUI toolkit library
|   |-- libtiss/    # Core TissOS utility library
|
|-- apps/           # User-space applications
|   |-- terminal/
|   |-- editor/
|   |-- file-manager/
|   |-- shell/      # The command-line shell
|
|-- build/          # Build system scripts and toolchain files
|   |-- toolchain/  # Scripts for building the cross-compiler
|   |-- scripts/    # Helper scripts for building the OS
|
|-- docs/           # Documentation for TissOS (distinct from the parent project docs)
