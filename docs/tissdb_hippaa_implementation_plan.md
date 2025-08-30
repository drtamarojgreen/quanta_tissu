# TissDB HIPAA Implementation Plan

## 1. Introduction

This document provides a detailed, step-by-step engineering plan to implement the technical safeguards required for TissDB to become HIPAA compliant. The plan is derived from the remediation strategy outlined in the `docs/tissdb_hippaa_compliance.md` document.

The work is broken down into four main pillars, each with specific, actionable tasks. Each task will include the key files to be modified and the engineering steps required for completion. This plan will serve as the primary guide for all development, testing, and review activities related to this compliance effort.

## 2. Pillar 1: Access Control

This pillar focuses on ensuring that only authorized users and systems can access ePHI.

### Task 2.1: Implement API Token Authentication

**Objective:** To introduce a mandatory, token-based authentication system for all API requests, preventing unauthorized access at the network edge.

*   **Engineering Steps:**
    1.  **Create Token Management Module:**
        *   **File:** `tissdb/auth/token_manager.cpp`, `tissdb/auth/token_manager.h` (new files)
        *   **Action:** Develop a new module responsible for generating secure, high-entropy API tokens. It will also handle the secure storage of these tokens by hashing them (e.g., using SHA-256) and storing them in a dedicated, protected system collection within TissDB.
    2.  **Modify HTTP Server for Token Handling:**
        *   **File:** `tissdb/api/http_server.cpp`, `tissdb/api/http_server.h`
        *   **Action:** Modify the HTTP server to inspect the `Authorization` header of all incoming requests. It must extract the bearer token for validation. Requests without a valid token format will be rejected immediately with a `401 Unauthorized` error.
    3.  **Implement Token Validation Logic:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** Integrate the `token_manager` to validate the extracted token against the stored hashes. If the token is invalid or not found, the request is rejected with a `401 Unauthorized` error.
    4.  **Add Unit Tests:**
        *   **File:** `tests/db/test_auth.cpp` (new file)
        *   **Action:** Create unit tests to verify that the server correctly accepts requests with valid tokens and rejects those with invalid, missing, or malformed tokens.

### Task 2.2: Implement Role-Based Access Control (RBAC)

**Objective:** To enforce the principle of least privilege by ensuring that authenticated users can only perform actions for which they are explicitly authorized.

*   **Engineering Steps:**
    1.  **Create Role Management Module:**
        *   **File:** `tissdb/auth/rbac.cpp`, `tissdb/auth/rbac.h` (new files)
        *   **Action:** Develop a new module to define a set of roles (e.g., `admin`, `read_write`, `read_only`). This module will manage the association between API tokens and these roles.
    2.  **Integrate RBAC Checks:**
        *   **File:** `tissdb/api/h_server.cpp`
        *   **Action:** After a token is successfully authenticated, the RBAC module will be called to retrieve the roles associated with that token.
    3.  **Enforce Permissions on API Endpoints:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** Implement logic within the request handling code for each endpoint to check if the user's roles grant permission for the requested action (e.g., `DELETE`, `POST`). Administrative endpoints (like user management) must be restricted to the `admin` role.
    4.  **Enforce Permissions at Query Execution:**
        *   **File:** `tissdb/query/executor.cpp`
        *   **Action:** (Optional but recommended) Pass the user's role down to the query executor to enable future fine-grained checks, such as preventing a `read_only` user from executing a `WRITE` query that was somehow submitted.
    5.  **Add Unit Tests:**
        *   **File:** `tests/db/test_auth.cpp`
        *   **Action:** Add unit tests to verify that users with different roles can only access the appropriate resources and actions. For example, test that a `read_only` user cannot perform a write operation.

## 3. Pillar 2: Encryption

This pillar focuses on making ePHI unreadable to unauthorized parties, both when it is stored (at rest) and when it is being transmitted over a network (in transit).

### Task 3.1: Implement Encryption in Transit (HTTPS)

**Objective:** To encrypt all API communication using Transport Layer Security (TLS), preventing eavesdropping and man-in-the-middle attacks.

