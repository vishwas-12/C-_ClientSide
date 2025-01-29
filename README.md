This project is a C++ application that connects to a server over TCP using Winsock2 to receive financial trade packets in real-time. 
The application:
Receives and Parses Packets: Captures binary trade data, extracts fields (symbol, price, quantity, etc.), and converts them into a structured format.
Handles Missing Packets: Detects missing sequence numbers and requests retransmission from the server.
Stores Data as JSON: Converts received packets into JSON format and saves them in output.json for further analysis.
Uses Winsock2 for Networking: Implements TCP-based communication to fetch trade data from a remote server.
This tool is useful for market data processing, network packet analysis, and real-time financial applications.
