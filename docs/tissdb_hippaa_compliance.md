# TissDB HIPAA Compliance Remediation Plan

This document outlines the necessary engineering tasks to make TissDB compliant with the technical safeguards required by the Health Insurance Portability and Accountability Act (HIPAA).

## 1. Introduction

TissDB, in its current state, was not designed with the stringent security controls required for handling Protected Health Information (PHI). Achieving HIPAA compliance requires a comprehensive effort to remediate gaps in several key areas of the database architecture.

This plan is a living document that will guide the development efforts. Each section outlines a specific deficiency, the required remediation, and the key source files that will be impacted.

## 2. Core Compliance Pillars

The technical safeguards of the HIPAA Security Rule can be broken down into four core pillars. All of the proposed changes are designed to satisfy requirements within these domains:

1.  **Access Control**: Ensuring that only authorized persons and systems can access ePHI. This involves implementing robust authentication and authorization mechanisms.
2.  **Encryption**: Making ePHI unreadable, both when it is stored (at rest) and when it is being transmitted over a network (in transit).
3.  **Auditing & Logging**: Creating and maintaining a detailed, immutable record of all activities involving ePHI, including who accessed it, what they did, and when.
4.  **Data Integrity**: Protecting ePHI from improper alteration or destruction, and ensuring data is what it purports to be.

## 3. Pillar 1: Access Control

**Deficiency:** TissDB currently has no authentication or authorization layer. Any user with network access to the API endpoint can perform any action, including reading, writing, and deleting data.

**Remediation Plan:**

### 3.1. API Token Authentication

A mandatory, token-based authentication system must be implemented for all API requests.

*   **Implementation Steps:**
    1.  **Generate and Manage API Tokens:** Create a secure, internal mechanism or a command-line utility to generate high-entropy API tokens. These tokens should be stored securely (e.g., hashed in a dedicated system collection).
    2.  **HTTP Header Requirement:** The `http_server` must be modified to require an `Authorization: Bearer <token>` header for all incoming requests, except for a potential public-facing status endpoint.
    3.  **Token Validation:** Upon receiving a request, the server will validate the token against the stored hashes. Requests with missing or invalid tokens will be rejected with a `401 Unauthorized` status.

*   **Key Files to Modify:**
    *   `tissdb/api/http_server.cpp`: To inspect incoming requests for the `Authorization` header and trigger the validation logic.
    *   `tissdb/api/http_server.h`: To add new private methods for token handling.
    *   A new module for token generation and management will need to be created.

### 3.2. Role-Based Access Control (RBAC)

Once a user is authenticated, their permissions must be checked. An RBAC system will be implemented to enforce the principle of least privilege.

*   **Implementation Steps:**
    1.  **Define Roles:** Create a set of predefined roles (e.g., `admin`, `read_write`, `read_only`).
    2.  **Associate Roles with Tokens:** Each API token will be associated with one or more roles.
    3.  **Enforce Permissions:** Before executing any operation (e.g., `DELETE /collection`, `POST /collection/_query`), the `http_server` or a dedicated authorization module must check if the token's roles grant permission for that action on that specific resource.
    4.  **Admin-Only Endpoints:** Critical operations like creating/deleting collections or managing users must be restricted to the `admin` role.

*   **Key Files to Modify:**
    *   `tissdb/api/http_server.cpp`: To integrate RBAC checks after successful authentication.
    *   `tissdb/query/executor.cpp`: Potentially, to enforce row-level or collection-level security during query execution.
    *   A new module for defining and managing roles and permissions will need to be created.

## 4. Pillar 2: Encryption

**Deficiency:** TissDB does not encrypt data, either in transit or at rest. All data is transmitted and stored in cleartext.

**Remediation Plan:**

### 4.1. Encryption in Transit (HTTPS)

All API communication must be encrypted using Transport Layer Security (TLS).

*   **Implementation Steps:**
    1.  **Integrate a TLS Library:** The `http_server` must be modified to use a TLS-capable C++ HTTP library or to directly integrate a library like OpenSSL or Boost.Asio with TLS support.
    2.  **Certificate Management:** The server will need a mechanism to load and use SSL/TLS certificates (private key and public certificate). This should be configurable.
    3.  **Disable HTTP:** Once HTTPS is successfully implemented, the non-encrypted HTTP endpoint must be disabled entirely to prevent insecure communication.

*   **Key Files to Modify:**
    *   `tissdb/api/http_server.cpp`: This file will require significant changes to replace the underlying HTTP server implementation with a secure HTTPS server.
    *   `tissdb/main.cpp`: To handle the new server configuration, including certificate paths.

### 4.2. Encryption at Rest

All data stored on disk, including data files (SSTables) and the Write-Ahead Log (WAL), must be encrypted.

*   **Implementation Steps:**
    1.  **Choose a Cryptographic Library:** Integrate a standard, well-vetted cryptographic library such as OpenSSL or libsodium.
    2.  **Key Management:** Implement a Key Management System (KMS). This could involve a master key that encrypts per-file or per-collection keys. The master key must be stored securely, ideally outside the database data directory (e.g., in a secure vault or provided at startup).
    3.  **Encrypt SSTables:** Before writing an SSTable to disk, the entire file buffer must be encrypted using an authenticated encryption mode like AES-256-GCM. The reverse decryption process must be applied when reading an SSTable.
    4.  **Encrypt WAL:** Similarly, all entries written to the WAL must be encrypted before being flushed to disk.