*   **Engineering Steps:**
    1.  **Integrate TLS Library:**
        *   **Action:** Choose and integrate a robust, well-vetted C++ library for TLS, such as OpenSSL or a higher-level library that uses it (e.g., a modern C++ HTTP server framework).
    2.  **Refactor HTTP Server for HTTPS:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** This is a major refactoring task. The existing HTTP server implementation must be replaced with one that supports HTTPS. This involves handling TLS handshakes, certificate loading, and session management.
    3.  **Implement Certificate Configuration:**
        *   **File:** `tissdb/main.cpp`
        *   **Action:** Add configuration options to allow administrators to specify the file paths for their SSL/TLS private key and public certificate.
    4.  **Disable Plain HTTP:**
        *   **Action:** Once HTTPS is confirmed to be working correctly, the non-encrypted HTTP endpoint must be completely disabled in the code to prevent accidental insecure deployments.
    5.  **Add Integration Tests:**
        *   **File:** `tests/db/test_https.cpp` (new file)
        *   **Action:** Create integration tests that use a TLS-capable client to connect to the server and verify that secure communication is established correctly.

### Task 3.2: Implement Encryption at Rest

**Objective:** To encrypt all data stored on disk, including data files and logs, making it unreadable without the correct decryption keys.

*   **Engineering Steps:**
    1.  **Integrate Cryptographic Library:**
        *   **Action:** Choose and integrate a standard cryptographic library like OpenSSL or libsodium. The library will be used for authenticated encryption (AEAD), specifically a mode like AES-256-GCM.
    2.  **Design and Implement Key Management System (KMS):**
        *   **File:** `tissdb/crypto/kms.cpp`, `tissdb/crypto/kms.h` (new files)
        *   **Action:** Develop a new module to manage encryption keys. This will involve a master encryption key (which must be securely stored/provided) that encrypts per-collection or per-file data encryption keys (DEKs).
    3.  **Encrypt SSTables:**
        *   **File:** `tissdb/storage/sstable.cpp`
        *   **Action:** Before writing an SSTable's data to disk, use the KMS to get an encryption key and encrypt the data buffer with AES-256-GCM. The reverse decryption and authentication tag verification must occur when data is read from an SSTable.
    4.  **Encrypt Write-Ahead Log (WAL):**
        *   **File:** `tissdb/storage/wal.cpp`
        *   **Action:** Before writing an entry to the WAL, encrypt it. When the WAL is replayed on startup, each entry must be decrypted.
    5.  **Add Unit Tests:**
        *   **File:** `tests/db/test_crypto.cpp` (new file)
        *   **Action:** Create unit tests to verify that data is correctly encrypted and decrypted. Include tests for failure cases, such as when a decryption key is incorrect or when the GCM authentication tag is invalid (indicating tampering).

## 4. Pillar 3: Auditing and Logging

This pillar focuses on creating a detailed, immutable record of all activities involving ePHI.

### Task 4.1: Implement Comprehensive Audit Logging

**Objective:** To create a dedicated, security-focused, append-only logging system that records all events related to data access and system administration.

*   **Engineering Steps:**
    1.  **Create Audit Logger Module:**
        *   **File:** `tissdb/audit/audit_logger.cpp`, `tissdb/audit/audit_logger.h` (new files)
        *   **Action:** Develop a new module to handle the formatting and writing of audit logs. It must ensure that logs are written in an append-only fashion to a dedicated log file protected by strict file system permissions.
    2.  **Define Structured Log Format:**
        *   **File:** `tissdb/audit/audit_logger.h`
        *   **Action:** Define a structured, machine-readable log format (e.g., JSON) that captures all required fields: timestamp, user/token ID, source IP, API endpoint, HTTP method, resource accessed, event type (e.g., `DOC_READ`, `AUTH_FAILURE`), and event status.
    3.  **Integrate Logging in API Server:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** Insert calls to the `AuditLogger` at all critical points in the API server. This includes the start and end of every request, all authentication attempts (success and failure), and all authorization decisions.
    4.  **Integrate Logging in Query Executor:**
        *   **File:** `tissdb/query/executor.cpp`
        *   **Action:** Log the specifics of each data operation (e.g., `SELECT`, `INSERT`, `DELETE`) after it has been authorized but before it is executed.
    5.  **Add Unit Tests:**
        *   **File:** `tests/db/test_audit.cpp` (new file)
        *   **Action:** Create unit tests to verify that audit log entries are generated correctly for various API calls and data operations.

### Task 4.2: Implement Secure Log Access API

**Objective:** To provide a secure, administrator-only interface for reviewing audit logs.

