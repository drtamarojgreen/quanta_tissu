import socket
import struct
import json

class TissDBClient:
    """
    A pure Python client for TissDB that communicates over a TCP socket
    using a custom length-prefix binary protocol.
    """
    def __init__(self, host='127.0.0.1', port=8080):
        """
        Initializes the client with the server's host and port.

        Args:
            host (str): The hostname or IP address of the TissDB server.
            port (int): The port number of the TissDB server.
        """
        self.host = host
        self.port = port
        self._sock = None
        self._is_connected = False

    def connect(self):
        """
        Establishes a connection to the TissDB server.
        Raises ConnectionRefusedError if the connection is not successful.
        """
        if self._is_connected:
            return
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.connect((self.host, self.port))
            self._is_connected = True
        except socket.error as e:
            raise ConnectionRefusedError(f"Failed to connect to TissDB at {self.host}:{self.port}: {e}")

    def close(self):
        """
        Closes the connection to the server.
        """
        if self._sock:
            self._sock.close()
            self._sock = None
            self._is_connected = False

    def _send_message(self, message: str):
        """
        Sends a message with a 4-byte length prefix.
        """
        if not self._is_connected:
            raise ConnectionError("Not connected to TissDB server.")

        encoded_message = message.encode('utf-8')
        # Pack the length as a 4-byte unsigned integer in big-endian format
        len_prefix = struct.pack('!I', len(encoded_message))
        self._sock.sendall(len_prefix + encoded_message)

    def _receive_message(self) -> str:
        """
        Receives a message with a 4-byte length prefix.
        """
        if not self._is_connected:
            raise ConnectionError("Not connected to TissDB server.")

        # Receive the 4-byte length prefix
        len_prefix_data = self._sock.recv(4)
        if not len_prefix_data:
            raise ConnectionError("Connection closed by server while reading length prefix.")

        msg_len = struct.unpack('!I', len_prefix_data)[0]

        # Receive the full message
        chunks = []
        bytes_recd = 0
        while bytes_recd < msg_len:
            chunk = self._sock.recv(min(msg_len - bytes_recd, 4096))
            if not chunk:
                raise ConnectionError("Connection closed by server while reading message body.")
            chunks.append(chunk)
            bytes_recd += len(chunk)

        return b''.join(chunks).decode('utf-8')

    def _format_param(self, value) -> str:
        """Formats a Python value for inclusion in a TissQL query."""
        if isinstance(value, str):
            # Escape single quotes and wrap in single quotes
            return "'" + value.replace("'", "''") + "'"
        if isinstance(value, (int, float)):
            return str(value)
        if isinstance(value, bool):
            return "TRUE" if value else "FALSE"
        if value is None:
            return "NULL"
        # For lists or other types, we can serialize to JSON string
        return "'" + json.dumps(value) + "'"


    def query(self, tissql_query: str, params: dict = None) -> str:
        """
        Sends a query to the TissDB server and returns the result.

        Args:
            tissql_query (str): The TissQL query string to execute.
            params (dict, optional): A dictionary of parameters to bind to the query.
                                     Placeholders in the query should be like `$key`.

        Returns:
            str: The JSON response from the server.
        """
        if not self._is_connected:
            self.connect()

        # Simple, client-side parameter substitution
        if params:
            for key, value in params.items():
                placeholder = f'${key}'
                formatted_value = self._format_param(value)
                tissql_query = tissql_query.replace(placeholder, formatted_value)

        try:
            self._send_message(tissql_query)
            return self._receive_message()
        except (ConnectionError, ConnectionRefusedError):
            # If the connection drops, reset state and re-raise
            self.close()
            raise

    def __enter__(self):
        """Context manager entry point."""
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit point."""
        self.close()
