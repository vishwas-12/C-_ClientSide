# 📡 Real-Time Trade Data Client (C++ with Winsock2)

This C++ application connects to a server over **TCP using Winsock2** to receive **financial trade packets** in real-time.  
It processes trade data, handles missing packets, and stores structured JSON output for further analysis.

---

## ✨ Features  

✅ **Receives and Parses Packets**  
- Captures binary trade data from the server.  
- Extracts key fields (**symbol, price, quantity, sequence number, etc.**).  
- Converts the raw packet data into a structured format.  

✅ **Handles Missing Packets**  
- Detects missing sequence numbers.  
- Requests retransmission from the server for lost packets.  

✅ **Stores Data as JSON**  
- Converts received packets into a **JSON format**.  
- Saves the structured data into `output.json` for further processing.  

✅ **Uses Winsock2 for Networking**  
- Implements **TCP-based** communication to fetch trade data from a remote server.  
- Uses Winsock2 for seamless and efficient data transfer.  

---

## 🛠 Setup and Installation  

### **Clone the Repository**  
```sh
git clone https://github.com/your-repo/TradeDataClient.git
```
### **Compile CPP Client**  
```sh
g++ -o client ClientSide.cpp -lws2_32
```
### **Run Client Application via terminal**  
```sh
client.exe
```
