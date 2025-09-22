# TissDB-Lite

`tissdb-lite` is a lightweight, in-memory database and Wix Data query engine designed for the Tiss ecosystem, specifically for use within the Wix Velo environment.

## Features

*   **In-Memory Database:** Provides a simple, non-persistent, in-memory database for temporary data storage.
*   **Wix Data Query Engine:** Acts as a translation layer to execute TissLang `QUERY` commands against Wix Data collections for persistent storage.
*   **Pure JavaScript:** Built with no native dependencies, for easy integration into Wix Velo projects.
*   **API Compatibility:** Aims to provide a familiar API for developers working with Wix Data.

## Usage

This module is intended to be used in the backend of a Wix site with Velo. It allows developers to write TissLang scripts that can interact with both temporary in-memory data and persistent Wix Data collections.