*   **Key Files to Modify:**
    *   `tissdb/storage/sstable.cpp`: The `write_from_memtable` and `merge` functions must be modified to call the encryption library before writing to the file stream. The `find` and `scan` functions will need to decrypt data after reading it.
    *   `tissdb/storage/wal.cpp`: The function that writes to the log file must encrypt the data first. The log replay function will need to decrypt it.
    *   A new module for Key Management will need to be created.

## 5. Pillar 3: Auditing and Logging

**Deficiency:** TissDB lacks a dedicated, security-focused audit logging mechanism. While some logging may exist for debugging, it is not sufficient to meet HIPAA's requirement for a comprehensive audit trail of all ePHI access.

**Remediation Plan:**

### 5.1. Implement Comprehensive Audit Logging

A new, dedicated logging system must be created to record all relevant events.

*   **Implementation Steps:**
    1.  **Create a Secure Log Format:** Define a structured log format (e.g., JSON) that captures, at a minimum:
        *   Timestamp (UTC)
        *   Authenticated User/Token ID
        *   Source IP Address
        *   API Endpoint and HTTP Method
        *   Resource Accessed (e.g., collection name, document ID)
        *   Event Type (e.g., `DOC_READ`, `DOC_WRITE`, `COLLECTION_DELETE`, `AUTH_SUCCESS`, `AUTH_FAILURE`)
        *   Event Status (Success/Failure)
    2.  **Integrate Logging Calls:** Insert logging calls at critical points in the code before any data operation is executed.
    3.  **Append-Only, Immutable Logs:** Logs should be written to a dedicated file that is protected by file system permissions. The logging mechanism should only be able to append to this file, not modify or delete existing entries.

*   **Key Files to Modify:**
    *   `tissdb/api/http_server.cpp`: To log all incoming requests, authentication attempts, and authorization results.
    *   `tissdb/query/executor.cpp`: To log the details of specific data operations (read, write, delete).
    *   A new `AuditLogger` module will need to be created to handle log formatting and writing.

### 5.2. Secure Log Access API

Administrators must be able to review the audit logs. A secure, admin-only API endpoint will be created for this purpose.

*   **Implementation Steps:**
    1.  **Create a New Endpoint:** Add a new endpoint, such as `GET /_admin/audit_log`.
    2.  **RBAC Protection:** This endpoint must be protected by the RBAC system and should only be accessible to users with the `admin` role.
    3.  **Query Capabilities:** The endpoint should support basic filtering capabilities, such as by date range, user ID, or event type.

*   **Key Files to Modify:**
    *   `tissdb/api/http_server.cpp`: To add the new endpoint and its associated logic.

## 6. Pillar 4: Data Integrity, Backup, and Disposal

**Deficiency:** While TissDB has basic checksums, it lacks a comprehensive data integrity strategy. Furthermore, there are no defined procedures for secure backup, restoration, or data disposal.

**Remediation Plan:**

### 6.1. Data Integrity

The existing checksum mechanism will be enhanced by the authenticated encryption specified in the Encryption pillar.

*   **Implementation Steps:**
    1.  **Leverage Authenticated Encryption:** The use of an Authenticated Encryption with Associated Data (AEAD) cipher mode like AES-256-GCM for encryption at rest will provide strong cryptographic guarantees of both confidentiality and integrity. This is superior to a simple checksum.
    2.  **Verify on Read:** Every time an encrypted block of data is read from disk (from an SSTable or WAL), the GCM authentication tag must be verified. A failed verification indicates data tampering and must be treated as a critical security event, preventing the data from being used.

*   **Key Files to Modify:**
    *   `tissdb/storage/sstable.cpp`: During decryption, the GCM tag must be verified.
    *   `tissdb/storage/wal.cpp`: During decryption, the GCM tag must be verified.

### 6.2. Backup and Restoration

Formal procedures for creating and restoring secure, encrypted backups must be established.

*   **Implementation Steps:**
    1.  **Offline Backup Utility:** Develop a command-line utility that can safely create a point-in-time, offline backup of the database. The backup process will involve copying the encrypted SSTable and WAL files.
    2.  **Secure Restoration:** The same utility will provide a mechanism to restore a backup. This process must only be available to a privileged administrator.
    3.  **Documentation:** Clear documentation must be written for administrators, outlining the backup and restoration procedures.

### 6.3. Data Disposal

When data is deleted, it must be rendered permanently unrecoverable.

*   **Implementation Steps:**
    1.  **Cryptographic Shredding:** Because all data is encrypted at rest, the most effective way to achieve secure disposal is through "cryptographic shredding." Instead of overwriting the physical disk sectors, the encryption keys associated with the data are securely and permanently deleted. Without the keys, the encrypted data is rendered useless garbage.
    2.  **Key Deletion:** The Key Management System must provide a secure method for destroying data encryption keys when a collection or the entire database is deleted.