*   **Engineering Steps:**
    1.  **Create New Admin Endpoint:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** Add a new API endpoint, such as `GET /_admin/audit_log`.
    2.  **Protect Endpoint with RBAC:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** Use the RBAC system developed in Pillar 1 to ensure this endpoint is only accessible to users with the `admin` role.
    3.  **Implement Log Querying Logic:**
        *   **File:** `tissdb/api/http_server.cpp`
        *   **Action:** Implement the backend logic to read from the audit log file and provide basic filtering capabilities based on query parameters, such as `start_date`, `end_date`, `user_id`, or `event_type`.
    4.  **Add Integration Tests:**
        *   **File:** `tests/db/test_audit.cpp`
        *   **Action:** Add integration tests to verify that the log access API correctly returns log data and that access is properly restricted by the RBAC rules.

## 5. Pillar 4: Data Integrity, Backup, and Disposal

This pillar focuses on protecting ePHI from improper alteration or destruction and ensuring its availability and secure disposal.

### Task 5.1: Enhance Data Integrity with Authenticated Encryption

**Objective:** To provide strong cryptographic guarantees of data integrity, preventing undetected tampering of data at rest.

*   **Engineering Steps:**
    1.  **Verify AEAD Implementation:**
        *   **Action:** This task is largely accomplished by the work in Pillar 2. Confirm that the chosen cryptographic library and mode (e.g., AES-256-GCM) are correctly implemented for all at-rest encryption.
    2.  **Implement and Test Integrity Check Failures:**
        *   **File:** `tissdb/storage/sstable.cpp`, `tissdb/storage/wal.cpp`
        *   **Action:** Ensure that during decryption, the GCM authentication tag is *always* verified. A failed verification must be treated as a critical security event. The operation must fail, the data must not be returned to the user, and a high-severity error must be logged to the audit trail.
    3.  **Add Unit Tests for Tampering:**
        *   **File:** `tests/db/test_crypto.cpp`
        *   **Action:** Create specific unit tests that manually corrupt encrypted data on disk and then attempt to read it. Verify that the system correctly identifies the tampering and fails safely.

### Task 5.2: Develop Secure Backup and Restoration Utility

**Objective:** To provide administrators with a secure method for creating and restoring encrypted backups.

*   **Engineering Steps:**
    1.  **Create Backup/Restore Command-Line Tool:**
        *   **File:** `tissdb/tools/backup_tool.cpp` (new file)
        *   **Action:** Develop a new command-line utility for database administrators. This tool will not run as part of the server.
    2.  **Implement Backup Logic:**
        *   **Action:** The tool's "backup" mode should safely create a point-in-time snapshot of the database. This involves copying all encrypted SSTable and WAL files to a specified backup location. The master encryption key must be handled separately and securely.
    3.  **Implement Restore Logic:**
        *   **Action:** The tool's "restore" mode should allow an administrator to restore the database from a backup. This will involve stopping the server, replacing the data files with the backed-up versions, and ensuring the correct master key is available for the restored data.
    4.  **Create Documentation:**
        *   **File:** `docs/admin_guide.md` (new or existing file)
        *   **Action:** Write clear, detailed documentation explaining the backup and restoration procedures, including any security considerations for handling the master key.

### Task 5.3: Implement Data Disposal via Cryptographic Shredding

**Objective:** To ensure that when data is deleted, it is rendered permanently unrecoverable.

*   **Engineering Steps:**
    1.  **Implement Secure Key Deletion in KMS:**
        *   **File:** `tissdb/crypto/kms.cpp`
        *   **Action:** Add a method to the Key Management System to securely and permanently delete a data encryption key (DEK) associated with a specific collection or data set.
    2.  **Integrate Key Deletion with Data Deletion Operations:**
        *   **File:** `tissdb/storage/database_manager.cpp` (or equivalent)
        *   **Action:** When a user issues a command to delete an entire collection, the system must not only delete the data files but also call the KMS to destroy the corresponding DEK.
    3.  **Add Unit Tests:**
        *   **File:** `tests/db/test_crypto.cpp`
        *   **Action:** Create unit tests to verify that after a collection is deleted, its data is unreadable even if the underlying files are somehow recovered (because the key has been destroyed). This can be tested by trying to decrypt the data with the old key, which should now be gone.
